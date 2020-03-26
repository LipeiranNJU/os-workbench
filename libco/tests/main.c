#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <assert.h>
#include <string.h>
#include <setjmp.h>
#include <stdint.h>
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
  if (co->status == CO_NEW) {
    stack_switch_call(&co->stack[STACK_SIZE], co->func, (uintptr_t)co->arg);

  }

  if (times > 1000) {
    return ;
  }

}

void co_yield() {
//   printf("TTT");
  // 显然使用stack_switch_call 来切换栈的，估计是把co最高地址/最高地址+1当做sp，func作为entry，arg作为参数
  // save data
  int val = setjmp(current->context);
//   printf("World\n");
//   assert(current->waiter != NULL);
//   printf("after setjmp current addr:%llx\tcurrent->waiter:%llx\n", (unsigned long long) (uintptr_t) (current), (unsigned long long) (uintptr_t) (current->waiter));
//   printf("current->waiter:%llx\n", ((unsigned long long)((uintptr_t)(current->waiter))));
//   assert(current->waiter != NULL);v
//   printf("UUU\n");
//   printf("Value:%d\n", val);
//   assert(current->waiter != NULL);
  if (val == 0) {
    // printf("VVV\n");
    co* tmp = current;
    if (tmp->waiter == NULL) {
    //   printf("$$$$$$$%s\n", tmp->name);
    //   printf("current addr:%llx\n", (unsigned long long) (uintptr_t) (tmp));
      // emmm 现在来分析一下为什么current明明就是thread1还是出了问题…在 thread2创建的时候，显然已经成果将thread1的waiter设置成
      // 了thread2，thread1是因为错误的setjmp出的问题吗
    }
    // assert(tmp->waiter != NULL);
    while (tmp->waiter != NULL) {
      tmp = tmp->waiter;
    }
    // printf("WWW\n");
    tmp->waiter = current;
    co* next = current->waiter;
    current->waiter = NULL;
    current = next;
    // printf("XXX\n");
    
    stack_switch_call(&current->stack[STACK_SIZE], next->func, (uintptr_t) next->arg);
    // printf("RRR\n");
    longjmp(next->context, 1);
    // 一个之前没考虑的问题，longjmp后怎么保证还能执行下面的调整栈顶
    // 因为跳转的是等待的协程， 所以之前等待的协程发出co_yield()的时候必然已经执行过保存了
    // 接下来那一个协程就能够自然而然地进入下面那个情形返回继续执行了
  } else {
    times++;
    return;
  }
}







struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}


static inline void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}


static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

static inline void __list_del(struct list_head *prev, struct list_head *next)
{
	next->prev = prev;
	prev->next = next;
}


static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = (void *) 0;
	entry->prev = (void *) 0;
}

static inline void list_del_init(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}


static inline void list_move(struct list_head *list, struct list_head *head)
{
        __list_del(list->prev, list->next);
        list_add(list, head);
}


static inline void list_move_tail(struct list_head *list,
				  struct list_head *head)
{
        __list_del(list->prev, list->next);
        list_add_tail(list, head);
}


static inline int list_empty(struct list_head *head)
{
	return head->next == head;
}

static inline void __list_splice(struct list_head *list,
				 struct list_head *head)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;
	struct list_head *at = head->next;

	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}


static inline void list_splice(struct list_head *list, struct list_head *head)
{
	if (!list_empty(list))
		__list_splice(list, head);
}


static inline void list_splice_init(struct list_head *list,
				    struct list_head *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head);
		INIT_LIST_HEAD(list);
	}
}

#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))


#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); \
        	pos = pos->next)

#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); \
        	pos = pos->prev)


#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)


#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = list_entry(pos->member.next, typeof(*pos), member))

#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_entry((head)->next, typeof(*pos), member),	\
		n = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head); 					\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))


typedef struct Queue_t {
    struct list_head list;
    int sz;
    int cap;
} Queue;

typedef struct Item_t {
    void *data;
    struct list_head link;
} Item;

static inline Queue* q_new() {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    if (!queue) {
        fprintf(stderr, "New queue failure\n");
        exit(1);
    }
    queue->cap = 100;
    queue->sz = 0;
    INIT_LIST_HEAD(&queue->list);
    return queue;
}

static inline void q_free(Queue *queue) {
    Item *pos, *next;
    list_for_each_entry_safe(pos, next, &queue->list, link) {
        list_del(&pos->link);
        free(pos);
    }
    free(queue);
}

static inline int q_is_full(Queue *queue) {
    return queue->sz == queue->cap;
}

static inline int q_is_empty(Queue *queue) {
    return list_empty(&queue->list);
}

static inline void q_push(Queue *queue, Item *item) {
    if (q_is_full(queue)) {
        fprintf(stderr, "Push queue failure\n");
        return;
    }
    list_add_tail(&item->link, &queue->list);
    queue->sz += 1;
}

static inline Item* q_pop(Queue *queue) {
    if (q_is_empty(queue)) {
        return NULL;
    }

    Item *item = list_entry(queue->list.next, Item, link);
    list_del(&item->link);

    queue->sz -= 1;
    return item;
}

extern uintptr_t arg1;
int g_count = 0;

static void add_count() {
    g_count++;
}

static int get_count() {
    return g_count;
}

static void work_loop(void *arg) {
    const char *s = (const char*)arg;
    for (int i = 0; i < 100; ++i) {
        // printf("hello\n");
        int i = get_count();
        // assert(0);
        printf("%s%d  ", s, i);
        add_count();
        co_yield();
    }
}

static void work(void *arg) {
    work_loop(arg);
}

static void test_1() {
    struct co *thd1 = co_start("thread-1", work, "X");
    struct co *thd2 = co_start("thread-2", work, "Y");
    co_wait(thd1);
    co_wait(thd2);

//    printf("\n");
}

// -----------------------------------------------

static int g_running = 1;

static void do_produce(Queue *queue) {
    assert(!q_is_full(queue));
    Item *item = (Item*)malloc(sizeof(Item));
    if (!item) {
        fprintf(stderr, "New item failure\n");
        return;
    }
    item->data = (char*)malloc(10);
    if (!item->data) {
        fprintf(stderr, "New data failure\n");
        free(item);
        return;
    }
    memset(item->data, 0, 10);
    sprintf(item->data, "libco-%d", g_count++);
    q_push(queue, item);
}

static void producer(void *arg) {
    Queue *queue = (Queue*)arg;
    for (int i = 0; i < 100; ) {
        if (!q_is_full(queue)) {
            // co_yield();
            do_produce(queue);
            i += 1;
        }
        co_yield();
    }
}

static void do_consume(Queue *queue) {
    assert(!q_is_empty(queue));

    Item *item = q_pop(queue);
    if (item) {
        printf("%s  ", (char *)item->data);
        free(item->data);
        free(item);
    }
}

static void consumer(void *arg) {
    Queue *queue = (Queue*)arg;
    while (g_running) {
        if (!q_is_empty(queue)) {
            do_consume(queue);
        }
        co_yield();
    }
}

static void test_2() {

    Queue *queue = q_new();

    struct co *thd1 = co_start("producer-1", producer, queue);
    struct co *thd2 = co_start("producer-2", producer, queue);
    struct co *thd3 = co_start("consumer-1", consumer, queue);
    struct co *thd4 = co_start("consumer-2", consumer, queue);

    co_wait(thd1);
    co_wait(thd2);

    g_running = 0;

    co_wait(thd3);
    co_wait(thd4);

    while (!q_is_empty(queue)) {
        do_consume(queue);
    }

    q_free(queue);
}

int main() {
    setbuf(stdout, NULL);

    printf("Test #1. Expect: (X|Y){0, 1, 2, ..., 199}\n");
    test_1();

    printf("\n\nTest #2. Expect: (libco-){200, 201, 202, ..., 399}\n");
    test_2();

    printf("\n\n");

    return 0;
}
