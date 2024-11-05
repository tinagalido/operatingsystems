/**
 * Submitted by: Maria Cristina Galido (2092015); 
 * Shabnam Alizalde (2084527); 
 * Romina Sharifi (2055127)
 * 
 * Compiled by executing: gcc sh.c
 * Run by executing: ./a.out
 * Result: The new shell will display and ready for any commands to be executed
 * (Task 1): Execute any single command
 * (Task 2): Execute redirection using the '>' or '<' commands
 * (Task 3): Execute pipe execution using the '|' command
*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>


// Simplifed xv6 shell.

#define MAXARGS 10
#define TIMEBUFFER 80
#define FILESIZE 1024
#define MAX_CMD_LEN 1024

// All commands have at least a type. Have looked at the type, the code
// typically casts the *cmd to some specific cmd type.
struct cmd {
  int type;          //  ' ' (exec), | (pipe), '<' or '>' for redirection
};

struct execcmd {
  int type;              // ' '
  char *argv[MAXARGS];   // arguments to the command to be exec-ed
};

struct redircmd {
  int type;          // < or > 
  struct cmd *cmd;   // the command to be run (e.g., an execcmd)
  char *file;        // the input/output file
  int mode;          // the mode to open the file with
  int fd;            // the file descriptor number to use for the file
};

struct pipecmd {
  int type;          // |
  struct cmd *left;  // left side of pipe
  struct cmd *right; // right side of pipe
};

int fork1(void);  // Fork but exits on failure.
struct cmd *parsecmd(char*);

// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
  int p[2];       // Used in the pipeline implementation
  struct execcmd *ecmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;
  
  if(cmd == 0)
    exit(0);
  
  switch(cmd->type){
  default:
    fprintf(stderr, "unknown runcmd\n");
    exit(-1);

  case ' ':   // TASK 1
    // This is for executing single commands
    ecmd = (struct execcmd*)cmd;

     // Execute the command specified by the user
    if (execvp(ecmd->argv[0], ecmd->argv) == -1) {
      fprintf(stderr, "exec not implemented\n");
      exit(EXIT_FAILURE);
    } else {
      exit(EXIT_SUCCESS);
    }

    break;

  case '>':  // TASK 2

    // This is for I/O redirection for > (output)
    rcmd = (struct redircmd*)cmd;

    // Open the file for writing
    int fd_out = open(rcmd->file, rcmd->mode, 0644);
    if (fd_out == -1) {
      fprintf(stderr, "redir > not implemented\n");
      exit(EXIT_FAILURE);
    }

    // Redirect standard output to the file
    dup2(fd_out, STDOUT_FILENO);
    close(fd_out);

    // Recursively execute the subcommand
    runcmd(rcmd->cmd);

    // Reset standard output to original state after redirection
    dup2(1, STDOUT_FILENO);

    break;

  case '<':  // TASK 2
    
    // This is for I/O redirection for < (input)
    rcmd = (struct redircmd*)cmd;

    // Open the file for reading
    int fd_in = open(rcmd->file, rcmd->mode);
    if (fd_in == -1) {
      fprintf(stderr, "redir < not implemented\n");
      exit(EXIT_FAILURE);
    }

    // Redirect standard input to the file
    dup2(fd_in, STDIN_FILENO);
    close(fd_in);

    // Recursively execute the subcommand
    runcmd(rcmd->cmd);

    // Reset standard input to orignal state after redirection
    dup2(0, STDIN_FILENO);

    break;

  case '|': // TASK 3

    // This is for the pipe implementation
    pcmd = (struct pipecmd*)cmd;

    // Create a pipe
    if (pipe(p) == -1) {
      fprintf(stderr, "pipe not implemented\n");
      exit(EXIT_FAILURE);
    }

    // Fork a child process
    pid_t pid = fork();

    if (pid == -1) {
      fprintf(stderr, "error in fork process\n");
      exit(EXIT_FAILURE);
    }

    if (pid == 0) {   // The child executes the left side of the pipe
      // Child process
      close(p[0]); // Close read end of the pipe
      dup2(p[1], STDOUT_FILENO); // Redirect stdout to the write end of the pipe
      close(p[1]); // Close the write end of the pipe

      // Execute the left side of the pipe
      runcmd(pcmd->left);

      exit(EXIT_SUCCESS);
    } else {        // The parent executes the right side of the pipe
      // Parent process
      close(p[1]); // Close the write end of the pipe
      dup2(p[0], STDIN_FILENO); // Redirect stdin to the read end of the pipe
      close(p[0]); // Close the read end of the pipe

      // Execute the right side of the pipe
      runcmd(pcmd->right);

      // Wait for the child process to finish
      wait(NULL);
    }

    break;
  }    
  exit(0);
}

int
getcmd(char *buf, int nbuf)
{
  
  if (isatty(fileno(stdin)))
    fprintf(stdout, "$ ");
  memset(buf, 0, nbuf);
  fgets(buf, nbuf, stdin);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

int
main(void)
{
  static char buf[100];
  int fd, r;

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
      // Clumsy but will have to do for now.
      // Chdir has no effect on the parent if run in the child.
      buf[strlen(buf)-1] = 0;  // chop \n
      if(chdir(buf+3) < 0)
        fprintf(stderr, "cannot cd %s\n", buf+3);
      continue;
    }
    if(fork1() == 0)
      runcmd(parsecmd(buf));
    wait(&r);
  }
  exit(0);
}

int
fork1(void)
{
  int pid;
  
  pid = fork();
  if(pid == -1)
    perror("fork");
  return pid;
}

struct cmd*
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = ' ';
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, int type)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = type;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->mode = (type == '<') ?  O_RDONLY : O_WRONLY|O_CREAT|O_TRUNC;
  cmd->fd = (type == '<') ? 0 : 1;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = '|';
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>";

int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;
  
  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  if(q)
    *q = s;
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '|':
  case '<':
    s++;
    break;
  case '>':
    s++;
    break;
  default:
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;
  
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return ret;
}

int
peek(char **ps, char *es, char *toks)
{
  char *s;
  
  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return *s && strchr(toks, *s);
}

struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);

// make a copy of the characters in the input buffer, starting from s through es.
// null-terminate the copy to make it a string.
char 
*mkcopy(char *s, char *es)
{
  int n = es - s;
  char *c = malloc(n+1);
  assert(c);
  strncpy(c, s, n);
  c[n] = 0;
  return c;
}

struct cmd*
parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseline(&s, es);
  peek(&s, es, "");
  if(s != es){
    fprintf(stderr, "leftovers: %s\n", s);
    exit(-1);
  }
  return cmd;
}

struct cmd*
parseline(char **ps, char *es)
{
  struct cmd *cmd;
  cmd = parsepipe(ps, es);
  return cmd;
}

struct cmd*
parsepipe(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);
  if(peek(ps, es, "|")){
    gettoken(ps, es, 0, 0);
    cmd = pipecmd(cmd, parsepipe(ps, es));
  }
  return cmd;
}

struct cmd*
parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;

  while(peek(ps, es, "<>")){
    tok = gettoken(ps, es, 0, 0);
    if(gettoken(ps, es, &q, &eq) != 'a') {
      fprintf(stderr, "missing file for redirection\n");
      exit(-1);
    }
    switch(tok){
    case '<':
      cmd = redircmd(cmd, mkcopy(q, eq), '<');
      break;
    case '>':
      cmd = redircmd(cmd, mkcopy(q, eq), '>');
      break;
    }
  }
  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;
  
  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while(!peek(ps, es, "|")){
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a') {
      fprintf(stderr, "syntax error\n");
      exit(-1);
    }
    cmd->argv[argc] = mkcopy(q, eq);
    argc++;
    if(argc >= MAXARGS) {
      fprintf(stderr, "too many args\n");
      exit(-1);
    }
    ret = parseredirs(ret, ps, es);
  }
  cmd->argv[argc] = 0;
  return ret;
}
