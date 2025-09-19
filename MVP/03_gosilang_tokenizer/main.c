// gosilang_tokenizer.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// Token types for gosilang
typedef enum {
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_OPERATOR,
    TOKEN_KEYWORD,
    TOKEN_DELIMITER,
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char* value;
    int line;
    int column;
    size_t length;
} Token;

// Tokenizer state
typedef struct {
    const char* input;
    size_t position;
    size_t length;
    int line;
    int column;
    Token* tokens;
    size_t token_count;
    size_t token_capacity;
} Tokenizer;

// R"" raw string patterns for token matching
const char* IDENTIFIER_PATTERN = R"([a-zA-Z_][a-zA-Z0-9_]*)";
const char* NUMBER_PATTERN = R"([0-9]+(\.[0-9]+)?)";
const char* STRING_PATTERN = R"("([^"\\]|\\.)*")";
const char* OPERATOR_PATTERN = R"([+\-*/=<>!&|]+)";
const char* DELIMITER_PATTERN = R"([(){}\[\],;:])";

// Keywords for gosilang
const char* KEYWORDS[] = {
    "func", "var", "const", "if", "else", "for", 
    "while", "return", "import", "export", "class"
};
const size_t KEYWORD_COUNT = sizeof(KEYWORDS) / sizeof(KEYWORDS[0]);

// Initialize tokenizer
Tokenizer* tokenizer_create(const char* input) {
    Tokenizer* t = malloc(sizeof(Tokenizer));
    t->input = input;
    t->position = 0;
    t->length = strlen(input);
    t->line = 1;
    t->column = 1;
    t->token_capacity = 100;
    t->tokens = malloc(sizeof(Token) * t->token_capacity);
    t->token_count = 0;
    return t;
}

// Check if string matches pattern (simplified)
bool matches_pattern(const char* str, size_t pos, const char* pattern) {
    // In production, use regex library
    // This is a simplified implementation
    if (strcmp(pattern, IDENTIFIER_PATTERN) == 0) {
        return isalpha(str[pos]) || str[pos] == '_';
    } else if (strcmp(pattern, NUMBER_PATTERN) == 0) {
        return isdigit(str[pos]);
    } else if (strcmp(pattern, STRING_PATTERN) == 0) {
        return str[pos] == '"';
    } else if (strcmp(pattern, OPERATOR_PATTERN) == 0) {
        return strchr("+-*/=<>!&|", str[pos]) != NULL;
    } else if (strcmp(pattern, DELIMITER_PATTERN) == 0) {
        return strchr("(){}[],;:", str[pos]) != NULL;
    }
    return false;
}

// Check if identifier is keyword
bool is_keyword(const char* identifier) {
    for (size_t i = 0; i < KEYWORD_COUNT; i++) {
        if (strcmp(identifier, KEYWORDS[i]) == 0) {
            return true;
        }
    }
    return false;
}

// Add token to tokenizer
void add_token(Tokenizer* t, TokenType type, const char* value, size_t length) {
    if (t->token_count >= t->token_capacity) {
        t->token_capacity *= 2;
        t->tokens = realloc(t->tokens, sizeof(Token) * t->token_capacity);
    }
    
    Token* token = &t->tokens[t->token_count++];
    token->type = type;
    token->value = malloc(length + 1);
    strncpy(token->value, value, length);
    token->value[length] = '\0';
    token->line = t->line;
    token->column = t->column;
    token->length = length;
    
    // Check if identifier is actually a keyword
    if (type == TOKEN_IDENTIFIER && is_keyword(token->value)) {
        token->type = TOKEN_KEYWORD;
    }
}

// Tokenize input
void tokenize(Tokenizer* t) {
    while (t->position < t->length) {
        char c = t->input[t->position];
        
        // Skip whitespace
        if (isspace(c)) {
            if (c == '\n') {
                t->line++;
                t->column = 1;
            } else {
                t->column++;
            }
            t->position++;
            continue;
        }
        
        // Skip comments
        if (c == '/' && t->position + 1 < t->length && t->input[t->position + 1] == '/') {
            while (t->position < t->length && t->input[t->position] != '\n') {
                t->position++;
            }
            continue;
        }
        
        // Match tokens
        size_t start = t->position;
        int start_column = t->column;
        
        if (matches_pattern(t->input, t->position, IDENTIFIER_PATTERN)) {
            while (t->position < t->length && 
                   (isalnum(t->input[t->position]) || t->input[t->position] == '_')) {
                t->position++;
                t->column++;
            }
            add_token(t, TOKEN_IDENTIFIER, &t->input[start], t->position - start);
        }
        else if (matches_pattern(t->input, t->position, NUMBER_PATTERN)) {
            while (t->position < t->length && 
                   (isdigit(t->input[t->position]) || t->input[t->position] == '.')) {
                t->position++;
                t->column++;
            }
            add_token(t, TOKEN_NUMBER, &t->input[start], t->position - start);
        }
        else if (matches_pattern(t->input, t->position, STRING_PATTERN)) {
            t->position++; // Skip opening quote
            t->column++;
            while (t->position < t->length && t->input[t->position] != '"') {
                if (t->input[t->position] == '\\') {
                    t->position++; // Skip escape sequence
                    t->column++;
                }
                t->position++;
                t->column++;
            }
            t->position++; // Skip closing quote
            t->column++;
            add_token(t, TOKEN_STRING, &t->input[start], t->position - start);
        }
        else if (matches_pattern(t->input, t->position, OPERATOR_PATTERN)) {
            while (t->position < t->length && 
                   strchr("+-*/=<>!&|", t->input[t->position])) {
                t->position++;
                t->column++;
            }
            add_token(t, TOKEN_OPERATOR, &t->input[start], t->position - start);
        }
        else if (matches_pattern(t->input, t->position, DELIMITER_PATTERN)) {
            t->position++;
            t->column++;
            add_token(t, TOKEN_DELIMITER, &t->input[start], 1);
        }
        else {
            // Unknown character
            t->position++;
            t->column++;
            add_token(t, TOKEN_ERROR, &t->input[start], 1);
        }
    }
    
    // Add EOF token
    add_token(t, TOKEN_EOF, "", 0);
}

// Parser structures
typedef enum {
    AST_PROGRAM,
    AST_FUNCTION,
    AST_VARIABLE,
    AST_EXPRESSION,
    AST_STATEMENT,
    AST_IDENTIFIER,
    AST_NUMBER,
    AST_STRING,
    AST_BINARY_OP
} ASTNodeType;

typedef struct ASTNode {
    ASTNodeType type;
    char* value;
    struct ASTNode** children;
    size_t child_count;
    size_t child_capacity;
} ASTNode;

typedef struct {
    Tokenizer* tokenizer;
    size_t current;
} Parser;

// Create AST node
ASTNode* create_ast_node(ASTNodeType type, const char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = type;
    if (value) {
        node->value = strdup(value);
    } else {
        node->value = NULL;
    }
    node->child_capacity = 4;
    node->children = malloc(sizeof(ASTNode*) * node->child_capacity);
    node->child_count = 0;
    return node;
}

// Add child to AST node
void add_child(ASTNode* parent, ASTNode* child) {
    if (parent->child_count >= parent->child_capacity) {
        parent->child_capacity *= 2;
        parent->children = realloc(parent->children, 
                                 sizeof(ASTNode*) * parent->child_capacity);
    }
    parent->children[parent->child_count++] = child;
}

// Parser functions
Parser* parser_create(Tokenizer* tokenizer) {
    Parser* p = malloc(sizeof(Parser));
    p->tokenizer = tokenizer;
    p->current = 0;
    return p;
}

Token* current_token(Parser* p) {
    if (p->current < p->tokenizer->token_count) {
        return &p->tokenizer->tokens[p->current];
    }
    return NULL;
}

bool match_token(Parser* p, TokenType type) {
    Token* tok = current_token(p);
    return tok && tok->type == type;
}

bool match_keyword(Parser* p, const char* keyword) {
    Token* tok = current_token(p);
    return tok && tok->type == TOKEN_KEYWORD && strcmp(tok->value, keyword) == 0;
}

void advance(Parser* p) {
    if (p->current < p->tokenizer->token_count) {
        p->current++;
    }
}

// Parse expression
ASTNode* parse_expression(Parser* p) {
    Token* tok = current_token(p);
    if (!tok) return NULL;
    
    ASTNode* left = NULL;
    
    if (tok->type == TOKEN_NUMBER) {
        left = create_ast_node(AST_NUMBER, tok->value);
        advance(p);
    } else if (tok->type == TOKEN_STRING) {
        left = create_ast_node(AST_STRING, tok->value);
        advance(p);
    } else if (tok->type == TOKEN_IDENTIFIER) {
        left = create_ast_node(AST_IDENTIFIER, tok->value);
        advance(p);
    }
    
    // Check for binary operator
    tok = current_token(p);
    if (tok && tok->type == TOKEN_OPERATOR) {
        ASTNode* op = create_ast_node(AST_BINARY_OP, tok->value);
        add_child(op, left);
        advance(p);
        
        ASTNode* right = parse_expression(p);
        if (right) {
            add_child(op, right);
        }
        return op;
    }
    
    return left;
}

// Parse variable declaration
ASTNode* parse_variable(Parser* p) {
    if (!match_keyword(p, "var")) return NULL;
    advance(p);
    
    Token* name = current_token(p);
    if (!name || name->type != TOKEN_IDENTIFIER) return NULL;
    
    ASTNode* var = create_ast_node(AST_VARIABLE, name->value);
    advance(p);
    
    // Check for assignment
    if (match_token(p, TOKEN_OPERATOR) && strcmp(current_token(p)->value, "=") == 0) {
        advance(p);
        ASTNode* expr = parse_expression(p);
        if (expr) {
            add_child(var, expr);
        }
    }
    
    return var;
}

// Parse program
ASTNode* parse_program(Parser* p) {
    ASTNode* program = create_ast_node(AST_PROGRAM, "main");
    
    while (current_token(p) && current_token(p)->type != TOKEN_EOF) {
        ASTNode* stmt = NULL;
        
        if (match_keyword(p, "var")) {
            stmt = parse_variable(p);
        } else {
            // Skip unknown tokens
            advance(p);
        }
        
        if (stmt) {
            add_child(program, stmt);
        }
    }
    
    return program;
}

// Print AST (for debugging)
void print_ast(ASTNode* node, int depth) {
    for (int i = 0; i < depth; i++) printf("  ");
    
    const char* type_names[] = {
        "PROGRAM", "FUNCTION", "VARIABLE", "EXPRESSION", 
        "STATEMENT", "IDENTIFIER", "NUMBER", "STRING", "BINARY_OP"
    };
    
    printf("%s", type_names[node->type]);
    if (node->value) {
        printf(": %s", node->value);
    }
    printf("\n");
    
    for (size_t i = 0; i < node->child_count; i++) {
        print_ast(node->children[i], depth + 1);
    }
}

// Main function
int main() {
    const char* code = R"(
var x = 42
var y = x + 10
var message = "Hello, Gosilang!"
)";
    
    printf("Input code:\n%s\n", code);
    
    // Tokenize
    Tokenizer* tokenizer = tokenizer_create(code);
    tokenize(tokenizer);
    
    printf("\nTokens:\n");
    for (size_t i = 0; i < tokenizer->token_count; i++) {
        Token* t = &tokenizer->tokens[i];
        const char* type_names[] = {
            "IDENTIFIER", "NUMBER", "STRING", "OPERATOR", 
            "KEYWORD", "DELIMITER", "EOF", "ERROR"
        };
        printf("%s: %s (line %d, col %d)\n", 
               type_names[t->type], t->value, t->line, t->column);
    }
    
    // Parse
    Parser* parser = parser_create(tokenizer);
    ASTNode* ast = parse_program(parser);
    
    printf("\nAST:\n");
    print_ast(ast, 0);
    
    return 0;
}
