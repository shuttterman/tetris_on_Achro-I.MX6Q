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

#define CSEMAD_SSEG_MAJOR 261
#define CSEMAD_SSEG_NAME "csemad_sseg"

#define CSEMAD_SSEG_ADDRESS 0x08000004

//Global variable
static int ssegport_usage = 0;
static unsigned char *csemad_sseg_addr;

//define functions..

ssize_t csemad_sseg_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t csemad_sseg_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);
int csemad_sseg_open(struct inode *minode, struct file *mfile);
int csemad_sseg_release(struct inode *minode, struct file *mfile);

// define file_operations structure
struct file_operations csemad_sseg_fops = {
				.owner		= THIS_MODULE,
				.open		= csemad_sseg_open,
				.write		= csemad_sseg_write,
				.read		= csemad_sseg_read,
				.release	= csemad_sseg_release,
};

static int __init csemad_sseg_init(void) {
	if(register_chrdev(CSEMAD_SSEG_MAJOR, CSEMAD_SSEG_NAME, &csemad_sseg_fops)<0) {
		printk(KERN_ALERT"register_chrdev() error\n");
		return -1;
	}
	csemad_sseg_addr = ioremap(CSEMAD_SSEG_ADDRESS, 20);
	return 0;
}

static void __exit csemad_sseg_exit(void) {
	iounmap(csemad_sseg_addr);
	unregister_chrdev(CSEMAD_SSEG_MAJOR, CSEMAD_SSEG_NAME);
}

int csemad_sseg_open(struct inode *minode, struct file *mfile) {
	if(ssegport_usage != 0) return -EBUSY;
	ssegport_usage = 1;
	return 0;
}

int csemad_sseg_release(struct inode *minode, struct file *mfile) {
	ssegport_usage = 0;
	return 0;
}

ssize_t csemad_sseg_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) {
	int i;
	for(i=0 ; i<length ; i++) {
		if(copy_to_user(gdata++, csemad_sseg_addr+2*i, 1) != 0) return -EFAULT;			
	}
	return length;
}

ssize_t csemad_sseg_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
    if(copy_from_user(csemad_sseg_addr, gdata, length) != 0) return -EFAULT;
	return length;
}

module_init(csemad_sseg_init);
module_exit(csemad_sseg_exit);
