#include <stdlib.h>
//#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

typedef enum {
    WORD,
    LT,         // <
    RT,         // >
    PARENTL,    // (
    PARENTR,    // )
    PIPE,       // |
    BACKGROUND, // &
    BLANK       // (' ' | \n | \t)+
} T_Kind;

typedef struct Token {
    T_Kind kind;
    union {
        const char* word;
    } data;
    struct Token* next;
} Token;

void delete_token(Token* token);
void delete_tokens(Token* token);

Token* lex(const char* source);