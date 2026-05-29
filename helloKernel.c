#include <linux/module.h>
#include <linux/printk.h>

int init_module(void){ //This happens whenever the module is loaded into the kernel with insmod
	pr_info("Jesus don't want me for a sunbeam\n"); //printf basically, but it chucks it into the kernel log file not the terminal
	return 0; //Any other return is an error, just like regular c
}

void cleanup_module(){ //This runs when the module is REMOVED from the kernel with rmmod
	pr_info("Sunbeams are not made like me\n");
}

MODULE_LICENSE("GPL"); //Unsure? A flag for other kernel applications maybe?
