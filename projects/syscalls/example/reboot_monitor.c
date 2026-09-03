#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/cred.h>
#include <linux/sched.h>
#include <linux/ptrace.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OS Course Staff");
MODULE_DESCRIPTION("Sample Kprobe Interceptor for sys_reboot");

/* --------------------------------------------------------------------------
 * Portable Architecture Symbol & Argument Helpers
 * -------------------------------------------------------------------------- */
#if defined(__x86_64__)
    #define SYSCALL_SYMBOL "__x64_sys_reboot"
    /* On x86_64: 1st arg (magic1) is in di, 4th arg (passphrase pointer) is in r10 */
    #define GET_MAGIC1(pt)         (((struct pt_regs *)(pt)->di)->di)
    #define GET_SYSCALL_ARG4(pt)   ((void __user *)((struct pt_regs *)(pt)->di)->r10)

#elif defined(__aarch64__)
    #define SYSCALL_SYMBOL "__arm64_sys_reboot"
    /* On arm64: 1st arg (magic1) is in x0 (regs[0]), 4th arg is in x3 (regs[3]) */
    #define GET_MAGIC1(pt)         (((struct pt_regs *)(pt)->regs[0])->regs[0])
    #define GET_SYSCALL_ARG4(pt)   ((void __user *)((struct pt_regs *)(pt)->regs[0])->regs[3])

#else
    #error "Unsupported architecture"
#endif

/*
 * Pre-Handler: Runs immediately upon entry into sys_reboot, before any
 * internal kernel validation or execution occurs.
 */
static int reboot_pre_handler(struct kprobe *p, struct pt_regs *regs)
{
    /* Get caller information using standard kernel helpers */
    uid_t uid = __kuid_val(current_uid());
    pid_t pid = current->pid;

    /* Extract system call arguments from pt_regs using our helpers */
    unsigned long magic1 = GET_MAGIC1(regs);
    void __user *arg4 = GET_SYSCALL_ARG4(regs);

    pr_info("[RebootMonitor] Process '%s' (PID %d, UID %d) invoked reboot()\n",
            current->comm, pid, uid);
    pr_info("[RebootMonitor]   -> magic1: 0x%lx, arg4 pointer: %p\n", 
            magic1, arg4);

    /* Demonstration audit check */
    if (uid != 0) {
        pr_warn("[RebootMonitor] Security alert: Non-root user (UID %d) attempted reboot!\n", uid);
    }

    return 0; /* Returning 0 allows sys_reboot execution to proceed */
}

static struct kprobe my_kprobe = {
    .pre_handler = reboot_pre_handler,
    .symbol_name = SYSCALL_SYMBOL,
};

static int __init monitor_init(void)
{
    int ret = register_kprobe(&my_kprobe);
    if (ret < 0) {
        pr_err("[RebootMonitor] Failed to register kprobe on %s: %d\n", 
               SYSCALL_SYMBOL, ret);
        return ret;
    }
    pr_info("[RebootMonitor] Loaded successfully. Monitoring reboot syscall entry...\n");
    return 0;
}

static void __exit monitor_exit(void)
{
    unregister_kprobe(&my_kprobe);
    pr_info("[RebootMonitor] Unloaded successfully.\n");
}

module_init(monitor_init);
module_exit(monitor_exit);
