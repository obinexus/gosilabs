// svg_generator.c
void generate_svg_from_tokens(PhenoToken* tokens, int count, const char* output) {
    FILE* svg = fopen(output, "w");
    if (!svg) return;
    
    // SVG header
    fprintf(svg, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    fprintf(svg, "<svg xmlns=\"http://www.w3.org/2000/svg\" ");
    fprintf(svg, "width=\"800\" height=\"600\" viewBox=\"0 0 800 600\">\n");
    
    // Background
    fprintf(svg, "<rect width=\"800\" height=\"600\" fill=\"#1a1a1a\"/>\n");
    
    // Title
    fprintf(svg, "<text x=\"400\" y=\"30\" text-anchor=\"middle\" ");
    fprintf(svg, "fill=\"white\" font-size=\"20\">Phenomenological Bitfield Map</text>\n");
    
    // Generate nodes for each token
    for (int i = 0; i < count; i++) {
        PhenoToken* t = &tokens[i];
        PhenoRelation* r = &t->relation;
        
        // Calculate position using bitfield values
        float x = 100 + (r->subject_id * 5) % 600;
        float y = 100 + (r->class_id * 7) % 400;
        float radius = 5 + (r->instance_state & 0x0F);
        
        // Color based on bitfield patterns
        uint32_t color = 0;
        color |= (r->subject_type << 16);  // Red channel
        color |= (r->class_category << 8);  // Green channel
        color |= r->instance_type;          // Blue channel
        
        // Draw node
        fprintf(svg, "<circle cx=\"%.1f\" cy=\"%.1f\" r=\"%.1f\" ", x, y, radius);
        fprintf(svg, "fill=\"#%06X\" opacity=\"0.7\" ", color);
        fprintf(svg, "stroke=\"white\" stroke-width=\"0.5\">\n");
        fprintf(svg, "  <title>%s [%02X:%02X:%02X]</title>\n", 
                t->token_name, r->subject_id, r->class_id, r->instance_id);
        fprintf(svg, "</circle>\n");
        
        // Draw connections based on relationships
        if (i > 0) {
            PhenoRelation* prev = &tokens[i-1].relation;
            
            // Check bit patterns for connection criteria
            if ((r->subject_class & prev->subject_class) || 
                (r->class_taxonomy == prev->class_taxonomy)) {
                
                float px = 100 + (prev->subject_id * 5) % 600;
                float py = 100 + (prev->class_id * 7) % 400;
                
                // Connection line
                fprintf(svg, "<line x1=\"%.1f\" y1=\"%.1f\" ", px, py);
                fprintf(svg, "x2=\"%.1f\" y2=\"%.1f\" ", x, y);
                fprintf(svg, "stroke=\"#%06X\" stroke-width=\"0.3\" ", 
                        color & 0x7F7F7F);
                fprintf(svg, "opacity=\"0.5\"/>\n");
            }
        }
    }
    
    // Legend
    fprintf(svg, "<g transform=\"translate(650, 100)\">\n");
    fprintf(svg, "  <text fill=\"white\" font-size=\"12\">Legend:</text>\n");
    fprintf(svg, "  <text y=\"20\" fill=\"#FF0000\" font-size=\"10\">Subject</text>\n");
    fprintf(svg, "  <text y=\"35\" fill=\"#00FF00\" font-size=\"10\">Class</text>\n");
    fprintf(svg, "  <text y=\"50\" fill=\"#0000FF\" font-size=\"10\">Instance</text>\n");
    fprintf(svg, "</g>\n");
    
    fprintf(svg, "</svg>\n");
    fclose(svg);
}
