#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "grouplock.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_freemem(void) {
    return freemem_amount();
}

// A helper function to print PTE flags
static void
print_pte_flags(pte_t pte)
{
  if (pte & PTE_V) printf(" V"); else printf(" !V");
  if (pte & PTE_U) printf(" U"); else printf(" !U");
  if (pte & PTE_R) printf(" R"); else printf(" !R");
  if (pte & PTE_W) printf(" W"); else printf(" !W");
  if (pte & PTE_X) printf(" X"); else printf(" !X");
}

// Recursive function to walk the page table and print entries
void pte_print(pagetable_t pagetable, int level, uint64 va_base)
{
  for(int i = 0; i < 512; i++){
    pte_t pte = pagetable[i];
    uint64 va = va_base + (i << PXSHIFT(level));
    if(va >= MAXVA) continue;

    if(pte & PTE_V){
      uint64 pa = PTE2PA(pte);
      // Check if it's a leaf PTE
      if((pte & (PTE_R | PTE_W | PTE_X)) != 0){
        if(level == 1) { // L1 leaf is a 2MB page
          printf(" %p - %p | %p               | %p (2MB) |",
             (void *)va, (void *)(va + PGSIZE_2M - 1),
             (void *)pte, (void *)pa);
        } else { // L0 leaf is a 4KB page
          printf(" %p         | %p               | %p         |",
             (void *)va, (void *)pte, (void *)pa);
        }
        print_pte_flags(pte);
        printf("\n");
      } else { // Not a leaf, recurse
        pagetable_t child_pgtbl = (pagetable_t)PTE2PA(pte);
        pte_print(child_pgtbl, level - 1, va);
      }
    }
  }
}

// The implementation of our new system call
uint64
sys_pgtableinfo(void)
{
  struct proc *p = myproc();
  pagetable_t pagetable = p->pagetable;
  uint64 sz = p->sz;

  printf("\n------------------------------------ pgtableinfo for process %d, size 0x%lx ------------------------------------\n", p->pid, sz);
  printf("VA                          | PTE                              | PA                         | Flags\n");
  printf("----------------------------+----------------------------------+----------------------------+----------------\n");

  // Start walking from the top-level page table (L2)
  pte_print(pagetable, 2, 0);

  printf("-------------------------------------------------------------------------------------------------------------------------------\n\n");

  return 0;
}



uint64 sys_grouplock_create(void) {
    int group_id;
    char name[16];
    
    argint(0, &group_id);
    if (argstr(1, name, 16) < 0) {
        return -1;
    }
    
    return grouplock_create(group_id, name);
}

uint64 sys_grouplock_acquire(void) {
    int group_id;
    
    argint(0, &group_id);
    
    return grouplock_acquire(group_id);
}

uint64 sys_grouplock_release(void) {
    int group_id;
    
    argint(0, &group_id);
    
    return grouplock_release(group_id);
}

uint64 sys_grouplock_destroy(void) {
    int group_id;
    
    argint(0, &group_id);
    
    return grouplock_destroy(group_id);
}

uint64 sys_grouplock_verify(void) {
    int result1 = verify_group_properties();
    int result2 = verify_deadlock_freedom();
    int result3 = verify_atomic_group_operations();
    
    return (result1 == 0 && result2 == 0 && result3 == 0) ? 0 : -1;
}

uint64 sys_grouplock_debug(void) {
    int group_id;
    
    argint(0, &group_id);
    
    grouplock_debug_info(group_id);
    return 0;
}
