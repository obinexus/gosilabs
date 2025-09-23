// pheno_platform.c - Complete phenomenological bitfield platform
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

// Bit manipulation macros
#define BIT_SET(value, bit)     ((value) |= (1U << (bit)))
#define BIT_CLEAR(value, bit)   ((value) &= ~(1U << (bit)))
#define BIT_TOGGLE(value, bit)  ((value) ^= (1U << (bit)))
#define BIT_CHECK(value, bit)   (((value) >> (bit)) & 1U)
#define ROTATE_LEFT(n, b)  (((n) << (b)) | ((n) >> (8 - (b))))
#define ROTATE_RIGHT(n, b) (((n) >> (b)) | ((n) << (8 - (b))))
#define UNSIGNED_SHIFT(val) ((size_t)(val) >> 0)

// Bitfield structure for phenomenological relationships
typedef struct {
    // Subject-to-Subject mapping (8 bits each)
    unsigned int subject_id     : 8;
    unsigned int subject_type   : 8;
    unsigned int subject_state  : 8;
    unsigned int subject_class  : 8;
    
    // Class-to-Class mapping (8 bits each)
    unsigned int class_id       : 8;
    unsigned int class_category : 8;
    unsigned int class_taxonomy : 8;
    unsigned int class_level    : 8;
    
    // Instance-to-Instance mapping (8 bits each)
    unsigned int instance_id    : 8;
    unsigned int instance_type  : 8;
    unsigned int instance_state : 8;
    unsigned int instance_flags : 8;
    
    // Person-to-Person model (8 bits each)
    unsigned int person_id      : 8;
    unsigned int person_role    : 8;
    unsigned int person_auth    : 8;
    unsigned int person_state   : 8;
} PhenoRelation;

// Token structure
typedef struct {
    uint8_t  token_type;
    uint32_t token_value;
    char     token_name[64];
    PhenoRelation relation;
} PhenoToken;

// Parse tokens from file
PhenoToken* parse_token_file(const char* filename, int* count) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file %s\n", filename);
        return NULL;
    }
    
    // Count tokens
    *count = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (line[0] != '#' && line[0] != '\n' && line[0] != '\0') {
            (*count)++;
        }
    }
    
    if (*count == 0) {
        printf("Warning: No tokens found in file\n");
        fclose(file);
        return NULL;
    }
    
    // Allocate token array
    PhenoToken* tokens = calloc(*count, sizeof(PhenoToken));
    if (!tokens) {
        printf("Error: Memory allocation failed\n");
        fclose(file);
        return NULL;
    }
    
    rewind(file);
    
    // Parse tokens with bitfield operations
    int idx = 0;
    while (fgets(line, sizeof(line), file) && idx < *count) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\0') continue;
        
        // Parse: TYPE VALUE NAME
        int parsed = sscanf(line, "%hhu %x %63s", 
                           &tokens[idx].token_type,
                           &tokens[idx].token_value,
                           tokens[idx].token_name);
        
        if (parsed < 3) {
            printf("Warning: Malformed line, using defaults\n");
            tokens[idx].token_type = idx & 0xFF;
            tokens[idx].token_value = 0x12345678;
            sprintf(tokens[idx].token_name, "TOKEN_%d", idx);
        }
        
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
        rel->class_level = 0;
        
        // Count bits for class level
        uint32_t val = tokens[idx].token_value;
        while (val) {
            rel->class_level += val & 1;
            val >>= 1;
        }
        
        // Instance mapping
        rel->instance_id = idx & 0xFF;
        rel->instance_type = tokens[idx].token_type;
        rel->instance_state = 0;
        BIT_SET(rel->instance_state, 0); // Active
        BIT_SET(rel->instance_state, 3); // Valid
        rel->instance_flags = 0xFF;
        
        // Person mapping
        rel->person_id = (tokens[idx].token_value >> 24) & 0xFF;
        rel->person_role = 0x01;
        rel->person_auth = 0x0F;
        rel->person_state = 0x07;
        
        idx++;
    }
    
    fclose(file);
    return tokens;
}

// Generate SVG visualization
void generate_svg_from_tokens(PhenoToken* tokens, int count, const char* output) {
    FILE* svg = fopen(output, "w");
    if (!svg) {
        printf("Error: Cannot create SVG file %s\n", output);
        return;
    }
    
    // SVG header
    fprintf(svg, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(svg, "<svg xmlns=\"http://www.w3.org/2000/svg\" ");
    fprintf(svg, "width=\"800\" height=\"600\" viewBox=\"0 0 800 600\">\n");
    
    // Background
    fprintf(svg, "  <rect width=\"800\" height=\"600\" fill=\"#1a1a1a\"/>\n");
    
    // Title
    fprintf(svg, "  <text x=\"400\" y=\"30\" text-anchor=\"middle\" ");
    fprintf(svg, "fill=\"white\" font-size=\"20\" font-family=\"monospace\">");
    fprintf(svg, "Phenomenological Bitfield Map</text>\n");
    
    // Generate nodes for each token
    for (int i = 0; i < count; i++) {
        PhenoToken* t = &tokens[i];
        PhenoRelation* r = &t->relation;
        
        // Calculate position using bitfield values
        float x = 100.0 + (float)((r->subject_id * 5) % 600);
        float y = 100.0 + (float)((r->class_id * 7) % 400);
        float radius = 5.0 + (float)(r->instance_state & 0x0F);
        
        // Color based on bitfield patterns
        uint32_t color = 0;
        color |= ((r->subject_type & 0xFF) << 16);   // Red channel
        color |= ((r->class_category & 0xFF) << 8);  // Green channel
        color |= (r->instance_type & 0xFF);          // Blue channel
        
        // Draw node
        fprintf(svg, "  <circle cx=\"%.1f\" cy=\"%.1f\" r=\"%.1f\" ", x, y, radius);
        fprintf(svg, "fill=\"#%06X\" opacity=\"0.7\" ", color & 0xFFFFFF);
        fprintf(svg, "stroke=\"white\" stroke-width=\"0.5\">\n");
        fprintf(svg, "    <title>%s [%02X:%02X:%02X]</title>\n", 
                t->token_name, r->subject_id, r->class_id, r->instance_id);
        fprintf(svg, "  </circle>\n");
        
        // Label for important nodes
        if (i < 10) {
            fprintf(svg, "  <text x=\"%.1f\" y=\"%.1f\" ", x, y - radius - 5);
            fprintf(svg, "fill=\"#FFFFFF\" font-size=\"8\" ");
            fprintf(svg, "text-anchor=\"middle\" font-family=\"monospace\">");
            fprintf(svg, "%s</text>\n", t->token_name);
        }
        
        // Draw connections based on relationships
        if (i > 0) {
            PhenoRelation* prev = &tokens[i-1].relation;
            
            // Check bit patterns for connection criteria
            if ((r->subject_class & prev->subject_class) || 
                (r->class_taxonomy == prev->class_taxonomy)) {
                
                float px = 100.0 + (float)((prev->subject_id * 5) % 600);
                float py = 100.0 + (float)((prev->class_id * 7) % 400);
                
                // Connection line
                fprintf(svg, "  <line x1=\"%.1f\" y1=\"%.1f\" ", px, py);
                fprintf(svg, "x2=\"%.1f\" y2=\"%.1f\" ", x, y);
                fprintf(svg, "stroke=\"#%06X\" stroke-width=\"0.3\" ", 
                        (color & 0x7F7F7F));
                fprintf(svg, "opacity=\"0.5\"/>\n");
            }
        }
    }
    
    // Legend
    fprintf(svg, "  <g transform=\"translate(650, 100)\">\n");
    fprintf(svg, "    <rect x=\"-10\" y=\"-20\" width=\"140\" height=\"100\" ");
    fprintf(svg, "fill=\"#000000\" opacity=\"0.7\" stroke=\"white\" stroke-width=\"1\"/>\n");
    fprintf(svg, "    <text fill=\"white\" font-size=\"12\" font-family=\"monospace\">Legend:</text>\n");
    fprintf(svg, "    <text y=\"20\" fill=\"#FF7F7F\" font-size=\"10\" font-family=\"monospace\">Subject Bits</text>\n");
    fprintf(svg, "    <text y=\"35\" fill=\"#7FFF7F\" font-size=\"10\" font-family=\"monospace\">Class Bits</text>\n");
    fprintf(svg, "    <text y=\"50\" fill=\"#7F7FFF\" font-size=\"10\" font-family=\"monospace\">Instance Bits</text>\n");
    fprintf(svg, "    <text y=\"65\" fill=\"#FFFF7F\" font-size=\"10\" font-family=\"monospace\">Person Model</text>\n");
    fprintf(svg, "  </g>\n");
    
    fprintf(svg, "</svg>\n");
    fclose(svg);
}

// Object-to-Object mapping function
void map_obj_to_obj(PhenoRelation* src, PhenoRelation* dst) {
    // XOR for differential mapping
    dst->subject_id = src->subject_id ^ dst->subject_id;
    dst->class_id = src->class_id ^ dst->class_id;
    
    // Bit-level merging
    dst->instance_state |= src->instance_state;
    dst->person_state = ROTATE_LEFT(src->person_state, 2);
}

// Person-to-Person model implementation
void apply_person_model(PhenoRelation* rel, uint8_t person_a, uint8_t person_b) {
    // Set person IDs using bit operations
    rel->person_id = person_a;
    rel->person_role = person_b;
    
    // Calculate authority level using bit counting
    uint8_t xor_val = person_a ^ person_b;
    rel->person_auth = 0;
    while (xor_val) {
        rel->person_auth += xor_val & 1;
        xor_val >>= 1;
    }
    
    // Set state flags
    rel->person_state = 0;
    if (person_a & 0x01) BIT_SET(rel->person_state, 0); // Active
    if (person_b & 0x02) BIT_SET(rel->person_state, 1); // Connected
    if ((person_a ^ person_b) & 0x04) BIT_SET(rel->person_state, 2); // Differential
}

// Main function
int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <token_file> <output.svg>\n", argv[0]);
        printf("\nExample token file format:\n");
        printf("# Comment lines start with #\n");
        printf("1 0x12345678 SUBJECT_ALPHA\n");
        printf("2 0x87654321 CLASS_BETA\n");
        return 1;
    }
    
    // Parse tokens from file
    int count = 0;
    PhenoToken* tokens = parse_token_file(argv[1], &count);
    if (!tokens) {
        printf("Failed to parse token file\n");
        return 1;
    }
    
    printf("Parsed %d tokens\n", count);
    
    // Apply phenomenological transformations
    for (int i = 0; i < count - 1; i++) {
        // Object-to-object mapping
        map_obj_to_obj(&tokens[i].relation, &tokens[i+1].relation);
        
        // Person-to-person model
        apply_person_model(&tokens[i].relation, 
                          tokens[i].token_type, 
                          tokens[i+1].token_type);
    }
    
    // Generate SVG visualization
    generate_svg_from_tokens(tokens, count, argv[2]);
    printf("Generated SVG: %s\n", argv[2]);
    
    // Display bit patterns for first few tokens
    printf("\nBit Pattern Analysis:\n");
    printf("====================\n");
    for (int i = 0; i < (count < 3 ? count : 3); i++) {
        PhenoRelation* r = &tokens[i].relation;
        printf("Token %d [%s]:\n", i, tokens[i].token_name);
        printf("  Subject:  %02X %02X %02X %02X\n", 
               r->subject_id, r->subject_type, r->subject_state, r->subject_class);
        printf("  Class:    %02X %02X %02X %02X\n",
               r->class_id, r->class_category, r->class_taxonomy, r->class_level);
        printf("  Instance: %02X %02X %02X %02X\n",
               r->instance_id, r->instance_type, r->instance_state, r->instance_flags);
        printf("  Person:   %02X %02X %02X %02X\n",
               r->person_id, r->person_role, r->person_auth, r->person_state);
        printf("\n");
    }
    
    free(tokens);
    return 0;
}
