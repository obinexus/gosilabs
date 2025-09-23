// gosiuml/src/pheno_state_machine.c
#include "phenomology_platform.h"

typedef enum {
    STATE_NIL,
    STATE_ALLOCATED,
    STATE_LOCKED,
    STATE_ACTIVE,
    STATE_DEGRADED,
    STATE_SHARED,
    STATE_FREED
} PhenoState;

typedef struct StateMachine {
    PhenoState current_state;
    PhenoToken* token;
    
    // State transition table
    PhenoState (*transitions[7][7])(struct StateMachine*);
    
    // State entry/exit actions
    void (*on_entry[7])(PhenoToken*);
    void (*on_exit[7])(PhenoToken*);
    
    // Metrics
    uint32_t transition_count;
    uint64_t state_duration[7];
} StateMachine;

// State transition functions
PhenoState transition_nil_to_allocated(StateMachine* sm) {
    if (!sm->token) {
        sm->token = pheno_token_alloc((PhenoTokenType){
            .category = NODE_IDENTITY,
            .node_level = 0
        });
    }
    return STATE_ALLOCATED;
}

PhenoState transition_allocated_to_locked(StateMachine* sm) {
    if (atomic_compare_exchange_weak(&sm->token->mem_flags.locked, 0, 1)) {
        return STATE_LOCKED;
    }
    return STATE_ALLOCATED; // Stay in current state
}

PhenoState transition_locked_to_active(StateMachine* sm) {
    if (verify_geometric_proof(sm->token)) {
        atomic_store(&sm->token->mem_flags.coherent, 1);
        return STATE_ACTIVE;
    }
    return STATE_LOCKED;
}

PhenoState transition_active_to_degraded(StateMachine* sm) {
    float degradation = calculate_degradation_score(sm->token);
    if (degradation > 0.6) {
        sm->token->value->metrics.score = (uint32_t)(degradation * 1023);
        return STATE_DEGRADED;
    }
    return STATE_ACTIVE;
}

// Initialize state machine
StateMachine* create_state_machine() {
    StateMachine* sm = calloc(1, sizeof(StateMachine));
    sm->current_state = STATE_NIL;
    
    // Setup transition table
    sm->transitions[STATE_NIL][STATE_ALLOCATED] = transition_nil_to_allocated;
    sm->transitions[STATE_ALLOCATED][STATE_LOCKED] = transition_allocated_to_locked;
    sm->transitions[STATE_LOCKED][STATE_ACTIVE] = transition_locked_to_active;
    sm->transitions[STATE_ACTIVE][STATE_DEGRADED] = transition_active_to_degraded;
    
    return sm;
}

// Execute state machine step
void step_state_machine(StateMachine* sm, PhenoEvent event) {
    PhenoState next_state = sm->current_state;
    
    // Check transition table based on event
    if (sm->transitions[sm->current_state][event.target_state]) {
        next_state = sm->transitions[sm->current_state][event.target_state](sm);
    }
    
    // Execute exit action for current state
    if (sm->on_exit[sm->current_state]) {
        sm->on_exit[sm->current_state](sm->token);
    }
    
    // Transition to new state
    sm->current_state = next_state;
    
    // Execute entry action for new state
    if (sm->on_entry[next_state]) {
        sm->on_entry[next_state](sm->token);
    }
    
    sm->transition_count++;
}
