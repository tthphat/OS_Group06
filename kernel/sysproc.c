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
int
sys_pgaccess(void)
{
    uint64 base; // địa chỉ ảo bắt đầu
    int len; // số trang cần kiểm tra
    uint64 user_mask; // địa chỉ người dùng để lưu kết quả mặt nạ

    argaddr(0, &base); // lấy argument
    argint(1, &len);
    argaddr(2, &user_mask);

    if (len > 64)
        len = 64;
    uint64 mask = 0;
    struct proc *p = myproc();

    for (int i = 0; i < len; i++) {
        uint64 va = base + i * PGSIZE; // mỗi trang cách nhau PGSIZE = 4096 bytes

        pte_t *pte = walk(p->pagetable, va, 0); 
        // Hàm walk dùng để tìm PTE (Page Table Entry) tương ứng với địa chỉ ảo (va)
        // | -------------- | ---------------------------------- |
        // | `p->pagetable` | page table của process             |
        // | `va`           | virtual address                    |
        // | `0`            | **không tạo mới** nếu chưa tồn tại |

        // walk() trả về: pte_t* nếu tồn tại, 0 nếu page chưa map

        if (pte == 0)
            continue;

        if (*pte & PTE_A) {
            mask |= (1L << i); // dịch trái bit số 1 sang vị trí thứ i
            *pte &= ~PTE_A;   // clear accessed bit. bitwise NOT đảo tất cả bit, mọi bit = 1, trừ bit Accessed = 0
        }
    }

    if (copyout(p->pagetable, user_mask, (char *)&mask, sizeof(mask)) < 0) // user_mask là địa chỉ ảo user, kernel không được phép truy cập trực tiếp
        return -1; // Page table của process hiện tại

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