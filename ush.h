#include <unistd.h>
#include <sys/wait.h>


#define USH_EXIT 1
#define USH_CONTINUE 0

int run(const char* source);
int is_path(const char* file);

void simple_ls(size_t n, char** words);
void simple_cd(size_t n, char** words);
void simple_pwd(size_t n, char** words);