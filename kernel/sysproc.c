#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sleeplock.h"
#include "fs.h"
#include "fcntl.h"
#include "file.h"

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
  if(n < 0)
    n = 0;
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

uint64 sys_mmap(void){
  uint64 addr;
  uint64 length;
  int prot;
  int flags;
  int fd;
  int offset;

  argaddr(0,&addr);
  if(addr<0){
      return -1;
  }
  argaddr(1,&length);
  if(length<0){
      return -1;
  }
  argint(2,&prot);
  if(prot<0){
     return -1;
  }
  argint(3,&flags);
  if(flags<0){
      return -1;
  }
  argint(4,&fd);
  if(fd<0){
      return -1;
  }
  argint(5,&offset);
  if(offset<0){
      return -1;
  }

  struct proc*p = myproc();
  struct file*f;
  if(fd<0||fd>NOFILE||(f=p->ofile[fd])==0){
      return -1;
  }
  if(flags == MAP_SHARED){
    if((prot & PROT_READ) && (!f->readable))
        return -1;
    if((prot & PROT_WRITE) && (!f->writable))
        return -1;
  }
  uint64 mapaddr = p->sz;
  p->sz += length;
  f= filedup(f);

  int i=0;
  for(;i<VMASIZE;++i){
      if(p->vmas[i].valid==-1){
          break;
      }
  }
  if(i==VMASIZE){
      return -1;
  }
  //lazy alloc
  p->vmas[i].addr = mapaddr;
  p->vmas[i].length = length;
  p->vmas[i].prot=prot;
  p->vmas[i].flags = flags;
  p->vmas[i].fd=fd;
  p->vmas[i].valid = 0;
  p->vmas[i].file = f;
  p->vmas[i].offset =offset;
  return mapaddr;
}

uint64 sys_munmap(void){
    uint64 addr;
    uint64 length;
    int i;
    struct proc *p = myproc();

    argaddr(0,&addr);
    argaddr(1,&length);
    if(addr < 0 || length < 0)
        return -1;

    //get right vma
    for(i = 0; i < VMASIZE; i++){
        if(p->vmas[i].valid == 0){
            uint64 start = p->vmas[i].addr;
            uint64 end   = start + p->vmas[i].length;
            if(addr >= start && addr < end){
                break;
            }
        }
    }

    if(i == VMASIZE)
        return -1;
    uint64 sz;
    for(sz = 0; sz < length; sz+=PGSIZE){
        int mapped = walkaddr(p->pagetable,addr+sz);
        //need write back to file?
        // unmap always unmap from the start to end and PGSIZE aligned
        // so it simplify our implemention
        if(mapped){
            if(p->vmas[i].flags == MAP_SHARED){
                if(filewrite(p->vmas[i].file,addr+sz,PGSIZE) <= 0)
                    return -1;
            }
            uvmunmap(p->pagetable,PGROUNDDOWN(addr+sz),1,1);
        }
        p->vmas[i].file->off += sz;
    }
    //set vma
    p->vmas[i].addr = addr+length;
    p->vmas[i].length -= length;
    p->sz -= length;

    //all free
    if(p->vmas[i].length == 0){
       p->vmas[i].valid = -1;
       fileclose(p->vmas[i].file);
    }
    return 0;
    }
