#define pr_fmt(fmt) KBUILD_MODNAME ": [%s] " fmt,__func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

long lfmod_ioctl(struct file *fp, unsigned int cmd, unsigned long arg) {
    long rc = 0;


    // dispatch
    switch (cmd) {
        default:
            pr_warn("unhandled ioctl number: %d\n", _IOC_NR(cmd));
            rc = -EINVAL;
            break;
    }

    return rc;
}
