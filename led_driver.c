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

#define CSEMAD_LED_MAJOR 260
#define CSEMAD_LED_NAME "csemad_led"

#define CSEMAD_LED_ADDRESS 0x08000016

//Global variable
static int ledport_usage = 0;
static unsigned char *csemad_led_addr;

//define functions..

ssize_t csemad_led_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t csemad_led_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);
int csemad_led_open(struct inode *minode, struct file *mfile);
int csemad_led_release(struct inode *minode, struct file *mfile);

// define file_operations structure
struct file_operations csemad_led_fops = {
				.owner		= THIS_MODULE,
				.open		= csemad_led_open,
				.write		= csemad_led_write,
				.read		= csemad_led_read,
				.release	= csemad_led_release,
};

static int __init csemad_led_init(void) {
	if(register_chrdev(CSEMAD_LED_MAJOR, CSEMAD_LED_NAME, &csemad_led_fops)<0) {
		printk(KERN_ALERT"register_chrdev() error\n");
		return -1;
	}
	csemad_led_addr = ioremap(CSEMAD_LED_ADDRESS, 1);
	return 0;
}

static void __exit csemad_led_exit(void) {
	iounmap(csemad_led_addr);
	unregister_chrdev(CSEMAD_LED_MAJOR, CSEMAD_LED_NAME);
}

int csemad_led_open(struct inode *minode, struct file *mfile) {
	if(ledport_usage != 0) return -EBUSY;
	ledport_usage = 1;
	return 0;
}

int csemad_led_release(struct inode *minode, struct file *mfile) {
	ledport_usage = 0;
	return 0;
}

ssize_t csemad_led_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) {
	if(copy_to_user(gdata, csemad_led_addr, length) != 0) return -EFAULT;			
	return length;
}

ssize_t csemad_led_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
	if(copy_from_user(csemad_led_addr, gdata, 1) != 0) return -EFAULT;
	return length;
}

module_init(csemad_led_init);
module_exit(csemad_led_exit);
