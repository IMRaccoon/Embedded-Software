#include <linux/init.h>
#include <linux/module.h>

extern int get_my_id(void);
extern int set_my_id(int id);

static int __init ch1_mod2_init(void) {
    printk(KERN_NOTICE "My Id : %d\n", get_my_id());
    set_my_id(201511292);
    printk(KERN_NOTICE "My Id : %d\n", get_my_id());
    return 0;
}

static void __exit ch1_mod2_exit(void) {}

module_init(ch1_mod2_init);
module_exit(ch1_mod2_exit);