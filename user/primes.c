#include "kernel/types.h"
#include "user/user.h"

void new_proc(int p[2]);
// void new_proc(int p[2]) __attribute__((noreturn));

// To avoid the CFLAGS in Makefile(warnning = error)
void child_branch(int p[2]) {
  new_proc(p);
}

// Implement the recursion of the process
void new_proc(int p[2]) {
  int prime;
  int n;

  close(p[1]);

  // If read returns 0, it means the previous place closed the write pipe
  // and there are no more numbers to process. So we can exit.
  if (read(p[0], &prime, sizeof(int)) == 0) {
    close(p[0]);
    exit(0);
  }

  // The first number is prime.
  printf("prime %d\n", prime);

  int fd[2];
  pipe(fd);

  int pid = fork();

  if (pid < 0) {
    fprintf(2, "fork failed\n");
    close(p[0]);
    close(fd[0]);
    close(fd[1]);
    exit(1);
  } else if (pid == 0) {
    // Child process of the child process
    close(p[0]);
    child_branch(fd);
  } else {
    // Child process
    close(fd[0]);

    // Read numbers from input pipe and filter them.
    while (read(p[0], &n, sizeof(int)) > 0) {
      if (n % prime != 0) { // Exclude the multiple of the prime
        write(fd[1], &n, sizeof(int)); // Pass it to the next place.
      }
    }

    close(p[0]);
    close(fd[1]);
    wait(0);
    exit(0);
  }
}

int main(int argc, char** argv) {
  int p[2];
  pipe(p);

  int pid = fork();

  if (pid < 0) {
    fprintf(2, "fork failed\n");
    exit(1);
  } else if (pid == 0) {
    // Child process
    new_proc(p);
  } else {
    // Parent process (main)
    close(p[0]);

    for (int i = 2; i <= 280; i++) {
      write(p[1], &i, sizeof(int));
    }

    close(p[1]);
    wait(0);     
    // while (wait(0) != -1)
    //   ;
  }
  return 0;
}
