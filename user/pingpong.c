#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if(argc > 1 && strcmp("pingpong", argv[0]) !=1){
    fprintf(2, "Invalid argument\n");
  }

  int p[2];
  char buf[2];
  int pid;
  pipe(p);

  if(pid = fork(), pid == 0){
    read(p[0], buf, sizeof(buf));
    fprintf(1,"%d: received ping\n", getpid());
    close(p[0]);
    write(p[1], "1", 1);
    close(p[1]);
    exit(0);
  }else if(pid > 0){
    write(p[1], "1", 1);
    close(p[1]);
    wait(0);
    read(p[0], buf, sizeof(buf));
    fprintf(1, "%d: received pong\n", getpid());
    close(p[0]);
  }else{
    write(2, "Error fork\n", 11);
    exit(1);
  }

  exit(0);
}
