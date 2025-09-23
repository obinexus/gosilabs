// src/cli/main.c - CLI interface for gosiuml
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "gosiuml.h"
#include "cli_parser.h"

// Version info
#define GOSIUML_VERSION "1.0.0"
#define GOSIUML_BUILD "2025.01.20"

// CLI options
typedef struct {
    char* input_file;
    char* output_file;
    int verbose;
    int format; // 0=SVG, 1=XML, 2=JSON
    int show_bitfields;
    int show_state_machine;
} CliOptions;

// Print usage information
void print_usage(const char* program_name) {
    printf("GosiUML - PhenoMemory State Machine Visualizer v%s\n", GOSIUML_VERSION);
    printf("Usage: %s [OPTIONS] <input_file> <output_file>\n\n", program_name);
    printf("Options:\n");
    printf("  -h, --help              Show this help message\n");
    printf("  -v, --verbose           Enable verbose output\n");
    printf("  -V, --version           Show version information\n");
    printf("  -f, --format FORMAT     Output format (svg|xml|json) [default: svg]\n");
    printf("  -b, --bitfields         Show detailed bitfield visualization\n");
    printf("  -s, --state-machine     Generate state machine diagram\n");
    printf("  -t, --test              Run built-in test suite\n");
    printf("\nExamples:\n");
    printf("  %s token_file.txt output.svg\n", program_name);
    printf("  %s -f xml -v token_file.txt output.xml\n", program_name);
    printf("  %s --state-machine -b tokens.txt state_machine.svg\n", program_name);
}

// Parse command line arguments
int parse_arguments(int argc, char* argv[], CliOptions* options) {
    static struct option long_options[] = {
        {"help",          no_argument,       0, 'h'},
        {"verbose",       no_argument,       0, 'v'},
        {"version",       no_argument,       0, 'V'},
        {"format",        required_argument, 0, 'f'},
        {"bitfields",     no_argument,       0, 'b'},
        {"state-machine", no_argument,       0, 's'},
        {"test",          no_argument,       0, 't'},
        {0, 0, 0, 0}
    };
    
    int opt;
    int option_index = 0;
    
    // Initialize options
    memset(options, 0, sizeof(CliOptions));
    options->format = FORMAT_SVG; // Default to SVG
    
    while ((opt = getopt_long(argc, argv, "hvVf:bst", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'h':
                print_usage(argv[0]);
                exit(0);
                
            case 'v':
                options->verbose = 1;
                break;
                
            case 'V':
                printf("GosiUML version %s (build %s)\n", GOSIUML_VERSION, GOSIUML_BUILD);
                printf("OBINexus PhenoMemory State Machine Visualizer\n");
                exit(0);
                
            case 'f':
                if (strcmp(optarg, "svg") == 0) {
                    options->format = FORMAT_SVG;
                } else if (strcmp(optarg, "xml") == 0) {
                    options->format = FORMAT_XML;
                } else if (strcmp(optarg, "json") == 0) {
                    options->format = FORMAT_JSON;
                } else {
                    fprintf(stderr, "Error: Unknown format '%s'\n", optarg);
                    return -1;
                }
                break;
                
            case 'b':
                options->show_bitfields = 1;
                break;
                
            case 's':
                options->show_state_machine = 1;
                break;
                
            case 't':
                // Run test suite
                return gosiuml_run_tests();
                
            default:
                print_usage(argv[0]);
                return -1;
        }
    }
    
    // Check for required positional arguments
    if (optind + 2 > argc) {
        fprintf(stderr, "Error: Missing required arguments\n");
        print_usage(argv[0]);
        return -1;
    }
    
    options->input_file = argv[optind];
    options->output_file = argv[optind + 1];
    
    return 0;
}

int main(int argc, char* argv[]) {
    CliOptions options;
    
    // Parse command line arguments
    if (parse_arguments(argc, argv, &options) != 0) {
        return 1;
    }
    
    // Initialize gosiuml library
    if (gosiuml_init() != 0) {
        fprintf(stderr, "Failed to initialize gosiuml library\n");
        return 1;
    }
    
    if (options.verbose) {
        printf("GosiUML v%s starting...\n", GOSIUML_VERSION);
        printf("Input file: %s\n", options.input_file);
        printf("Output file: %s\n", options.output_file);
        printf("Format: %s\n", 
            options.format == FORMAT_SVG ? "SVG" :
            options.format == FORMAT_XML ? "XML" : "JSON");
    }
    
    // Parse input tokens
    int token_count = 0;
    PhenoToken* tokens = gosiuml_parse_file(options.input_file, &token_count);
    if (!tokens) {
        fprintf(stderr, "Failed to parse input file: %s\n", options.input_file);
        gosiuml_cleanup();
        return 1;
    }
    
    if (options.verbose) {
        printf("Parsed %d tokens successfully\n", token_count);
    }
    
    // Process tokens through state machine
    GosiUMLContext* context = gosiuml_create_context();
    if (!context) {
        fprintf(stderr, "Failed to create processing context\n");
        gosiuml_free_tokens(tokens, token_count);
        gosiuml_cleanup();
        return 1;
    }
    
    // Configure context based on options
    gosiuml_set_option(context, GOSIUML_OPT_SHOW_BITFIELDS, options.show_bitfields);
    gosiuml_set_option(context, GOSIUML_OPT_STATE_MACHINE, options.show_state_machine);
    gosiuml_set_option(context, GOSIUML_OPT_VERBOSE, options.verbose);
    
    // Process tokens
    for (int i = 0; i < token_count; i++) {
        if (gosiuml_process_token(context, &tokens[i]) != 0) {
            fprintf(stderr, "Warning: Failed to process token %d\n", i);
        }
    }
    
    // Generate output based on format
    int result = 0;
    switch (options.format) {
        case FORMAT_SVG:
            result = gosiuml_generate_svg(context, tokens, token_count, options.output_file);
            break;
        case FORMAT_XML:
            result = gosiuml_generate_xml(context, tokens, token_count, options.output_file);
            break;
        case FORMAT_JSON:
            result = gosiuml_generate_json(context, tokens, token_count, options.output_file);
            break;
    }
    
    if (result != 0) {
        fprintf(stderr, "Failed to generate output file\n");
    } else if (options.verbose) {
        printf("Successfully generated: %s\n", options.output_file);
    }
    
    // Cleanup
    gosiuml_free_context(context);
    gosiuml_free_tokens(tokens, token_count);
    gosiuml_cleanup();
    
    return result;
}

// src/cli/cli_parser.c - Additional CLI utilities
#include "cli_parser.h"
#include <stdio.h>

int gosiuml_run_tests() {
    printf("Running GosiUML test suite...\n");
    
    // Test 1: Basic token creation
    printf("Test 1: Token creation... ");
    PhenoToken* test_token = gosiuml_create_token(NODE_IDENTITY, "TEST_NODE");
    if (test_token) {
        printf("PASS\n");
        gosiuml_free_token(test_token);
    } else {
        printf("FAIL\n");
        return 1;
    }
    
    // Test 2: State machine transitions
    printf("Test 2: State machine... ");
    GosiUMLContext* ctx = gosiuml_create_context();
    if (ctx && gosiuml_test_state_machine(ctx) == 0) {
        printf("PASS\n");
        gosiuml_free_context(ctx);
    } else {
        printf("FAIL\n");
        return 1;
    }
    
    // Test 3: Bitfield operations
    printf("Test 3: Bitfield operations... ");
    if (gosiuml_test_bitfields() == 0) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        return 1;
    }
    
    printf("\nAll tests passed!\n");
    return 0;
}
