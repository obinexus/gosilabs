# Overview: `Obison` — Actor-First Generational Iteration Library

Purpose: provide **generators** (producers) and **consumers** as actor-owned primitives, expose both push and pull semantics, let gosi routines yield items cheaply, support generational promotion for long-lived streams, and present an `async/await`-style API that is isomorphic to actor message flows.

Key guarantees:

* Actors *own* their generators (no shared mutable state).
* Pause/resume friendly (works with Gosi routines).
* Backpressure and QA gating supported.
* Generational GC-aware: short-lived items are scavenged quickly; survivors promote.
* HITL and GINI-Sanctuary cooperate for safety-critical streams.

---

# Concepts & Terminology

* **Generator (Gen<T>)** — actor-owned producer; yields items via `gen.yield(item)`.
* **Consumer (Cons<T>)** — actor/consumer that pulls or receives items; supports `await`-like interface.
* **StreamEpoch / Generation** — generations for items: `Young`, `Survived`, `Old`.
* **ObisonToken** — pause/resume token (compatible with Gosi pause tokens).
* **Isomorph (Obison Isomorph)** — mapping between `await item` and `receive message` semantics. Async/await syntax becomes sugar for "send message to actor/gate and suspend".

---

# API (Gosilang-ish) — drop-in usable

```gosilang
module obison {
    // create a generator attached to current actor
    fn spawn_generator<T>(producer: fn(GenHandle<T>) -> Never, opts: GenOptions) -> GenHandle<T>

    // consumer side: pull one item (suspends if none available)
    fn pull<T>(gen: GenHandle<T>, timeout_ms: i64) -> Option<T>

    // async-style sugar: await next item (isomorphic to pull)
    fn await_next<T>(gen: GenHandle<T>) -> Option<T>

    // push-style: subscribe a consumer (callback or actor mailbox)
    fn subscribe<T>(gen: GenHandle<T>, consumer: ConsumerRef<T>, qos: QoSOptions) -> SubscriptionHandle

    // backpressure hint: producer may inspect subscribe count / credits
    fn credits<T>(gen: GenHandle<T>) -> i32

    // generational controls
    fn mark_survived<T>(gen: GenHandle<T>, item_id: ItemId)
    fn promote_generation<T>(gen: GenHandle<T>, threshold_ms: i64)

    // safety controls for critical resources
    fn quarantine_candidate(candidate: ReclaimCandidate)
}
```

`GenOptions` includes `yield_frequency`, `buffer_capacity`, `generation_ttl`, `safety_level`.

---

# Usage Examples

### 1) Producer as actor (Gosi routine friendly)

```gosilang
actor TelemetryProducer {
    state: isolated;
    gen: GenHandle<Datum>;

    fn init() {
        self.gen = obison.spawn_generator(|g| {
            loop {
                let datum = read_sensor()
                g.yield(datum)            // cheap, yield-friendly
                gosi.yield_if_needed()    // cooperate with scheduler
            }
        }, GenOptions{ buffer_capacity: 256, generation_ttl: 60000 })
    }
}
```

### 2) Consumer pulling with await (isomorph to actor mailbox)

```gosilang
actor Analyzer {
    state: isolated;
    fn main(prod: GenHandle<Datum>) -> Never {
        loop {
            // same semantics as "await next item" in async/await
            match obison.await_next(prod) {
                Some(d) => analyze(d),
                None    => rest(50) // no item => backoff
            }
        }
    }
}
```

### 3) Subscribe (push) with backpressure credits

```gosilang
actor Logger {
    fn attach(gen) {
        obison.subscribe(gen, self.mailbox_ref(), QoS{ credits: 100 })
    }

    fn on_message(msg: Datum) {
        write_log(msg)
        // consumer implicitly returns credit; small constant overhead
    }
}
```

---

# Obison Isomorph: `await` ⇄ `actor message`

Design: `obison.await_next(gen)` desugars to:

1. send `PULL_REQUEST` to `gen` actor (lightweight message),
2. suspend current gosi routine (save context),
3. `gen` responds with `PUSH_ITEM` or `EMPTY` when available,
4. resume consumer (restore context).

Because both sides are actors/gosi routines, this is pure actor-first — `async/await` is syntactic sugar around message passing with saved context (pause tokens).

---

# Generational Policy (practical)

* Items live in `Young` generation when created.
* If consumer accesses same item multiple times across `promote_threshold` or it survives `N` GC passes → `Survived` → `Old`.
* TTL fast-path: `Young` items eligible for quick reclamation via local refcounts and quarantine.
* Promotion reduces GC chatter for hot streams (AVL-Huffman biases high-weight modules to fewer checks).

Parameters:

* `young_ttl_ms` (default 5s)
* `promote_hits` (default 3)
* `old_reclaim_strategy` (batch/streamed)

---

# Backpressure & QoS

* Producer can query `obison.credits(gen)` to avoid overproduction.
* Consumers can request credit windows via `subscribe(..., QoS{ credits })`.
* If no credits: producer can `yield_backpressure()` or `park()` until credits available.
* Gate QA integration: QA bounds influence credits (low confidence => lower consumer credits).

---

# GC & GINI-Sanctuary interactions

* Items created by generator are tracked in local `RefCatalog` with `owner = GenHandle`.
* If generator returns `IOTA` or leaf exception occurs, items move to **QuarantineSet** with `ExceptionTrace` tagging.
* GC may request `pause_for_reclaim` for producer to flush or checkpoint a generator (uses ObisonToken).
* For `safety_level=MAX` items: HITL confirm required before reclaim.

---

# Implementation sketch (runtime internals)

* **Per-gen ring buffer**: producer appends to buffer, consumers pop; keep item metadata (id, epoch, owner).
* **Epoch vector + AVLHuffman**: nodes gossip small summaries of which generations they have; heavier modules get less aggressive GC.
* **Checkpointing**: generators can be checkpointed (serialize buffer tail) to disk prior to reclaim — required for recovery artifacts.
* **Yield integration**: `gosi.yield_if_needed()` points inserted automatically by compiler at heavy loops and on `gen.yield()`.

Rust/Pseudo code for a lightweight core loop:

```rust
struct GenCore<T> {
    buffer: VecDeque<Item<T>>,
    credits: AtomicI32,
    epoch: Epoch,
    owner: ActorId,
    // metadata for GC
}

impl<T> GenCore<T> {
    fn yield_item(&mut self, item: T) -> Result<(), Backpressure> {
        if self.credits.load() <= 0 {
            return Err(Backpressure);
        }
        let id = self.next_id();
        self.buffer.push_back(Item{ id, value: item, epoch: self.epoch });
        notify_consumers();
        Ok(())
    }

    fn pull_item(&mut self) -> Option<T> {
        self.buffer.pop_front().map(|it| it.value)
    }
}
```

---

# Tests to add

* **Throughput**: produce 100k items/sec with 1 consumer, measure latency and GC pauses.
* **Backpressure**: producer backs off when credits exhausted, resumes when consumer returns credits.
* **Generation promotion**: produce hot stream; ensure survivors promote and GC skips them in young collections.
* **HITL reclaim**: create safety-critical items, drop refs, confirm `GINI.ask_human()` invoked and reclaim blocks until decision.

---

# API ergonomics (sugar)

* `await_next(gen)` → suspends; returns `Option<T>`.
* `.for_each_async(|item| ...)` → runs a consumer loop in a gosi routine with automatic backpressure management.
* `gen.map(|x| f(x)).filter(...)` → chainable but implemented under the hood as actor pipelines (each stage a gosi routine).

---

# Safety & Manifesto compliance

* No implicit shared memory: buffers live in the owner actor’s memory.
* No global stop-the-world: pause at gosi yield points + local refcounts.
* Always produce recovery artifact for any reclaim that touches `@safety_critical` resources.

---

# File / Repo layout suggestion

```
/libs/obison
  /src
    gen.gs        // Gosilang API wrappers
    runtime.rs    // runtime core (Rust/C)
    tests/
      test_backpressure.gs
      test_generation_promotion.gs
  /specs
    obison-spec.md
  README.md
```

---

# Final tiny example — `for_each_async` sugar

```gosilang
// Producer
let gen = obison.spawn_generator(|g| {
    for i in 0..1_000_000 {
        g.yield(i)
        gosi.yield_if_needed()
    }
})

// Consumer: async-style, actor-first
obison.for_each_async(gen, |item| {
    process(item)   // runs inside a gosi routine; backpressure auto-managed
})
```

