// src/core/pheno_memory.c - Core memory management
#include "gosiuml.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// Global library state
static struct {
    bool initialized;
    pthread_mutex_t mutex;
    char error_buffer[256];
    bool debug_enabled;
} g_gosiuml_state = {
    .initialized = false,
    .debug_enabled = false
};

// Library initialization
int gosiuml_init(void) {
    if (g_gosiuml_state.initialized) {
        return 0;
    }
    
    if (pthread_mutex_init(&g_gosiuml_state.mutex, NULL) != 0) {
        return -1;
    }
    
    g_gosiuml_state.initialized = true;
    return 0;
}

void gosiuml_cleanup(void) {
    if (g_gosiuml_state.initialized) {
        pthread_mutex_destroy(&g_gosiuml_state.mutex);
        g_gosiuml_state.initialized = false;
    }
}

const char* gosiuml_version(void) {
    static char version[32];
    snprintf(version, sizeof(version), "%d.%d.%d",
             GOSIUML_VERSION_MAJOR,
             GOSIUML_VERSION_MINOR,
             GOSIUML_VERSION_PATCH);
    return version;
}

// Error handling
const char* gosiuml_get_error(void) {
    return g_gosiuml_state.error_buffer;
}

void gosiuml_set_debug(bool enable) {
    g_gosiuml_state.debug_enabled = enable;
}

// Token management
PhenoToken* gosiuml_create_token(uint8_t type, const char* name) {
    PhenoToken* token = calloc(1, sizeof(PhenoToken));
    if (!token) {
        return NULL;
    }
    
    token->token_type = type;
    if (name) {
        strncpy(token->token_name, name, sizeof(token->token_name) - 1);
    }
    
    token->mem_flags.allocated = 1;
    token->mem_flags.ref_count = 1;
    
    // Allocate value structure
    token->value = calloc(1, sizeof(PhenoTokenValue));
    
    return token;
}

void gosiuml_free_token(PhenoToken* token) {
    if (!token) return;
    
    if (token->value) {
        free(token->value);
    }
    free(token);
}

void gosiuml_free_tokens(PhenoToken* tokens, int count) {
    if (!tokens) return;
    
    for (int i = 0; i < count; i++) {
        if (tokens[i].value) {
            free(tokens[i].value);
        }
    }
    free(tokens);
}

// ========================================
// src/core/pheno_state_machine.c
// ========================================
#include "gosiuml.h"
#include <stdio.h>

// Context structure
struct GosiUMLContext {
    int current_state;
    int options;
    void* user_data;
    // State machine specific data
    struct {
        int state_count[7];  // Count of each state
        int transition_count;
    } stats;
};

GosiUMLContext* gosiuml_create_context(void) {
    GosiUMLContext* ctx = calloc(1, sizeof(GosiUMLContext));
    if (!ctx) {
        return NULL;
    }
    ctx->current_state = STATE_NIL;
    return ctx;
}

void gosiuml_free_context(GosiUMLContext* ctx) {
    if (ctx) {
        free(ctx);
    }
}

int gosiuml_set_option(GosiUMLContext* ctx, GosiUMLOption option, int value) {
    if (!ctx) return -1;
    
    switch (option) {
        case GOSIUML_OPT_VERBOSE:
            if (value) ctx->options |= (1 << 0);
            else ctx->options &= ~(1 << 0);
            break;
        case GOSIUML_OPT_SHOW_BITFIELDS:
            if (value) ctx->options |= (1 << 1);
            else ctx->options &= ~(1 << 1);
            break;
        case GOSIUML_OPT_STATE_MACHINE:
            if (value) ctx->options |= (1 << 2);
            else ctx->options &= ~(1 << 2);
            break;
        case GOSIUML_OPT_MEMORY_TRACKING:
            if (value) ctx->options |= (1 << 3);
            else ctx->options &= ~(1 << 3);
            break;
        default:
            return -1;
    }
    return 0;
}

int gosiuml_process_token(GosiUMLContext* ctx, PhenoToken* token) {
    if (!ctx || !token) return -1;
    
    // Simple state machine processing
    int current = gosiuml_get_state(token);
    ctx->stats.state_count[current]++;
    
    // Simulate some processing
    if (token->mem_flags.allocated && !token->mem_flags.locked) {
        token->mem_flags.locked = 1;
        ctx->stats.transition_count++;
    }
    
    return 0;
}

int gosiuml_get_state(PhenoToken* token) {
    if (!token) return STATE_NIL;
    
    if (!token->mem_flags.allocated) return STATE_NIL;
    if (token->mem_flags.locked) return STATE_LOCKED;
    if (token->mem_flags.shared) return STATE_SHARED;
    if (token->value && token->value->metrics.score > 600) return STATE_DEGRADED;
    
    return STATE_ACTIVE;
}

int gosiuml_transition(PhenoToken* token, int new_state) {
    if (!token) return -1;
    
    int current = gosiuml_get_state(token);
    
    // Validate transition
    // (Simplified - would have full transition table in production)
    switch (new_state) {
        case STATE_ALLOCATED:
            if (current != STATE_NIL) return -1;
            token->mem_flags.allocated = 1;
            break;
        case STATE_LOCKED:
            if (current != STATE_ALLOCATED && current != STATE_ACTIVE) return -1;
            token->mem_flags.locked = 1;
            break;
        case STATE_FREED:
            token->mem_flags.allocated = 0;
            token->mem_flags.locked = 0;
            break;
        default:
            return -1;
    }
    
    return 0;
}

// ========================================
// src/core/token_parser.c
// ========================================
#include "gosiuml.h"
#include <stdio.h>
#include <string.h>

PhenoToken* gosiuml_parse_file(const char* filename, int* count) {
    return parse_token_file(filename, count);  // Use existing implementation
}

PhenoToken* gosiuml_parse_buffer(const char* buffer, size_t size, int* count) {
    // Create temporary file for parsing
    FILE* tmpfile = fopen("/tmp/gosiuml_buffer.tmp", "w");
    if (!tmpfile) return NULL;
    
    fwrite(buffer, 1, size, tmpfile);
    fclose(tmpfile);
    
    PhenoToken* result = parse_token_file("/tmp/gosiuml_buffer.tmp", count);
    remove("/tmp/gosiuml_buffer.tmp");
    
    return result;
}

// ========================================
// src/core/svg_generator.c
// ========================================
#include "gosiuml.h"
#include <stdio.h>

int gosiuml_generate_svg(GosiUMLContext* ctx, PhenoToken* tokens, 
                         int count, const char* output_file) {
    generate_svg_from_tokens(tokens, count, output_file);
    return 0;
}

int gosiuml_generate_xml(GosiUMLContext* ctx, PhenoToken* tokens, 
                         int count, const char* output_file) {
    FILE* xml = fopen(output_file, "w");
    if (!xml) return -1;
    
    fprintf(xml, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(xml, "<gosiuml version=\"%s\">\n", gosiuml_version());
    fprintf(xml, "  <tokens count=\"%d\">\n", count);
    
    for (int i = 0; i < count; i++) {
        PhenoToken* t = &tokens[i];
        fprintf(xml, "    <token id=\"%u\" name=\"%s\">\n", t->token_id, t->token_name);
        fprintf(xml, "      <type value=\"0x%02X\"/>\n", t->token_type);
        fprintf(xml, "      <state>%d</state>\n", gosiuml_get_state(t));
        fprintf(xml, "      <mem_flags allocated=\"%d\" locked=\"%d\" ref_count=\"%d\"/>\n",
                t->mem_flags.allocated, t->mem_flags.locked, t->mem_flags.ref_count);
        fprintf(xml, "    </token>\n");
    }
    
    fprintf(xml, "  </tokens>\n");
    fprintf(xml, "</gosiuml>\n");
    fclose(xml);
    
    return 0;
}

int gosiuml_generate_json(GosiUMLContext* ctx, PhenoToken* tokens, 
                          int count, const char* output_file) {
    FILE* json = fopen(output_file, "w");
    if (!json) return -1;
    
    fprintf(json, "{\n");
    fprintf(json, "  \"version\": \"%s\",\n", gosiuml_version());
    fprintf(json, "  \"tokens\": [\n");
    
    for (int i = 0; i < count; i++) {
        PhenoToken* t = &tokens[i];
        fprintf(json, "    {\n");
        fprintf(json, "      \"id\": %u,\n", t->token_id);
        fprintf(json, "      \"name\": \"%s\",\n", t->token_name);
        fprintf(json, "      \"type\": \"0x%02X\",\n", t->token_type);
        fprintf(json, "      \"state\": %d,\n", gosiuml_get_state(t));
        fprintf(json, "      \"mem_flags\": {\n");
        fprintf(json, "        \"allocated\": %s,\n", t->mem_flags.allocated ? "true" : "false");
        fprintf(json, "        \"locked\": %s,\n", t->mem_flags.locked ? "true" : "false");
        fprintf(json, "        \"ref_count\": %d\n", t->mem_flags.ref_count);
        fprintf(json, "      }\n");
        fprintf(json, "    }%s\n", (i < count - 1) ? "," : "");
    }
    
    fprintf(json, "  ]\n");
    fprintf(json, "}\n");
    fclose(json);
    
    return 0;
}

// ========================================
// src/core/pheno_relation.c
// ========================================
#include "gosiuml.h"

// Test functions
int gosiuml_test_state_machine(GosiUMLContext* ctx) {
    if (!ctx) return -1;
    
    // Create test token
    PhenoToken* token = gosiuml_create_token(NODE_IDENTITY, "TEST");
    if (!token) return -1;
    
    // Test transitions
    int result = 0;
    
    // NIL -> ALLOCATED (should already be allocated)
    if (gosiuml_get_state(token) != STATE_ACTIVE) {
        result = -1;
    }
    
    // ACTIVE -> LOCKED
    if (gosiuml_transition(token, STATE_LOCKED) == 0) {
        if (gosiuml_get_state(token) != STATE_LOCKED) {
            result = -1;
        }
    }
    
    gosiuml_free_token(token);
    return result;
}

int gosiuml_test_bitfields(void) {
    PhenoToken token = {0};
    
    // Test bitfield operations
    token.mem_flags.allocated = 1;
    if (!token.mem_flags.allocated) return -1;
    
    token.mem_flags.ref_count = 255;
    if (token.mem_flags.ref_count != 255) return -1;
    
    token.mem_flags.mem_zone = 15;
    if (token.mem_flags.mem_zone != 15) return -1;
    
    return 0;
}
