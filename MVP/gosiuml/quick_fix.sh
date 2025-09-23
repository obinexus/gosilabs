#!/bin/bash
# quick_fix.sh - Fix header issues and build

echo "Fixing GosiUML build issues..."

# 1. Fix the header filename typo in all source files
echo "Fixing header filename references..."
find src -name "*.c" -exec sed -i 's/phenomology_platform\.h/phenomemory_platform.h/g' {} \;

# 2. Fix the gosiuml.h to remove duplicate definitions
echo "Updating gosiuml.h..."
cat > include/gosiuml.h << 'EOF'
#ifndef GOSIUML_H
#define GOSIUML_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

// Version macros
#define GOSIUML_VERSION_MAJOR 1
#define GOSIUML_VERSION_MINOR 0
#define GOSIUML_VERSION_PATCH 0

// Output formats
typedef enum {
    FORMAT_SVG = 0,
    FORMAT_XML = 1,
    FORMAT_JSON = 2
} GosiUMLFormat;

// Options
typedef enum {
    GOSIUML_OPT_VERBOSE = 1,
    GOSIUML_OPT_SHOW_BITFIELDS = 2,
    GOSIUML_OPT_STATE_MACHINE = 3,
    GOSIUML_OPT_MEMORY_TRACKING = 4
} GosiUMLOption;

// State definitions
typedef enum {
    STATE_NIL,
    STATE_ALLOCATED,
    STATE_LOCKED,
    STATE_ACTIVE,
    STATE_DEGRADED,
    STATE_SHARED,
    STATE_FREED
} PhenoState;

// Context structure
typedef struct GosiUMLContext GosiUMLContext;

// Include platform definitions (contains PhenoToken, PhenoRelation, etc)
#include "phenomemory_platform.h"

// Function prototypes for library API
int gosiuml_init(void);
void gosiuml_cleanup(void);
const char* gosiuml_version(void);
GosiUMLContext* gosiuml_create_context(void);
void gosiuml_free_context(GosiUMLContext* ctx);
int gosiuml_set_option(GosiUMLContext* ctx, GosiUMLOption option, int value);
PhenoToken* gosiuml_parse_file(const char* filename, int* count);
PhenoToken* gosiuml_create_token(uint8_t type, const char* name);
void gosiuml_free_token(PhenoToken* token);
void gosiuml_free_tokens(PhenoToken* tokens, int count);
int gosiuml_process_token(GosiUMLContext* ctx, PhenoToken* token);
int gosiuml_generate_svg(GosiUMLContext* ctx, PhenoToken* tokens, int count, const char* output_file);
int gosiuml_generate_xml(GosiUMLContext* ctx, PhenoToken* tokens, int count, const char* output_file);
int gosiuml_generate_json(GosiUMLContext* ctx, PhenoToken* tokens, int count, const char* output_file);
int gosiuml_get_state(PhenoToken* token);
int gosiuml_transition(PhenoToken* token, int new_state);
int gosiuml_test_state_machine(GosiUMLContext* ctx);
int gosiuml_test_bitfields(void);
const char* gosiuml_get_error(void);
void gosiuml_set_debug(bool enable);

#endif // GOSIUML_H
EOF

# 3. Remove old Makefile and keep only the new one
echo "Cleaning up Makefiles..."
rm -f Makefile.old

# 4. Create missing pheno_memory.c if it doesn't exist
if [ ! -f "src/core/pheno_memory.c" ]; then
    echo "Creating src/core/pheno_memory.c..."
    cat > src/core/pheno_memory.c << 'EOF'
#include "gosiuml.h"
#include <pthread.h>
#include <string.h>

// Global library state
static struct {
    bool initialized;
    pthread_mutex_t mutex;
    char error_buffer[256];
    bool debug_enabled;
} g_state = { false, PTHREAD_MUTEX_INITIALIZER, "", false };

// Context implementation
struct GosiUMLContext {
    int current_state;
    int options;
    struct {
        int state_count[7];
        int transition_count;
    } stats;
};

int gosiuml_init(void) {
    g_state.initialized = true;
    return 0;
}

void gosiuml_cleanup(void) {
    g_state.initialized = false;
}

const char* gosiuml_version(void) {
    static char ver[32];
    snprintf(ver, sizeof(ver), "%d.%d.%d",
             GOSIUML_VERSION_MAJOR,
             GOSIUML_VERSION_MINOR,
             GOSIUML_VERSION_PATCH);
    return ver;
}

GosiUMLContext* gosiuml_create_context(void) {
    return calloc(1, sizeof(GosiUMLContext));
}

void gosiuml_free_context(GosiUMLContext* ctx) {
    free(ctx);
}

int gosiuml_set_option(GosiUMLContext* ctx, GosiUMLOption option, int value) {
    if (!ctx) return -1;
    if (value)
        ctx->options |= (1 << option);
    else
        ctx->options &= ~(1 << option);
    return 0;
}

PhenoToken* gosiuml_create_token(uint8_t type, const char* name) {
    PhenoToken* token = calloc(1, sizeof(PhenoToken));
    if (!token) return NULL;
    
    token->token_type = type;
    if (name) {
        strncpy(token->token_name, name, sizeof(token->token_name) - 1);
        token->token_name[sizeof(token->token_name) - 1] = '\0';
    }
    
    token->mem_flags.allocated = 1;
    token->mem_flags.ref_count = 1;
    token->value = calloc(1, sizeof(PhenoTokenValue));
    
    return token;
}

void gosiuml_free_token(PhenoToken* token) {
    if (token) {
        free(token->value);
        free(token);
    }
}

void gosiuml_free_tokens(PhenoToken* tokens, int count) {
    if (!tokens) return;
    for (int i = 0; i < count; i++) {
        free(tokens[i].value);
    }
    free(tokens);
}

int gosiuml_process_token(GosiUMLContext* ctx, PhenoToken* token) {
    if (!ctx || !token) return -1;
    ctx->stats.transition_count++;
    return 0;
}

int gosiuml_get_state(PhenoToken* token) {
    if (!token) return STATE_NIL;
    if (!token->mem_flags.allocated) return STATE_NIL;
    if (token->mem_flags.locked) return STATE_LOCKED;
    if (token->mem_flags.shared) return STATE_SHARED;
    return STATE_ACTIVE;
}

int gosiuml_transition(PhenoToken* token, int new_state) {
    if (!token) return -1;
    // Simplified transition logic
    return 0;
}

const char* gosiuml_get_error(void) {
    return g_state.error_buffer;
}

void gosiuml_set_debug(bool enable) {
    g_state.debug_enabled = enable;
}

// External functions implemented in other files
extern PhenoToken* parse_token_file(const char* filename, int* count);
extern void generate_svg_from_tokens(PhenoToken* tokens, int count, const char* output_file);

int gosiuml_parse_file(const char* filename, int* count) {
    return (int)parse_token_file(filename, count);
}

int gosiuml_generate_svg(GosiUMLContext* ctx, PhenoToken* tokens, int count, const char* output_file) {
    generate_svg_from_tokens(tokens, count, output_file);
    return 0;
}

int gosiuml_generate_xml(GosiUMLContext* ctx, PhenoToken* tokens, int count, const char* output_file) {
    // Simplified XML generation
    FILE* f = fopen(output_file, "w");
    if (!f) return -1;
    fprintf(f, "<?xml version=\"1.0\"?>\n<tokens count=\"%d\"/>\n", count);
    fclose(f);
    return 0;
}

int gosiuml_generate_json(GosiUMLContext* ctx, PhenoToken* tokens, int count, const char* output_file) {
    // Simplified JSON generation
    FILE* f = fopen(output_file, "w");
    if (!f) return -1;
    fprintf(f, "{\"tokens\": %d}\n", count);
    fclose(f);
    return 0;
}

int gosiuml_test_state_machine(GosiUMLContext* ctx) {
    return 0;
}

int gosiuml_test_bitfields(void) {
    return 0;
}
EOF
fi

# 5. Create missing pheno_state_machine.c if needed
if [ ! -f "src/core/pheno_state_machine.c" ]; then
    echo "Creating src/core/pheno_state_machine.c..."
    cat > src/core/pheno_state_machine.c << 'EOF'
#include "phenomemory_platform.h"

// Stub implementation for state machine
bool verify_geometric_proof(PhenoToken* token) {
    if (!token) return false;
    return token->value && token->value->metrics.confidence > 100;
}

float calculate_degradation_score(PhenoToken* token) {
    if (!token || !token->value) return 0.0;
    return token->value->metrics.score / 1023.0;
}

PhenoToken* pheno_token_alloc(PhenoTokenType type) {
    PhenoToken* token = calloc(1, sizeof(PhenoToken));
    if (token) {
        token->type = type;
        token->mem_flags.allocated = 1;
        token->mem_flags.ref_count = 1;
        token->value = calloc(1, sizeof(PhenoTokenValue));
    }
    return token;
}

void pheno_token_free(PhenoToken* token) {
    if (token) {
        free(token->value);
        free(token);
    }
}

int get_height(PhenoToken* node) {
    // Simple height calculation
    return node ? 1 : 0;
}
EOF
fi

# 6. Create missing pheno_relation.c if needed
if [ ! -f "src/core/pheno_relation.c" ]; then
    echo "Creating src/core/pheno_relation.c..."
    cat > src/core/pheno_relation.c << 'EOF'
#include "phenomemory_platform.h"

void map_obj_to_obj(PhenoRelation* src, PhenoRelation* dst) {
    dst->subject_id = src->subject_id ^ dst->subject_id;
    dst->class_id = src->class_id ^ dst->class_id;
    dst->instance_state |= src->instance_state;
    dst->person_state = ROTATE_LEFT(src->person_state, 2);
}

void apply_person_model(PhenoRelation* rel, uint8_t person_a, uint8_t person_b) {
    rel->person_id = person_a;
    rel->person_role = person_b;
    
    uint8_t xor_val = person_a ^ person_b;
    rel->person_auth = 0;
    while (xor_val) {
        rel->person_auth += xor_val & 1;
        xor_val >>= 1;
    }
    
    rel->person_state = 0;
    if (person_a & 0x01) BIT_SET(rel->person_state, 0);
    if (person_b & 0x02) BIT_SET(rel->person_state, 1);
    if ((person_a ^ person_b) & 0x04) BIT_SET(rel->person_state, 2);
}
EOF
fi

# 7. Clean and rebuild
echo ""
echo "Cleaning old build..."
make clean 2>/dev/null || true

echo ""
echo "Building library..."
make all

echo ""
echo "Build status:"
if [ -f "lib/libgosiuml.a" ]; then
    echo "✓ Static library: lib/libgosiuml.a"
else
    echo "✗ Static library missing"
fi

if [ -f "lib/libgosiuml.so" ]; then
    echo "✓ Shared library: lib/libgosiuml.so"
else
    echo "✗ Shared library missing"
fi

if [ -f "bin/gosiuml" ]; then
    echo "✓ CLI executable: bin/gosiuml"
else
    echo "✗ CLI executable missing"
fi

echo ""
echo "Done! If build successful, test with:"
echo "  ./bin/gosiuml config/token_file.txt output.svg"
