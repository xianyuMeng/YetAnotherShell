#include "parser.h"

void driver(const char* source) {
    Token* tokens = lex(source);
    Cmd* cmd = parse(tokens);
    delete_tokens(tokens);
    if (cmd == NULL) {
        // Failed parsing.
        printf("Failed: %s\n", source);
    } else {
        print_cmd(cmd);
        printf("\n");
        delete_cmd(cmd);
    }
}

int main() {
    driver("ls cd whatever");
    driver("ls");
    driver("ls > out < in");
    driver("ls < in > out");
    driver("ls < in");

    // illegal test.
    driver("ls < in < in");
    driver("ls > <");
    return 0;
}