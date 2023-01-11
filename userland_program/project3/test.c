#include <dlfcn.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#define N_THREADS 8

int get_CPU_number() {
	return syscall(8790);
}
void *child(void *p) {
	unsigned int i = 1;
	while (1) {
		if (i % 1000000000 == 0) {
			printf("Thread %d: %d\n", p, get_CPU_number());
		}
		i += 1;
	}
}
int main() {
	pthread_t thread[N_THREADS];
	for (int i = 0; i < N_THREADS; i++) {
        printf("=== Thread %d created ===\n", i);
		pthread_create(&thread[i], NULL, child, i);
		sleep(4);
	}
    sleep(10);
    return 0;
}
// gcc test.c -m32 -lpthread && ./a.out
