#if __X86_64__
#include "co.h"
#include <stdlib.h>
// #define __DEBUG__
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
  char name[32];
  void (*func)(void *); // co_start 指定的入口地址和参数
  void *arg;

  enum co_status status;  // 协程的状态
  struct co *    waiter;  // 当前协程是否在等待其他携程，如果在等待，等待的是谁
  jmp_buf        context; // 寄存器现场 (setjmp.h)
  long double buffer;
  uint8_t        stack[STACK_SIZE]; // 协程的堆栈
};

struct co* coPool;
struct co* current = NULL;
int coroutinesCanBeUsed;

void co_init(void) {
  coPool = malloc(192*sizeof(struct co));
  coroutinesCanBeUsed = 0;
  for (int i = 0; i < 192; i++) {
      coPool[i].arg = NULL;
      coPool[i].buffer = 0;
      coPool[i].func = NULL;
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
  for (i = 0; i < 192; i++) {
    if (coPool[i].status == CO_NOTHING) {
      coPool[i].arg = arg;
      coPool[i].func = func;
      print("Init name:%s in the pool %d\n", name, i);
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
      : : "b"((uintptr_t)sp - 16), "d"(entry), "a"(arg)
#endif
  );
}

static void* co_wrapper(struct co* co) {
  co->status = CO_RUNNING;
  co->func(co->arg);
  co->status = CO_DEAD;
  coroutinesCanBeUsed -= 1;
  // print("Can't reach here!\n");
  print("\n%shas Dead\t", co->name);
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
  // print("%shas finished!\n", co->name);
  current->waiter = NULL;
  current->status = CO_RUNNING;
  co->func = NULL;
  co->status = CO_NOTHING;

}

void co_yield() {
  fflush(stdout);
  int val = setjmp(current->context);
  if (strcmp("main", current->name) == 0) {
    print("in yield! val is %d", val);
    fflush(stdout);
  }
  if (val == 0) {
    // printf("jmp to base\n");
    longjmp(base, 1);
  } else {
    // assert(current->status != CO_WAITING);
    print("return!");
    fflush(stdout);
    return ;
  }
}

void __attribute__((constructor)) start() {
  srand(time(0));
  print("befor main\n");
  co_init();
  coroutinesCanBeUsed += 1;
  strcpy(coPool[0].name, "main");
  print("%d co can be used\n", coroutinesCanBeUsed);
  // start to schedule cos
  int status = setjmp(base);
  // printf("have saved base\n");
  if (status == 0) {
    current = &coPool[0];
    current->status = CO_RUNNING;
  } else {
    // printf("has jmped in base\n");
    int selected = rand() % coroutinesCanBeUsed;
    int now = -1;
    int i;
    for (i = 0; i < 192; i++) {
      if (coPool[i].status == CO_RUNNING || coPool[i].status == CO_WAITING || coPool[i].status == CO_NEW) {
        now += 1;
        if (now == selected) {
          break;
        }
      }
    }
    current = &coPool[i];
    while (current->status == CO_WAITING) {
      assert(current->status != CO_DEAD);
      // print("we selcet waiter:%s, because now %s is wait it\n", current->waiter->name, current->name);
      if (current->waiter != NULL && current->waiter->status != CO_DEAD)
        current = current->waiter;
      else
        break;
    }
    if (current->status == CO_DEAD) {
      print("%s is scheduled and it is dead\n", current->name);
      print("selected is %d\t, now is %d \t", selected, now);
      assert(current->status != CO_DEAD);
    }
    if (current->status == CO_NEW) {
      current->status = CO_RUNNING;
      stack_switch_call(&current->stack[STACK_SIZE], co_wrapper, (uintptr_t) current);
    } else {
      if (current->status == CO_WAITING) {
        assert(current->waiter->status == CO_DEAD);
      }
      if (current->status == CO_WAITING) {
        print("Hey\n");
        if (strcmp("main", current->name) == 0) {
          print("name is %s\n", current->name);
        }
        // print("%s is waitting %s\n", current->name, current->waiter->name);
      }
      // assert(current->status != CO_WAITING);
      assert(current->status != CO_DEAD);
      print("s");
      fflush(stdout);
      longjmp(current->context, 1);
    }
  }
  
}
#else

#include "co.h"
#include <stdlib.h>
// #define __DEBUG__
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
int co_nums = 0;
enum co_status {
  CO_NOTHING, // as its name, it is nothing
  CO_NEW, // 新创建，还未执行过
  CO_RUNNING, // 已经执行过
  CO_WAITING, // 在 co_wait 上等待
  CO_DEAD,    // 已经结束，但还未释放资源
};
int haswaited = false;
struct co {
  char *name;
  void (*func)(void *); // co_start 指定的入口地址和参数
  void *arg;

  enum co_status status;  // 协程的状态
  struct co *    waiter;  // 是否有其他协程在等待当前协程
  jmp_buf        context; // 寄存器现场 (setjmp.h)
  long double buffer;
  uint8_t        stack[STACK_SIZE]; // 协程的堆栈
};
struct co coPool[256];
bool hasInitCoPool = false;
struct co* current = NULL;
struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  co_nums += 1;
  if (strcmp(name, "consumer-2") == 0) {
    for (int i = 3; i < 256; i++) {
      assert(coPool[i].status == CO_NOTHING);
    }
    // assert(0);
  }
  if (hasInitCoPool == false) {
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
    hasInitCoPool = true;
  }
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
    if (i == 255) {
      print("There is no more coThread can be used!\n");
      assert(0);
    }
  }
  print("Start a co thread(but it has not been executed), its name is %s its index in coPool is:%d\n", coPool[i].name, i);
  return &coPool[i];
}

static void stack_switch_call(void *sp, void *entry, uintptr_t arg) {
  print("In stack switch call\n");
  print("sp is %lx\n", (long)sp);
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
  // printf("wrapper arg:%s\n", co->name);
  char* name = co->name;
  co->status = CO_RUNNING;
  co->func(co->arg);
  co->status = CO_DEAD;
  co_nums -= 1;
  // if (co_nums == 0) {
  //   current = &coPool[0];
  // }
  co_yield();
  if (strcmp(co->name, "thread-2") == 0) {
    assert(0);
  }
  return 0;
}

void co_wait(struct co *co) {
  haswaited = true;
  if (strcmp(co->name, "producer-1") == 0) {
    for (int i = 0; i < 256; i++) {
      assert(coPool[i].status != CO_DEAD);
    } 
    // assert(0);
  }
  int a = setjmp(base);
  if (co->status == CO_DEAD) {
    // printf("co->name%s\n", co->name);
    free(co->name);
    co->func = NULL;
    co->name = NULL;
    co->status = CO_NOTHING;
    int lpr;
    for (lpr = 0; lpr < 256; lpr++) {
      if (coPool[lpr].status != CO_NOTHING)
      break;
    }
    if (lpr == 256) {
      haswaited = false;
    }
    return ;
  }
 

  if (a == 0) { // 考虑把wait作为调度的基础，这样子在发现执行完了后就可以及时return了。补充：突然意识到，在这种情况下，因为co_wait()一直没有return，所以不用担心栈空间会遭到破坏，而且我们给协程分配了单独的4KB栈空间，这一部分是作为全局变量，更不会覆盖，所以longjmp回来的时候貌似很安全啊
    // 这种情况下即是“启动”
    // 使用wrapper启动的话，首先会用stack_switch_call来强行跳入函数，这种情况下未进行压栈操作，所以，wrapper最后必须使用yield逃离，并且正确地将状态设置为CO_DEAD，然后开始新的调度
    if (co->status == CO_NEW) {
      print("CO is NEW, its name is %s\n", co->name);
      co->status = CO_RUNNING;
      current = co;
      stack_switch_call(&co->stack[STACK_SIZE], co_wrapper, (uintptr_t) current);
    }
    assert(co->status != CO_NOTHING);
    longjmp(co->context, 1);
  } else {
    print("Start schedule\n");
    // 这种情况下即是“调度”
    // 先假设这个思路的正确性，那么此时就有别的协程成功地通过longjmp(base, 1)来到了前面，然后此时需要的是找一个正确的协程然后继续下去这个

    // if (exist a co program can be used and co is not dead )
    //     switch to it
    // else 
    //    return ;
    int toBeSelected = rand() % co_nums;
    // int now = -1;
    int i;
    for (i = 0; i < 256; i++)
      if (coPool[i].status != CO_DEAD && coPool[i].status != CO_NOTHING&& strncmp(coPool[i].name, current->name, 8) != 0) {
        // now += 1;
        // printf(" %d can be selected ", co_nums);
        // fflush(stdout);
        // if (now == toBeSelected) {
          // printf("selected:%s", coPool[i].name);
          // fflush(stdout);
          break;
        // }
      }
    print("i is %d\n", i);
    if (i != 256 && co->status != CO_DEAD) {
      if (coPool[i].status == CO_NEW) {
        print("The selected CO is NEW, its name is %s\n", coPool[i].name);
        current = &coPool[i];
        current->status = CO_RUNNING;
        stack_switch_call(&coPool[i].stack[STACK_SIZE], co_wrapper, (uintptr_t) current);
      } else {
        print("Schedule running!\n");
        current = &coPool[i];
        // printf("The selected CO is Running, its name is %s\n", current->name);
        longjmp(coPool[i].context, 1);
      }
    }
  }
  
}

  // #if __X86_64__
  //   register void* sp asm("rsp");
  // #else
  //   register void* sp asm("esp");
  // #endif

void co_yield() {
  // if (current->name != NULL && strcmp(current->name, "main") == 0) {
  //   printf("It is in main\n");
  //   return;
  // }
  #if __X86_64__
    register void* sp asm("rsp");
  #else
    register void* sp asm("esp");
  #endif
  uintptr_t uintsp = (uintptr_t) sp;
  if (uintsp>(uintptr_t)&coPool[256] || uintsp <(uintptr_t)&coPool[0]) {
    // assert(0);
    return;
  }
  if (!haswaited)
    return;
  print("hello yield\n");
  int b = setjmp(current->context);
  print("current is:%s\n", current->name);
  print("hello yield2\n");
  // 尝试一些激进的想法，让yield彻底废掉，只是作为进入co_wait调度的入口，但是这样需要维持栈空间。
  // 上述想法有些问题，如果仅仅是这样子的话，回到yield的时候会遇到重大的问题。
  print("B:%d\n", b);
  if (b == 0) {
    print("In Save part in yield\n");
    longjmp(base, 1);
  } else {
    print("hello yield3\n");
    print("hello yield%d\n", 8-4);
  }
}

void __attribute__((constructor)) start() {
  srand(time(0));
  // coPool[0].name = malloc(sizeof("main"+1));
  // strcpy(coPool[0].name, "main");
  // current = &coPool[0];
}

#endif