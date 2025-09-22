#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define FOUR_MB (4 * 1024 * 1024)
#define TWO_MB (2 * 1024 * 1024)

void
test_small_allocations()
{
  printf("\n=== Test 1: Small allocations (should use 4KB pages) ===\n");
  
  int initial_freemem = freemem();
  printf("Initial free memory: %d KB\n", initial_freemem);
  
  // 分配 8KB (2个4KB页面)
  char *mem1 = sbrk(8192);
  if(mem1 == (char*)-1){
    printf("sbrk failed\n");
    return;
  }
  
  // 访问内存
  mem1[0] = 'A';
  mem1[4096] = 'B';
  
  int freemem_after_small = freemem();
  printf("After 8KB allocation: %d KB free (consumed: %d KB)\n", 
         freemem_after_small, initial_freemem - freemem_after_small);
  
  printf("\n--- Page table after small allocation ---\n");
  pgtableinfo();
}

void
test_huge_allocation()
{
  printf("\n=== Test 2: Large allocation (should trigger huge pages) ===\n");
  
  int initial_freemem = freemem();
  printf("Initial free memory: %d KB\n", initial_freemem);
  
  // 分配4MB，这应该触发huge page
  printf("Allocating 4MB (watch for '2MB page' messages in kernel output)...\n");
  char *mem = sbrk(FOUR_MB);
  if(mem == (char*)-1){
    printf("sbrk failed\n");
    return;
  }
  
  // 按2MB边界访问内存
  printf("Accessing memory at 2MB boundaries...\n");
  mem[0] = 'A';                    // 第一个2MB页面
  mem[TWO_MB] = 'B';               // 第二个2MB页面
  
  int freemem_after_huge = freemem();
  int consumed = initial_freemem - freemem_after_huge;
  printf("After 4MB allocation: %d KB free (consumed: %d KB)\n", 
         freemem_after_huge, consumed);
  
  // 分析消耗详情
  printf("Memory consumption analysis:\n");
  printf("  - Expected data pages: 4096 KB (4MB)\n");
  printf("  - Actual consumption: %d KB\n", consumed);
  printf("  - Overhead: %d KB\n", consumed - 4096);
  printf("  - Overhead likely includes page table structures\n");
  
  printf("\n--- Page table after huge allocation ---\n");
  pgtableinfo();
  
  // 分析页表项数量
  printf("\nPage table analysis:\n");
  if(consumed - 4096 > 0) {
    int overhead_kb = consumed - 4096;
    printf("  - Page table overhead: %d KB\n", overhead_kb);
    printf("  - If using 4KB pages: would need %d PTEs for 4MB\n", 4096 / 4);
    printf("  - If using 2MB huge pages: would need %d PTEs for 4MB\n", 4096 / 2048);
    printf("  - Each page table page holds 512 PTEs (4KB per page table page)\n");
    printf("  - Overhead suggests: %s\n", 
           overhead_kb <= 8 ? "efficient huge page usage" : "many small pages used");
  }
  
  printf("Memory access completed successfully\n");
}

void
hugepagetest()
{
  printf("Huge page test starting...\n");
  printf("======================================\n");
  printf("This test compares small vs large memory allocations\n");
  printf("Large allocations should use 2MB huge pages for efficiency\n");
  printf("======================================\n");

  // 显示初始状态
  printf("\n--- Initial system state ---\n");
  printf("Initial free memory: %d KB\n", freemem());
  pgtableinfo();
  
  // 测试1：小分配
  test_small_allocations();
  
  // 测试2：大分配（应该使用huge page）
  test_huge_allocation();
  
  // 总结
  printf("\n======================================\n");
  printf("Key points to observe:\n");
  printf("1. Look for 'mapping 2MB page' messages in kernel output\n");
  printf("2. Compare page table entries between small and large allocations\n");
  printf("3. Huge pages reduce page table overhead for large memory regions\n");
  printf("4. Normal behavior: sbrk() allocates memory immediately\n");
  printf("   (freemem changes right after sbrk, not during access)\n");
  printf("======================================\n");
  printf("Huge page test finished.\n");
}

int
main(int argc, char *argv[])
{
  hugepagetest();
  exit(0);
}
