#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

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


#ifdef LAB_PGTBL
/**
 * This system call checks the access bits of a given number of pages starting from a given virtual address.
 * It sets the corresponding bits in a mask and clears the accessed bits in the page table entries.
 * The mask is then copied to a user space address.
 * 
 * @param void
 * @return int 0 on success
 */
int
sys_pgaccess(void)
{
  uint64 va; // virtual address
  int pagenum; // number of pages to check
  uint64 abitsaddr; // user space address to copy the mask to

  argaddr(0, &va);
  argint(1, &pagenum); 
  argaddr(2, &abitsaddr);

  uint64 maskbits = 0;
  struct proc *proc = myproc();

  // loop through each page and check its access bit
  for (int i = 0; i < pagenum; i++) {
    // get page table entry for the page
    pte_t *pte = walk(proc->pagetable, va+i*PGSIZE, 0);

    if (pte == 0)
      panic("page does not exist.");

    // if access bit is set, set the corresponding bit in the mask
    if (PTE_FLAGS(*pte) & PTE_A) { 
      maskbits = maskbits | (1L << i);
    }
    // clear PTE_A, set PTE_A bits zero
    *pte = ((*pte&PTE_A) ^ *pte) ^ 0;
  }
  
  // copy the mask to user space
  if (copyout(proc->pagetable, abitsaddr, (char *)&maskbits, sizeof(maskbits)) < 0)
    panic("sys_pgacess copyout error");

  return 0;
}

#endif

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
