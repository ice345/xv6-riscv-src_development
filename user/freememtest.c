#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  printf("free memory: %dKB\n", freemem());
  exit(0);
}

