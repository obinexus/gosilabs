actor MySafeActor {
    state: isolated;
    private var count: i32 = 0;

    @constant_time(verified=true)
    fn increment() -> i32 {
        count += 1;  // This is safe because only one message can call this at a time
        return count;
    }
}
