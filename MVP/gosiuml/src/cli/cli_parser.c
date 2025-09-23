#include "cli_parser.h"
#include <stdio.h>

int parse_cli_config(const char* config_file) {
    // Simple config parser implementation
    FILE* f = fopen(config_file, "r");
    if (!f) return -1;
    
    // Parse config...
    fclose(f);
    return 0;
}

void print_token_info(PhenoToken* token) {
    if (!token) return;
    
    printf("Token ID: 0x%08X\n", token->token_id);
    printf("Token Name: %s\n", token->token_name);
    printf("Token Type: 0x%02X\n", token->token_type);
    printf("Memory Flags: allocated=%d, locked=%d, ref_count=%d\n",
           token->mem_flags.allocated,
           token->mem_flags.locked,
           token->mem_flags.ref_count);
}

void display_state_diagram(GosiUMLContext* ctx) {
    printf("State Machine Diagram:\n");
    printf("  NIL -> ALLOCATED -> LOCKED -> ACTIVE\n");
    printf("           |                      |\n");
    printf("         FREED <-----------------+\n");
}
