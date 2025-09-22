#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(2, "usage: sleep times(ms)\n");
    exit(-1);
  }
  
  int times = atoi(*(++argv));
  
  if (times == 0) {
    fprintf(2, "times can not less than or equal 0\n");
    exit(-1);
  }
  
  sleep(times);
  return 0;
}