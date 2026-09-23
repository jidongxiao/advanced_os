#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Course Instructor");
MODULE_DESCRIPTION("Demo comparing list_add_tail and list_move_tail");
MODULE_VERSION("1.0");

/* Custom data structure containing a list_head node */
struct item_node {
    int id;
    struct list_head node;
};

/* Define two distinct list heads representing Queue A and Queue B */
static LIST_HEAD(queue_a);
static LIST_HEAD(queue_b);

static void print_queue(const char *name, struct list_head *head)
{
    struct item_node *entry;
    pr_info("[list_demo] %s contents: ", name);
    if (list_empty(head)) {
        pr_cont("(empty)\n");
        return;
    }
    list_for_each_entry(entry, head, node) {
        pr_cont("[%d] -> ", entry->id);
    }
    pr_cont("END\n");
}

static int __init list_demo_init(void)
{
    struct item_node *item1, *item2, *item_to_move = NULL;

    pr_info("[list_demo] Module loaded.\n");

    /* -------------------------------------------------------------
     * 1. DEMO: list_add_tail
     * Used for NEWLY allocated items that are not in any list yet.
     * ------------------------------------------------------------- */
    item1 = kmalloc(sizeof(*item1), GFP_KERNEL);
    if (!item1) return -ENOMEM;
    item1->id = 101;

    item2 = kmalloc(sizeof(*item2), GFP_KERNEL);
    if (!item2) {
        kfree(item1);
        return -ENOMEM;
    }
    item2->id = 202;

    /* Add brand-new items to Queue A */
    list_add_tail(&item1->node, &queue_a);
    list_add_tail(&item2->node, &queue_a);

    pr_info("[list_demo] === Phase 1: Added items to Queue A using list_add_tail ===");
    print_queue("Queue A", &queue_a);
    print_queue("Queue B", &queue_b);

    /* -------------------------------------------------------------
     * 2. DEMO: list_move_tail
     * Used to MOVE an EXISTING item from Queue A to Queue B.
     * It detaches the node from Queue A and attaches it to Queue B.
     * ------------------------------------------------------------- */
    if (!list_empty(&queue_a)) {
        /* Grab the first item in Queue A to migrate */
        item_to_move = list_first_entry(&queue_a, struct item_node, node);

        /* Move it from Queue A to Queue B */
        list_move_tail(&item_to_move->node, &queue_b);
    }

    pr_info("[list_demo] === Phase 2: Moved Item %d to Queue B using list_move_tail ===",
            item_to_move ? item_to_move->id : -1);
    print_queue("Queue A", &queue_a);
    print_queue("Queue B", &queue_b);

    return 0;
}

static void __exit list_demo_exit(void)
{
    struct item_node *entry, *tmp;

    /* Clean up remaining items in Queue A */
    list_for_each_entry_safe(entry, tmp, &queue_a, node) {
        list_del(&entry->node);
        kfree(entry);
    }

    /* Clean up remaining items in Queue B */
    list_for_each_entry_safe(entry, tmp, &queue_b, node) {
        list_del(&entry->node);
        kfree(entry);
    }

    pr_info("[list_demo] Cleaned up lists and unloaded module.\n");
}

module_init(list_demo_init);
module_exit(list_demo_exit);
