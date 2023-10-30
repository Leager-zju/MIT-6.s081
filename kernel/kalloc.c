// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

#define NPAGES (PHYSTOP-KERNBASE)/PGSIZE

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

static uint8 ref_count[NPAGES];

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

// arg start_pa must be PGROUNDed
uint64
page_idx(void* start_pa)
{
  uint64 idx = ((uint64)start_pa-KERNBASE) >> 12;
  if (idx >= NPAGES) {
    return -1;
  }
  return idx;
}

// plus the reference count of page at pa
void
pin(void* pa)
{
  void* start_pa = (void*)PGROUNDDOWN((uint64)pa);
  uint64 idx = page_idx(start_pa);
  if (idx == -1) {
    panic("invalid page\n");
  }

  ref_count[idx]++;
}

// minus the reference count of page at pa
// if the count is 0 after unpin, free it
void
unpin(void* pa)
{
  void* start_pa = (void*)PGROUNDDOWN((uint64)pa);
  uint64 idx = page_idx(start_pa);
  if (idx == -1) {
    panic("invalid page\n");
  }

  if (--ref_count[idx] == 0) {
    kfree(start_pa);
  }
}

uint8
getcount(void* pa)
{
  void* start_pa = (void*)PGROUNDDOWN((uint64)pa);
  uint64 idx = page_idx(start_pa);
  if (idx == -1) {
    panic("invalid page\n");
  }
  return ref_count[idx];
}

int
pinned(void* pa)
{
  return getcount(pa) > 0;
}

int
exown(void* pa)
{
  return getcount(pa) == 1;
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  memset(ref_count, 0, sizeof(ref_count));
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

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
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r){
    memset((char*)r, 5, PGSIZE); // fill with junk
    pin((void*)r);
  }
  return (void*)r;
}
