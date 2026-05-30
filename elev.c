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
#define MAX_QUEUE_LENGTH 100
#define IOCTL_MN 0x5b //Magic number fo the IO control stuff outlined by the spec
#define IOCTL_TOGGLE_SERVICE _IO(IOCTL_MN, 0) //cmd = 0 is for toggle service so if cmd = 0 then toggle service
#define IOCTL_GET_STATE _IOR(IOCTL_MN, 1, char[200]) //200 length string for the return buffer, command ID is basically 1 so if cmd = 1 to this

MODULE_LICENSE("GPL"); //Public license to use everything

//Function signatures for fops stuff =]
static ssize_t elevator_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t elevator_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);
static int elevator_open(struct inode *inode, struct file *filp);
static int elevator_release(struct inode *inode, struct file *filp);
static long elevator_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

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
	int queue[MAX_QUEUE_LENGTH]; //50 is low key arbitrary idk what to make it reasonably but you can BET I'm not making this shit dynamic
	int q_size; //How many things are ACTUALLY on the queue
};
static struct elevator_state *elevators; //The array of elevators, needs to be dynamic with #of devices

static struct file_operations fops = {
	.read = elevator_read,
	.write = elevator_write,
	.open = elevator_open,
	.release = elevator_release,
	.unlocked_ioctl = elevator_ioctl,
};

static int __init start(void){
	int err;
	int cderr;
	pr_info("Device %s inserted\n", DEVICE_NAME);
	elevators = kmalloc(sizeof(struct elevator_state) * dev_quantity, GFP_KERNEL); //Dynamic array size of device number argument
	if(!elevators){ //kmalloc returned null </3
		pr_err("Failed to allocate elevator state\n");
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

	pr_info("Elevator Driver Major Number: %d\n", MAJOR(dev_num));

	for(int i = 0; i < dev_quantity; i++){ //Create as many elevator devices as specified
		struct device *derr;
		derr = device_create(cls, NULL, MKDEV(MAJOR(dev_num), i), NULL, "elevator%d", i);
		if(IS_ERR(derr)){
			//Since we're making devices in order of i I'm pretty sure i will always be the correct minor number for the one that failed
			pr_err("Error adding device %s%d\n", DEVICE_NAME, i); //Assignment failed device instance initialisation log thing :fire:
			for(int x = 0; x < i; x++){ //HAS TO BE IN REVERSE ORDER!!! NO X=I X!=0 X-- BS
				device_destroy(cls, MKDEV(MAJOR(dev_num), x));
			}
			class_destroy(cls);
			unregister_chrdev_region(dev_num, dev_quantity);
			kfree(elevators);
			return PTR_ERR(derr);
		}
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
	for(int i = 0; i < dev_quantity; i++){ //Remove all the devices
		device_destroy(cls, MKDEV(MAJOR(dev_num), i));
	}
	class_destroy(cls); //Destroy the class struct
	unregister_chrdev_region(dev_num, dev_quantity); //Tell the kernel the driver is gone
	kfree(elevators); //Free the memory for the array of elevator states
}

static ssize_t elevator_read(struct file *filp, char __user *buf, size_t len, loff_t *off){
	int minor = iminor(filp->f_inode);
	struct elevator_state *this_elevator = &elevators[minor];
	char floor;
	
	if(*off > 0) return 0;
	if(len < 1) return 0; //User asked for zero bytes

	//Simulation loop goes here
	this_elevator->time_since_service++; //Increment the simulation time
	if(!this_elevator->service && this_elevator->q_size > 0){ //Only move if the elevator is on (not in service)
		if(this_elevator->current_floor == this_elevator->queue[0]){ //Serve the floor then pop the request off
			pr_info("%s%d: Serviced level %d\n", DEVICE_NAME, minor, this_elevator->current_floor);
			this_elevator->time_since_service = 0;
			for(int i = 0; i < this_elevator->q_size - 1; i++){ //Shift everything down
				this_elevator->queue[i] = this_elevator->queue[i+1];
			}
			this_elevator->q_size--; //Move the cursor down
		}
		else{ //Move up or down towards the floor at the front of the queue and log it
			if(this_elevator->current_floor < this_elevator->queue[0]){
				this_elevator->current_floor++;
				pr_info("%s%d moved %d->%d\n", DEVICE_NAME, minor, this_elevator->current_floor - 1, this_elevator->current_floor);
			}
			else{
				this_elevator->current_floor--;
				pr_info("%s%d moved %d->%d\n", DEVICE_NAME, minor, this_elevator->current_floor + 1, this_elevator->current_floor);
			} 
		}
	}

	floor = (char)this_elevator->current_floor; //Get the current floor
	//Can't ACTUALLY just write to the *buf since it's in the user space so we have to use this ugly function
	if(copy_to_user(buf, &floor, 1)){ //Put the current floor of this device in the buffer for the user to read
		pr_info("%s%d Failed copy in read\n", DEVICE_NAME, minor);
		return -EFAULT;
	}
	*off += 1; //Move the cursor along
	pr_debug("%s%d: read %d\n", DEVICE_NAME, minor, floor); //Read call log
	return 1; //oine byte at a time
}

static ssize_t elevator_write(struct file *filp, const char __user *buf, size_t len, loff_t *off){
	int minor = iminor(filp->f_inode);
	struct elevator_state *this_elevator = &elevators[minor];
	bool in_queue = false;
	signed char floor = 0;
	char ibuf[64]; //Copy of the *buf
	size_t to_cpy = min(len, sizeof(ibuf));

	//*buf is a user space pointer so we can't dereference it directly from within the kernel,
	//we have to go through copy_from_user(). ibuf is for all intents and purposes the uinput buffer
	if(copy_from_user(ibuf, buf, to_cpy)){
		pr_info("%s%d Failed copy in write\n", DEVICE_NAME, minor);
		return -EFAULT;
	}
	
	for(int i = 0; i < to_cpy; i++){
		in_queue = false;
		floor = ibuf[i];
		pr_debug("%s%d: write %d\n", DEVICE_NAME, minor, floor); //Write call log
		if(((signed int)floor > (floor_qty - (1 + underground_qty))) || (signed int)floor < (-1*underground_qty)){
			pr_info("Error: Invalid floor number written to elevator%d\n", minor);
		}
		else{
			for(int x = 0; x < this_elevator->q_size; x++){
				if((signed int)floor == this_elevator->queue[x]) in_queue = true;
			}
			if(!in_queue && this_elevator->q_size < MAX_QUEUE_LENGTH){
				this_elevator->queue[this_elevator->q_size] = (signed int)floor; //Add the requested floor to the queue
				this_elevator->q_size++; //Increment the q_size so that it matches next free index
			}
		}
	}
	return to_cpy; //Returns the number of bytes written, and anything more than ibuf lenmgth 64 will be ignored. Lucky us we already have this numebr at the top
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

static long elevator_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    int minor = iminor(filp->f_inode);
    struct elevator_state *this_elevator = &elevators[minor];
	char ret_str[200];
	int ret_len;
    
    switch (cmd) {
		case IOCTL_TOGGLE_SERVICE:
			if(this_elevator->service){//If already turned off
				this_elevator->service = false; //Turn the elevator back on
			}
			else{
				this_elevator->service = true; //Turn the elevator off
				this_elevator->q_size = 0; //Don't have to "delete" the queue, it'll all get overwritten anyways and is inaccessable
			}
			return 0;
		case IOCTL_GET_STATE:
			//Make the return string in ret_str and store it's length in ret_len
			ret_len = snprintf(ret_str, sizeof(ret_str), "Current Floor: %d\tIn Service: %d\tTime Since Last Service: %d\t Number of Requests: %d\n", this_elevator->current_floor, this_elevator->service, this_elevator->time_since_service, this_elevator->q_size);
			//Write ret_str to the userspace buffer arg, if it fails print an error
			if(copy_to_user((char __user *)arg, ret_str, ret_len + 1)){//+1 for null term
				pr_info("%s%d Failed copy in IOCTL\n", DEVICE_NAME, minor);
				return -EFAULT;
			}
			return 0;
		default:
			//Bad command- invalid IOCTL notice #assignmentSpecification :fire: #I'veBeenAwakeForTooLong
			pr_notice("%s%d: Invalid IOCTL cmd %d\n", DEVICE_NAME, minor, cmd);
			return -ENOTTY;
    }
}

module_init(start);
module_exit(end);
