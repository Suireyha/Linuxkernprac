#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
static int arr[6] = {0, 0, 0, 0, 0, 0};
static int arr_argc = 0; //Used to index the array I'm about to make
module_param_array(arr, int, &arr_argc, 0); //


static int __init start(void){
	if(arr[0] == NULL){
		pr_info("Error: Parameter array empty!\n");
		return 0;
	}
	char str[50];
	int pos = 0;
	int sum = 0;
	pos += snprintf(str + pos, sizeof(str) - pos, "The sum of: ");
	for(int i = 0; i < arr_argc; i++){
		sum += arr[i];
		pos += snprintf(str + pos, sizeof(str) - pos, "%d", arr[i]);
		if((i + 1) != arr_argc){
			pos += snprintf(str + pos, sizeof(str) - pos, ",");
		}
		pos += snprintf(str + pos, sizeof(str) - pos, " ");
	}
	pos += snprintf(str + pos, sizeof(str) - pos, "= %d\n", sum);
	pr_info("%s", str);
	return 0;
}

static void __exit end(void){
	pr_info("Kernel sum ended!\n");
}

module_init(start);
module_exit(end);
