// token_parser.c
typedef struct {
    uint8_t  token_type;
    uint32_t token_value;
    char     token_name[64];
    PhenoRelation relation;
} PhenoToken;

// Parse tokens from file
PhenoToken* parse_token_file(const char* filename, int* count) {
    FILE* file = fopen(filename, "r");
    if (!file) return NULL;
    
    // Count tokens
    *count = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] != '#' && line[0] != '\n') (*count)++;
    }
    
    // Allocate token array
    PhenoToken* tokens = calloc(*count, sizeof(PhenoToken));
    rewind(file);
    
    // Parse tokens with bitfield operations
    int idx = 0;
    while (fgets(line, sizeof(line), file) && idx < *count) {
        if (line[0] == '#' || line[0] == '\n') continue;
        
        // Parse: TYPE VALUE NAME
        sscanf(line, "%hhu %u %63s", 
               &tokens[idx].token_type,
               &tokens[idx].token_value,
               tokens[idx].token_name);
        
        // Apply bitfield transformations
        PhenoRelation* rel = &tokens[idx].relation;
        
        // Subject mapping using bit operations
        rel->subject_id = tokens[idx].token_type & 0xFF;
        rel->subject_type = (tokens[idx].token_value >> 0) & 0xFF;
        rel->subject_state = (tokens[idx].token_value >> 8) & 0xFF;
        rel->subject_class = (tokens[idx].token_value >> 16) & 0xFF;
        
        // Class mapping with rotation
        rel->class_id = ROTATE_LEFT(tokens[idx].token_type, 3);
        rel->class_category = ROTATE_RIGHT(tokens[idx].token_type, 2);
        rel->class_taxonomy = tokens[idx].token_type ^ 0xAA;
        rel->class_level = __builtin_popcount(tokens[idx].token_value);
        
        // Instance mapping
        rel->instance_id = idx & 0xFF;
        rel->instance_type = tokens[idx].token_type;
        rel->instance_state = 0;
        BIT_SET(rel->instance_state, 0); // Active
        BIT_SET(rel->instance_state, 3); // Valid
        
        idx++;
    }
    
    fclose(file);
    return tokens;
}
