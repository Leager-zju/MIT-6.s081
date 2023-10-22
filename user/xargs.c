#include "kernel/types.h"
#include "kernel/param.h"
#include "user/user.h"

int
main(int argc, char *argv[]) {
  // find . b | xargs grep hello
  char *cmd = argv[1];
  char *new_args[MAXARG];
  int k;
  for (k = 0; k < argc-1; k++) {
    new_args[k] = argv[k+1];
  }

  char extra_arg[32];
  int p = 0;
  // To read individual lines of input,
  // read a character at a time until a newline ('\n') appears.
  while (read(STDIN, extra_arg+p, 1) > 0) {
    if (extra_arg[p] == '\n') {
      extra_arg[p] = '\0';
      int pid;
      pid = fork();
      if (pid > 0) {
        wait(&pid);
      } else if (pid == 0) {
        new_args[k] = extra_arg;
        exec(cmd, new_args);
        fprintf(2, "exec %s failed\n", cmd);
        exit(0);
      } else {
        fprintf(2, "xargs: fork failed\n");
        exit(1);
      }
      p = 0;
    } else {
      p++;
    }
  }
  exit(0);
}