/* lexer.c */
#include "token.h"
#include <ctype.h>

TokenList global_tokens;
static Position cur = {1, 1, 0};

static void advance(const char **src, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        if ((*src)[i] == '\n') { cur.line++; cur.column = 1; }
        else cur.column++;
        cur.offset++;
    }
    *src += n;
}

/* helpers */
static Token make(TokenType t, const char *start, const char *end) {
    Position p = cur;
    size_t len = end - start;
    char *lex = malloc(len + 1); memcpy(lex, start, len); lex[len] = 0;
    advance(&start, len);
    return (Token){ .type = t, .lexeme = lex, .pos = p };
}

static Token make_int(const char *start, const char *end) {
    Token t = make(TOKEN_INTEGER, start, end);
    t.value.int_val = atoi(t.lexeme);
    return t;
}
static Token make_float(const char *start, const char *end) {
    Token t = make(TOKEN_FLOAT, start, end);
    t.value.float_val = atof(t.lexeme);
    return t;
}

/* main lexer */
int lex_and_store(const char *input) {
    token_list_init(&global_tokens);
    const char *p = input;

    while (*p) {
        const char *start = p;

        /* skip whitespace (but not newline) */
        while (isspace(*p) && *p != '\n') { p++; cur.column++; cur.offset++; }
        if (*p == '\0') break;

        /* newline */
        if (*p == '\n') {
            token_list_add(&global_tokens,
                           make(TOKEN_NEWLINE, p, p+1));
            p++; continue;
        }

        /* identifiers & keywords */
        if (isalpha(*p) || *p == '_') {
            while (isalnum(*p) || *p == '_') p++;
            Token t = make(TOKEN_IDENTIFIER, start, p);

            /* keyword table */
            struct { const char *kw; TokenType tt; } kwtab[] = {
                {"#def", TOKEN_DEF}, {"#bind", TOKEN_BIND}, {"#unbind", TOKEN_UNBIND},
                {"span", TOKEN_SPAN}, {"range", TOKEN_RANGE}, {"vec", TOKEN_VEC},
                {"nil", TOKEN_NIL}, {"null", TOKEN_NULL}, {"let", TOKEN_LET}
            };
            for (size_t i = 0; i < sizeof kwtab / sizeof kwtab[0]; ++i)
                if (!strcmp(t.lexeme, kwtab[i].kw)) { t.type = kwtab[i].tt; break; }
            token_list_add(&global_tokens, t);
            continue;
        }

        /* numbers */
        if (isdigit(*p)) {
            while (isdigit(*p)) p++;
            if (*p == '.') {
                p++;
                while (isdigit(*p)) p++;
                token_list_add(&global_tokens, make_float(start, p));
            } else {
                token_list_add(&global_tokens, make_int(start, p));
            }
            continue;
        }

        /* two-char operators */
        if (!strncmp(p, ":=", 2)) { token_list_add(&global_tokens, make(TOKEN_ASSIGN, p, p+2)); p+=2; continue; }
        if (!strncmp(p, "->", 2)) { token_list_add(&global_tokens, make(TOKEN_ARROW, p, p+2)); p+=2; continue; }
        if (!strncmp(p, "..", 2)) { token_list_add(&global_tokens, make(TOKEN_DOT_DOT, p, p+2)); p+=2; continue; }

        /* single-char tokens */
        switch (*p) {
            #define ONE(c,t) case c: token_list_add(&global_tokens, make(t, p, p+1)); p++; break
            ONE('!', TOKEN_BANG); ONE('#', TOKEN_HASH); ONE('=', TOKEN_EQUAL);
            ONE('(', TOKEN_LPAREN); ONE(')', TOKEN_RPAREN);
            ONE('<', TOKEN_LT); ONE('>', TOKEN_GT);
            ONE('[', TOKEN_LBRACKET); ONE(']', TOKEN_RBRACKET);
            ONE('{', TOKEN_LBRACE); ONE('}', TOKEN_RBRACE);
            ONE(',', TOKEN_COMMA); ONE(':', TOKEN_COLON); ONE(';', TOKEN_SEMICOLON);
            #undef ONE
            default:
                token_list_add(&global_tokens, make(TOKEN_UNKNOWN, p, p+1));
                p++;
        }
    }

    /* EOF */
    Token eof = { .type = TOKEN_EOF, .lexeme = strdup(""), .pos = cur };
    token_list_add(&global_tokens, eof);
    return (int)global_tokens.count;
}
