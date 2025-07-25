#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/cdev.h>
#include <linux/string.h>

#define DEVICE_NAME "wav_notify"
#define MAX_FILENAME_LEN 64

static dev_t dev_num;
static struct cdev wav_cdev;
static wait_queue_head_t wq;
static int notify_flag = 0;
static char filename_buf[MAX_FILENAME_LEN] = {0};

static ssize_t wav_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    if (len >= MAX_FILENAME_LEN) return -EINVAL;
    if (copy_from_user(filename_buf, buf, len)) return -EFAULT;

    filename_buf[len] = '\0';
    notify_flag = 1;
    wake_up_interruptible(&wq);

    pr_info("[wav_notify] Triggered with file: %s\n", filename_buf);
    return len;
}

static ssize_t wav_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    size_t file_len = strnlen(filename_buf, MAX_FILENAME_LEN);

    if (!notify_flag) return 0;
    if (len < file_len + 1) return -EINVAL;

    if (copy_to_user(buf, filename_buf, file_len + 1)) return -EFAULT;

    notify_flag = 0;
    memset(filename_buf, 0, MAX_FILENAME_LEN);
    return file_len + 1;
}

static unsigned int wav_poll(struct file *file, poll_table *wait)
{
    poll_wait(file, &wq, wait);
    return notify_flag ? POLLIN | POLLRDNORM : 0;
}

static struct file_operations wav_fops = {
    .owner = THIS_MODULE,
    .write = wav_write,
    .read  = wav_read,
    .poll  = wav_poll,
};

static int __init wav_notify_init(void)
{
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0)
        return -1;

    cdev_init(&wav_cdev, &wav_fops);
    if (cdev_add(&wav_cdev, dev_num, 1) < 0)
        return -1;

    init_waitqueue_head(&wq);
    pr_info("wav_notify driver loaded: /dev/wav_notify\n");
    return 0;
}

static void __exit wav_notify_exit(void)
{
    cdev_del(&wav_cdev);
    unregister_chrdev_region(dev_num, 1);
    pr_info("wav_notify driver unloaded\n");
}

module_init(wav_notify_init);
module_exit(wav_notify_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yunji");
MODULE_DESCRIPTION("WAV Notify Trigger Device with filename passing");
