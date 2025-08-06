#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/wait.h>
#include <linux/poll.h>

#define DEVICE_NAME "alert_trigger"

static dev_t dev_num;
static struct cdev alert_cdev;
static wait_queue_head_t wq;
static int alert_flag = 0;

static ssize_t alert_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    char kbuf[16];
    if (len >= sizeof(kbuf)) return -EINVAL;
    if (copy_from_user(kbuf, buf, len)) return -EFAULT;

    kbuf[len] = '\0';
    if (kbuf[0] == '1') {
        alert_flag = 1;
        wake_up_interruptible(&wq);
    }

    return len;
}

static unsigned int alert_poll(struct file *file, poll_table *wait)
{
    poll_wait(file, &wq, wait);
    if (alert_flag)
        return POLLOUT | POLLWRNORM;
    return 0;
}

static ssize_t alert_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    if (alert_flag) {
        alert_flag = 0;
        return 0; // 간단히 읽기 완료 처리
    }
    return 0;
}

static struct file_operations alert_fops = {
    .owner = THIS_MODULE,
    .write = alert_write,
    .poll  = alert_poll,
    .read  = alert_read,
};

static int __init alert_init(void)
{
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0)
        return -1;

    cdev_init(&alert_cdev, &alert_fops);
    if (cdev_add(&alert_cdev, dev_num, 1) < 0)
        return -1;

    init_waitqueue_head(&wq);

    pr_info("alert_trigger driver initialized\n");
    return 0;
}

static void __exit alert_exit(void)
{
    cdev_del(&alert_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("alert_trigger driver exited\n");
}

module_init(alert_init);
module_exit(alert_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yunji LEE");
MODULE_DESCRIPTION("Virtual alert trigger device");
