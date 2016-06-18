#include "lexer.h"

static Token* make_token(T_Kind kind, const char* source, int len) {
    Token* token = malloc(sizeof(Token));
    token->kind = kind;
    token->next = NULL;
    if (kind == WORD) {
        // Copy the string.
        char* word = malloc((len + 1) * sizeof(char));
        memcpy(word, source, len);
        word[len] = '\0';
        token->data.word = word;
    }
    return token;
}

void delete_token(Token* token) {
    if (token->kind == WORD) {
        free((void*)token->data.word);
    }
    free(token);
}

void delete_tokens(Token* tokens) {
    while (tokens != NULL) {
        Token* prev = tokens;
        tokens = tokens->next;
        delete_token(prev);
    }
}

typedef enum {
    INITIAL,
    RUNNING,
    FAILED,
    SUCCEED
} Status;

static int is_char(char c) {
    return (c <= 'z' && c >= 'a') || (c <= 'Z' && c >= 'A') || (c <= '9' && c >= '0');
}

int is_not_metachar(char c);
inline int is_not_metachar(char c) {
    return c != '<' && c != '>' && c != '|' &&
        c != '&' && c != '\'' && c != '"' && c != '$' &&
        c != ';' &&
        c != ' ' && c != '\n' && c != '\t' && c != 0;
}

// Finite State Machine:
//              none-metachar       metachar
// _INITIAL     _RUNNING            _FAILED
// _RUNNING     _RUNNING            _SUCCEED
// _SUCCEED     _FAILED             _FAILED
// _FAILED      _FAILED             _FAILED
static Status word(int reset, char c, T_Kind* kind) {
    static enum {
        _INITIAL,
        _FAILED,
        _SUCCEED,
        _RUNNING
    } status = _INITIAL;
    if (reset) {
        status = _INITIAL;
        return INITIAL;
    }
    switch (status) {
        case _INITIAL:
            if (is_not_metachar(c)) {
                status = _RUNNING;
                return RUNNING;
            } else {
                status = _FAILED;
                return FAILED;
            }
        case _RUNNING:
            if (is_not_metachar(c)) {
                return RUNNING;
            } else {
                status = _SUCCEED;
                *kind = WORD;
                return SUCCEED;
            }
        case _FAILED:
            return FAILED;
        case _SUCCEED:
            status = _FAILED;
            return FAILED;
        default:
            perror("Unknown internal status ><!");
            exit(-1);
    }
}

// Finite State Machine:
//              <                   other
// _INITIAL     _RUNNING            _FAILED
// _RUNNING     _SUCCEED            _SUCCEED
// _SUCCEED     _FAILED             _FAILED
// _FAILED      _FAILED             _FAILED
static Status lt(int reset, char c, T_Kind* kind) {
    static enum {
        _INITIAL,
        _FAILED,
        _SUCCEED,
        _RUNNING
    } status = _INITIAL;
    if (reset) {
        status = _INITIAL;
        return INITIAL;
    }
    switch (status) {
        case _INITIAL:
            if (c == '<') {
                status = _RUNNING;
                return RUNNING;
            } else {
                status = _FAILED;
                return FAILED;
            }
        case _RUNNING:
            status = _SUCCEED;
            *kind = LT;
            return SUCCEED;
        case _FAILED:
            return FAILED;
        case _SUCCEED:
            status = _FAILED;
            return FAILED;
        default:
            perror("unknown internal status ><!");
            exit(-1);
    }
}

// Finite State Machine:
//              >                   other
// _INITIAL     _RUNNING            _FAILED
// _RUNNING     _SUCCEED            _SUCCEED
// _SUCCEED     _FAILED             _FAILED
// _FAILED      _FAILED             _FAILED
static Status rt(int reset, char c, T_Kind* kind) {
    static enum {
        _INITIAL,
        _FAILED,
        _SUCCEED,
        _RUNNING
    } status = _INITIAL;
    if (reset) {
        status = _INITIAL;
        return INITIAL;
    }
    switch (status) {
        case _INITIAL:
            if (c == '>') {
                status = _RUNNING;
                return RUNNING;
            } else {
                status = _FAILED;
                return FAILED;
            }
        case _RUNNING:
            status = _SUCCEED;
            *kind = RT;
            return SUCCEED;
        case _FAILED:
            return FAILED;
        case _SUCCEED:
            status = _FAILED;
            return FAILED;
        default:
            perror("unknown internal status ><!");
            exit(-1);
    }
}
// Finite State Machine:
//              |                  other
// _INITIAL     _RUNNING            _FAILED
// _RUNNING     _SUCCEED            _SUCCEED
// _SUCCEED     _FAILED             _FAILED
// _FAILED      _FAILED             _FAILED
static Status pipe(int reset, char c, T_Kind* kind) {
    static enum {
        _INITIAL,
        _FAILED,
        _SUCCEED,
        _RUNNING
    } status = _INITIAL;
    if (reset) {
        status = _INITIAL;
        return INITIAL;
    }
    switch (status) {
        case _INITIAL:
            if (c == '|') {
                status = _RUNNING;
                return RUNNING;
            } else {
                status = _FAILED;
                return FAILED;
            }
        case _RUNNING:
            status = _SUCCEED;
            *kind = PIPE;
            return SUCCEED;
        case _FAILED:
            return FAILED;
        case _SUCCEED:
            status = _FAILED;
            return FAILED;
        default:
            perror("unknown internal status ><!");
            exit(-1);
    }
}

static int is_blank(char c) {
    return c == ' ' || c == '\t' || c == '\n';
}
// Finite State Machine:
//              &                   other
// _INITIAL     _RUNNING            _FAILED
// _RUNNING     _SUCCEED            _SUCCEED
// _SUCCEED     _FAILED             _FAILED
// _FAILED      _FAILED             _FAILED
static Status background(int reset, char c, T_Kind* kind) {
    static enum {
        _INITIAL,
        _FAILED,
        _SUCCEED,
        _RUNNING
    } status = _INITIAL;
    if (reset) {
        status = _INITIAL;
        return INITIAL;
    }
    switch (status) {
        case _INITIAL:
            if (c == '&') {
                status = _RUNNING;
                return RUNNING;
            } else {
                status = _FAILED;
                return FAILED;
            }
        case _RUNNING:
            status = _SUCCEED;
            *kind = BACKGROUND;
            return SUCCEED;
        case _FAILED:
            return FAILED;
        case _SUCCEED:
            status = _FAILED;
            return FAILED;
        default:
            perror("unknown internal status ><!");
            exit(-1);
    }
}


// Finite State Machine:
//              is-blank            other
// _INITIAL     _RUNNING            _FAILED
// _RUNNING     _RUNNING            _SUCCEED
// _SUCCEED     _FAILED             _FAILED
// _FAILED      _FAILED             _FAILED
static Status blank(int reset, char c, T_Kind* kind) {
    static enum {
        _INITIAL,
        _FAILED,
        _SUCCEED,
        _RUNNING
    } status = _INITIAL;
    if (reset) {
        status = _INITIAL;
        return INITIAL;
    }
    switch (status) {
        case _INITIAL:
            if (is_blank(c)) {
                status = _RUNNING;
                return RUNNING;
            } else {
                status = _FAILED;
                return FAILED;
            }
        case _RUNNING:
            if (!is_blank(c)) {
                status = _SUCCEED;
                *kind = BLANK;
                return SUCCEED;
            } else {
                return RUNNING;
            }
        case _FAILED:
            return FAILED;
        case _SUCCEED:
            status = _FAILED;
            return FAILED;
        default:
            perror("unknown internal status ><!");
            exit(-1);
    }
}

// Bind all the lexer together.
static Status bind(int reset, char c, T_Kind* kind) {

    static Status (*lexers[])(int, char, T_Kind*) = {
        word,   // word
        lt,     // <
        rt,     // >
        blank,
        pipe,
        background
    };
    static const int N_LEXER = sizeof(lexers) / sizeof(lexers[0]);
    static T_Kind kinds[N_LEXER];
    static Status ss[N_LEXER];

    if (reset) {
        for (int i = 0; i < N_LEXER; ++i) {
            lexers[i](1, 0, NULL);
        }
        return INITIAL;
    }
    for (int i = 0; i < N_LEXER; ++i) {
        ss[i] = lexers[i](0, c, kinds + i);
    }
    // Check the status of every lexer.
    for (int i = 0; i < N_LEXER; ++i) {
        if (ss[i] == RUNNING) return RUNNING;
    }
    for (int i = 0; i < N_LEXER; ++i) {
        if (ss[i] == SUCCEED) {
            *kind = kinds[i];
            return SUCCEED;
        }
    }
    return FAILED;
}

Token* lex(const char* source) {

    int len = strlen(source);

    // Reset all the lexers.
    bind(1, 0, NULL);

    // For the lexers.
    T_Kind kind;
    Status status;

    // Token list;
    Token* head = NULL;
    Token* last = NULL;

    int curr = 0;
    int prev = 0;

    while (curr < len) {

        // Feed the char to the lexers.
        status = bind(0, source[curr], &kind);

        switch (status) {
            case RUNNING:
                curr++;
                break;
            case SUCCEED:
                // Get the recognized token.
                if (kind != BLANK) {
                    Token* token = make_token(kind, source + prev, curr - prev);
                    if (head == NULL) {
                        head = token;
                        last = token;
                    } else {
                        last->next = token;
                        last = token;
                    }
                }
                // Reset the lexers.
                bind(1, 0, NULL);
                // Set the prev to curr.
                prev = curr;
                // Notice that in this case we do not increase curr.
                break;
            case FAILED:
                // Unknown token.
                perror("Unknown token ><!");
                exit(-1);
            default:
                perror("Illegal status ><!");
                exit(-2);
        }
    }

    // Remember to feed the EOF char to the lexers.
    if (status == RUNNING) {
        status = bind(0, 0, &kind);
        if (status == SUCCEED) {
            if (kind != BLANK) {
                Token* token = make_token(kind, source + prev, curr - prev);
                if (head == NULL) {
                    head = token;
                    last = token;
                } else {
                    last->next = token;
                    last = token;
                }
            }
            // Reset the lexers.
            bind(1, 0, NULL);
        }
    }

    return head;
}
