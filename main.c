#include "IO.h"
#include "ush.h"

int main(void) {

    while (1) {
        const char* source = fetch();
        if (run(source) == USH_EXIT) break;
    }
    return 0;
}