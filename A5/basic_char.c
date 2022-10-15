#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");

static uintptr_t counter = 0;

static dev_t basic_char_device_no;
static struct cdev *basic_char_cdev;
static struct class *basic_char_class;
static struct device *basic_char_device;

static int basic_char_open(struct inode *inode, struct file *file)
{
	file->private_data = (void *)counter;
	printk(KERN_INFO "Basic Char Device %lu: open (number %lu)\n", counter++, (uintptr_t)file->private_data);
	return 0;
}

static int basic_char_release(struct inode *inode, struct file *file)
{

	printk(KERN_INFO "Basic Char Device %lu: release (number %lu)\n", counter++, (uintptr_t)file->private_data);
	return 0;
}

static ssize_t basic_char_read(struct file *file, char __user *data, size_t count, loff_t *f_pos)
{
	printk(KERN_INFO "Basic Char Device %lu: read (number %lu)\n", counter++, (uintptr_t)file->private_data);
	return 0;
}

static ssize_t basic_char_write(struct file *file, const char __user *data, size_t count, loff_t *f_pos)
{
	printk(KERN_INFO "Basic Char Device %lu: write (number %lu)\n", counter++, (uintptr_t)file->private_data);
	return count;
}

static const struct file_operations basic_char_fops = {
	.owner = THIS_MODULE,
	.open = basic_char_open,
	.release = basic_char_release,
	.read = basic_char_read,
	.write = basic_char_write,
};

static char *basic_char_node(struct device *dev, umode_t *mode)
{
	if(mode)
		*mode = 0666;
	return NULL;
}

static int basic_char_init(void)
{
	int ret;
	printk(KERN_INFO "Basic Char Device %lu: init\n", counter++);

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

	basic_char_class = class_create(THIS_MODULE, "basic_char");
	if (IS_ERR(basic_char_class)) {
		ret = PTR_ERR(basic_char_class);
		printk(KERN_ERR "Basic Char Device: unable to create device class %i\n", ret);
		goto fail_add;
	}

	basic_char_class->devnode = basic_char_node;
	basic_char_device = device_create(basic_char_class, NULL, basic_char_device_no, NULL, "basic_char");
	if(IS_ERR(basic_char_device)) {
		ret = PTR_ERR(basic_char_device);
		printk(KERN_ERR "Basic Char Device: unable to create device %i\n", ret);
		goto fail_class;
	}

	return 0;

fail_class:
	class_destroy(basic_char_class);
fail_add:
	cdev_del(basic_char_cdev);
fail_cdev:
	unregister_chrdev_region(basic_char_device_no, 1);
fail_region:
	return ret;
}

static void basic_char_exit(void)
{
	printk(KERN_INFO "Basic Char Device %lu: exit\n", counter++);
	device_destroy(basic_char_class, basic_char_device_no);
	class_destroy(basic_char_class);
	cdev_del(basic_char_cdev);
	unregister_chrdev_region(basic_char_device_no, 1);
}
module_init(basic_char_init);
module_exit(basic_char_exit);
