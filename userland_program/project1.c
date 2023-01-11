#include <dlfcn.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

// declare variables to print later
int data = 0x8763;
int bss = 0;
int *heap;  // initialized in main
__thread long tls;
void text() {}
void *lib;  // initialized in main

// thread data
typedef struct {
	int id;
	int sleep;
} task_t;

long get_phy_addr(void *p) {
	return syscall(8788, p);
}

typedef struct {
	uint64_t start;
	uint64_t end;
	uint64_t perm;
	char name[0x100];
} segment_t;

int get_segment(void *ptr, segment_t *out) {
	return syscall(8789, ptr, out);
}
void print_segment(segment_t *seg) {
	printf("%llx-%llx %llu %s\n", seg->start, seg->end, seg->perm, seg->name);
}

typedef struct {
	const char *name;
	void *addr;
} child_info_t;

void *child(void *arg) {
	long stack;
	task_t *task = arg;
	sleep(task->sleep);
	printf("Thread %d\n", task->id);

	child_info_t addrs[] = {
	    {"stack", &stack}, {"tls", &tls},   {"data", &data}, {"bss", &bss},
	    {"heap", heap},    {"text", &text}, {"lib", lib},
	};
	for (int i = 0; i < sizeof(addrs) / sizeof(addrs[0]); i++) {
		segment_t seg;
		printf("%s addr=%p phy=%p\n", addrs[i].name, addrs[i].addr,
		       (void *)get_phy_addr(addrs[i].addr));
		get_segment(addrs[i].addr, &seg);
		printf("\t");
		print_segment(&seg);
	}
	return NULL;
}

const int MAX_THREAD = 2;

int main() {
	heap = malloc(0x10);
	lib = &printf;

	pthread_t threads[MAX_THREAD];
	task_t tasks[MAX_THREAD];
	task_t main_task = {.id = 0, .sleep = 0};
	child(&main_task);
	for (int i = 0; i < MAX_THREAD; i++) {
		tasks[i].id = i + 1;
		tasks[i].sleep = i + 1;
		pthread_create(&threads[i], NULL, child, (void *)&tasks[i]);
	}
	// char buf[0x100];
	// sprintf(buf, "cat /proc/%d/maps", getpid());
	// system(buf);
	for (int i = 0; i < MAX_THREAD; i++) {
		pthread_join(threads[i], NULL);
	}
	return 0;
}
