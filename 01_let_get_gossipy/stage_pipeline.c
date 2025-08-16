
#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern FILE *yyin;
extern TokenList global_tokens;
extern int lex_and_store();

// ===== TOKEN UTILITIES =====
const char* token_type_name(TokenType type) {
    switch(type) {
        case TOKEN_BANG: return "BANG";
        case TOKEN_HASH: return "HASH";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_EQUAL: return "EQUAL";
        case TOKEN_ARROW: return "ARROW";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LT: return "LT";
        case TOKEN_GT: return "GT";
        case TOKEN_LBRACKET: return "LBRACKET";
        case TOKEN_RBRACKET: return "RBRACKET";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_COLON: return "COLON";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_DOT_DOT: return "DOT_DOT";
        case TOKEN_DEF: return "DEF";
        case TOKEN_BIND: return "BIND";
        case TOKEN_UNBIND: return "UNBIND";
        case TOKEN_SPAN: return "SPAN";
        case TOKEN_RANGE: return "RANGE";
        case TOKEN_VEC: return "VEC";
        case TOKEN_NIL: return "NIL";
        case TOKEN_NULL: return "NULL";
        case TOKEN_LET: return "LET";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_INTEGER: return "INTEGER";
        case TOKEN_FLOAT: return "FLOAT";
        case TOKEN_NEWLINE: return "NEWLINE";
        case TOKEN_EOF: return "EOF";
        case TOKEN_UNKNOWN: return "UNKNOWN";
        default: return "INVALID";
    }
}

Token create_token(TokenType type, const char *lexeme, Position pos) {
    Token token;
    token.type = type;
    token.lexeme = strdup(lexeme ? lexeme : "");
    token.pos = pos;
    token.value.int_val = 0; // default
    return token;
}

void token_list_init(TokenList *list) {
    list->tokens = malloc(sizeof(Token) * 32);
    list->count = 0;
    list->capacity = 32;
}

void token_list_add(TokenList *list, Token token) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->tokens = realloc(list->tokens, sizeof(Token) * list->capacity);
    }
    list->tokens[list->count++] = token;
}

void token_list_free(TokenList *list) {
    for (size_t i = 0; i < list->count; i++) {
        free(list->tokens[i].lexeme);
    }
    free(list->tokens);
    list->tokens = NULL;
    list->count = 0;
    list->capacity = 0;
}

void print_token_json(const Token *token) {
    printf("    {\n");
    printf("      \"type\": \"%s\",\n", token_type_name(token->type));
    printf("      \"lexeme\": \"%s\",\n", token->lexeme);
    printf("      \"position\": {\n");
    printf("        \"line\": %d,\n", token->pos.line);
    printf("        \"column\": %d,\n", token->pos.column);
    printf("        \"offset\": %d\n", token->pos.offset);
    printf("      }");
    
    if (token->type == TOKEN_INTEGER) {
        printf(",\n      \"value\": %d", token->value.int_val);
    } else if (token->type == TOKEN_FLOAT) {
        printf(",\n      \"value\": %.6f", token->value.float_val);
    }
    
    printf("\n    }");
}

void print_token_table(const Token *token) {
    printf("| %-12s | %-15s | %4d:%-2d | %-10s |\n", 
           token_type_name(token->type),
           token->lexeme,
           token->pos.line, token->pos.column,
           (token->type == TOKEN_INTEGER) ? "int" :
           (token->type == TOKEN_FLOAT) ? "float" : "string");
}

// ===== PIPELINE STAGES =====

void stage1_raw_lexemes(const char *filename) {
    printf("\n=== STAGE 1: Raw Lexemes ===\n");
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Cannot open file");
        return;
    }
    
    printf("Raw file content:\n");
    printf("ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n");
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file)) {
        printf("%s", buffer);
    }
    printf("ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n");
    fclose(file);
}

void stage2_token_stream(const char *filename) {
    printf("\n=== STAGE 2: Token Stream ===\n");
    
    yyin = fopen(filename, "r");
    if (!yyin) {
        perror("Cannot open file");
        return;
    }
    
    int token_count = lex_and_store();
    fclose(yyin);
    
    printf("Generated %d tokens:\n\n", token_count);
    
    // Table format
    printf("Token Table:\n");
    printf("ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄ¿\n");
    printf("³ Token Type  ³ Lexeme          ³ Pos     ³ Value Type ³\n");
    printf("ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄ´\n");
    
    for (size_t i = 0; i < global_tokens.count; i++) {
        print_token_table(&global_tokens.tokens[i]);
    }
    printf("ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÙ\n");
    
    // JSON format
    printf("\nJSON Format:\n");
    printf("{\n  \"tokens\": [\n");
    for (size_t i = 0; i < global_tokens.count; i++) {
        print_token_json(&global_tokens.tokens[i]);
        if (i < global_tokens.count - 1) printf(",");
        printf("\n");
    }
    printf("  ]\n}\n");
}

void stage3_ast_preview() {
    printf("\n=== STAGE 3: AST Preview ===\n");
    printf("(Parser will build AST nodes from token stream)\n\n");
    
    // Mock AST analysis of token patterns
    printf("Detected patterns:\n");
    for (size_t i = 0; i < global_tokens.count; i++) {
        Token *token = &global_tokens.tokens[i];
        
        if (token->type == TOKEN_BANG) {
            printf(" Invocation pattern starting at %d:%d\n", 
                   token->pos.line, token->pos.column);
        }
        if (token->type == TOKEN_BIND || token->type == TOKEN_UNBIND) {
            printf(" Bind operation at %d:%d\n", 
                   token->pos.line, token->pos.column);
        }
        if (token->type == TOKEN_VEC) {
            printf(" Vector construction at %d:%d\n", 
                   token->pos.line, token->pos.column);
        }
    }
}

void stage4_codegen_preview() {
    printf("\n=== STAGE 4: Codegen Preview ===\n");
    printf("(Will generate C skeleton from AST)\n\n");
    
    printf("Expected C output patterns:\n");
    printf(" #bind()  parallel_diff() calls\n");
    printf(" !vec<N>()  vec_make() + norm() calls\n");
    printf(" span[..]  normalize_to_span() calls\n");
    printf(" NIL handling  NaN or NIL_PTR checks\n");
}
