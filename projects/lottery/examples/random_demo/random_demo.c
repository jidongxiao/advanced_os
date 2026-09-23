#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/random.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Course Instructor");
MODULE_DESCRIPTION("Minimal kernel random number generation demo");
MODULE_VERSION("1.0");

static int __init random_demo_init(void)
{
    unsigned long total_tickets = 100;
    unsigned long rand_ticket;

    pr_info("[random_demo] Module loaded.\n");

    /*
     * Fill raw bytes into rand_ticket using kernel CSPRNG entropy.
     */
    get_random_bytes(&rand_ticket, sizeof(rand_ticket));

    /*
     * Bound to range [1, total_tickets]
     * Modulo gives [0, total_tickets - 1], adding 1 shifts to [1, total_tickets].
     */
    rand_ticket = (rand_ticket % total_tickets) + 1;

    pr_info("[random_demo] Total Pool: %lu | Drawn Winning Ticket: %lu\n",
            total_tickets, rand_ticket);

    return 0;
}

static void __exit random_demo_exit(void)
{
    pr_info("[random_demo] Module unloaded.\n");
}

module_init(random_demo_init);
module_exit(random_demo_exit);
