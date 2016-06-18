#include "stdio.h"
#include "lexer.h"


/// Grammar for this simple unix shell.
/// simple-cmd
///     : word-list
///     ;
/// 	ls 
///		cd
/// redir-cmd
///     : simple-cmd RT WORD LT WORD
///     | simple-cmd LT WORD RT WORD
///     | simple-cmd LT WORD
///     | simple-cmd RT WORD
///     | simple-cmd
///     ;
///
/// pipe-cmd
///     : redir-cmd
///     | redir-cmd PIPE pipe-cmd
///     ;
///
///
///
///
///
///
///

typedef struct {
    size_t n;
    char** words;
} SimpleCmd;

void print_simple_cmd(const SimpleCmd* cmd);

typedef struct {
    SimpleCmd* simple;
    char* lhs;
    char* rhs;
} RedirCmd;

void print_redir_cmd(const RedirCmd* cmd);

typedef struct PipeCmd {
    RedirCmd* redir;
    struct PipeCmd* next;
} PipeCmd;

void print_pipe_cmd(const PipeCmd* cmd);

typedef PipeCmd Cmd;

Cmd* parse(const Token* tokens);
void print_cmd(const Cmd* cmd);
void delete_cmd(Cmd* cmd);