// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);
void kfree_huge(void *pa);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  struct run *huge_freelist; // Free list for 2MB pages
  uint ref_counts[(PHYSTOP - KERNBASE) / PGSIZE];
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);

  // Debug print to check huge page availability at boot.
  struct run *r;
  int huge_count = 0;
  acquire(&kmem.lock);
  r = kmem.huge_freelist;
  while(r){
    huge_count++;
    r = r->next;
  }
  release(&kmem.lock);
  printf("kinit: %d huge pages available.\n", huge_count);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p = (char*)PGROUNDUP((uint64)pa_start);
  char *end_addr = (char*)pa_end;

  while(p + PGSIZE <= end_addr){
    if(((uint64)p % PGSIZE_2M) == 0 && p + PGSIZE_2M <= end_addr){
      // Found a 2MB-aligned chunk that fits.
      // Mark all its 4KB sub-pages' ref counts as 1 before freeing.
      for(int i = 0; i < 512; i++) {
        kmem.ref_counts[(((uint64)p) - KERNBASE) / PGSIZE + i] = 1;
      }
      kfree_huge(p);
      p += PGSIZE_2M;
    } else {
      // Free as a normal 4KB page.
      kmem.ref_counts[((uint64)p - KERNBASE) / PGSIZE] = 1;
      kfree(p);
      p += PGSIZE;
    }
  }
}

void
inc_ref(void *pa)
{
  acquire(&kmem.lock);
  kmem.ref_counts[((uint64)pa - KERNBASE) / PGSIZE]++;
  release(&kmem.lock);
}

int
get_ref(void *pa)
{
  int ref;
  acquire(&kmem.lock);
  ref = kmem.ref_counts[((uint64)pa - KERNBASE) / PGSIZE];
  release(&kmem.lock);
  return ref;
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&kmem.lock);
  if(--kmem.ref_counts[((uint64)pa - KERNBASE) / PGSIZE] > 0){
    release(&kmem.lock);
    return;
  }
  release(&kmem.lock);

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    kmem.ref_counts[((uint64)r - KERNBASE) / PGSIZE] = 1;
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}

// Allocate one 2MB page of physical memory.
void *
kalloc_huge(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.huge_freelist;
  if(r){
    kmem.huge_freelist = r->next;
    // The ref_counts are already set to 1 by freerange
  }
  release(&kmem.lock);

  if(r)
    memset((char*)r, 6, PGSIZE_2M); // fill with junk
  return (void*)r;
}

// Free one 2MB page of physical memory.
void
kfree_huge(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE_2M) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree_huge");

  // Caller must ensure there are no other references.
  // For kinit, we marked ref_counts to 1, now we reset them to 0 on free.
  acquire(&kmem.lock);
  for(int i = 0; i < 512; i++) {
    if(--kmem.ref_counts[(((uint64)pa) - KERNBASE) / PGSIZE + i] > 0){
        // still in use
        release(&kmem.lock);
        return;
    }
  }
  release(&kmem.lock);

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE_2M);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.huge_freelist;
  kmem.huge_freelist = r;
  release(&kmem.lock);
}


// Returans the free space of the memory in KBytes
uint64
freemem_amount(void)
{
  struct run *r;
  uint64 amount = 0;
  acquire(&kmem.lock);
  r = kmem.freelist;
  while(r){
    amount += PGSIZE;
    r = r->next;
  }
  // Also count huge pages
  r = kmem.huge_freelist;
  while(r){
    amount += PGSIZE_2M;
    r = r->next;
  }
  release(&kmem.lock);
  return amount / 1024;
}
