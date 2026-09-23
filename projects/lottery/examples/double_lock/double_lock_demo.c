#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Course Instructor");
MODULE_DESCRIPTION("Demo of ordered multi-lock acquisition and lockdep nesting");
MODULE_VERSION("1.0");

struct dummy_queue {
    int cpu_id;
    spinlock_t lock;
    int items;
};

static struct dummy_queue queue_a;
static struct dummy_queue queue_b;

/**
 * safe_double_lock_transfer - Safely lock two queues and transfer an item.
 * @q1: Pointer to the first queue
 * @q2: Pointer to the second queue
 */
static void safe_double_lock_transfer(struct dummy_queue *q1, struct dummy_queue *q2)
{
    unsigned long flags1, flags2;
    struct dummy_queue *first, *second;

    /*
     * 1. AB-BA DEADLOCK PREVENTION:
     * Enforce a strict global lock hierarchy based on CPU/Queue ID.
     * Regardless of which queue is passed as q1 or q2, we ALWAYS acquire
     * the lock with the lower ID first.
     */
    if (q1->cpu_id < q2->cpu_id) {
        first = q1;
        second = q2;
    } else {
        first = q2;
        second = q1;
    }

    /*
     * 2. LOCKDEP COMPLIANCE:
     * Use spin_lock_irqsave_nested with SINGLE_DEPTH_NESTING for the second lock.
     * This informs lockdep: "I am intentionally taking a 2nd lock of the same class."
     */
    spin_lock_irqsave(&first->lock, flags1);
    spin_lock_irqsave_nested(&second->lock, flags2, SINGLE_DEPTH_NESTING);

    /* Critical Section: Safe to access both queues simultaneously */
    pr_info("[double_lock_demo] Holding locks for CPU %d and CPU %d cleanly.\n",
            first->cpu_id, second->cpu_id);

    if (q1->items > 0) {
        q1->items--;
        q2->items++;
        pr_info("[double_lock_demo] Transferred item from CPU %d (%d remaining) to CPU %d (%d total)\n",
                q1->cpu_id, q1->items, q2->cpu_id, q2->items);
    }

    /*
     * 3. LIFO UNLOCK ORDER:
     * Release locks in exact reverse order of acquisition.
     */
    spin_unlock_irqrestore(&second->lock, flags2);
    spin_unlock_irqrestore(&first->lock, flags1);
}

static int __init double_lock_demo_init(void)
{
    pr_info("[double_lock_demo] Module loaded.\n");

    /* Initialize dummy queues */
    queue_a.cpu_id = 0;
    spin_lock_init(&queue_a.lock);
    queue_a.items = 10;

    queue_b.cpu_id = 1;
    spin_lock_init(&queue_b.lock);
    queue_b.items = 2;

    /* Test Case 1: Transfer direction A (CPU 0) -> B (CPU 1) */
    pr_info("[double_lock_demo] --- Test 1: Transfer CPU 0 -> CPU 1 ---\n");
    safe_double_lock_transfer(&queue_a, &queue_b);

    /* Test Case 2: Transfer direction B (CPU 1) -> A (CPU 0) */
    /* Ordered locking ensures CPU 0 lock is STILL acquired first! */
    pr_info("[double_lock_demo] --- Test 2: Transfer CPU 1 -> CPU 0 ---\n");
    safe_double_lock_transfer(&queue_b, &queue_a);

    return 0;
}

static void __exit double_lock_demo_exit(void)
{
    pr_info("[double_lock_demo] Module unloaded cleanly.\n");
}

module_init(double_lock_demo_init);
module_exit(double_lock_demo_exit);
