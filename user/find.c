#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

#define BUFSIZE 512

int match(char*, char*);

void
searchFile(char *dir, char *pattern) {
  int fd; // point to the directory
  if ((fd = open(dir, 0)) < 0){
    fprintf(STDERR, "find: cannot open %s\n", dir);
    exit(1);
  }

  if (strlen(dir) + 1 + DIRSIZ + 1 > BUFSIZE){ // path + '/' + name + '\0'
    fprintf(STDERR, "find: path too long\n");
    exit(1);
  }

  // make dir path name
  char buf[BUFSIZE], *p;
  strcpy(buf, dir);
  p = buf+strlen(buf);
  *p++ = '/';

  // travesal the directory entries
  struct dirent de;
  struct stat st;
  while (read(fd, &de, sizeof(de)) == sizeof(de)) {
    if (de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) { // skip invalid dir
      continue;
    }
    
    // make file path name
    memmove(p, de.name, DIRSIZ);
    p[DIRSIZ] = 0;
    if (stat(buf, &st) < 0) {
      fprintf(STDERR, "find: cannot stat %s\n", buf);
      continue;
    }

    switch (st.type) {
      case T_FILE: // check if the file name is the same
        if (match(pattern, de.name)) {
          printf("%s\n", buf);
        }
        break;
      case T_DIR: // recurse into sub dir
        searchFile(buf, pattern);
        break;
    }
  }
}

int
main(int argc, char *argv[]) {
  if (argc <= 2) {
    fprintf(2, "Usage: find dir files...");
    exit(1);
  }

  for (int i = 2; i < argc; i++) {
    searchFile(argv[1], argv[i]);
  }
  exit(0);
}

// Regexp matcher from Kernighan & Pike,
// The Practice of Programming, Chapter 9.

int matchhere(char*, char*);
int matchstar(int, char*, char*);

int
match(char *re, char *text)
{
  if(re[0] == '^')
    return matchhere(re+1, text);
  do{  // must look at empty string
    if(matchhere(re, text))
      return 1;
  }while(*text++ != '\0');
  return 0;
}

// matchhere: search for re at beginning of text
int matchhere(char *re, char *text)
{
  if(re[0] == '\0')
    return 1;
  if(re[1] == '*')
    return matchstar(re[0], re+2, text);
  if(re[0] == '$' && re[1] == '\0')
    return *text == '\0';
  if(*text!='\0' && (re[0]=='.' || re[0]==*text))
    return matchhere(re+1, text+1);
  return 0;
}

// matchstar: search for c*re at beginning of text
int matchstar(int c, char *re, char *text)
{
  do{  // a * matches zero or more instances
    if(matchhere(re, text))
      return 1;
  }while(*text!='\0' && (*text++==c || c=='.'));
  return 0;
}