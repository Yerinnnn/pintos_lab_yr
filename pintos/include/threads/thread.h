#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include "threads/interrupt.h"
#ifdef VM
#include "vm/vm.h"
#endif

/* States in a thread's life cycle. */
enum thread_status
{
	THREAD_RUNNING, /* Running thread. */
	THREAD_READY,		/* Not running but ready to run. */
	THREAD_BLOCKED, /* Waiting for an event to trigger. */
	THREAD_DYING		/* About to be destroyed. */
};

/* Thread identifier type.
	 You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) - 1) /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0			 /* Lowest priority. */
#define PRI_DEFAULT 31 /* Default priority. */
#define PRI_MAX 63		 /* Highest priority. */

/* A kernel thread or user process.
 *
 * Each thread structure is stored in its own 4 kB page.  The
 * thread structure itself sits at the very bottom of the page
 * (at offset 0).  The rest of the page is reserved for the
 * thread's kernel stack, which grows downward from the top of
 * the page (at offset 4 kB).  Here's an illustration:
 *
 *      4 kB +---------------------------------+
 *           |          kernel stack           |
 *           |                |                |
 *           |                |                |
 *           |                V                |
 *           |         grows downward          |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           +---------------------------------+
 *           |              magic              |
 *           |            intr_frame           |
 *           |                :                |
 *           |                :                |
 *           |               name              |
 *           |              status             |
 *      0 kB +---------------------------------+
 *
 * The upshot of this is twofold:
 *
 *    1. First, `struct thread' must not be allowed to grow too
 *       big.  If it does, then there will not be enough room for
 *       the kernel stack.  Our base `struct thread' is only a
 *       few bytes in size.  It probably should stay well under 1
 *       kB.
 *
 *    2. Second, kernel stacks must not be allowed to grow too
 *       large.  If a stack overflows, it will corrupt the thread
 *       state.  Thus, kernel functions should not allocate large
 *       structures or arrays as non-static local variables.  Use
 *       dynamic allocation with malloc() or palloc_get_page()
 *       instead.
 *
 * The first symptom of either of these problems will probably be
 * an assertion failure in thread_current(), which checks that
 * the `magic' member of the running thread's `struct thread' is
 * set to THREAD_MAGIC.  Stack overflow will normally change this
 * value, triggering the assertion. */

/* 커널 스레드 또는 사용자 프로세스.

각 스레드 구조체는 자체적인 4KB 페이지에 저장됩니다.
스레드 구조체 자체는 페이지의 가장 아래(오프셋 0)에 위치합니다.
페이지의 나머지 부분은 스레드의 커널 스택을 위해 예약되어 있으며,
이는 페이지의 상단(오프셋 4KB)에서 아래로 성장합니다. 다음은 그림입니다:

이러한 구성의 결과는 두 가지입니다:

1. 첫째, struct thread가 너무 커지지 않도록 해야 합니다.
	만약 커지게 되면, 커널 스택을 위한 공간이 충분하지 않게 됩니다.
	우리의 기본 struct thread는 크기가 단지 몇 바이트에 불과합니다.
	아마도 1KB보다 훨씬 작게 유지되어야 할 것입니다.

2. 둘째, 커널 스택이 너무 커지지 않도록 해야 합니다.
	스택이 오버플로우되면, 스레드 상태를 손상시킬 것입니다.
	따라서 커널 함수는 큰 구조체나 배열을 비정적(non-static) 지역 변수로 할당해서는 안 됩니다.
	대신 malloc() 또는 palloc_get_page()를 사용하여 동적 할당을 사용하십시오.

이러한 문제 중 하나의 첫 번째 증상은 아마도 thread_current()에서 어설션 실패로 나타날 것입니다.
thread_current()는 실행 중인 스레드의 struct thread 내 magic 멤버가 THREAD_MAGIC으로 설정되어 있는지 확인합니다.
스택 오버플로우는 일반적으로 이 값을 변경하여 어설션을 트리거할 것입니다.
*/

/* The `elem' member has a dual purpose.  It can be an element in
 * the run queue (thread.c), or it can be an element in a
 * semaphore wait list (synch.c).  It can be used these two ways
 * only because they are mutually exclusive: only a thread in the
 * ready state is on the run queue, whereas only a thread in the
 * blocked state is on a semaphore wait list. */
/* elem 멤버는 이중 용도를 가집니다. 이는 실행 큐(run queue, thread.c에 구현)의 요소가 될 수도 있고,
	세마포어 대기 리스트(synch.c에 구현)의 요소가 될 수도 있습니다.
	이 두 가지 방식으로 사용될 수 있는 유일한 이유는 상호 배타적(mutually exclusive)이기 때문입니다:
	준비(ready) 상태의 스레드만이 실행 큐에 있으며, 블록(blocked) 상태의 스레드만이 세마포어 대기 리스트에 있습니다. */
struct thread
{
	/* Owned by thread.c. */
	tid_t tid;								 /* Thread identifier. */
	enum thread_status status; /* Thread state. */
	char name[16];						 /* Name (for debugging purposes). */
	int priority;							 /* Priority. */
	int effective_priority;
	int64_t tick;

	/* Shared between thread.c and synch.c. */
	struct list_elem elem; /* List element. */

#ifdef USERPROG
	/* Owned by userprog/process.c. */
	uint64_t *pml4; /* Page map level 4 */
#endif
#ifdef VM
	/* Table for whole virtual memory owned by thread. */
	struct supplemental_page_table spt;
#endif

	/* Owned by thread.c. */
	struct intr_frame tf; /* Information for switching */
	unsigned magic;				/* Detects stack overflow. */
};

/* If false (default), use round-robin scheduler.
	 If true, use multi-level feedback queue scheduler.
	 Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init(void);
void thread_start(void);

void thread_tick(void);
void thread_print_stats(void);

typedef void thread_func(void *aux);
tid_t thread_create(const char *name, int priority, thread_func *, void *);

bool priority_high_cmp(const struct list_elem *a,
											 const struct list_elem *b,
											 void *aux UNUSED);

void thread_block(void);
void thread_unblock(struct thread *);

struct thread *thread_current(void);
tid_t thread_tid(void);
const char *thread_name(void);

void thread_exit(void) NO_RETURN;
void thread_yield(void);

int thread_get_priority(void);
void thread_set_priority(int);

int thread_get_nice(void);
void thread_set_nice(int);
int thread_get_recent_cpu(void);
int thread_get_load_avg(void);

void do_iret(struct intr_frame *tf);

#endif /* threads/thread.h */
