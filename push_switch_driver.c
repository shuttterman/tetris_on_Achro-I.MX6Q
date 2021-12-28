#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/version.h>

#define CSEMAD_SWITCH_MAJOR 265
#define CSEMAD_SWITCH_NAME "csemad_switch"

#define CSEMAD_SWITCH_ADDRESS 0x08000050

//Global variable
static int switchport_usage = 0;
static unsigned char *csemad_switch_addr;

//define functions..

ssize_t csemad_switch_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t csemad_switch_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);
int csemad_switch_open(struct inode *minode, struct file *mfile);
int csemad_switch_release(struct inode *minode, struct file *mfile);

// define file_operations structure
struct file_operations csemad_switch_fops = {
				.owner		= THIS_MODULE,
				.open		= csemad_switch_open,
				.write		= csemad_switch_write,
				.read		= csemad_switch_read,
				.release	= csemad_switch_release,
};

static int __init csemad_switch_init(void) {
	if(register_chrdev(CSEMAD_SWITCH_MAJOR, CSEMAD_SWITCH_NAME, &csemad_switch_fops)<0) {
		printk(KERN_ALERT"register_chrdev() error\n");
		return -1;
	}
	csemad_switch_addr = ioremap(CSEMAD_SWITCH_ADDRESS, 18);
	return 0;
}

static void __exit csemad_switch_exit(void) {
	iounmap(csemad_switch_addr);
	unregister_chrdev(CSEMAD_SWITCH_MAJOR, CSEMAD_SWITCH_NAME);
}

int csemad_switch_open(struct inode *minode, struct file *mfile) {
	if(switchport_usage != 0) return -EBUSY;
	switchport_usage = 1;
	return 0;
}

int csemad_switch_release(struct inode *minode, struct file *mfile) {
	switchport_usage = 0;
	return 0;
}

ssize_t csemad_switch_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) {
	int i;
	for(i=0 ; i<length ; i++) {
		if(copy_to_user(gdata++, csemad_switch_addr+2*i, 1) != 0) return -EFAULT;			
	}
	return length;
}

ssize_t csemad_switch_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
	if(copy_from_user(csemad_switch_addr, gdata, 1) != 0) return -EFAULT;
	return length;
}

module_init(csemad_switch_init);
module_exit(csemad_switch_exit);
