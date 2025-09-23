// main.c
#include <string.h>

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
    rel->person_auth = __builtin_popcount(person_a ^ person_b);
    
    // Set state flags
    rel->person_state = 0;
    if (person_a & 0x01) BIT_SET(rel->person_state, 0); // Active
    if (person_b & 0x02) BIT_SET(rel->person_state, 1); // Connected
    if ((person_a ^ person_b) & 0x04) BIT_SET(rel->person_state, 2); // Differential
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <token_file> <output.svg>\n", argv[0]);
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
    for (int i = 0; i < (count < 3 ? count : 3); i++) {
        PhenoRelation* r = &tokens[i].relation;
        printf("Token %d [%s]:\n", i, tokens[i].token_name);
        printf("  Subject: %02X %02X %02X %02X\n", 
               r->subject_id, r->subject_type, r->subject_state, r->subject_class);
        printf("  Class:   %02X %02X %02X %02X\n",
               r->class_id, r->class_category, r->class_taxonomy, r->class_level);
        printf("  Instance: %02X %02X %02X %02X\n",
               r->instance_id, r->instance_type, r->instance_state, r->instance_flags);
    }
    
    free(tokens);
    return 0;
}
