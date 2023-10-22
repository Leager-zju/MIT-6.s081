#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h"

int
main(int argc, char *argv[]) {
  int res;
  res = uptime();
  printf("%d\n", res);
  exit(0);
}