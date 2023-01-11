#include <linux/kernel.h>

// https://blog.kaibro.tw/2016/11/07/Linux-Kernel%E7%B7%A8%E8%AD%AF-Ubuntu/

asmlinkage int sys_helloworld(void) {
    printk("konpeko!\n");
    return 1;
}
