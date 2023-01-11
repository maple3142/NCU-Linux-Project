#include <linux/init_task.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/types.h>

typedef struct {
	uint64_t start;
	uint64_t end;
	uint64_t perm;
	char name[0x100];
} segment_t;

// see show_map_vma in fs/proc/task_mmu.c

asmlinkage unsigned long sys_get_segment(unsigned long vaddr, void *out) {
	struct vm_area_struct *vma = current->mm->mmap;
	struct mm_struct *mm;
	// printk("get segment %lx\n", vaddr);
	// printk("vma start=%p end=%p\n", vma->vm_start, vma->vm_end);
	// struct vm_area_struct *vma_next = vma->vm_next;
	// printk("vma->vm_next=%p\n", vma_next);
	// if(vma_next != NULL) {
	//     printk("vma_next start=%p end=%p\n", vma_next->vm_start, vma_next->vm_end);
	// }
	printk("sizeof(segment_t)=%d\n", sizeof(segment_t));
	segment_t seg;
	memset(&seg, 0, sizeof(segment_t));
	while (vma) {
		mm = vma->vm_mm;
		if (vma->vm_start <= vaddr && vaddr < vma->vm_end) {
			printk("found segment for %lx\n", vaddr);
			printk("vma start=%p end=%p file=%p\n", vma->vm_start, vma->vm_end,
			       vma->vm_file);
			seg.start = vma->vm_start;
			seg.end = vma->vm_end;
			if ((vma->vm_flags) & VM_READ) {
				seg.perm |= 0x4;
			}
			if ((vma->vm_flags) & VM_WRITE) {
				seg.perm |= 0x2;
			}
			if ((vma->vm_flags) & VM_EXEC) {
				seg.perm |= 0x1;
			}
			struct file *file = vma->vm_file;
			if (file) {
				// dentry_path_raw(file->f_path.dentry, seg.name, 256);
				char tmp[256];
				char *ret = d_path(&file->f_path, tmp, 256);
				// https://www.kernel.org/doc/htmldocs/filesystems/API-d-path.html
				// Note: Callers should use the returned pointer, not the passed in buffer, to use the name! The implementation often starts at an offset into the buffer, and may leave 0 bytes at the start.
				// QAQ
				printk("passed in buffer=%p\n", tmp);
				printk("d_path ret=%p\n", ret);
				printk("ret deref=%s\n", ret);
				strcpy(seg.name, ret);
			} else {
				char *name = arch_vma_name(vma);
				if (name) {
					strcpy(seg.name, name);
				} else {
					if (!mm) {
						strcpy(seg.name, "[vdso]");
					} else if (vma->vm_start <= mm->brk &&
					           vma->vm_end >= mm->start_brk) {
						strcpy(seg.name, "[heap]");
					}
					int is_pid = 1;
					int tid = vm_is_stack(current, vma, is_pid);
					if (tid != 0) {
						if (!is_pid || (vma->vm_start <= mm->start_stack &&
						                vma->vm_end >= mm->start_stack)) {
							strcpy(seg.name, "[stack]");
						} else {
							sprintf(seg.name, "[stack:%d]", tid);
						}
					}
				}
			}
			printk("segment name=%s\n", seg.name);
			copy_to_user(out, &seg, sizeof(segment_t));
			return 0;
		}
		vma = vma->vm_next;
	}
	return -1;
}
