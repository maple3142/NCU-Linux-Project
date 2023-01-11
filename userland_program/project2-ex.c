#include <dlfcn.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

// declare variables to print later
int data = 0x8763;
int bss = 0;
int *heap;  // initialized in main
void text() {}
void *lib;  // initialized in main
long *custom; // initialized in main

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
	printf("Proecss %d\n", task->id);

	child_info_t addrs[] = {{"stack", &stack},  {"data", &data}, {"bss", &bss},
	                        {"heap", heap},     {"text", &text}, {"lib", lib},
	                        {"custom", custom}};
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

int main() {
	heap = malloc(0x10);
	lib = &printf;
	custom = mmap(NULL, 0x1000, PROT_READ | PROT_WRITE,
	              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	custom[0] = 0x90;
	printf("%p\n", custom);

	pid_t pid = fork();
	// trigger CoW
	// custom[0] = 0x87;
	if (pid == 0) {
		task_t child_task = {.id = 1, .sleep = 1};
		child(&child_task);
	} else {
		task_t main_task = {.id = 0, .sleep = 1};
		child(&main_task);
		wait(NULL);
	}
	return 0;
}
// gcc -m32 project2.c -o project2 -lpthread -Wall -fPIE
// ./project2 > out1 &; ./project2 > out2 &; colordiff out1 out2
