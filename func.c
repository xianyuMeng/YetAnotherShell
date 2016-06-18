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
	fprintf(stderr, "this is simple cmd");
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

void simple_wc(size_t n, char** argv){
	int lines = 1;
	int characters = 0;
	int words = 0;
	int ch = 0;
	FILE* in;
	int previousSpace = 0;
	if(argv[1] != NULL){
		in = fopen(argv[1], "r");
		if(in == NULL){
			fprintf(stderr, "failed to open file, %s\n", strerror(errno));
			exit(-1);
		}
	} else {
		in = stdin;
	}
	while(!feof(in)) {
		ch = fgetc(in);
		characters++;
		if (isspace(ch) && !previousSpace) {
			previousSpace = 1;
			words++;
		}
		else if(ch == '\n'){
			previousSpace = 0;
			words++;
			lines++;
		} else {
			previousSpace = 0;
		}
	}
	if(argv[1] != NULL) {
		fclose(in);
	}
	fprintf(stdout, "%d %d %d\n", lines, words, characters);
}
