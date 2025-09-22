#include "kernel/types.h"
#include "user/user.h"

#define MSG "ping"
#define REPLY "pong"

int main(int argc, char *argv[]) {
  int pipe1[2]; // parent -> child
  int pipe2[2]; // child -> parent
  char buf[10];

  if (pipe(pipe1) < 0 || pipe(pipe2) < 0) {
    fprintf(2, "pipe failed\n");
    exit(1);
  }
  int pid = fork();

  if (pid < 0) {
    fprintf(2, "fork failed\n");
    exit(1);
  } else if (pid == 0) {
    // child process
    close(pipe1[1]); // close write end of pipe1
    close(pipe2[0]); // close read end of pipe2

    // send message and receive message
    read(pipe1[0], buf, sizeof(buf));
    printf("[child] received: %s\n", buf);
    
    write(pipe2[1], REPLY, strlen(REPLY) + 1); // send pong
    printf("[child] sent: %s\n", REPLY);

    close(pipe1[0]);
    close(pipe2[1]);
    exit(0);
  } else {
    // parent process
    close(pipe1[0]); // close read end of pipe1
    close(pipe2[1]); // close write end of pipe2

    printf("[parent] sent: %s\n", MSG);
    write(pipe1[1], MSG, strlen(MSG) + 1); // send ping
    
    // Wait for the child to finish
    wait(0);

    read(pipe2[0], buf, sizeof(buf));
    printf("[parent] received: %s\n", buf);

    close(pipe1[1]);
    close(pipe2[0]);
  }
  return 0;
}
