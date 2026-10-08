#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/mm.h>

static int __init get_cache_page_init(void)
{
    const char *filename = "data.bin";

    struct file *file;
    struct address_space *mapping;
    struct page *page;

    unsigned long pfn;

    /*
     * ------------------------------------------------------------
     * Open the file from kernel space.
     * ------------------------------------------------------------
     */

    file = filp_open(filename, O_RDONLY | O_LARGEFILE, 0);

    if (IS_ERR(file)) {
        pr_err("get_cache_page: filp_open failed\n");
        return PTR_ERR(file);
    }

    /*
     * ------------------------------------------------------------
     * Obtain the file's address_space.
     * ------------------------------------------------------------
     *
     * The address_space represents the mapping between the file
     * and its page-cache pages.
     */

    mapping = file_inode(file)->i_mapping;

    /*
     * ------------------------------------------------------------
     * Find the page-cache page for file offset 0.
     * ------------------------------------------------------------
     *
     * Page index 0 corresponds to file offset 0.
     */

    page = find_get_page(mapping, 0);

    if (page == NULL) {
        pr_err("get_cache_page: page is NOT in page cache\n");

        filp_close(file, NULL);
        return -ENOENT;
    }

    /*
     * ------------------------------------------------------------
     * Obtain the physical frame number.
     * ------------------------------------------------------------
     */

    pfn = page_to_pfn(page);

    pr_info("get_cache_page:\n");
    pr_info("  file            = %s\n", filename);
    pr_info("  file offset     = 0\n");
    pr_info("  page index      = 0\n");
    pr_info("  PFN             = %lu\n", pfn);
    pr_info("  physical addr   = 0x%lx\n",
            pfn << PAGE_SHIFT);

    /*
     * find_get_page() increments the page reference count.
     */
    put_page(page);

    filp_close(file, NULL);

    return 0;
}

static void __exit get_cache_page_exit(void)
{
    pr_info("get_cache_page: unloaded\n");
}

module_init(get_cache_page_init);
module_exit(get_cache_page_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("Example of finding a file page in the page cache");
