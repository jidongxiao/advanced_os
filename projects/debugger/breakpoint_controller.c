#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/completion.h>
#include <linux/sched/debug.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Developer");
MODULE_DESCRIPTION("Interactive Kernel Debugger Controller");

static char symbol_name[KSYM_NAME_LEN] = "target_module_function";
module_param_string(symbol_name, symbol_name, sizeof(symbol_name), 0644);

static struct kprobe kp = {
    .symbol_name = symbol_name,
};

// Debugger state & synchronization
static DECLARE_COMPLETION(cmd_done);
static struct pt_regs *current_regs;
static bool single_step_mode = false;
static bool in_breakpoint = false;

/* --- KPROBE HANDLERS --- */

static int handler_pre(struct kprobe *p, struct pt_regs *regs) {
    current_regs = regs;
    in_breakpoint = true;

    pr_info("\n=========================================\n");
    pr_info("[KDBG] Breakpoint hit at %p (IP: %lx)\n", p->addr, regs->ip);
    pr_info("[KDBG] Type 'bt', 'ss', or 'c' via /dev/kdbg\n");
    pr_info("=========================================\n");

    // Block the target thread until user sends a command
    reinit_completion(&cmd_done);
    wait_for_completion(&cmd_done);

    in_breakpoint = false;
    return 0;
}

static void handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags) {
    pr_info("[KDBG] Single-step execution complete. New IP: %lx\n", regs->ip);
    
    if (single_step_mode) {
        // Pause again after single step
        current_regs = regs;
        in_breakpoint = true;
        pr_info("[KDBG] Single-stepped to IP: %lx. Awaiting next command...\n", regs->ip);
        
        reinit_completion(&cmd_done);
        wait_for_completion(&cmd_done);
        in_breakpoint = false;
    }
}

/* --- INTERACTIVE COMMAND INTERFACE (/dev/kdbg) --- */

static ssize_t kdbg_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    char kbuf[32];
    
    if (count >= sizeof(kbuf))
        return -EINVAL;

    if (copy_from_user(kbuf, buf, count))
        return -EFAULT;

    kbuf[count] = '\0';

    if (!in_breakpoint) {
        pr_info("[KDBG] Debugger is idle. No active breakpoint.\n");
        return count;
    }

    /* COMMAND: bt (Backtrace) */
    if (strncmp(kbuf, "bt", 2) == 0) {
        pr_info("--- [Stack Backtrace] ---\n");
        dump_stack();
        pr_info("-------------------------\n");
    } 
    /* COMMAND: ss (Single Step) */
    else if (strncmp(kbuf, "ss", 2) == 0) {
        pr_info("[KDBG] Single stepping one instruction...\n");
        single_step_mode = true;
        complete(&cmd_done); // Release target to step once
    } 
    /* COMMAND: c (Continue) */
    else if (strncmp(kbuf, "c", 1) == 0) {
        pr_info("[KDBG] Continuing execution...\n");
        single_step_mode = false;
        complete(&cmd_done); // Release target execution
    } 
    else {
        pr_info("[KDBG] Unknown command: %s. Commands: bt, ss, c\n", kbuf);
    }

    return count;
}

static const struct file_operations kdbg_fops = {
    .owner = THIS_MODULE,
    .write = kdbg_write,
};

static struct miscdevice kdbg_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = "kdbg",
    .fops  = &kdbg_fops,
    .mode  = 0666,
};

/* --- MODULE INIT / EXIT --- */

static int __init controller_init(void) {
    int ret;

    ret = misc_register(&kdbg_dev);
    if (ret) {
        pr_err("[KDBG] Failed to register /dev/kdbg device\n");
        return ret;
    }

    kp.pre_handler = handler_pre;
    kp.post_handler = handler_post;

    ret = register_kprobe(&kp);
    if (ret < 0) {
        misc_deregister(&kdbg_dev);
        pr_err("[KDBG] register_kprobe failed: %d\n", ret);
        return ret;
    }

    pr_info("[KDBG] Breakpoint registered at %s (%p). Interface: /dev/kdbg\n", symbol_name, kp.addr);
    return 0;
}

static void __exit controller_exit(void) {
    // Unblock any waiting process before unloading
    complete_all(&cmd_done);
    unregister_kprobe(&kp);
    misc_deregister(&kdbg_dev);
    pr_info("[KDBG] Debugger module unloaded.\n");
}

module_init(controller_init);
module_exit(controller_exit);
