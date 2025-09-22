#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  uint64 old_sz;

  printf("sbrktest: starting\n");

  // Get the initial size of the process memory
  old_sz = (uint64)sbrk(0);
  printf("sbrktest: initial size = %ld\n", old_sz);

  printf("\nsbrktest: --- Page table BEFORE sbrk(1) ---\n");
  pgtableinfo();

  printf("sbrktest: calling sbrk(1)...\n");
  if(sbrk(1) == (void *)-1){
    fprintf(2, "sbrk(1) failed\n");
    exit(1);
  }
  
  uint64 new_sz = (uint64)sbrk(0);
  printf("sbrktest: new size = %ld\n", new_sz);

  printf("\nsbrktest: --- Page table AFTER sbrk(1) ---\n");
  if(new_sz != old_sz + 1){
    fprintf(2, "sbrk(1) did not increase size by 1 byte\n");
    exit(1);
  }
  pgtableinfo();

  printf("\nsbrktest: calling sbrk(1) again...\n");
  if(sbrk(1) == (void *)-1){
    fprintf(2, "sbrk(1) failed again\n");
    exit(1);
  }
  new_sz = (uint64)sbrk(0);
  printf("sbrktest: new size = %ld\n", new_sz);
  printf("\nsbrktest: --- Page table AFTER second sbrk(1) ---\n");
  if(new_sz != old_sz + 2){
    fprintf(2, "sbrk(1) did not increase size by 1 byte again\n");
    exit(1);
  }
  pgtableinfo();


  printf("sbrktest: calling sbrk(-1)...\n");
  if(sbrk(-1) == (void *)-1){
    fprintf(2, "sbrk(-1) failed\n");
    exit(1);
  }
  pgtableinfo();

  printf("sbrktest: finished\n");
  exit(0);
}
