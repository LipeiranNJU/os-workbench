#include <common.h>
void* tmp;
static void *kalloc(size_t size) {
  int i = 0;
  while ((1<<i) < size)
    i++;
  
  tmp = tmp+(1<<i);
  return tmp;
}

static void kfree(void *ptr) {
}

static void pmm_init() {
  tmp = _heap.start;
  uintptr_t pmsize = ((uintptr_t)_heap.end - (uintptr_t)_heap.start);
  printf("Got %d MiB heap: [%p, %p)\n", pmsize >> 20, _heap.start, _heap.end);
}

MODULE_DEF(pmm) = {
  .init  = pmm_init,
  .alloc = kalloc,
  .free  = kfree,
};
