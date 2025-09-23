static void* shard_run(void *p) {
    // This runs in parallel, but only on read-only data (no shared state)
    shard_t *s = (shard_t*)p;
    for (size_t i=s->from; i<s->to; i++) {
        double u = s->universe[i];
        if (isnan(u)) s->out[i] = NAN; else s->out[i] = s->everything - u;
    }
    return NULL;
}
