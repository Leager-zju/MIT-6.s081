#include "kernel/types.h"
#include "user/user.h"

void
createNewProc(int p, int *left_fd) {
  int right_fd[2];
  if (pipe(right_fd) < 0) {
    fprintf(2, "primes: pipe failed\n");
    exit(1);
  }
  
  int pid = fork();
  if (pid > 0) {
    close(right_fd[0]);
    for (;;) {
      int n;
      if (read(left_fd[0], &n, sizeof(int)) == 0) {
        break;
      }

      if (n % p != 0) {
        if (write(right_fd[1], &n, sizeof(int)) != sizeof(int)) {
          fprintf(2, "primes: proc %d write failed\n", getpid());
          exit(1);
        }
      }
    }
    close(right_fd[1]);
    wait((int*)0);
  } else if (pid == 0) {
    close(right_fd[1]);

    int p;
    read(right_fd[0], &p, sizeof(int));
    printf("prime %d\n", p);

    if (p != 31) {
      createNewProc(p, right_fd);
    }
  } else {
    fprintf(2, "primes: fork failed\n");
    exit(1);
  }
}

int
main(int argc, char *argv[]) {
  int fd[2];
  if (pipe(fd) < 0) {
    fprintf(2, "primes: pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if (pid > 0) {
    close(fd[0]);
    int i;
    for (i = 2; i <= 35; i++) {
      if (write(fd[1], &i, sizeof(int)) != sizeof(int)) {
        fprintf(2, "primes: proc %d write failed\n", getpid());
        exit(1);
      }
    }
    close(fd[1]);
    wait((int*)0);
  } else if (pid == 0) {
    close(fd[1]);
    int p;
    read(fd[0], &p, sizeof(int));
    printf("prime %d\n", p);

    createNewProc(p, fd);
  } else {
    fprintf(2, "primes: fork failed\n");
    exit(1);
  }

  exit(0);
}