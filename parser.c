#include "parser.h"

static char* token_cpy(const Token* t) {
    if (t->kind != WORD) {
        perror("internal error: non-word in token copy.");
        exit(-1);
    }
    char* dst = malloc(strlen(t->data.word) + sizeof(char));
    strcpy(dst, t->data.word);
    return dst;
}

/// This function consumes one specific kind of token
/// and return it.
static const Token* expect(const Token** ts, T_Kind kind) {
    if ((*ts) == NULL || (*ts)->kind != kind) {
        return NULL;
    } else {
        const Token* t = *ts;
        // Move the token stream to next one.
        *ts = (*ts)->next;
        return t;
    }
}

static SimpleCmd* make_simple_cmd(const Token* words, size_t n) {
    SimpleCmd* cmd = malloc(sizeof(SimpleCmd));
    cmd->words = malloc((n + 1) * sizeof(char*));
    cmd->n = n;
    const Token* t = words;
    for (size_t i = 0; i < n; ++i, t = t->next) {
        cmd->words[i] = token_cpy(t);
    }

    // Set the last pointer to NULL so that exec
    // can recognize.
    cmd->words[n] = NULL;
    return cmd;
}

static void delete_simple_cmd(SimpleCmd* cmd) {
    for (size_t i = 0; i < cmd->n; ++i) {
        free(cmd->words[i]);
    }
    free(cmd->words);
    free(cmd);
}

void print_simple_cmd(const SimpleCmd* cmd) {
    printf("SimpleCmd(%s", cmd->words[0]);
    for (size_t i = 1; i < cmd->n; ++i) {
        printf(" %s", cmd->words[i]);
    }
    printf(")");
}

/// Grammar:
/// simple-cmd
/// : word-list
/// ;
/// A simple command is just a none empty list of word.
static SimpleCmd* parse_simple_cmd(const Token** tokens) {
    const Token* start = *tokens;
    if ((*tokens)->kind == WORD) {
        // Parse succeed.
        size_t n = 0;
        while (*tokens != NULL && (*tokens)->kind == WORD) {
            n++;
            *tokens = (*tokens)->next;
        }
        return make_simple_cmd(start, n);
    } else {
        // Parse failed and returns NULL.
        return NULL;
    }
}

/// Notice that this function doesn't copy SimpleCmd,
/// so if the simple command is freed,
/// the pointer 'cmd' in redirection command will be indeterminate.
static RedirCmd* make_redir_cmd(SimpleCmd* simple, const Token* lt, const Token* rt) {
    RedirCmd* cmd = malloc(sizeof(RedirCmd));
    cmd->simple = simple;
    cmd->lhs = lt != NULL ? token_cpy(lt) : NULL;
    cmd->rhs = rt != NULL ? token_cpy(rt) : NULL;
    return cmd;
}


static void delete_redir_cmd(RedirCmd* cmd) {
    delete_simple_cmd(cmd->simple);
    if (cmd->lhs != NULL) free(cmd->lhs);
    if (cmd->rhs != NULL) free(cmd->rhs);
    free(cmd);
}

void print_redir_cmd(const RedirCmd* cmd) {
    printf("Redir(");
    print_simple_cmd(cmd->simple);
    if (cmd->lhs != NULL) {
        printf(" < %s", cmd->lhs);
    }
    if (cmd->rhs != NULL) {
        printf(" > %s", cmd->rhs);
    }
    printf(")");
}

/// This is our first 'real' recursive descent parser.
/// Grammar:
/// redir-cmd:
///     : simple-cmd RT WORD LT WORD
///     | simple-cmd LT WORD RT WORD
///     | simple-cmd LT WORD
///     | simple-cmd RT WORD
///     | simple-cmd
///     ;
/// We apply each expansion (or rule) until we find one legal and construct the RedirCmd structure.
/// This version is very inefficient, we will see how to improve it.
static RedirCmd* parse_redir_cmd_waste(const Token** tokens) {
    // Backup the token stream so that we can
    // start again when one rule failed.
    const Token* backup = *tokens;
    SimpleCmd* simple;
    const Token* lt;
    const Token* rt;

    // redir-cmd: simple-cmd RT WORD LT WORD;
    if ((simple = parse_simple_cmd(tokens)) != NULL &&
        (expect(tokens, RT)) != NULL &&
        (rt = expect(tokens, WORD)) != NULL &&
        (expect(tokens, LT)) != NULL &&
        (lt = expect(tokens, WORD)) != NULL) {
        return make_redir_cmd(simple, lt, rt);
    }

    // Failed the last rule.
    // Restore the backup.
    *tokens = backup;

    // redir-cmd: simple-cmd LT WORD RT WORD;
    if ((simple = parse_simple_cmd(tokens)) != NULL &&
        (expect(tokens, LT)) != NULL &&
        (lt = expect(tokens, WORD)) != NULL &&
        (expect(tokens, RT)) != NULL &&
        (rt = expect(tokens, WORD)) != NULL) {
        return make_redir_cmd(simple, lt, rt);
    }

    // Failed the last rule.
    // Restore the backup.
    *tokens = backup;

    // redir-cmd: simple-cmd LT WORD;
    if ((simple = parse_simple_cmd(tokens)) != NULL &&
        (expect(tokens, LT)) != NULL &&
        (lt = expect(tokens, WORD)) != NULL) {
        return make_redir_cmd(simple, lt, NULL);
    }

    // Failed the last rule.
    // Restore the backup.
    *tokens = backup;

    // redir-cmd: simple-cmd RT WORD;
    if ((simple = parse_simple_cmd(tokens)) != NULL &&
        (expect(tokens, RT)) != NULL &&
        (rt = expect(tokens, WORD)) != NULL) {
        return make_redir_cmd(simple, NULL, rt);
    }

    *tokens = backup;
    // redir-cmd: simple-cmd;
    if ((simple = parse_simple_cmd(tokens)) != NULL) {
        return make_redir_cmd(simple, NULL, NULL);
    }

    // Failed all rules, return NULL.
    return NULL;
}

/// However, 'parse_redir_cmd_waste' is really inefficient:
/// it calls 'parse_simple_cmd' five times!
/// 
/// This can be improved by factoring the common part and rewrite the rule as
/// redir-cmd
///     : simple-cmd redir-cmd-tail
///     ;
/// 
/// redir-cmd-tail
///     : LT WORD RT WORD
///     | RT WORD LT WORD
///     | LT WORD
///     | RT WORD
///     | epsilon
///     ;
static RedirCmd* parse_redir_cmd_better(const Token** tokens) {
    SimpleCmd* simple;
    const Token* lt;
    const Token* rt;

    if ((simple = parse_simple_cmd(tokens)) == NULL) {
        return NULL;
    }

    const Token* backup = *tokens;
    if ((expect(tokens, RT)) != NULL &&
        (rt = expect(tokens, WORD)) != NULL) {
        if ((expect(tokens, LT)) != NULL &&
            (lt = expect(tokens, WORD)) != NULL) {
            return make_redir_cmd(simple, lt, rt);
        } else {
            return make_redir_cmd(simple, NULL, rt);
        }
    }

    *tokens = backup;
    if ((expect(tokens, LT)) != NULL &&
        (lt = expect(tokens, WORD)) != NULL) {
        if ((expect(tokens, RT)) != NULL &&
            (rt = expect(tokens, WORD)) != NULL) {
            return make_redir_cmd(simple, lt, rt);
        } else {
            return make_redir_cmd(simple, lt, NULL);            
        }
    } else {
        return make_redir_cmd(simple, NULL, NULL);
    }
    
}

/// Therefore we will use the better version.
static RedirCmd* parse_redir_cmd(const Token** tokens) {
    return parse_redir_cmd_better(tokens);
}

static PipeCmd* make_pipe_cmd(RedirCmd* redir) {
    PipeCmd* cmd = malloc(sizeof(PipeCmd));
    cmd->redir = redir;
    cmd->next = NULL;
    return cmd;
}

static void delete_pipe_cmd(PipeCmd* cmd) {
    while (cmd != NULL) {
        PipeCmd* next = cmd->next;
        delete_redir_cmd(cmd->redir);
        free(cmd);
        cmd = next;
    }
}

void print_pipe_cmd(const PipeCmd* cmd) {
    printf("Pipe(");
    print_redir_cmd(cmd->redir);
    for (PipeCmd* iter = cmd->next; iter != NULL; iter = iter->next) {
        printf(" | ");
        print_redir_cmd(iter->redir);
    }
    printf(")");
}

/// TODO: RECURSIVE DESCENT PARSING PIPE COMMAND USING GRAMMAR:
/// pipe-cmd
///     : redir-cmd PIPE pipe-cmd
///     | redir-cmd
///     ;
/// 
/// FOR NOW, PIPE-CMD IS JUST REDIR-CMD.
static PipeCmd* parse_pipe_cmd(const Token** tokens) {
    
    //const Token* pipe;
    PipeCmd* pipe;
    RedirCmd* redir = parse_redir_cmd(tokens);
    if (redir == NULL) {
        return NULL;
    }
    else{
        const Token* backup = *tokens;
        if(expect(tokens, PIPE) != NULL && (pipe = parse_pipe_cmd(tokens)) != NULL) {
            PipeCmd* head = make_pipe_cmd(redir);
            head->next = pipe;
            return head;
        }
        else{
            *tokens = backup;
            return make_pipe_cmd(redir);
        }
    }
}


Cmd* parse(const Token* tokens) {
    // For now support simple command only.
    Cmd* cmd = parse_pipe_cmd(&tokens);

    if (cmd == NULL) {
        perror("Failed parsing: unknown expansion.");
        return NULL;
    }

    // Make sure that there are no remain tokens.
    if (tokens != NULL) {
        perror("Failed parsing: there are remain tokens.");
        delete_cmd(cmd);
        return NULL;
    }

    return cmd;
}

void delete_cmd(Cmd* cmd) {
    delete_pipe_cmd(cmd);
}

void print_cmd(const Cmd* cmd) {
    print_pipe_cmd(cmd);
}
