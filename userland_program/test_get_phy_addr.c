#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

int data = 0x8763;
int bss = 0;
int *heap;  // initialized in main
__thread long tls;

void text() {}
void *lib;

typedef struct {
	int id;
	int sleep;
} task_t;

typedef struct {
	long start;
	long end;
	int perm;
	char name[0x100];
} segment_t;

long get_phy_addr(void *p) {
	return syscall(8788, p);
}
void print_segment(segment_t *seg) {
	if (seg != NULL)
		printf("%lx-%lx %d %s\n", seg->start, seg->end, seg->perm, seg->name);
	else
		printf("NULL Segement\n");
}
segment_t **read_segments() {
	char pathname[100];
	sprintf(pathname, "/proc/%d/maps", getpid());
	FILE *fp = fopen(pathname, "r");

	segment_t **segments =
	    malloc(0x100 * sizeof(segment_t *));  // assuming max 0x100 segments
	int i = 0;
	while (1) {
		segment_t *seg = malloc(sizeof(segment_t));
		char perm[10];  // rwxp
		if (fscanf(fp, "%lx-%lx %4c %*s %*s %*[0-9 ]%256[^\n]\n", &seg->start,
		           &seg->end, perm, seg->name) == EOF) {
			free(seg);
			break;
		}
		if (perm[0] == 'r')
			seg->perm |= PROT_READ;
		if (perm[1] == 'w')
			seg->perm |= PROT_WRITE;
		if (perm[2] == 'x')
			seg->perm |= PROT_EXEC;
		segments[i] = seg;
		i++;
	}
	segments[i] = NULL;
	fclose(fp);
	return segments;
}
void free_segments(segment_t **segments) {
	for (int i = 0; segments[i]; i++) {
		free(segments[i]);
	}
	free(segments);
}
segment_t *find_segment(long addr, segment_t **segments) {
	for (int i = 0; segments[i]; i++) {
		if (segments[i]->start <= addr && addr < segments[i]->end) {
			return segments[i];
		}
	}
	return NULL;
}

void *child(void *arg) {
	long stack;
	task_t *task = arg;
	sleep(task->sleep);
	printf("Thread %d\n", task->id);
	printf("stack addr=%p phy=%lx\n", &stack, get_phy_addr(&stack));
	printf("tls addr=%p phy=%lx\n", &tls, get_phy_addr(&tls));
	printf("data addr=%p phy=%lx\n", &data, get_phy_addr(&data));
	printf("bss addr=%p phy=%lx\n", &bss, get_phy_addr(&bss));
	printf("heap addr=%p phy=%lx\n", heap, get_phy_addr(heap));
	printf("text addr=%p phy=%lx\n", &text, get_phy_addr(&text));
	printf("lib addr=%p phy=%lx\n", lib, get_phy_addr(lib));

	segment_t **segments = read_segments();
	print_segment(find_segment((long)&stack, segments));
	free_segments(segments);
	return NULL;
}

const int MAX_THREAD = 2;

int main() {
	heap = malloc(0x10);
	lib = &printf;

	pthread_t threads[MAX_THREAD];
	task_t tasks[MAX_THREAD];
	task_t main_task = {0, 0};
	child(&main_task);
	for (int i = 0; i < MAX_THREAD; i++) {
		tasks[i].id = i + 1;
		tasks[i].sleep = i + 1;
		pthread_create(&threads[i], NULL, child, (void *)&tasks[i]);
	}
	for (int i = 0; i < MAX_THREAD; i++) {
		pthread_join(threads[i], NULL);
	}
	return 0;
}
