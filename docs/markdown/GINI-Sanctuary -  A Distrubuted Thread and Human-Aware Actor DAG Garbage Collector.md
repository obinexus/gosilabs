

## **GINI-Sanctuary**: a distributed, thread-aware, human-aware garbage-collector actor that integrates with Gosi routines and your gate/DAG semantics; supports generational + reference-tracing + gossip-assisted reclamation; treats `IOTA`/`nil` specially and cooperates with HITL for safe reclamation of safety-critical resources.

# Design goals

* Safe for life-critical actors (respect hardware isolation, no unexpected reclamation).
* Non-blocking for Gosi routines (pause/resume friendly).
* Network-aware: nodes gossip liveness & references; AVL-Huffman prioritizes scavenging frequency.
* Human-in-the-loop for risky reclamations (especially `safety_level=MAX`).
* Traceable: every reclaim has provenance, exception trace, and recovery artifact.

---

# High-level architecture (components)

1. **GINI-Sanctuary Actor (global/local instances)**

   * Runs as an actor with its own `state: isolated`.
   * Maintains distributed **Reference Lattice** (per-node) and **Epoch Vector** for generational collection.
   * Accepts `GossipRefUpdate`, `GC_PROBE`, `GC_RECLAIM_REQUEST`, `HITL_CONFIRM`, `PAUSE_FOR_RECLAIM` messages.

2. **Gosi Routine Hooks**

   * Runtimes instrument allocations and references (lightweight shadow refs).
   * Provide `pause_for_gc()`/`resume_after_gc()` tokens; yield-friendly.

3. **Gate Integration**

   * QA gates tag messages with `ref_footprint` metadata so the GC knows live references crossing gates.

4. **AVLHuffman Module Loader**

   * Weighs modules by usage; high-weight modules are collected less aggressively. The loader publishes module reachability to GINI-Sanctuary.

5. **HITL Manager**

   * For safety-critical reclamation (hardware-enforced resources), GINI asks: `GINI.ask_human(reclaim_candidate)` before finalizing.

6. **DAG + Exception Model**

   * Exceptions track `origin_node` and `trace`. If a node produces `IOTA`/nil as persistent state, GINI marks the associated allocations for *quarantine* rather than immediate reclaim — giving time for DAG resolution.

---

# Core concepts & rules

* **Sanctuary States**: `OBSERVE`, `QUARANTINE`, `RECLAIM_PENDING`, `RECLAIMING`, `RECLAIMED`, `ROLLBACK`
* **Safety Policy**: any allocation owned by `@safety_critical(level=MAX)` cannot be reclaimed without `HITL_CONFIRM`.
* **Pause semantics**: Gosi routines near yield points call `gosi.pause_for_gc_hint()` to help coordinated collections, but collection must not force synchronous global stop-the-world.
* **IOTA handling**: `IOTA` triggers quarantine of associated allocations; after a TTL or DAG resolution it becomes `RECLAIM_PENDING`.

---

# Messaging API (Gosilang-ish)

```gosilang
// GINI-Sanctuary Actor API
actor GINI_Sanctuary {
    state: isolated;

    // Updates reference information from local runtime
    fn gossip_ref_update(node_id: NodeId, refs: RefCatalog) -> Ok;

    // Probe candidate for reclaim (returns metadata + safety_token)
    fn probe_reclaim(candidate: ReclaimCandidate) -> Result<ProbeResult, Exception>;

    // Request reclaim; returns Ack / asks for HITL if needed
    fn request_reclaim(candidate: ReclaimCandidate) -> Result<Ack, ReclaimException>;

    // HITL answer: human confirms approach
    fn hitl_confirm(candidate_id: CandidateId, decision: HumanDecision) -> Ok;

    // Observe: stream of GC events
    fn stream_events() -> Stream<GcEvent>;
}
```

Types:

```gosilang
struct RefCatalog { entries: Vec<RefEntry> }  // light-weight refs: (addr, owner, type, module)
struct ReclaimCandidate { addr: Ptr, type: TypeId, epoch: Epoch, traces: Vec<ExceptionTrace> }
enum HumanDecision { ACCEPT, REJECT, DELAY }
struct ProbeResult { safety_level: SafetyLevel, requires_hitl: bool, risk_score: f32 }
enum ReclaimException { HITL_REQUIRED, NODE_DISCONNECTED, UNKNOWN_OWNER }
```

---

# GC algorithm (distributed hybrid)

1. **Local fast path**: per-node *refcounts + write barrier* for hot objects (low overhead). Quick check reclaim if refcount==0 and not safety-critical.

2. **Generational quarantine**: objects with `IOTA` or transient `Err->Exception` flows go into **Quarantine Generation** for a min TTL or until DAG stabilizes.

3. **Gossip probe**: local node sends `GossipRefUpdate` to sanctuary peers with summarised `RefCatalog` (AVL-Huffman ordering so high-weight modules cause less chatter).

4. **Consensus reclaim**: if quorum of nodes report no refs and `probe_reclaim()` says `requires_hitl=false`, move to `RECLAIM_PENDING`.

5. **HITL path**: if `safety_level=MAX` OR `risk_score > threshold`, ask human by `GINI.ask_human(...)`. Human chooses `ACCEPT`/`DELAY`/`REJECT`. `ACCEPT` triggers `PAUSE_FOR_RECLAIM` token to owning gosi routines; GC performs reclaim; `DELAY` extends quarantine; `REJECT` aborts.

6. **Failure recovery**: reclamation always writes a `recovery_artifact` (serialized object + provenance) to disk before destroying in case rollback is required.

---

# Interaction with Exceptions / DAG bubbling

* When a leaf returns `Err(IOTA)`:

  * Mark all objects allocated during that leaf's lifetime into **QuarantineSet** tied to the `ExceptionTrace`.
  * Do not reclaim until either (a) DAG resolves to `Ok`, (b) TTL expires, (c) manual HITL decision.
* If `Err` escalates to `PANIC`, immediate actions:

  * If objects are memory-only and not safety-critical → fast reclaim.
  * If safety-critical → emergency HITL and produce `emergency_recovery_artifact`.

---

# Sample Gosilang actor snippet: GINI-Sanctuary (implementable)

```gosilang
@safety_critical(level=MAX)
actor GINI_Sanctuary {
    state: isolated;
    local_refdb: RefDB;
    quarantine: Map<CandidateId, ReclaimCandidate>;
    epoch: Epoch = 0;

    // runtime calls this on allocation / when references change
    fn gossip_ref_update(node_id: NodeId, catalog: RefCatalog) -> Ok {
        self.local_refdb.merge(node_id, catalog)
        return Ok()
    }

    // runtime asks: can I reclaim?
    fn probe_reclaim(candidate: ReclaimCandidate) -> Result<ProbeResult, ReclaimException> {
        // compute safety/risk
        let safety = compute_safety(candidate)
        if safety.level == MAX {
            return Err(HITL_REQUIRED)
        }
        return Ok(ProbeResult{ safety_level: safety, requires_hitl: false, risk_score: safety.risk })
    }

    // external request to actually reclaim after probing
    fn request_reclaim(candidate: ReclaimCandidate) -> Result<Ack, ReclaimException> {
        if self.requires_hitl(candidate) {
            // create a human task
            GINI.ask_human("Reclaim object", candidate)
            self.quarantine.insert(candidate.id, candidate)
            return Err(HITL_REQUIRED)
        }
        // write recovery artifact
        let artifact = self.serialize_artifact(candidate)
        disk.write(artifact)
        self.perform_reclaim(candidate)
        return Ok(Ack{ success: true })
    }

    fn perform_reclaim(candidate: ReclaimCandidate) -> Ok {
        // notify owning gosi routines to pause if needed
        for owner in candidate.owners {
            GOSI.pause_routine_for_owner(owner)
        }
        low_level.free(candidate.addr)
        log("reclaimed", candidate.addr)
        return Ok()
    }
}
```

---

# SVG schematic (ASCII + SVG-ready idea)

I made the ASCII version so your brain (and the README) can drink it quickly. Below is an SVG-ready description you can drop into an SVG generator or paste into a file.

ASCII:

```
[ GosiRoutine ] ---writes---> [ RefCatalog ] ---gossip--> [ GINI-Sanctuary (OBSERVE) ]
       |                                \                        |
       | pause_for_gc_hint               \ quarantine candidate   | probe_reclaim()
       v                                 v                        |
  yield points                         [ Quarantine ] --HITL?--> [ HITL Manager ]
       |                                                        |
       '---> [ Gate (QA) ] --tags msg refs--> [ AVL-Huffman Loader ] 
```

Minimal SVG skeleton (paste into an .svg file; tweak styles as you like):

```svg
<svg xmlns="http://www.w3.org/2000/svg" width="1000" height="420">
  <style>
    .box{fill:#0b1220; stroke:#9be7ff; stroke-width:2; rx:8; }
    .txt{font-family:Inter,monospace; fill:#e6fdff; font-size:13px}
    .arrow{stroke:#9be7ff; stroke-width:2; marker-end:url(#arrow)}
  </style>
  <defs>
    <marker id="arrow" markerWidth="10" markerHeight="10" refX="8" refY="5"
      orient="auto"><path d="M0,0 L10,5 L0,10 z" fill="#9be7ff"/></marker>
  </defs>

  <!-- Gosi -->
  <rect x="40" y="40" width="180" height="80" class="box"/>
  <text x="50" y="65" class="txt">GosiRoutine</text>
  <text x="50" y="85" class="txt small">yield / pause hooks</text>

  <!-- Gate -->
  <rect x="260" y="40" width="160" height="80" class="box"/>
  <text x="275" y="70" class="txt">Gate (QA)</text>
  <text x="275" y="90" class="txt small">tags: ref_footprint</text>

  <!-- AVL-Huffman -->
  <rect x="460" y="40" width="200" height="80" class="box"/>
  <text x="475" y="65" class="txt">AVLHuffman Loader</text>
  <text x="475" y="85" class="txt small">module weights</text>

  <!-- Sanctuary -->
  <rect x="720" y="30" width="240" height="120" class="box"/>
  <text x="735" y="55" class="txt">GINI-Sanctuary</text>
  <text x="735" y="75" class="txt small">observe / quarantine / reclaim</text>

  <!-- Quarantine & HITL -->
  <rect x="460" y="160" width="200" height="80" class="box"/>
  <text x="475" y="195" class="txt">Quarantine</text>
  <rect x="720" y="160" width="240" height="80" class="box"/>
  <text x="735" y="195" class="txt">HITL Manager</text>

  <!-- arrows -->
  <line x1="220" y1="80" x2="260" y2="80" class="arrow"/>
  <line x1="420" y1="80" x2="460" y2="80" class="arrow"/>
  <line x1="660" y1="80" x2="720" y2="80" class="arrow"/>
  <line x1="560" y1="120" x2="560" y2="160" class="arrow" />
  <line x1="660" y1="200" x2="720" y2="200" class="arrow" />
</svg>
```

---

# Tests (what to add to `test/`)

1. **Local reclaim test**

   * allocate object on node, drop all refs, expect `GINI_Sanctuary.request_reclaim()` to succeed without HITL for non-critical objects.

2. **IOTA quarantine test**

   * leaf returns `Err(IOTA)` with allocations; ensure those allocations are in Quarantine for TTL and then reclaimed after TTL if DAG resolves.

3. **HITL safety test**

   * allocate `@safety_critical(level=MAX)` resource, drop refs; verify `probe_reclaim()` returns `HITL_REQUIRED` and that no reclaim happens without human confirm.

4. **Gossip quorum disconnect test**

   * simulate node disconnection during probe; expect `request_reclaim()` to `Err(NODE_DISCONNECTED)` and keep object in Quarantine.

---

# Quick policy snippets to add to `@manifesto` or `@system_guarantee`

```gosilang
@gc_policy {
    default_ttl_ms: 300_000,        // quarantine TTL (5 minutes)
    iota_policy: "quarantine",
    safety_hitl_threshold: 0.75,    // risk score
    recovery_artifact: true,
    avl_huffman_bias: "module_weight"
}
```

---

# Notes & gotchas (you’ll thank me later)

* **Disk artifacts**: always write recovery artifact before destructive reclaim — life-critical systems require it.
* **Pause races**: don’t force synchronous global stop-the-world. Use gosi yield points plus local refcounts to avoid blocking latencies.
* **Module weights**: AVL-Huffman reduces traffic but increases decision complexity — test with real workloads.
* **Human fatigue**: HITL for reclamation must minimize interruptions. Batch confirmations or allow safe auto-accept policies for low-risk classes.
