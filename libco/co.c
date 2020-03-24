#include "co.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>


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

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  co* pco = malloc(sizeof(co));
  // strcpy(pco->name, name);
  printf("HHH\n");
  return NULL;
}

void co_wait(struct co *co) {

}

void co_yield() {
}
