#include "co.h"
#include <stdlib.h>
#define __DEBUG__
#include <stdio.h>
#include <setjmp.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#define STACK_SIZE 1 << 17
#include <stdint.h>
#include <time.h>
#ifdef __DEBUG__
#define print(...) printf(__VA_ARGS__)
#else
#define print(...) 
#endif
jmp_buf base;
enum co_status {
  CO_NOTHING, // as its name, it is nothing
  CO_NEW, // 新创建，还未执行过
  CO_RUNNING, // 已经执行过
  CO_WAITING, // 在 co_wait 上等待
  CO_DEAD,    // 已经结束，但还未释放资源
};

struct co {
  char *name;
  void (*func)(void *); // co_start 指定的入口地址和参数
  void *arg;

  enum co_status status;  // 协程的状态
  struct co *    waiter;  // 当前协程是否在等待其他携程，如果在等待，等待的是谁
  jmp_buf        context; // 寄存器现场 (setjmp.h)
  long double buffer;
  uint8_t        stack[STACK_SIZE]; // 协程的堆栈
};

struct co coPool[256];
struct co* current = NULL;
int coroutinesCanBeUsed;

void co_init(void) {
  coroutinesCanBeUsed = 0;
  for (int i = 0; i < 256; i++) {
      coPool[i].arg = NULL;
      coPool[i].buffer = 0;
      coPool[i].func = NULL;
      coPool[i].name = NULL;
      memset(coPool[i].stack, 0, STACK_SIZE);
      coPool[i].status = CO_NOTHING;
      coPool[i].waiter = NULL;
  }
  print("coPool has been inited.\n");
}
struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  for (int i = 0; i < 256; i++);
  return NULL;
}

void co_wait(struct co *co) {
}

void co_yield() {
}

void __attribute__((constructor)) start() {
  srand(time(0));
  print("befor main\n");
  co_init();
  coPool[0].name = malloc(strlen("main") + 1);
  coroutinesCanBeUsed += 1;
  strcpy(coPool[0].name, "main");
  print("%d co can be used\n", coroutinesCanBeUsed);
  // start to schedule cos
  int status = setjmp(base);
  if (status == 0) {

  } else {
    int selected = rand() % coroutinesCanBeUsed;
    int now = -1;
    for (int i = 0; i < 256; i++) {
      if (coPool[i].status == CO_RUNNING || coPool[i].status == CO_WAITING || coPool[i].status == CO_NEW) {
        now += 1;
        if (now == selected) {
          break;
        }
      }
    }
    current = &coPool[now];
    while (current->waiter != NULL) {
      current = current->waiter;
    }
    if (current->status == CO_NEW) {
      
    } else {
      longjmp(current->context, 1);
    }
  }
  
}