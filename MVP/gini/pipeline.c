/* pipeline.c */
#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- token utilities (same as in your original) ---------- */
const char* token_type_name(TokenType type) {
    switch(type) {
        #define C(x) case TOKEN_##x: return #x
        C(BANG); C(HASH); C(ASSIGN); C(EQUAL); C(ARROW);
        C(LPAREN); C(RPAREN); C(LT); C(GT);
        C(LBRACKET); C(RBRACKET); C(LBRACE); C(RBRACE);
        C(COMMA); C(COLON); C(SEMICOLON); C(DOT_DOT);
        C(DEF); C(BIND); C(UNBIND); C(SPAN); C(RANGE);
        C(VEC); C(NIL); C(NULL); C(LET);
        C(IDENTIFIER); C(INTEGER); C(FLOAT);
        C(NEWLINE); C(EOF); C(UNKNOWN);
        #undef C
        default: return "INVALID";
    }
}
Token create_token(TokenType t, const char *lex, Position p) {
    Token tok = { .type = t, .lexeme = lex?strdup(lex):strdup(""),
                  .pos = p, .value.int_val = 0 };
    return tok;
}
void token_list_init(TokenList *l){ l->capacity=32; l->tokens=malloc(sizeof(Token)*32); l->count=0; }
void token_list_add(TokenList *l, Token t){
    if(l->count==l->capacity){ l->capacity*=2; l->tokens=realloc(l->tokens,sizeof(Token)*l->capacity); }
    l->tokens[l->count++]=t;
}
void token_list_free(TokenList *l){
    for(size_t i=0;i<l->count;i++) free(l->tokens[i].lexeme);
    free(l->tokens); l->tokens=NULL; l->count=l->capacity=0;
}
void print_token_table(const Token *t){
    printf("| %-12s | %-15s | %4d:%-2d |\n",
           token_type_name(t->type), t->lexeme, t->pos.line, t->pos.column);
}

/* ---------- pipeline stages ---------- */
static void stage1_raw_lexemes(const char *filename){
    printf("\n=== STAGE 1: Raw Lexemes ===\n");
    FILE *f=fopen(filename,"r"); if(!f){perror("open");return;}
    printf("Raw file content:\n─────────────────\n");
    char buf[1024];
    while(fgets(buf,sizeof buf,f)) printf("%s",buf);
    printf("─────────────────\n");
    fclose(f);
}
static void stage2_token_stream(const char *filename){
    printf("\n=== STAGE 2: Token Stream ===\n");
    FILE *f=fopen(filename,"r"); if(!f){perror("open");return;}
    fseek(f,0,SEEK_END); long sz=ftell(f); rewind(f);
    char *src=malloc(sz+1); fread(src,1,sz,f); src[sz]='\0';
    fclose(f);

    int cnt=lex_and_store(src); free(src);
    printf("Generated %d tokens:\n\n",cnt);
    printf("Token Table:\n");
    printf("┌─────────────┬─────────────────┬─────────┐\n");
    printf("│ Token Type  │ Lexeme          │ Pos     │\n");
    printf("├─────────────┼─────────────────┼─────────┤\n");
    for(size_t i=0;i<global_tokens.count;i++) print_token_table(&global_tokens.tokens[i]);
    printf("└─────────────┴─────────────────┴─────────┘\n");
}
static void stage3_ast_preview(void){
    printf("\n=== STAGE 3: AST Preview ===\n");
    printf("(Parser will build AST nodes from token stream)\n\n");
    printf("Detected patterns:\n");
    for(size_t i=0;i<global_tokens.count;i++){
        Token *t=&global_tokens.tokens[i];
        if(t->type==TOKEN_BANG) printf("→ Invocation pattern starting at %d:%d\n",t->pos.line,t->pos.column);
        if(t->type==TOKEN_BIND||t->type==TOKEN_UNBIND) printf("→ Bind operation at %d:%d\n",t->pos.line,t->pos.column);
        if(t->type==TOKEN_VEC) printf("→ Vector construction at %d:%d\n",t->pos.line,t->pos.column);
    }
}
static void stage4_codegen_preview(void){
    printf("\n=== STAGE 4: Codegen Preview ===\n");
    printf("(Will generate C skeleton from AST)\n\n");
    printf("Expected C output patterns:\n");
    printf("• #bind() → parallel_diff() calls\n");
    printf("• !vec<N>() → vec_make() + norm() calls\n");
    printf("• span[..] → normalize_to_span() calls\n");
    printf("• NIL handling → NaN or NIL_PTR checks\n");
}

/* ---------- main ---------- */
int main(int argc,char **argv){
    if(argc<2){ fprintf(stderr,"Usage: %s <file.gs> [--tokens|--raw|--all]\n",argv[0]); return 1; }
    const char *file=argv[1];
    const char *mode=(argc>2)?argv[2]:"--all";

    printf("Gosilang MVP Lexer Pipeline\n");
    printf("Processing: %s\n",file);
    printf("OBINexus Computing - Services from the Heart <3\n");

    if(!strcmp(mode,"--all")){
        stage1_raw_lexemes(file);
        stage2_token_stream(file);
        stage3_ast_preview();
        stage4_codegen_preview();
    }else if(!strcmp(mode,"--tokens")) stage2_token_stream(file);
    else if(!strcmp(mode,"--raw")) stage1_raw_lexemes(file);

    token_list_free(&global_tokens);
    printf("\nPipeline complete - ready for Phase 2 (Parser)\n");
    printf("#hacc #noghosting #sorrynotsorry\n");
    return 0;
}
