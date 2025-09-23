#include <stdio.h>
#include <ctype.h>
#include <string.h>

typedef enum {
    TOKEN_ID,
    TOKEN_INT,
    TOKEN_OP,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char value[32];
} Token;

Token tokenize(const char **input) {
    Token token = {TOKEN_EOF, ""};
    
    // Skip whitespace
    while (isspace(**input)) (*input)++;
    
    if (**input == '\0') return token;
    
    // Handle identifiers [a-zA-Z][a-zA-Z0-9]*
    if (isalpha(**input)) {
        token.type = TOKEN_ID;
        int i = 0;
        while (isalnum(**input) && i < 31) {
            token.value[i++] = **input;
            (*input)++;
        }
        token.value[i] = '\0';
    }
    // Handle integers [0-9]+
    else if (isdigit(**input)) {
        token.type = TOKEN_INT;
        int i = 0;
        while (isdigit(**input) && i < 31) {
            token.value[i++] = **input;
            (*input)++;
        }
        token.value[i] = '\0';
    }
    // Handle operators
    else {
        token.type = TOKEN_OP;
        token.value[0] = **input;
        token.value[1] = '\0';
        (*input)++;
    }
    
    return token;
}

int main() {
    const char *input = "x = 1 + x";
    Token token;
    
    printf("Tokenizing: %s\n", input);
    
    while ((token = tokenize(&input)).type != TOKEN_EOF) {
        const char *type_str;
        switch (token.type) {
            case TOKEN_ID: type_str = "IDENTIFIER"; break;
            case TOKEN_INT: type_str = "INTEGER"; break;
            case TOKEN_OP: type_str = "OPERATOR"; break;
            default: type_str = "UNKNOWN";
        }
        printf("Token: %s\tValue: %s\n", type_str, token.value);
    }
    
    return 0;
}
