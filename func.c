#include "ush.h"
#include "parser.h"

void simple_ls(size_t n, char** words){
	char cwd[1024];
    if(getcwd(cwd, 1024) == NULL){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(-1);
    }
    else{
        DIR* dir;
        struct dirent* Dirent;
        if((dir = opendir(cwd)) == NULL){
            fprintf(stderr, "%s\n", strerror(errno));
            exit(-1);
        }
        else{
            while((Dirent = readdir(dir)) != NULL){
                fprintf(stdout, "%s\n", Dirent->d_name);
            }
        }
    }
}

void simple_pwd(size_t n, char** words){
	char cwd[1024];
    if(getcwd(cwd, 1024) == NULL){
        fprintf(stderr, "%s\n", strerror(errno));
        exit(-1);
    }
    else{
        fprintf(stdout, "%s\n", cwd);
    }
}

void simple_cd(size_t n, char** words){
	char cwd[1024];
	if(getcwd(cwd, 1024) == NULL){
		fprintf(stderr, "%s\n", strerror(errno));
		exit(-1);
	}
	else{
		if(is_path(words[1])){
			if(chdir(words[1]) == -1){
				fprintf(stderr, "%s\n", strerror(errno));
				exit(-1);
			}
		}
		else{
			strcat(cwd, "/");
			strcat(cwd, words[1]);
			fprintf(stdout, "the arg is %s\n", cwd);
			if(chdir(cwd) == -1){
				fprintf(stderr, "%s\n", strerror(errno));
				exit(-1);
			}
			//simple_pwd();
		}
	}
}
