#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

static int __init second_init(void){
	pr_info("I don't know why you 'goodbye'\n");
	return 0;
}

static void __exit second_exit(void){
	pr_info("I say hello!\n");
}

module_init(second_init);
module_exit(second_exit);

MODULE_LICENSE("GPL");
