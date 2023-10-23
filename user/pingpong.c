#include "kernel/types.h"
#include "user/user.h"

#define ONEBYTE sizeof(char)

int
main(int argc, char *argv[])
{
  int fd[2]; // fd[0] for reading, while fd[1] for writing
  if (pipe(fd) < 0) {
    fprintf(2, "pingpong: pipe failed\n");
    exit(1);
  }

  // 1. The parent should send a byte to the child;
  // 2. the child should print "<pid>: received ping", where <pid> is its process ID,
  //    and write the byte on the pipe to the parent, and exit;
  // 3. the parent should read the byte from the child, print "<pid>: received pong", and exit.
  int pid = fork();
  if (pid > 0) { // parent
    char *buf = (char*)malloc(ONEBYTE); // an one-byte memory
    if (write(fd[1], buf, ONEBYTE) != ONEBYTE) {
      fprintf(2, "pingpong: proc %d write failed\n", getpid());
      free(buf);
      exit(1);
    }

    wait(&pid);
    
    if (read(fd[0], buf, ONEBYTE) < 0) {
      fprintf(2, "pingpong: proc %d read failed\n", getpid());
      free(buf);
      exit(1);
    }

    printf("%d: received pong\n", getpid());
    free(buf);
  } else if (pid == 0) { // child
    char* buf = (char*)malloc(ONEBYTE);
    if (read(fd[0], buf, ONEBYTE) < 0) {
      fprintf(2, "pingpong: proc %d read failed\n", getpid());
      free(buf);
      exit(1);
    }

    printf("%d: received ping\n", getpid());

    if (write(fd[1], buf, ONEBYTE) != ONEBYTE) {
      fprintf(2, "pingpong: proc %d write failed\n", getpid());
      free(buf);
      exit(1);
    }
    free(buf);
  } else {
    fprintf(2, "pingpong: fork failed\n");
    exit(1);
  }

  exit(0);
}