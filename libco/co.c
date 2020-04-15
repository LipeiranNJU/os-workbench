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
  assert(coroutinesCanBeUsed < 130);
  coroutinesCanBeUsed += 1;
  int i;
  for (i = 0; i < 256; i++) {
    if (coPool[i].status == CO_NOTHING) {
      coPool[i].arg = arg;
      coPool[i].func = func;
      coPool[i].name = malloc(strlen(name)+1);
      // printf("Init name:%s\n", name);
      strcpy(coPool[i].name, name);
      coPool[i].status = CO_NEW;
      break;
    }
  }
  return &coPool[i];
}


static void stack_switch_call(void *sp, void *entry, uintptr_t arg) {

  asm volatile (
#if __x86_64__
    "movq %0, %%rsp; movq %2, %%rdi; jmp *%1"
      : : "b"((uintptr_t)sp - 8),     "d"(entry), "a"(arg)
#else
    "movl %0, %%esp; movl %2, 4(%0); jmp *%1"
      : : "b"((uintptr_t)sp - 24), "d"(entry), "a"(arg)
#endif
  );
}

static void* co_wrapper(struct co* co) {
  char* name = co->name;
  co->status = CO_RUNNING;
  co->func(co->arg);
  co->status = CO_DEAD;
  coroutinesCanBeUsed -= 1;
  // print("Can't reach here!\n");
  print("%shas Dead\n", co->name);
  print("%d co can be used Now\n", coroutinesCanBeUsed);
  co_yield();
  print("Can't reach here!\n");
  assert(0);
  return 0;
}

void co_wait(struct co *co) {
  print("in co_wait wait %s\n", co->name);
  assert(current != NULL);
  current->waiter = co;
  current->status = CO_WAITING;
  co_yield();
  print("%shas finished!\n", co->name);
  current->waiter = NULL;
  current->status = CO_RUNNING;
}

void co_yield() {
  // print("in yield!\n");
  int val = setjmp(current->context);
  if (val == 0) {
    longjmp(base, 1);
  } else {
    ;
  }
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
    current = &coPool[0];
    current->status = CO_RUNNING;
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
    while (current->waiter != NULL && ((current->waiter)->status != CO_DEAD)) {
      current = current->waiter;
      assert(current->status != CO_DEAD);
    }
    if (current->status == CO_DEAD) {
      print("%s is scheduled and it is dead\n", current->name);
      assert(current->status != CO_DEAD);
    }
    if (current->status == CO_NEW) {
      current->status = CO_RUNNING;
      stack_switch_call(&current->stack[STACK_SIZE], co_wrapper, (uintptr_t) current);
    } else {
      assert(current->status != CO_DEAD);
      longjmp(current->context, 1);
    }
  }
  
}