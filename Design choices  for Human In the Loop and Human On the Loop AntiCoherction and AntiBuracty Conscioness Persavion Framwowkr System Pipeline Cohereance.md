# Design choices  for Human In the Loop <-> Human On the Loop AntiCoherction and AntiBuracty Conscioness Persavion Framwowkr System Pipeline Cohereance

* Severity scale: `-1..-3 = WARNING`, `-4..-6 = DANGER`, `-7..-9 = CRITICAL`, `-10..-12 = PANIC`.
* Exceptions are first-class values (not immediate errors). They may be *accepted*, *handled*, or *escalated*.
* Each node in the computation graph (DAG) returns `Result<T, Exception>` or `ExceptionOnly` for `nil`/`iota` cases.
* DAG bubbling: exceptions propagate up; each parent can `resolve()`, `suppress()` (explicit), `escalate()` or call HITL.
* Human-in-the-loop matrix: decision depends on `(human_in_loop: bool, severity)`.
* `nil`/`iota` is an `ExceptionKind::Iota` — “not an error, unknown state”. Default: treat as `WARNING`, but policy can change.

---

# Example — Gosilang-ish code

```gosilang
// Exception system primitives
enum Severity { WARNING, DANGER, CRITICAL, PANIC }

struct Exception {
    kind: String;
    severity: Severity;
    code: i32;         // numeric level -1..-12
    message: str;
    origin_node: NodeId;
    metadata: Map<str,Any>;
}

// Special Nil/Iota exception (not necessarily an error)
const EX_IOTA = Exception { kind: "IOTA", severity: WARNING, code: -1, message: "nil/iota state", origin_node: 0, metadata: {} }

// Result alias
alias Result<T> = union { Ok(T), Err(Exception) }

// Node for DAG graph resolution
struct DagNode {
    id: NodeId;
    children: Vec<DagNodeRef>;
    fn work() -> Result<Any>;
    // local policy: can accept, escalate, or ask human
    policy: NodePolicy;
}

struct NodePolicy {
    acceptable_severity: Severity;   // highest severity this node will accept/auto-handle
    human_in_loop: bool;             // if true prefer HITL for above-warning
    suppressible: bool;              // whether node can explicitly silence exception
}

// Example of a node work function returning nil/iota
fn check_sensor() -> Result<i32> {
    let reading = sensor.read_optional()
    if reading == nil {
        // not an error — return iota exception
        return Err(EX_IOTA)
    }
    return Ok(reading as i32)
}

// DAG resolution engine (simplified)
fn resolve_node(node: DagNode) -> Result<Any> {
    // run children first (DAG)
    for child_ref in node.children {
        let child = load_node(child_ref)
        match resolve_node(child) {
            Ok(v) => continue, // child ok
            Err(exc) => {
                // Apply node's policy
                if can_handle(node.policy, exc) {
                    // node transforms exception into acceptable state
                    let handled = node_handle_exception(node, exc)
                    if handled.is_ok {
                        continue
                    } else {
                        // handled but returned a lower-severity exception -> continue or escalate
                        if handled.exception.severity <= node.policy.acceptable_severity {
                            continue
                        }
                    }
                }

                // Decide HITL vs escalate vs propagate
                if node.policy.human_in_loop && exc.severity >= Severity.DANGER {
                    // Ask human (HITL)
                    let human_decision = GINI.ask_human(node.id, exc)
                    match human_decision {
                        "accept"  => continue,
                        "fix"     => attempt_auto_fix(node, exc) // could re-run child
                        "escalate" => return Err(exc) // bubble up
                    }
                }

                // If suppressible and explicit silence requested in code, do it
                if node.policy.suppressible && should_suppress(node, exc) {
                    // convert to a logged warning and continue
                    log("suppressed", exc)
                    continue
                }

                // default: bubble up
                return Err(exc)
            }
        }
    }

    // If children OK, run own work
    return node.work()
}

// utility: whether node can handle exception locally
fn can_handle(policy: NodePolicy, exc: Exception) -> bool {
    // severity compare (WARNING < DANGER < CRITICAL < PANIC)
    return exc.severity <= policy.acceptable_severity
}

// node local handler example
fn node_handle_exception(node: DagNode, exc: Exception) -> union { Ok(bool), Err(Exception) } {
    // local conversions for IOTA -> default value
    if exc.kind == "IOTA" {
        // If node is tolerant, convert into default and continue
        if node.policy.acceptable_severity == Severity.WARNING {
            log("converted iota to default", node.id)
            return Ok(true)
        } else {
            // Not acceptable here
            return Err(exc)
        }
    }

    // otherwise no local handling
    return Err(exc)
}
```

---

# Walkthrough scenario

1. `check_sensor()` returns `Err(EX_IOTA)` because the hardware returned `nil`.
2. Child node `read_pipeline` receives `IOTA`:

   * Its policy: `acceptable_severity = WARNING`, `human_in_loop = false`, `suppressible = true`.
   * `can_handle(...)` → true, so `node_handle_exception` converts to a safe default (e.g., `0`) and continues. No HITL.
3. Parent node `safety_controller` sees converted default, but later its own `work()` finds impossible geometry -> returns `Err(Exception{kind:"struct_mismatch", severity: DANGER, code:-4})`.
4. Up the DAG, `deploy_node` has `human_in_loop=true`. `resolve_node` sees `DANGER`:

   * Calls `GINI.ask_human(node.id, exc)` → Human chooses `"fix"`.
   * System runs `attempt_auto_fix(...)`; if it fails, the system escalates to `CRITICAL` and `GINI.critical_intervention` is invoked.
5. If severity reaches `PANIC` (`-10 .. -12`), pipeline performs `emergency_shutdown(save_state=true)` and writes recovery artifact.

---

# Nil / Iota special handling notes

* `IOTA` is *not* an immediate error. It must be represented as `Err(Exception{kind:"IOTA", ...})`, so the DAG can decide:

  * auto-convert to default,
  * quarantine and ask human (if HITL),
  * or propagate as a warning that accumulates in a trace.
* This keeps behavior deterministic: nothing is silently swallowed unless `suppressible` is explicitly set.

---

# Human-in-the-loop matrix (policy pseudo-table)

| human_in_loop | severity | default action                |
| ------------- | -------- | ----------------------------- |
| false         | WARNING  | auto-handle if policy allows  |
| false         | DANGER   | propagate up                  |
| true          | DANGER   | ask human (preferred)         |
| true          | CRITICAL | immediate HITL + intervention |
| true          | PANIC    | emergency save & shutdown     |

---

# Bubble-to-DAG semantics (short)

* Each exception carries `origin_node` and `trace` (history). When bubbled, every node appends its id to `trace`.
* Use DAG topological order; when a leaf emits an exception, upstream nodes get a deterministic chance to resolve or escalate depending on their `NodePolicy`.
* Graph resolution returns a single `Result` and an `ExceptionTrace` when failed — helps you quickly find where the odd `nil` first appeared.

---

# Small log-style example (what you'd see)

```
[leaf:check_sensor] -> Err(IOTA, code:-1)
[read_pipeline] can_handle(IOTA) => converted to default 0
[safety_controller] -> Err(struct_mismatch, DANGER -4)
[deploy_node] human_in_loop => GINI.ask_human(deploy_node, DANGER -4)
Human decision: fix
[attempt_auto_fix] -> success
[safety_controller] resolved
All good. Continue.
```
