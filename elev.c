#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
//#include <linux/.h>
//#include <linux/.h>
//#include <linux/.h>
#define DEVICE_NAME "elevator"

MODULE_LICENSE("GPL"); //Public license to use everything

//Function signatures for fops stuff =]
static ssize_t elevator_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t elevator_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);
static int elevator_open(struct inode *inode, struct file *filp);
static int elevator_release(struct inode *inode, struct file *filp);

static struct cdev elevator_cdev;
static dev_t dev_num; //ACTUAL Major & Minor
static struct class *cls;

static int major_number = 0; //This drivers major number assigned by user (basically an ID)
static int dev_quantity = 4; //Number of elevators
static int floor_qty = 10; //The TOTAL number of floors (including underground floors)
static int underground_qty = 2; //The number of floors that are underground
//Actual floors are from -undergroun_qty -> floor_qty - (1 + underground_qty)
module_param(major_number, int, 0644);
module_param(dev_quantity, int, 0644);
module_param(floor_qty, int, 0644);
module_param(underground_qty, int, 0644);



static struct file_operations fops = {
	.read = elevator_read,
	.write = elevator_write,
	.open = elevator_open,
	.release = elevator_release,
};

static int __init start(void){
	pr_info("Device %s inserted\n", DEVICE_NAME);
	int err;
	if(major_number == 0){//User ddin't change pass a major_number to use or passed 0 (reserved by other shit)
		err = alloc_chrdev_region(&dev_num, 0, dev_quantity, DEVICE_NAME);
	}
	else{
		dev_num = MKDEV(major_number, 0);
		err = register_chrdev_region(dev_num, dev_quantity, DEVICE_NAME);
	}

	if(err < 0){
		pr_err("Failed to allocate chrdev region!\n");
		return err;
	}

	cls = class_create(DEVICE_NAME);
	if(IS_ERR(cls)){
		pr_err("Failed to create class for elevator!\n");
		unregister_chrdev_region(dev_num, dev_quantity);
		return PTR_ERR(cls);
	}

	pr_info("GOT MAJOR: %d", MAJOR(dev_num));

	for(int i = 0; i < dev_quantity; i++){ //Create as many elevator devices as specified
		struct device *derr;
		derr = device_create(cls, NULL, MKDEV(MAJOR(dev_num), i), NULL, "elevator%d", i);
		if(IS_ERR(derr)){
			pr_err("Error adding device %s%d\n", DEVICE_NAME, i);
			for(int x = 0; x < i; x++){ //HAS TO BE IN REVERSE ORDER!!! NO X=I X!=0 X-- BS
				device_destroy(cls, MKDEV(MAJOR(dev_num), x));
			}
			class_destroy(cls);
			unregister_chrdev_region(dev_num, dev_quantity);
			return PTR_ERR(derr);
		}
		pr_info("Created elevator%d!", i);
	}
	cdev_init(&elevator_cdev, &fops); //Set up the cdev struct to encapsulate fops
	cdev_add(&elevator_cdev, dev_num, dev_quantity); //Tell the kernel that fops exists and we can use its functions basically
	return 0;
}


static void __exit end(void){
	pr_info("Device %s removed\n", DEVICE_NAME);
	cdev_del(&elevator_cdev);
	for(int i = 0; i < dev_quantity; i++){
		device_destroy(cls, MKDEV(MAJOR(dev_num), i));
	}
	class_destroy(cls);
	unregister_chrdev_region(dev_num, dev_quantity);
}

static ssize_t elevator_read(struct file *filp, char __user *buf, size_t len, loff_t *off){
	pr_info("Elevator read called!");

	return 0;
}

static ssize_t elevator_write(struct file *filp, const char __user *buf, size_t len, loff_t *off){
	pr_info("Elevator write called!");

	return 0;
}

static int elevator_open(struct inode *inode, struct file *filp){
	pr_info("Elevator open called!");
	pr_info("Number of Elevators: %d", dev_quantity);
	pr_info("Total Floors: %d", floor_qty);
	pr_info("Underground Floors: %d", underground_qty);
	pr_info("Highest Floor: %d", (floor_qty - (1 + underground_qty)));
	return 0;
}

static int elevator_release(struct inode *inode, struct file *filp){
	pr_info("Elevator release called!");

	return 0;
}

module_init(start);
module_exit(end);
