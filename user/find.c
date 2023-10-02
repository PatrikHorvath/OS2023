#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

char*
cutpath(char *filename)
{
  // Find first character after last slash.
  char *p;
  for(p=filename+strlen(filename); p >= filename && *p != '/'; p--);
  p++;
  return p;
}

void
find(char *path, char *filename)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }


switch(st.type){
  case T_DEVICE:
  case T_FILE:
      if(strcmp(cutpath(path), filename) == 0)
	fprintf(1, "%s\n",path);
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(strcmp(p, ".") != 0 && strcmp(p, "..") != 0)
	find(buf, filename);
      }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    fprintf(2,"Usage: find [start path] <filename>");
    exit(1);
  }else if(argc == 2){
    find(".", argv[1]);
  }else{

    for(i=2; i<argc; i++){
      find(argv[1], argv[i]);
    }
  }
  exit(0);
}
