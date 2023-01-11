#include <../kernel/sched/sched.h>
#include <linux/init_task.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/uaccess.h>

asmlinkage unsigned long sys_start_to_count_number_of_process_switches(
    unsigned long x) {
	printk("start count %d\n", current->pid);
	current->num_process_switches = 0;
	return 0;
}
asmlinkage unsigned long sys_stop_to_count_number_of_process_switches(
    unsigned long x) {
	printk("stop count %d\n", current->pid);
	return current->num_process_switches;
}
