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

#define CSEMAD_DOT_MATRIX_MAJOR 262
#define CSEMAD_DOT_MATRIX_NAME "csemad_dot_matrix"

#define CSEMAD_DOT_MATRIX_ADDRESS 0x08000210

//Global variable
static int dot_matrixport_usage = 0;
static unsigned char *csemad_dot_matrix_addr;

//define functions..

ssize_t csemad_dot_matrix_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
ssize_t csemad_dot_matrix_read(struct file *inode, char *gdata, size_t length, loff_t *off_what);
int csemad_dot_matrix_open(struct inode *minode, struct file *mfile);
int csemad_dot_matrix_release(struct inode *minode, struct file *mfile);

// define file_operations structure
struct file_operations csemad_dot_matrix_fops = {
				.owner		= THIS_MODULE,
				.open		= csemad_dot_matrix_open,
				.write		= csemad_dot_matrix_write,
				.read		= csemad_dot_matrix_read,
				.release	= csemad_dot_matrix_release,
};

static int __init csemad_dot_matrix_init(void) {
	if(register_chrdev(CSEMAD_DOT_MATRIX_MAJOR, CSEMAD_DOT_MATRIX_NAME, &csemad_dot_matrix_fops)<0) {
		printk(KERN_ALERT"register_chrdev() error\n");
		return -1;
	}
	csemad_dot_matrix_addr = ioremap(CSEMAD_DOT_MATRIX_ADDRESS, 20);
	return 0;
}

static void __exit csemad_dot_matrix_exit(void) {
	iounmap(csemad_dot_matrix_addr);
	unregister_chrdev(CSEMAD_DOT_MATRIX_MAJOR, CSEMAD_DOT_MATRIX_NAME);
}

int csemad_dot_matrix_open(struct inode *minode, struct file *mfile) {
	if(dot_matrixport_usage != 0) return -EBUSY;
	dot_matrixport_usage = 1;
	return 0;
}

int csemad_dot_matrix_release(struct inode *minode, struct file *mfile) {
	dot_matrixport_usage = 0;
	return 0;
}

ssize_t csemad_dot_matrix_read(struct file *inode, char *gdata, size_t length, loff_t *off_what) {
	int i;
	for(i=0 ; i<length ; i++) {
		if(copy_to_user(gdata++, csemad_dot_matrix_addr+2*i, 1) != 0) return -EFAULT;			
	}
	return length;
}

ssize_t csemad_dot_matrix_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) {
	int i;
	for(i=0 ; i<length ; i++){
		if(copy_from_user(csemad_dot_matrix_addr+2*i, gdata++, 1) != 0) return -EFAULT;
	}
	return length;
}

module_init(csemad_dot_matrix_init);
module_exit(csemad_dot_matrix_exit);
