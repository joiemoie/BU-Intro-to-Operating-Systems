/*
*  adder.c - Demonstrates command line argument passing to a module.
*/
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

MODULE_LICENSE("Joseph Liba");
MODULE_AUTHOR("Joseph Liba");

static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file *);
static ssize_t device_read(struct file *, char*, size_t, loff_t*);
static ssize_t device_write(struct file *, const char *, size_t, loff_t*);

#define SUCCESS 0
#define DEVICE_NAME "adder"
#define BUF_LEN 80

static int device_reading = 0;
static int device_writing = 0;
static int sending = 0;
static int sum = 0;
static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

static int __init adder_init(void)
{
	register_chrdev(200, DEVICE_NAME, &fops);
	return 0;
}

static void __exit adder_exit(void)
{

}

static int device_open(struct inode *inode, struct file *filp)
{
	return SUCCESS;
}

static int device_release(struct inode *inode, struct file *filp)
{
	return SUCCESS;
}

static ssize_t device_read(struct file *filp, char * buffer,
	size_t length, loff_t *offset)
{
	if (device_reading == 1) {
		printk(KERN_WARNING "Error: More than 1 reader.\n");
		return 0;
	}
	device_reading = 1;
	if (sending) {
		sending = 0;
		device_reading = 0;
		return 0;

	}
	int bytes_read = 0;
	int x = sum;
	int numDigits = 0;
	while (x != 0) {
		x /= 10;
		numDigits++;
	}
	if (sum == 0) numDigits = 1;
	x = (sum > 0) ? sum : sum * -1;
	int i = (sum > 0) ? numDigits - 1 : numDigits;
	while (i >= 0) {
		char copy = x % 10 + '0';
		if (sum < 0 && i == 0) copy = '-';
		copy_to_user(&buffer[i], &copy, 1);
		x /= 10;
		i--;
		bytes_read++;
	}
	device_reading = 0;
	sending = 1;
	return bytes_read;
}

static int currentNum = 0;
static int neg = 0;
static int numberParsing = 0;
static ssize_t device_write(struct file *filp, const char *buf,
	size_t len, loff_t *off)
{
	if (device_writing == 1) {
		printk(KERN_WARNING "Error: Multiple writers.\n");
		return 0;
	}
	device_writing = 1;
	int i = 0;
	while (i<(int)len) {
		char c = buf[i];
		if (c == '-' && numberParsing == 0) {
			neg = 1;
			numberParsing = 1;
		}
		else if (c >= '0' && c <= '9') {
			currentNum = currentNum * 10 + (c - '0');
			numberParsing = 1;
		}
		else if (c == '\0' || c == ' ' || c == '\n') {
			if (neg) {
				neg = 0;
				currentNum *= -1;
			}
			sum += currentNum;
			printk(KERN_INFO "currentNum: %d\n", currentNum);
			numberParsing = 0;
			currentNum = 0;

		}
		else {
			printk(KERN_WARNING "Invalid character.");
			numberParsing = 0;
			neg = 0;
			currentNum = 0;
			device_writing = 0;
			return len;
		}
		i++;
	}
	if (neg) {
		neg = 0;
		currentNum *= -1;
	}
	sum += currentNum;
	numberParsing = 0;
	currentNum = 0;
	device_writing = 0;
	return len;
}
module_init(adder_init);
module_exit(adder_exit);

