#include <dlfcn.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#define N_THREADS 30

int startpc() {
	return syscall(8791);
}
int stoppc() {
	return syscall(8792);
}
int main() {
	startpc();
    int cnt = 0;
	time_t start = time(NULL);
	while (1) {

        // IO Bound
		// usleep(10);
        // printf("%d\n", cnt);

        cnt += 1;

		time_t end = time(NULL);
		if (end - start >= 2 * 60) {
			break;
		}
	}
	int r = stoppc();
	printf("%d\n", r);
	return 0;
}
// gcc test2.c -m32 -lpthread && ./a.out
