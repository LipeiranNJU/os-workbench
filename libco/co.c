#include "co.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

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
  printf("befor main\n");
  // coPool[0].name = malloc(sizeof("main"+1));
  // strcpy(coPool[0].name, "main");
  // current = &coPool[0];
}