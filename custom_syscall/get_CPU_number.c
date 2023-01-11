#include <linux/init_task.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <../kernel/sched/sched.h>


asmlinkage unsigned long sys_get_CPU_number(unsigned long x) {
	return current_thread_info()->cpu;
}
