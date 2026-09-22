#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Developer");
MODULE_DESCRIPTION("Kernel Module Breakpoint Controller");

// Name of the function in the target module you want to instrument
static char symbol_name[KSYM_NAME_LEN] = "target_module_function";
module_param_string(symbol_name, symbol_name, sizeof(symbol_name), 0644);

static struct kprobe kp = {
    .symbol_name = symbol_name,
};

/*
 * pre_handler: Triggered when CPU hits the breakpoint instruction 
 * right BEFORE the target instruction executes.
 */
static int handler_pre(struct kprobe *p, struct pt_regs *regs) {
#if defined(__x86_64__)
    pr_info("[Controller] Breakpoint hit at %p. IP: %lx, RAX: %lx\n",
            p->addr, regs->ip, regs->ax);
#elif defined(__aarch64__)
    pr_info("[Controller] Breakpoint hit at %p. PC: %lx, X0: %lx\n",
            p->addr, regs->pc, regs->regs[0]);
#endif

    /* 
     * Returning 0 tells kprobes to proceed with single-stepping 
     * the original instruction out-of-line.
     */
    return 0;
}

/*
 * post_handler: Triggered immediately AFTER the single instruction 
 * at the breakpoint has been executed out-of-line.
 */
static void handler_post(struct kprobe *p, struct pt_regs *regs, unsigned long flags) {
#if defined(__x86_64__)
    pr_info("[Controller] Single-step complete. New IP: %lx\n", regs->ip);
#elif defined(__aarch64__)
    pr_info("[Controller] Single-step complete. New PC: %lx\n", regs->pc);
#endif
}

static int __init controller_init(void) {
    int ret;

    kp.pre_handler = handler_pre;
    kp.post_handler = handler_post;

    ret = register_kprobe(&kp);
    if (ret < 0) {
        pr_err("[Controller] register_kprobe failed, returned %d\n", ret);
        return ret;
    }

    pr_info("[Controller] Placed breakpoint at %s (%p)\n", kp.symbol_name, kp.addr);
    return 0;
}

static void __exit controller_exit(void) {
    unregister_kprobe(&kp);
    pr_info("[Controller] Breakpoint removed from %s\n", kp.symbol_name);
}

module_init(controller_init);
module_exit(controller_exit);
