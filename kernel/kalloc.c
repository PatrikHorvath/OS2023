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

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;



// COW below
// structure for reference count
struct {
  struct spinlock lock;

  // (PGROUNDUP(PHYSTOP) - KERNBASE)/PGSIZE
  // is the number of pages in the kernel
  // so we need an array of that size

  int count[(PGROUNDUP(PHYSTOP) - KERNBASE)/PGSIZE];
} refcnt;
// COW above


// This macro takes a physical address (pa) and returns the corresponding page table index.
// It does this by subtracting the kernel base address from the physical address, 
// then dividing the result by the page size.
#define PA2IDX(pa) (((uint64)pa - KERNBASE) / PGSIZE)

void
kinit()
{
  initlock(&kmem.lock, "kmem");



  // COW below
  // initialize refcnt
  initlock(&refcnt.lock, "refcnt");
  for (int i = 0; i < (PGROUNDUP(PHYSTOP)-KERNBASE) / PGSIZE; i++)
    refcnt.count[i] = 1;
  // COW above



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



  // COW below
  // check refcnt
  if (krefget(pa) <= 0)
    panic("kfree_decr");

  // kfree was called, so a process is not using this page anymore
  // decrement refcnt and if there are no more references
  // then free the page
  krefdecr(pa);
  if (krefget(pa) > 0)
    return;
  // COW above



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
  


  // COW below
  if(r){
    memset((char*)r, 5, PGSIZE); // fill with junk // original code

    // acquire lock to edit the reference count
    // set it to 1, because we are allocating a new page
    acquire(&refcnt.lock);
    refcnt.count[PA2IDX(r)] = 1;
    release(&refcnt.lock);
    // COW above
  }



  return (void*)r;
}

// COW below
// function to increment the reference count
void
krefincr(void *pa)
{
  acquire(&refcnt.lock);
  refcnt.count[PA2IDX(pa)]++;
  release(&refcnt.lock);
}

// function to decrement the reference count
void
krefdecr(void *pa)
{
  acquire(&refcnt.lock);
  refcnt.count[PA2IDX(pa)]--;
  release(&refcnt.lock);
}

// function to get the reference count
int
krefget(void *pa)
{
  int cnt;
  acquire(&refcnt.lock);
  cnt = refcnt.count[PA2IDX(pa)];
  release(&refcnt.lock);
  return cnt;
}
// COW above