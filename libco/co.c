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
struct co {
};

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
  return NULL;
}

void co_wait(struct co *co) {
}

void co_yield() {
}

void __attribute__((constructor)) start() {
  srand(time(0));
  print("befor main\n");
  // coPool[0].name = malloc(sizeof("main"+1));
  // strcpy(coPool[0].name, "main");
  // current = &coPool[0];
}