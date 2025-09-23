// src/core/token_parser.c
#include "phenomemory_platform.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Implementation from existing parse_token_file
PhenoToken* parse_token_file(const char* filename, int* count) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open token file");
        return NULL;
    }
    
    // Count lines to allocate memory
    int line_count = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] != '#' && line[0] != '\n') {
            line_count++;
        }
    }
    
    // Allocate tokens array
    PhenoToken* tokens = calloc(line_count, sizeof(PhenoToken));
    if (!tokens) {
        fclose(file);
        return NULL;
    }
    
    // Rewind and parse
    rewind(file);
    int token_index = 0;
    
    while (fgets(line, sizeof(line), file) && token_index < line_count) {
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n') {
            continue;
        }
        
        // Parse token: ID TYPE NAME [PROPERTIES]
        unsigned int id, type;
        char name[64];
        
        if (sscanf(line, "%u %x %63s", &id, &type, name) >= 3) {
            PhenoToken* token = &tokens[token_index];
            
            // Set basic properties
            token->token_id = id;
            token->token_type = (uint8_t)type;
            
            // FIX: Properly handle string copy with guaranteed null termination
            size_t name_len = strlen(name);
            size_t max_len = sizeof(token->token_name) - 1;
            if (name_len > max_len) {
                memcpy(token->token_name, name, max_len);
                token->token_name[max_len] = '\0';
            } else {
                strcpy(token->token_name, name);  // Safe when we know it fits
            }
            
            // Alternative fix using snprintf (simpler):
            // snprintf(token->token_name, sizeof(token->token_name), "%s", name);
            
            // Initialize type based on parsed value
            token->type.category = type & 0x0F;
            token->type.node_level = (type >> 4) & 0x07;
            token->type.cluster_id = (type >> 7) & 0xFF;
            
            // Initialize relation with pseudo-random values based on ID
            token->relation.subject_id = (uint8_t)(id & 0xFF);
            token->relation.subject_type = type & 0xFF;
            token->relation.subject_state = 0x01; // Active
            token->relation.class_id = (uint8_t)((id >> 8) & 0xFF);
            token->relation.class_category = (type >> 8) & 0xFF;
            token->relation.instance_id = (uint8_t)((id >> 16) & 0xFF);
            
            // Allocate and initialize value
            token->value = calloc(1, sizeof(PhenoTokenValue));
            if (token->value) {
                token->value->header.frame_id = id;
                token->value->header.data_size = 256;
                token->value->metrics.confidence = 500; // Mid-range confidence
            }
            
            // Set memory flags
            token->mem_flags.allocated = 1;
            token->mem_flags.ref_count = 1;
            
            token_index++;
        }
    }
    
    fclose(file);
    *count = token_index;
    return tokens;
}
