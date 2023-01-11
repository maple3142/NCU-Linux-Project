#include <unistd.h>
#include <stdio.h>

int main() {
    int ret = syscall(8787);
    printf("%d\n", ret);
    return 0;
}
