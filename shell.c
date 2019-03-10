#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SHELL_READLINE_BUFFERSIZE 1024
#define SHELL_TOKEN_BUFFERSIZE 64
#define SHELL_TOKEN_DELIM " \t\r\n\a"
#define MALLOC_ERROR fprintf(stderr, "shell: memory reallocation error")
#define CD_ERROR fprintf(stderr, "shell: expected argument to \"cd\"\n")
#define DROP_ERROR perror("shell")


char *shell_readline(void) {
  int buffersize = SHELL_READLINE_BUFFERSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * buffersize);
  int c;

  if (!buffer) {
    MALLOC_ERROR; //fprintf(stderr, "shell: memory allocation error\n");
    exit(1);
  }

  while (1) {
    c = getchar();
    if (c == EOF || c == '\n') {
      buffer[position] = '\0';
      return buffer;
    }
    else
      buffer[position] = c;
    position++;

    if (position >= buffersize) {
      buffersize += SHELL_READLINE_BUFFERSIZE;
      buffer = realloc(buffer, buffersize);
      if (!buffer) {
        MALLOC_ERROR; //fprintf(stderr, "shell: memory reallocation error");
        exit(1);
      }
    }
  }
}

char **shell_splitline(char *line) {
  int buffersize = SHELL_TOKEN_BUFFERSIZE, postion = 0;
  char **tokens = malloc(buffersize * sizeof(char*));
  char *token;

  if (!tokens) {
    MALLOC_ERROR; //fprintf(stderr, "shell: allocation error\n");
    exit(1);
  }

  token = strtok(line, SHELL_TOKEN_DELIM);
  while (token != NULL) {
    tokens[postion] = token;
    postion++;

    if (postion >= buffersize) {
      buffersize += SHELL_TOKEN_BUFFERSIZE;
      tokens = realloc(tokens, buffersize * sizeof(char*));
      if (!tokens) {
        MALLOC_ERROR;
        exit(1);
      }
    }
    token = strtok(NULL, SHELL_TOKEN_DELIM);
  }
  tokens[postion] = NULL;
  return tokens;
}

int shell_launch(char **args) {
  pid_t pid;
  int status;

  pid = fork();
  if(pid == 0){
      if(execvp(args[0], args) == -1){
        DROP_ERROR;
    }
    exit(1);
  }
  else if(pid < 0){
      DROP_ERROR;
  }
  else{
      do{
          waitpid(pid, &status, WUNTRACED);
      }while(!WIFEXITED(status) && !WIFSIGNALED(status));
  }
  return 1;
}

int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);
int shell_mkdir(char **args);
int shell_touch(char **args);
int shell_ls(char **args);

char *builtin_str[] = {
    "cd",
    "help",
    "exit",
    "mkdir",
    "touch",
    "ls"
};

int (*builtin_func[]) (char **) = {
    &shell_cd,
    &shell_help,
    &shell_exit,
    &shell_mkdir,
    &shell_touch,
    &shell_ls
};

int shell_num_builtins(){
    return sizeof(builtin_str) / sizeof(char*);
}

int shell_cd(char **args){
    if(args[1] == NULL){
        CD_ERROR;
    }
    else{
        if(chdir(args[1]) != 0){
            DROP_ERROR;
        }
    }
    return 1;
}

int shell_help(char **args){
    int i;
    printf("This shell got that list builtin commands:\n");

    for(i = 0; i < shell_num_builtins(); i++){
        printf(" %s\n", builtin_str[i]);
    }

    return 1;
}

int shell_exit(char **args){
    return 0;
}

static char *elements_list[] = {""};

int elements_list_count(){
    return sizeof(elements_list) / sizeof(char*);    
};

char element_list_change(char **args, char *elements_list){
    printf("DEBUG: %d\n", elements_list_count());
    elements_list[elements_list_count() - 1] = args[1];
    printf("DEBUG: %s\n", elements_list[elements_list_count() - 1]);
    return elements_list;
}

int shell_mkdir(char **args){
    element_list_change(args, *elements_list);
    return 1;
}

int shell_touch(char **args){
    printf("doesn't work for now");
    return 1;
}

int shell_ls(char **args){
    int i;
    printf("DEBUG: %d\n", elements_list_count());
    printf("DEBUG: %s\n", elements_list[elements_list_count() - 1]);
    
    for(i = 0; i < elements_list_count(); i++){
        printf("%s ", elements_list[i]);
    }
    return 1;
}

int shell_execute(char **args){
    int i;
    if(args[0] == NULL){
        return 1;
    }

    for(i = 0; i < shell_num_builtins(); i++){
        if(strcmp(args[0], builtin_str[i]) == 0){
            return (*builtin_func[i])(args);
        }
    }
    return shell_launch(args);
}

void shell_loop(void) {
  char *line;
  char **args;
  int status;

  do {
    printf("> ");
    line = shell_readline();
    args = shell_splitline(line);
    status = shell_execute(args);

    free(line);
    free(args);
  } while (status);
}

int main(int argc, char **argv) {
  shell_loop();
  return 0;
}