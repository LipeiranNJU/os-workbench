#include "co.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <setjmp.h>

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

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  co* pco = malloc(sizeof(co));
  memset(pco, 0, sizeof(co)); // 初始化未使用变量，防止意想不到的事情
  pco->name = malloc(strlen(name)+1); // 分配协程名字空间
  pco->arg = arg; // 记录协程参数，因为待会要先切换到被调用的函数，参数就丢了，所以这里要先保存一下
  strcpy(pco->name, name); // 同上，把会丢失的变量标注
  pco->status = CO_NEW; // 标注新协程的状态
  printf("%s\n", pco->name); // 一些测试信息，最后得删掉
  // 问题来了…如何理解栈顶，先放一放
  return pco;
}

void co_wait(struct co *co) {

}

void co_yield() {
  // int val = setjmp(current->context);
  // if (val == 0) {

  // } else {

  // }
}
