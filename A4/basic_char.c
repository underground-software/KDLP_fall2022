#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");

static dev_t basic_char_device_no;
static struct cdev *basic_char_cdev;

static int basic_char_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "Basic Char Device: open\n");
	return 0;
}

static int basic_char_release(struct inode *inode, struct file *file)
{

	printk(KERN_INFO "Basic Char Device: release\n");
	return 0;
}

static ssize_t basic_char_read(struct file *file, char __user *data, size_t count, loff_t *f_pos)
{
	printk(KERN_INFO "Basic Char Device: read\n");
	return 0;
}

static ssize_t basic_char_write(struct file *file, const char __user *data, size_t count, loff_t *f_pos)
{
	printk(KERN_INFO "Basic Char Device: write\n");
	return count;
}

static const struct file_operations basic_char_fops = {
	.owner = THIS_MODULE,
	.open = basic_char_open,
	.release = basic_char_release,
	.read = basic_char_read,
	.write = basic_char_write,
};

static int basic_char_init(void)
{
	int ret;
	printk(KERN_INFO "Basic Char Device: init\n");

	ret = alloc_chrdev_region(&basic_char_device_no, 0, 1, "basic_char");
	if (ret < 0) {
		printk(KERN_ERR "Basic Char Device: unable to allocate region %i\n", ret);
		goto fail_region;
	}

	basic_char_cdev = cdev_alloc();
	if (!basic_char_cdev) {
		ret = -ENOMEM;
		printk(KERN_ERR "Basic Char Device: unable to allocate char dev %i\n", ret);
		goto fail_cdev;
	}

	cdev_init(basic_char_cdev, &basic_char_fops);

	ret = cdev_add(basic_char_cdev, basic_char_device_no, 1);
	if (ret < 0) {
		printk(KERN_ERR "Basic Char Device: unable to add char dev %i\n", ret);
		goto fail_add;
	}

	printk(KERN_INFO "Basic Char Device: got major %i, minor %i\n", MAJOR(basic_char_device_no), MINOR(basic_char_device_no));
	return 0;

fail_add:
	cdev_del(basic_char_cdev);
fail_cdev:
	unregister_chrdev_region(basic_char_device_no, 1);
fail_region:
	return ret;
}

static void basic_char_exit(void)
{
	printk(KERN_INFO "Basic Char Device: exit\n");
	cdev_del(basic_char_cdev);
	unregister_chrdev_region(basic_char_device_no, 1);
}
module_init(basic_char_init);
module_exit(basic_char_exit);
