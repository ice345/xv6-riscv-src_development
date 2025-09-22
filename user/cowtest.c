#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

void
cowtest()
{
  printf("COW test starting...\n");

  // Allocate one page of memory
  char *mem = sbrk(4096);
  if(mem == (char*)-1){
    printf("sbrk failed\n");
    return;
  }
  // Fill the memory with some data
  *mem = 'A';

  int initial_freemem = freemem();
  printf("1. Initial free memory: %d KB\n", initial_freemem);

  int pid = fork();
  if(pid < 0){
    printf("fork failed\n");
    return;
  }

  if(pid == 0){
    // Child process
    int freemem_after_fork = freemem();
    printf("2. Child: free memory after fork: %d KB\n", freemem_after_fork);
    printf("   (Memory change after fork: %d KB)\n", initial_freemem - freemem_after_fork);

    // Reading should not trigger COW
    if(*mem != 'A'){
        printf("Child: read incorrect data\n");
    }

    // Writing to the page should trigger COW
    printf("3. Child: writing to shared page...\n");
    *mem = 'X';
    
    int freemem_after_write = freemem();
    printf("4. Child: free memory after write: %d KB\n", freemem_after_write);
    printf("   (Memory change after write: %d KB)\n", freemem_after_fork - freemem_after_write);
    exit(0);
  } else {
    // Parent process
    wait(0);
    printf("5. Parent: child has exited.\n");
    int freemem_final = freemem();
    printf("6. Parent: final free memory: %d KB\n", freemem_final);
    printf("   (Total memory change for COW: %d KB)\n", initial_freemem - freemem_final);
  }
}

int
main(int argc, char *argv[])
{
  cowtest();
  exit(0);
}
