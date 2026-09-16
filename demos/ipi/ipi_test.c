#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/smp.h>

static void my_function(void *info)
{
    pr_info("IPI TEST: my_function() is running on Linux CPU %d\n",
            smp_processor_id());
}

static int __init ipi_test_init(void)
{
    pr_info("IPI TEST: module init running on CPU %d\n",
            smp_processor_id());

    smp_call_function_single(1, my_function, NULL, 1);

    pr_info("IPI TEST: smp_call_function_single() returned on CPU %d\n",
            smp_processor_id());

    return 0;
}

static void __exit ipi_test_exit(void)
{
    pr_info("IPI TEST: module unloaded\n");
}

module_init(ipi_test_init);
module_exit(ipi_test_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("IPI demonstration using smp_call_function_single");
