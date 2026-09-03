#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/cred.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("OS Course Staff");
MODULE_DESCRIPTION("Sample Kretprobe Interceptor for sys_reboot");

/*
 * Return Handler: Runs as sys_reboot returns its result back to user space.
 */
static int reboot_ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    /* Fetch the return value of sys_reboot */
    int retval = regs_return_value(regs);
    
    /* Get caller information using kernel helpers */
    uid_t uid = __kuid_val(current_uid());
    pid_t pid = current->pid;

    pr_info("[RebootMonitor] Process '%s' (PID %d, UID %d) called reboot(). Kernel returned: %d\n",
            current->comm, pid, uid, retval);

    /* 
     * Demonstration: If an unauthorized user attempted a reboot, 
     * we log a warning to dmesg.
     */
    if (retval == -EPERM) {
        pr_warn("[RebootMonitor] Security alert: Unauthorized reboot attempt detected!\n");
    }

    return 0;
}

static struct kretprobe my_kretprobe = {
    .handler        = reboot_ret_handler,
    /* Works out-of-the-box across both x86_64 and arm64 */
#if defined(__x86_64__)
    .kp.symbol_name = "__x64_sys_reboot",
#elif defined(__aarch64__)
    .kp.symbol_name = "__arm64_sys_reboot",
#endif
};

static int __init monitor_init(void)
{
    int ret = register_kretprobe(&my_kretprobe);
    if (ret < 0) {
        pr_err("[RebootMonitor] Failed to register kretprobe: %d\n", ret);
        return ret;
    }
    pr_info("[RebootMonitor] Loaded successfully. Monitoring reboot syscalls...\n");
    return 0;
}

static void __exit monitor_exit(void)
{
    unregister_kretprobe(&my_kretprobe);
    pr_info("[RebootMonitor] Unloaded successfully.\n");
}

module_init(monitor_init);
module_exit(monitor_exit);
