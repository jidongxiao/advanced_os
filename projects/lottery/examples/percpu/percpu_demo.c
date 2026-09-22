#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>      /* For kmalloc(), kfree() */
#include <linux/percpu.h>    /* For alloc_percpu(), free_percpu(), per_cpu_ptr() */
#include <linux/spinlock.h>  /* For spinlock_t, spin_lock_init(), spin_lock(), spin_unlock() */
#include <linux/list.h>      /* For struct list_head, INIT_LIST_HEAD(), list_add_tail(), list_for_each_entry_safe() */
#include <linux/smp.h>       /* For for_each_online_cpu() */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Course Instructor");
MODULE_DESCRIPTION("Demonstration of Per-CPU Linked List Operations");

/* Element to be stored inside the per-CPU list */
struct work_item {
    int id;
    struct list_head node; /* Embedded list node */
};

/* Per-CPU container structure */
struct demo_cpu_data {
    spinlock_t lock;         /* Per-core lock protecting 'items' */
    struct list_head items;  /* Head node for this CPU's doubly linked list */
    unsigned int item_count; /* Number of active elements in 'items' */
};

/* Base per-CPU pointer */
static struct demo_cpu_data __percpu *percpu_data;

static int __init demo_init(void)
{
    int cpu;

    pr_info("percpu_demo: Initializing per-CPU structures\n");

    /* 1. Allocate per-CPU structures */
    percpu_data = alloc_percpu(struct demo_cpu_data);
    if (!percpu_data) {
        pr_err("percpu_demo: Per-CPU allocation failed\n");
        return -ENOMEM;
    }

    /* 2. Initialize queues and populate sample nodes */
    for_each_online_cpu(cpu) {
        struct demo_cpu_data *d = per_cpu_ptr(percpu_data, cpu);
        int i;

        spin_lock_init(&d->lock);
        INIT_LIST_HEAD(&d->items);
        d->item_count = 0;

        /* Lock the per-CPU queue before modifying the list */
        spin_lock(&d->lock);

        /* Add 2 dummy work items to this core's list */
        for (i = 1; i <= 2; i++) {
            struct work_item *item = kmalloc(sizeof(*item), GFP_KERNEL);
            if (!item)
                continue;

            item->id = (cpu + 1) * 100 + i;

            /* Append item to the tail of this core's list */
            list_add_tail(&item->node, &d->items);
            d->item_count++;
        }

        spin_unlock(&d->lock);

        pr_info("percpu_demo: CPU %d initialized with %u items\n", cpu, d->item_count);
    }

    /* 3. Traversal Example: Read items on each CPU list */
    for_each_online_cpu(cpu) {
        struct demo_cpu_data *d = per_cpu_ptr(percpu_data, cpu);
        struct work_item *item;

        spin_lock(&d->lock);

        /* Iterate through list entries safely (Read-only) */
        list_for_each_entry(item, &d->items, node) {
            pr_info("percpu_demo: CPU %d has work_item ID %d\n", cpu, item->id);
        }

        spin_unlock(&d->lock);
    }

    return 0;
}

static void __exit demo_exit(void)
{
    int cpu;

    pr_info("percpu_demo: Cleaning up and deleting items\n");

    if (percpu_data) {
        /* 4. Deletion Example: Free list elements before freeing the per-CPU array */
        for_each_online_cpu(cpu) {
            struct demo_cpu_data *d = per_cpu_ptr(percpu_data, cpu);
            struct work_item *item, *tmp;

            spin_lock(&d->lock);

            /* Must use list_for_each_entry_safe when deleting elements during traversal! */
            list_for_each_entry_safe(item, tmp, &d->items, node) {
                list_del(&item->node);
                kfree(item);
            }
            d->item_count = 0;

            spin_unlock(&d->lock);
        }

        /* Free the per-CPU container array */
        free_percpu(percpu_data);
        percpu_data = NULL;
    }

    pr_info("percpu_demo: Module unloaded cleanly\n");
}

module_init(demo_init);
module_exit(demo_exit);
