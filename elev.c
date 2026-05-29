#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");

static int __init start(void){

}

static void __exit end(void){

}

module_init(start);
module_exit(end);
