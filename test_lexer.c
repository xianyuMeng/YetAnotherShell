#include "lexer.h"

void printTokens(const char* source) {
    Token* tokens = lex(source);
    Token* curr = tokens;
    while (curr != NULL) {
        switch (curr->kind) {
            case WORD: printf("WORD(%s) ", curr->data.word); break;
            case LT: printf("LT "); break;
            case RT: printf("RT "); break;
            case PIPE: printf("PIPE "); break;
            case BACKGROUND: printf("BACKGROUND "); break;
            default: printf("unknown %d", curr->kind); break;
        }
        curr = curr->next;
    }
    delete_tokens(tokens);
    printf("\n");
}

int main() {
    printTokens("| > <");
    printTokens(" & & > <");
    printTokens("<");
    printTokens("< < <   > <");
    printTokens("ls");
    printTokens("ls cd chmod");
    return 0;
}