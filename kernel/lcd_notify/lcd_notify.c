// lcd_st7789.c
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/cdev.h>

#define DEVICE_NAME "lcd_notify"

static dev_t dev_num;
static struct cdev lcd_cdev;
static int lcd_flag = 0;
static wait_queue_head_t wq;

static ssize_t lcd_write(struct file *file, const char __user *buf, size_t len, loff_t *off) {
    char kbuf[16];
    if (len >= sizeof(kbuf)) return -EINVAL;
    if (copy_from_user(kbuf, buf, len)) return -EFAULT;
    kbuf[len] = '\0';

    if (kbuf[0] == '1') {
        lcd_flag = 1;
        wake_up_interruptible(&wq);
    }
    return len;
}

static unsigned int lcd_poll(struct file *file, struct poll_table_struct *wait) {
    poll_wait(file, &wq, wait);
    if (lcd_flag) {
        lcd_flag = 0;
        return POLLIN | POLLRDNORM;
    }
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .write = lcd_write,
    .poll  = lcd_poll,
};

static int __init lcd_init(void) {
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0)
        return -1;

    cdev_init(&lcd_cdev, &fops);
    if (cdev_add(&lcd_cdev, dev_num, 1) < 0)
        return -1;

    init_waitqueue_head(&wq);

    pr_info("[lcd_st7789v] device initialized (major: %d)\n", MAJOR(dev_num));
    return 0;
}

static void __exit lcd_exit(void) {
    cdev_del(&lcd_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("[lcd_st7789v] device removed\n");
}

module_init(lcd_init);
module_exit(lcd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yunji");
MODULE_DESCRIPTION("Trigger-based LCD notifier device");
