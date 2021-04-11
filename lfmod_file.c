#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#include "lfmod.h"

int lfmod_open(struct inode *inode, struct file *fp) {

    // associate the lf device handle with the filenode
    struct lf_device *ldev = container_of(inode->i_cdev, struct lf_device, chrdev);
    fp->private_data = ldev;

    return 0;
}

int lfmod_release(struct inode *inode, struct file *fp) {

    fp->private_data = NULL;

    return 0;
}