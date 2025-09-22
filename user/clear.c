#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  printf("\x1b[2J\x1b[H");
  return 0;
}
