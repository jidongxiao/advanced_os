#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/percpu.h>
#include <linux/smp.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Course Instructor");
MODULE_DESCRIPTION("Minimal Demonstration of Per-CPU Memory Management");
MODULE_VERSION("1.0");

/* Custom structure to hold per-CPU metric counts */
struct cpu_stats {
    unsigned long process_count;
    unsigned long total_weight;
};

/* Pointer to hold the allocated per-CPU memory base address */
static struct cpu_stats __percpu *global_stats;

static int __init percpu_demo_init(void)
{
    int cpu;

    pr_info("percpu_demo: Initializing module\n");

    /* Dynamically allocate an instance of struct cpu_stats for each online CPU */
    global_stats = alloc_percpu(struct cpu_stats);
    if (!global_stats) {
        pr_err("percpu_demo: Failed to allocate per-CPU memory\n");
        return -ENOMEM;
    }

    /* Initialize values across all online CPUs */
    for_each_online_cpu(cpu) {
        struct cpu_stats *s = per_cpu_ptr(global_stats, cpu);
        s->process_count = cpu * 2;
        s->total_weight = (cpu + 1) * 100;

        pr_info("percpu_demo: CPU %d initialized -> process_count: %lu, total_weight: %lu\n",
                cpu, s->process_count, s->total_weight);
    }

    return 0;
}

static void __exit percpu_demo_exit(void)
{
    pr_info("percpu_demo: Cleaning up module\n");

    /* Free the dynamically allocated per-CPU memory array */
    if (global_stats) {
        free_percpu(global_stats);
        global_stats = NULL;
    }

    pr_info("percpu_demo: Module unloaded successfully\n");
}

module_init(percpu_demo_init);
module_exit(percpu_demo_exit);
