#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KWHT "\x1B[37m"
#define KGRN "\x1B[32m"


/* Deklaracja wbudowanych komend
*/
int sh_cd(char **args);
int sh_help(char **args);
int sh_exit(char **args);
int sh_cat(char **args);


char *builtin_str[] = {
  "cd",
  "help",
  "exit",
	"cat"
};

int (*builtin_func[]) (char **) = {
  &sh_cd,
  &sh_help,
  &sh_exit,
	&sh_cat
};

int sh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  implementacja wbudowanych komend
*/


int sh_cd(char **args)
{
  if (args[1] == NULL) {
    fprintf(stderr, "sh: expected argument to \"cd\"\n");
  } else {
    if (chdir(args[1]) != 0) {
      perror("sh");
    }
  }
  return 1;
}


int sh_help(char **args)
{
  int i;
  printf("Igor Grzankowski - softshell\n");
  printf("Wpisz komende i wcisnij enter\n");
  printf("Lista wbudowanych komend:\n");

	

  for (i = 0; i < sh_num_builtins(); i++) {
    printf("  %s\n", builtin_str[i]);
  }



  return 1;
}


int sh_exit(char **args)
{
	char* usr = getenv("USER");
	printf("Bye, %s! \n\n",usr);
  return 0;
}

#define cat_BUFFER_SIZE 1024
int sh_cat(char **args)
{

char buffer[cat_BUFFER_SIZE];
int fd_in, num;
char* plik = args[1];

if(args[1]==NULL)
fprintf(stderr, "sh: expected argument to \"cat\"\n");
else{

fd_in=open(plik,O_RDONLY);

while((num = read(fd_in, &buffer, cat_BUFFER_SIZE))>0)
{
	write(STDOUT_FILENO, &buffer, num);
}
close(fd_in);}

return 1;

}


int sh_launch(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0) {
    /* proces dziecko
	*/
    if (execvp(args[0], args) == -1) {
      perror("sh");
    }
    exit(EXIT_FAILURE);
  } else if (pid < 0) {
    /* blad podczas forkowania
	*/
    perror("sh");
  } else {
    /* proces rodzic
	*/
    do {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}


int sh_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    /* wprowadzona pusta komenda
	*/
    return 1;
  }

  for (i = 0; i < sh_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return sh_launch(args);
}

#define SH_RL_BUFSIZE 1024

char *sh_read_line(void)
{
  int bufsize = SH_RL_BUFSIZE;
  int position = 0;
  char *buffer = malloc(sizeof(char) * bufsize);
  int c;

  if (!buffer) {
    fprintf(stderr, "sh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  while (1) {
    /* wczytywanie liter
*/
    c = getchar();

    if (c == EOF) {
      exit(EXIT_SUCCESS);
    } else if (c == '\n') {
      buffer[position] = '\0';
      return buffer;
    } else {
      buffer[position] = c;
    }
    position++;

    /* realockajca w przypadku przekroczenia bufora */
    if (position >= bufsize) {
      bufsize += SH_RL_BUFSIZE;
      buffer = realloc(buffer, bufsize);
      if (!buffer) {
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }
  }
}

#define SH_TOK_BUFSIZE 64
#define SH_TOK_DELIM " \t\r\n\a"

char **sh_split_line(char *line)
{
  int bufsize = SH_TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token, **tokens_backup;

  if (!tokens) {
    fprintf(stderr, "sh: allocation error\n");
    exit(EXIT_FAILURE);
  }

  token = strtok(line, SH_TOK_DELIM);
  while (token != NULL) {
    tokens[position] = token;
    position++;

    if (position >= bufsize) {
      bufsize += SH_TOK_BUFSIZE;
      tokens_backup = tokens;
      tokens = realloc(tokens, bufsize * sizeof(char*));
      if (!tokens) {
		free(tokens_backup);
        fprintf(stderr, "sh: allocation error\n");
        exit(EXIT_FAILURE);
      }
    }

    token = strtok(NULL, SH_TOK_DELIM);
  }
  tokens[position] = NULL;
  return tokens;
}


void sh_loop(void)
{
  char *line;
  char **args;
  int status;
	char* username = getenv("USER");
	char* path = getenv("PWD");
	char* pc = getenv("COMPUTERNAME");
	

  do {


		
    printf("%s%s@%s%s:%s%s%s$ ",KRED,username,pc,KNRM,KGRN,path,KNRM);
    line = sh_read_line();
    args = sh_split_line(line);
    status = sh_execute(args);

    free(line);
    free(args);
  } while (status);
}


int main(int argc, char **argv)
{
  
  sh_loop();



  return EXIT_SUCCESS;
}
