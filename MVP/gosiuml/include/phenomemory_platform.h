// include/phenomology_platform.h
#ifndef PHENOMOLOGY_PLATFORM_H
#define PHENOMOLOGY_PLATFORM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>

// Maximum payload size
#define PHENO_MAX_PAYLOAD 4096

// Sentinel definitions
#define PHENO_NIL  ((void*)-1)
#define PHENO_NULL ((void*)0)

// Bit manipulation macros
#define BIT_SET(val, bit) ((val) |= (1 << (bit)))
#define BIT_CLEAR(val, bit) ((val) &= ~(1 << (bit)))
#define BIT_CHECK(val, bit) ((val) & (1 << (bit)))
#define ROTATE_LEFT(val, n) (((val) << (n)) | ((val) >> (8 - (n))))

// PhenoTokenType - 32-bit bitfield representation
typedef struct {
    unsigned int category    : 4;   // 16 possible categories
    unsigned int node_level  : 3;   // 8 node levels
    unsigned int cluster_id  : 8;   // 256 clusters
    unsigned int frame_ref   : 8;   // 256 frame references
    unsigned int degradation : 4;   // 16 degradation levels
    unsigned int reserved    : 5;   // Future expansion
} PhenoTokenType;

// Category definitions
#define NODE_IDENTITY     0x01
#define NODE_STATE        0x02
#define NODE_DEGRADATION  0x03
#define CLUSTER_TOPOLOGY  0x04
#define CLUSTER_CONSENSUS 0x05
#define FRAME_REFERENCE   0x06
#define FRAME_TRANSFORM   0x07
#define FRAME_COLLAPSE    0x08

// PhenoRelation structure for object-to-object and person-to-person mapping
typedef struct {
    // Subject relation (32 bits)
    uint8_t subject_id;
    uint8_t subject_type;
    uint8_t subject_state;
    uint8_t subject_class;
    
    // Class relation (32 bits)
    uint8_t class_id;
    uint8_t class_category;
    uint8_t class_taxonomy;
    uint8_t class_level;
    
    // Instance relation (32 bits)
    uint8_t instance_id;
    uint8_t instance_type;
    uint8_t instance_state;
    uint8_t instance_flags;
    
    // Person-to-Person relation (32 bits)
    uint8_t person_id;
    uint8_t person_role;
    uint8_t person_auth;
    uint8_t person_state;
} PhenoRelation;

// PhenoTokenValue - Variable-length bitfield with metadata
typedef struct {
    // Header (64 bits)
    struct {
        unsigned int data_size    : 16;  // Up to 64KB data
        unsigned int encoding     : 4;   // 16 encoding types
        unsigned int compression  : 3;   // 8 compression levels
        unsigned int encrypted    : 1;   // Encryption flag
        unsigned int frame_id     : 16;  // Frame identifier
        unsigned int timestamp    : 24;  // Microsecond precision
    } header;
    
    // Degradation metrics (32 bits)
    struct {
        unsigned int score        : 10;  // 0-1023 (maps to 0.0-1.0)
        unsigned int confidence   : 10;  // Confidence level
        unsigned int retry_count  : 6;   // Max 63 retries
        unsigned int priority     : 6;   // 64 priority levels
    } metrics;
    
    // Flexible data payload
    union {
        uint8_t  raw_bytes[PHENO_MAX_PAYLOAD];
        uint32_t encoded_words[PHENO_MAX_PAYLOAD/4];
        struct {
            float re;
            float im;
        } complex_nums[PHENO_MAX_PAYLOAD/8];
    } data;
} PhenoTokenValue;

// Complete PhenoToken with memory management
typedef struct PhenoToken {
    // Token identification (32 bits)
    uint32_t token_id;
    uint8_t token_type;  // Simple type for compatibility
    char token_name[64]; // Token name string
    
    // Type information
    PhenoTokenType type;
    
    // Value payload
    PhenoTokenValue* value;  // Pointer for variable-length data
    
    // Phenomenological relation
    PhenoRelation relation;
    
    // Memory management bitfield (32 bits)
    struct {
        unsigned int allocated    : 1;   // Memory allocated flag
        unsigned int locked       : 1;   // Thread lock status
        unsigned int dirty        : 1;   // Modified flag
        unsigned int pinned       : 1;   // Cannot be swapped
        unsigned int shared       : 1;   // Shared memory flag
        unsigned int coherent     : 1;   // Cache coherent
        unsigned int nil_state    : 1;   // NIL sentinel flag
        unsigned int null_state   : 1;   // NULL sentinel flag
        unsigned int ref_count    : 8;   // Reference counting
        unsigned int mem_zone     : 4;   // Memory zone (0-15)
        unsigned int access_level : 4;   // Access permissions
        unsigned int reserved     : 8;   // Future use
    } mem_flags;
    
    // AVL-Trie navigation
    struct PhenoToken* left;
    struct PhenoToken* right;
    struct PhenoToken* parent;
    int8_t balance_factor;
    
} PhenoToken;

// State machine states
typedef enum {
    STATE_NIL,
    STATE_ALLOCATED,
    STATE_LOCKED,
    STATE_ACTIVE,
    STATE_DEGRADED,
    STATE_SHARED,
    STATE_FREED
} PhenoState;

// Function prototypes
PhenoToken* pheno_token_alloc(PhenoTokenType type);
void pheno_token_free(PhenoToken* token);
bool verify_geometric_proof(PhenoToken* token);
int pheno_memory_init(size_t pool_size);
void pheno_memory_cleanup(void);

// Token parsing functions
PhenoToken* parse_token_file(const char* filename, int* count);
void generate_svg_from_tokens(PhenoToken* tokens, int count, const char* output_file);

// Relation mapping functions
void map_obj_to_obj(PhenoRelation* src, PhenoRelation* dst);
void apply_person_model(PhenoRelation* rel, uint8_t person_a, uint8_t person_b);

// Utility functions
float calculate_degradation_score(PhenoToken* token);
int get_height(PhenoToken* node);

#endif // PHENOMOLOGY_PLATFORM_H
