// include/gosiuml.h - Main GosiUML library header (FIXED)
#ifndef GOSIUML_H
#define GOSIUML_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Export macros for shared library
#ifdef _WIN32
  #ifdef GOSIUML_EXPORTS
    #define GOSIUML_API __declspec(dllexport)
  #else
    #define GOSIUML_API __declspec(dllimport)
  #endif
#else
  #define GOSIUML_API __attribute__((visibility("default")))
#endif

// Version macros
#define GOSIUML_VERSION_MAJOR 1
#define GOSIUML_VERSION_MINOR 0
#define GOSIUML_VERSION_PATCH 0

// Output formats
typedef enum {
    FORMAT_SVG = 0,
    FORMAT_XML = 1,
    FORMAT_JSON = 2,
    FORMAT_PLANTUML = 3
} GosiUMLFormat;

// Options
typedef enum {
    GOSIUML_OPT_VERBOSE = 1,
    GOSIUML_OPT_SHOW_BITFIELDS = 2,
    GOSIUML_OPT_STATE_MACHINE = 3,
    GOSIUML_OPT_MEMORY_TRACKING = 4
} GosiUMLOption;

// Forward declarations only - actual definitions in phenomemory_platform.h
struct PhenoToken;
struct GosiUMLContext;
struct PhenoTokenType;
struct PhenoRelation;

typedef struct PhenoToken PhenoToken;
typedef struct GosiUMLContext GosiUMLContext;

// State definitions (moved here to avoid conflicts)
typedef enum {
    STATE_NIL,
    STATE_ALLOCATED,
    STATE_LOCKED,
    STATE_ACTIVE,
    STATE_DEGRADED,
    STATE_SHARED,
    STATE_FREED
} PhenoState;

// Include the main platform header which has the actual struct definitions
#include "phenomemory_platform.h"

// Include CLI parser after platform definitions
#include "cli_parser.h"

// ============================================
// Library Initialization and Cleanup
// ============================================

/**
 * Initialize the GosiUML library
 * @return 0 on success, negative on error
 */
GOSIUML_API int gosiuml_init(void);

/**
 * Cleanup library resources
 */
GOSIUML_API void gosiuml_cleanup(void);

/**
 * Get library version string
 * @return Version string in format "major.minor.patch"
 */
GOSIUML_API const char* gosiuml_version(void);

// ============================================
// Context Management
// ============================================

/**
 * Create a new processing context
 * @return New context or NULL on error
 */
GOSIUML_API GosiUMLContext* gosiuml_create_context(void);

/**
 * Free a context and its resources
 * @param ctx Context to free
 */
GOSIUML_API void gosiuml_free_context(GosiUMLContext* ctx);

/**
 * Set context option
 * @param ctx Context
 * @param option Option to set
 * @param value Option value
 * @return 0 on success
 */
GOSIUML_API int gosiuml_set_option(GosiUMLContext* ctx, GosiUMLOption option, int value);

// ============================================
// Token Operations
// ============================================

/**
 * Parse tokens from file
 * @param filename Input file path
 * @param count Output parameter for token count
 * @return Array of tokens or NULL on error
 */
GOSIUML_API PhenoToken* gosiuml_parse_file(const char* filename, int* count);

/**
 * Parse tokens from memory buffer
 * @param buffer Input buffer
 * @param size Buffer size
 * @param count Output parameter for token count
 * @return Array of tokens or NULL on error
 */
GOSIUML_API PhenoToken* gosiuml_parse_buffer(const char* buffer, size_t size, int* count);

/**
 * Create a single token
 * @param type Token type
 * @param name Token name
 * @return New token or NULL on error
 */
GOSIUML_API PhenoToken* gosiuml_create_token(uint8_t type, const char* name);

/**
 * Free a single token
 * @param token Token to free
 */
GOSIUML_API void gosiuml_free_token(PhenoToken* token);

/**
 * Free token array
 * @param tokens Token array
 * @param count Number of tokens
 */
GOSIUML_API void gosiuml_free_tokens(PhenoToken* tokens, int count);

/**
 * Process token through state machine
 * @param ctx Context
 * @param token Token to process
 * @return 0 on success
 */
GOSIUML_API int gosiuml_process_token(GosiUMLContext* ctx, PhenoToken* token);

// ============================================
// Output Generation
// ============================================

/**
 * Generate SVG output
 * @param ctx Context
 * @param tokens Token array
 * @param count Token count
 * @param output_file Output file path
 * @return 0 on success
 */
GOSIUML_API int gosiuml_generate_svg(GosiUMLContext* ctx, PhenoToken* tokens, 
                                     int count, const char* output_file);

/**
 * Generate XML output
 * @param ctx Context
 * @param tokens Token array
 * @param count Token count
 * @param output_file Output file path
 * @return 0 on success
 */
GOSIUML_API int gosiuml_generate_xml(GosiUMLContext* ctx, PhenoToken* tokens, 
                                     int count, const char* output_file);

/**
 * Generate JSON output
 * @param ctx Context
 * @param tokens Token array
 * @param count Token count
 * @param output_file Output file path
 * @return 0 on success
 */
GOSIUML_API int gosiuml_generate_json(GosiUMLContext* ctx, PhenoToken* tokens, 
                                      int count, const char* output_file);

// ============================================
// State Machine Operations
// ============================================

/**
 * Get current state of token
 * @param token Token
 * @return Current state
 */
GOSIUML_API int gosiuml_get_state(PhenoToken* token);

/**
 * Transition token to new state
 * @param token Token
 * @param new_state Target state
 * @return 0 on success, -1 if transition invalid
 */
GOSIUML_API int gosiuml_transition(PhenoToken* token, int new_state);

// ============================================
// Testing Functions
// ============================================

/**
 * Run built-in tests
 * @return 0 on success
 */
GOSIUML_API int gosiuml_run_tests(void);

/**
 * Test state machine functionality
 * @param ctx Context
 * @return 0 on success
 */
GOSIUML_API int gosiuml_test_state_machine(GosiUMLContext* ctx);

/**
 * Test bitfield operations
 * @return 0 on success
 */
GOSIUML_API int gosiuml_test_bitfields(void);

// ============================================
// Utility Functions
// ============================================

/**
 * Get last error message
 * @return Error string or NULL
 */
GOSIUML_API const char* gosiuml_get_error(void);

/**
 * Enable debug logging
 * @param enable True to enable
 */
GOSIUML_API void gosiuml_set_debug(bool enable);

#ifdef __cplusplus
}
#endif

#endif // GOSIUML_H
