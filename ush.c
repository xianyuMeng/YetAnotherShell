#include "parser.h"
#include "ush.h"

static const char* PATH[] = {
    "/usr/bin"
};

struct Builtin{
    const char* cmd;
    void (*fun)(size_t, char*[]);
};

typedef struct Builtin Builtin;

static Builtin BUILT_IN[] = {
    {
        .cmd = "ls",
        .fun = simple_ls
        /********
        函数名在运算的时候会转换成函数指针；对于取地址运算符，函数名不会转换成函数指针，以下写法仍然正确
        .fun = &simple_ls
        *******/
    },
    {
        .cmd = "cd",
        .fun = simple_cd
    },
    {
        .cmd = "pwd",
        .fun = simple_pwd
    },
    {
        .cmd = "wc",
        .fun = simple_wc
    }
};

int is_built_in(const char* cmd){
    for(size_t i = 0; i != sizeof(BUILT_IN) / sizeof(Builtin); ++i){
        if(!strcmp(BUILT_IN[i].cmd, cmd)){
            return 1;
        }
    }
    return 0;
}

void run_built_in(const SimpleCmd* cmd){
    for(size_t i = 0; i < sizeof(BUILT_IN) / sizeof(Builtin); ++i){
        if(!strcmp(BUILT_IN[i].cmd, cmd->words[0])){
            BUILT_IN[i].fun(cmd->n, cmd->words);
            break;
        }
    }
}

// If the string contains a /, then this is a path.
int is_path(const char* file) {
    for (size_t i = 0; i < strlen(file); ++i) {
        if (file[i] == '/') return 1;
    }
    return 0;
}

// Simply call execv.
static void run_simple_cmd(const SimpleCmd* cmd) {

    // Check if this is a path.
    if (is_path(cmd->words[0])) {
        execv(cmd->words[0], cmd->words);
    } 
    else{ 
        if(is_built_in(cmd->words[0])){
            run_built_in(cmd);
        }
    }
}

static void run_redir_cmd(const RedirCmd* cmd) {

    //copy stdin stdout
    int stdin_cpy, stdout_cpy;
    int inf, ouf;
    if(cmd->lhs){
        if((inf = open(cmd->lhs, O_RDONLY)) < 0){
            fprintf(stderr, "the file name is %s, the fd is %d\n", cmd->lhs, inf);
            fprintf(stderr, "the value of lhs is %p\n", cmd->lhs);
            fprintf(stderr, "cannot open inf %s\n", strerror(errno));
            exit(-1);
        }
        stdin_cpy = dup(0);
        dup2(inf, 0);
    }
    if(cmd->rhs){
        if((ouf = open(cmd->rhs, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0){
            fprintf(stderr, "cannot open file , %s\n", strerror(errno));
            fprintf(stderr, "the file name is %s, the fd is %d\n", cmd->rhs, ouf);
            exit(-1);
        }
        stdout_cpy = dup(1);
        dup2(ouf, 1);
    }
    run_simple_cmd(cmd->simple);
    //close file descriptor
    //recover stdin stdout
    if(cmd->lhs){
        dup2(stdin_cpy, 0);
        close(stdin_cpy);
        close(inf);
    }
    if(cmd->rhs){
        dup2(stdout_cpy, 1);
        close(stdout_cpy);
        close(ouf);
    }
}



static void run_pipe_cmd(const PipeCmd* cmd) {
    printf("pipe\n");
    if(cmd->next != NULL){
        if(cmd->redir->rhs != NULL || cmd->next->redir->lhs != NULL){
            pid_t t = fork();
            if(t < 0){
                fprintf(stderr, "failed to fork, %s\n", strerror(errno));
            }
            if(t == 0){
                run_pipe_cmd(cmd->next);
            }
            else {
                run_redir_cmd(cmd->redir);
            }
        }
        else{
            int pfds[2];
            pipe(pfds);
            pid_t pid = fork();
            if(pid < 0){
                fprintf(stderr, "failed to fork, %s\n", strerror(errno));
            }
            else if(pid == 0){
                dup2(pfds[0], fileno(stdin));
                close(pfds[0]);
                run_pipe_cmd(cmd->next);
            }
            else{
                dup2(pfds[1], fileno(stdout));
                close(pfds[1]);
                run_redir_cmd(cmd->redir);
            }
        }
    }
    else{
        run_redir_cmd(cmd->redir);
    }
}

int run(const char* source) {
    Token* tokens = lex(source);
    Cmd* cmd = parse(tokens);
    delete_tokens(tokens);
    if (cmd == NULL) {
        // Failed parsing.
        printf("Failed: %s\n", source);
        return USH_CONTINUE;
    } else {
        print_cmd(cmd);
        printf("\n");
        if(is_built_in(cmd->redir->simple->words[0])){
            printf("haha\n");
            run_pipe_cmd(cmd);
        }
        else{
            pid_t pid;
            if((pid = fork()) < 0){
                fprintf(stderr, "%s\n", strerror(errno));
                exit(-1);
            }
            else{
                if(pid == 0){
                    //child
                    run_pipe_cmd(cmd);
                }
                else{
                    //parent
                    while (1) {
                        int status;
                        pid_t end = waitpid(pid, &status, WUNTRACED | WCONTINUED);
                        if (end == -1) {
                            perror("Failed waiting for child");
                            exit(-1);
                        }

                        if (WIFEXITED(status)) {
                            printf("exited, status = %d\n", WEXITSTATUS(status));
                        } else if (WIFSIGNALED(status)) {
                            printf("killed by signal %d\n", WTERMSIG(status));
                        } else if (WIFSTOPPED(status)) {
                            printf("stopped by signal %d\n", WSTOPSIG(status));
                        } else if (WIFCONTINUED(status)) {
                            printf("continued\n");
                        }

                        if (WIFEXITED(status) || WIFSIGNALED(status)) {
                            break;
                        }
                    }
                }
            }
        }
        delete_cmd(cmd);
        return USH_CONTINUE;
    }
}



