#include "co.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>
#include <assert.h>
int times = 0;
void* sp1;
uintptr_t arg1;
static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg) {
  sp1 = sp;
  arg1 = arg;

//   printf("sp1 in is:%llx\n arg in stack switch:%llx\n", (unsigned long long)((uintptr_t)(sp1)), (unsigned long long) arg);
  asm volatile (
#if __x86_64__
    "movq %0, %%rsp; movq %2, %%rdi; jmp *%1" : : "b"((uintptr_t)sp),     "d"(entry), "a"(arg)
#else
    "movl %0, %%esp; movl %2, 4(%0); jmp *%1" : : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg)
#endif
  );
}

static inline void stack_switch(void *sp) {
  sp1 = sp;

//   printf("sp1 in is:%llx\n arg in stack switch:%llx\n", (unsigned long long)((uintptr_t)(sp1)), (unsigned long long) arg);
  asm volatile (
#if __x86_64__
    "movq %0, %%rsp" : : "b"((uintptr_t)sp)
#else
    "movl %0, %%esp" : : "b"((uintptr_t)sp - 8)
#endif
  );
}

// co *current;
#define STACK_SIZE      1 << 16
typedef unsigned char uint8_t;
typedef enum co_status {
  CO_NEW = 1, // 新创建，还未执行过
  CO_RUNNING, // 已经执行过
  CO_WAITING, // 在 co_wait 上等待
  CO_DEAD,    // 已经结束，但还未释放资源
} co_status;



typedef struct co {
  char *name;
  void (*func)(void *); // co_start 指定的入口地址和参数
  char *arg;

  co_status status;  // 协程的状态
  struct co *    waiter;  // 是否有其他协程在等待当前协程
  jmp_buf        context; // 寄存器现场 (setjmp.h)
  uint8_t*        stack; // 协程的堆栈
} co;

co *current = NULL;

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  co* pco = malloc(sizeof(co));
  memset(pco, 0, sizeof(co)); // 初始化未使用变量，防止意想不到的事情
  co* tmp;
  assert(pco->waiter == NULL);
  if (current == NULL) {
    current = pco;
  } else {
    tmp = current;
    while (tmp->waiter != NULL) {
      tmp = tmp->waiter;
    }
    tmp->waiter = pco;
  }
  pco->name = malloc(strlen(name)+1); // 分配协程名字空间
  pco->arg = malloc(strlen(arg)+1);
  strcpy(pco->arg, arg); // 记录协程参数，因为待会要先切换到被调用的函数，参数就丢了，所以这里要先保存一下
  char* sss = pco->arg;
  strcpy(pco->name, name); // 同上，把会丢失的变量标注
  pco->func = func; // 同上，保留函数以便在co_wait里面调用
  pco->status = CO_NEW; // 标注新协程的状态
  pco->stack = malloc(sizeof(char) * STACK_SIZE);
  return pco;
}

void co_wait(struct co *co) {
    if (times > 100) {
        return ;
  }
  if (co->status == CO_NEW) {
    stack_switch_call(&co->stack[STACK_SIZE], co->func, (uintptr_t)co->arg);
    co->status=CO_DEAD;
    return;
  } else {
    //   printf("HILLO WORLD\n");
    // stack_switch_call(&co->stack[STACK_SIZE], co->func, (uintptr_t)co->arg);
    // longjmp(current->context, 1);
    // printf("HI\n");
    longjmp(current->context, 1);
  }


}

void co_yield() {
    if (current->status == CO_NEW) {
        current->status = CO_WAITING;
    }
  // 显然使用stack_switch_call 来切换栈的，估计是把co最高地址/最高地址+1当做sp，func作为entry，arg作为参数
  int val = setjmp(current->context);

//   printf("Value:%d\n", val);
  if (val == 0) {
          times++;
    co* tmp = current;
    while (tmp->waiter != NULL) {
      tmp = tmp->waiter;
    }
    tmp->waiter = current;
    co* next = current->waiter;
    current->waiter = NULL;
    current = next;
    co_wait(current);
    // 一个之前没考虑的问题，longjmp后怎么保证还能执行下面的调整栈顶
    // 因为跳转的是等待的协程， 所以之前等待的协程发出co_yield()的时候必然已经执行过保存了
    // 接下来那一个协程就能够自然而然地进入下面那个情形返回继续执行了
  } else {
    times++;
    // stack_switch(&current->stack[STACK_SIZE]);
    return;
  }
}

