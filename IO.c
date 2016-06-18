#include <stdio.h>
#include <stdlib.h>
#include "IO.h"

#define BUFLEN 1024
static char buffer[BUFLEN];
static const char* PROMPT = "咩~咩 > ";

const char* fetch() {

    enum {
        NORMAL
    } state = NORMAL;

    size_t n = 0;
    printf("%s", PROMPT);
    while (n < BUFLEN - 1) {
        int c = fgetc(stdin);
        if (c == EOF) {
            perror("Failed getting char");
            exit(-1);
        }
        switch (state) {
            case NORMAL:
                if (c == '\n') {
                    buffer[n] = 0;
                    return buffer;
                } else {
                    buffer[n++] = (char)c;
                }
                break;
            default:
                perror("Fetch: unknown internal state");
                exit(-1);
        }
    }
    perror("Such a long command is a bad style ><");
    exit(-1);
}