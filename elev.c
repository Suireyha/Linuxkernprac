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

//static int time = 0; //For the time steps and timesinceserver in each elevator state

static int major_number = 0; //This drivers major number assigned by user (basically an ID)
static int dev_quantity = 4; //Number of elevators
static int floor_qty = 10; //The TOTAL number of floors (including underground floors)
static int underground_qty = 2; //The number of floors that are underground
//Actual floors are from -undergroun_qty -> floor_qty - (1 + underground_qty)
module_param(major_number, int, 0644);
module_param(dev_quantity, int, 0644);
module_param(floor_qty, int, 0644);
module_param(underground_qty, int, 0644);

struct elevator_state{ //This is the 'local' data for each elevator device. We'll make an array of these later and index it with each devices minor number
	int current_floor; //The current floor this elevator is on
	bool service; //In service?
	int time_since_service; //Number of simulation cycles since last service
	int queue[50]; //50 is low key arbitrary idk what to make it reasonably but you can BET I'm not making this shit dynamic
	int q_size; //How many things are ACTUALLY on the queue
};
static struct elevator_state *elevators; //The array of elevators, needs to be dynamic with #of devices

static struct file_operations fops = {
	.read = elevator_read,
	.write = elevator_write,
	.open = elevator_open,
	.release = elevator_release,
};

static int __init start(void){
	int err;
	int cderr;
	pr_info("Device %s inserted\n", DEVICE_NAME);
	elevators = kmalloc(sizeof(struct elevator_state) * dev_quantity, GFP_KERNEL); //Dynamic array size of device number argument
	if(!elevators){ //kmalloc returned null </3
		pr_err("Failted to allocate elevator state\n");
		return -ENOMEM;
	}
	for(int i = 0; i < dev_quantity; i++){
		elevators[i].current_floor = 0;
		elevators[i].service = false;
		elevators[i].time_since_service = 0;
		elevators[i].q_size = 0; //QUeue curor
	}
	if(major_number == 0){//User ddin't change pass a major_number to use or passed 0 (reserved by other shit)
		err = alloc_chrdev_region(&dev_num, 0, dev_quantity, DEVICE_NAME);
	}
	else{
		dev_num = MKDEV(major_number, 0);
		err = register_chrdev_region(dev_num, dev_quantity, DEVICE_NAME);
	}

	if(err < 0){
		pr_err("Failed to allocate chrdev region!\n");
		kfree(elevators);
		return err;
	}

	cls = class_create(DEVICE_NAME);
	if(IS_ERR(cls)){
		pr_err("Failed to create class for elevator!\n");
		unregister_chrdev_region(dev_num, dev_quantity);
		kfree(elevators);
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
			kfree(elevators);
			return PTR_ERR(derr);
		}
		pr_info("Created elevator%d!", i);
	}
	cdev_init(&elevator_cdev, &fops); //Set up the cdev struct to encapsulate fops
	cderr = cdev_add(&elevator_cdev, dev_num, dev_quantity); //Tell the kernel that fops exists and we can use its functions basically
	if (cderr < 0) { //I am becoming unspeakably sick of this error catching bullshit there must be a more efficient way dwag :(
		pr_err("Failed to add cdev\n");
		for (int i = 0; i < dev_quantity; i++) {
			device_destroy(cls, MKDEV(MAJOR(dev_num), i));
		}
		class_destroy(cls);
		unregister_chrdev_region(dev_num, dev_quantity);
		kfree(elevators);
		return -1;
	}
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
	kfree(elevators); //Free the memory for the array of elevator states
}

static ssize_t elevator_read(struct file *filp, char __user *buf, size_t len, loff_t *off){
	int minor = iminor(filp->f_inode);
	struct elevator_state *this_elevator = &elevators[minor];
	char floor;
	
	if(*off > 0) return 0;
	if(len < 1) return 0; //User asked for zero bytes
	this_elevator->time_since_service++; //Increment the simulation time
	
	//Simulation loop goes here

	floor = (char)this_elevator->current_floor; //Get the current floor
	if(copy_to_user(buf, &floor, 1)){ //Put the current floor of this device in the buffer for the user to read
		pr_info("%s%d Failed copy in read\n", DEVICE_NAME, minor);
		return -EFAULT;
	}
	*off += 1; //Move the cursor along
	pr_debug("%s%d: read %d\n", DEVICE_NAME, minor, floor);
	return 1; //oine byte at a time
}

static ssize_t elevator_write(struct file *filp, const char __user *buf, size_t len, loff_t *off){
	//int minor = iminor(filp->f_inode);
	//struct elevator_state *this_elevator = &elevators[minor];

	return len;
}

static int elevator_open(struct inode *inode, struct file *filp){
	//int minor = iminor(inode);
	//struct elevator_state *this_elevator = &elevators[minor];

	return 0;
}

static int elevator_release(struct inode *inode, struct file *filp){
	//int minor = iminor(inode); //Get the minr number
	//struct elevator_state *this_elevator = &elevators[minor];
	return 0;
}

module_init(start);
module_exit(end);
