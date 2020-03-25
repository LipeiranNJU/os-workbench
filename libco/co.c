#include "co.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>
#include <stdint.h>
#include <assert.h>
int times = 0;
static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg) {
  asm volatile (
#if __x86_64__
    "movq %0, %%rsp; movq %2, %%rdi; jmp *%1" : : "b"((uintptr_t)sp),     "d"(entry), "a"(arg)
#else
    "movl %0, %%esp; movl %2, 4(%0); jmp *%1" : : "b"((uintptr_t)sp - 8), "d"(entry), "a"(arg)
#endif
  );
  printf("arg in stack switch:%s\n", (char *) arg);
}

// co *current;
#define STACK_SIZE      512
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
  void *arg;

  co_status status;  // 协程的状态
  struct co *    waiter;  // 是否有其他协程在等待当前协程
  jmp_buf        context; // 寄存器现场 (setjmp.h)
  uint8_t        stack[STACK_SIZE]; // 协程的堆栈
} co;

co *current = NULL;

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  co* pco = malloc(sizeof(co));
  // printf("Dot1\n");
  memset(pco, 0, sizeof(co)); // 初始化未使用变量，防止意想不到的事情
  co* tmp;
  assert(pco->waiter == NULL);
  // printf("Dot2\n");
  if (current == NULL) {
    current = pco;
    printf("%llx\n", ((unsigned long long)((uintptr_t)pco)));
  } else {
    printf("Thread 2 current:%llx\n",((unsigned long long)((uintptr_t)current)));
    printf("Thread 2 pco:%llx\n",((unsigned long long)((uintptr_t)pco)));
    tmp = current;
    while (tmp->waiter != NULL) {
      tmp = tmp->waiter;
    }
    tmp->waiter = pco;
  }
  if (current->waiter != NULL) {
    printf("Thread waiter:%llx\n",((unsigned long long)((uintptr_t)current->waiter)));
  }
  pco->name = malloc(strlen(name)+1); // 分配协程名字空间
  pco->arg = arg; // 记录协程参数，因为待会要先切换到被调用的函数，参数就丢了，所以这里要先保存一下
  strcpy(pco->name, name); // 同上，把会丢失的变量标注
  pco->func = func; // 同上，保留函数以便在co_wait里面调用
  pco->status = CO_NEW; // 标注新协程的状态
  return pco;
}

void co_wait(struct co *co) {
  printf("HHH\n");
  uint8_t s[10000];
  if (co->status == CO_NEW) {
    printf("%s\n", co->name);
    printf("new stack:%llx, stack[512]%llx", (unsigned long long) ((uintptr_t)(co+1)), (unsigned long long) ((uintptr_t)(&co->stack[512])));
    printf("CO->arg%s\n",(char *) co->arg);
    stack_switch_call(&co->stack[512], co->func, (uintptr_t)co->arg);
    printf("FUCK");
  }

  if (times > 1000) {
    return ;
  }

}

void co_yield() {
  printf("TTT\n");
  // 显然使用stack_switch_call 来切换栈的，估计是把co最高地址/最高地址+1当做sp，func作为entry，arg作为参数
  // save data
  int val = setjmp(current->context);
  // assert(current->waiter != NULL);
  printf("after setjmp current addr:%llx\tcurrent->waiter:%llx\n", (unsigned long long) (uintptr_t) (current), (unsigned long long) (uintptr_t) (current->waiter));
  printf("current->waiter:%llx\n", ((unsigned long long)((uintptr_t)(current->waiter))));
  assert(current->waiter != NULL);
  printf("UUU\n");
  printf("Value:%d\n", val);
  assert(current->waiter != NULL);
  if (val == 0) {
    printf("VVV\n");
    co* tmp = current;
    if (tmp->waiter == NULL) {
      printf("$$$$$$$%s\n", tmp->name);
      printf("current addr:%llx\n", (unsigned long long) (uintptr_t) (tmp));
      // emmm 现在来分析一下为什么current明明就是thread1还是出了问题…在 thread2创建的时候，显然已经成果将thread1的waiter设置成
      // 了thread2，thread1是因为错误的setjmp出的问题吗
    }
    assert(tmp->waiter != NULL);
    while (tmp->waiter != NULL) {
      tmp = tmp->waiter;
    }
    printf("WWW\n");
    tmp->waiter = current;
    co* next = current->waiter;
    current->waiter = NULL;
    printf("XXX\n");
    stack_switch_call(next, next->func, (uintptr_t) next->arg);
    printf("RRR\n");
    longjmp(next->context, 1);
    // 一个之前没考虑的问题，longjmp后怎么保证还能执行下面的调整栈顶
    // 因为跳转的是等待的协程， 所以之前等待的协程发出co_yield()的时候必然已经执行过保存了
    // 接下来那一个协程就能够自然而然地进入下面那个情形返回继续执行了
  } else {
    times++;
    return;
  }
}
