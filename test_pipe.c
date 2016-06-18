#include <stdio.h>

int main() {
    char buffer[1024];
    printf("test_pipe\n");
    while (fgets(buffer, 1024, stdin)) {
        printf("pipe %s", buffer);
    }
    printf("finished\n");
    return 0;
}