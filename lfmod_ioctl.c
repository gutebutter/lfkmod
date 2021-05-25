#define pr_fmt(fmt) KBUILD_MODNAME ": [%s] " fmt, __func__

#include "lfmod.h"
#include "lfmod_public.h"

#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/kernel.h>
#include <linux/module.h>

// union for ioctl memory allocation
union lfmod_ioctl_all {
    struct lfmod_register_io reg;
};

long lfmod_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
    long rc = 0;
    union lfmod_ioctl_all ioctl;
    void __user *user_arg = (void __user *)arg;
    struct lf_device *ldev = (struct lf_device *)(fp->private_data);
    uint32_t tmp;

    pr_info("in ioctl");

    // copy from user if indicated
    if (_IOC_DIR(cmd) & _IOC_WRITE) {
        if (copy_from_user(&ioctl, user_arg, _IOC_SIZE(cmd))) {
            // copy length mismatch
            pr_err("ioctl copy_from_user incomplete");
            return -EIO;
        }
    }

    // dispatch
    switch (cmd) {
        case LFMOD_REGISTER_READ:
            pr_info("in LFMOD_REGISTER_READ from %d", ioctl.reg.offset);
            if ((ioctl.reg.offset & 0x3) || (ioctl.reg.offset >= ldev->pbar.size)) {
                // check 32bit addresses or out of bounds
                pr_info("ioctl LFMOD_REGISTER_READ size or alignment mismatch");
                rc = -EFAULT;
                break;
            }
            ioctl.reg.data = BAR_READ_32(ldev, ioctl.reg.offset >> 2);
            pr_info("in LFMOD_REGISTER_READ read %08x", ioctl.reg.data);
            break;

        case LFMOD_REGISTER_WRITE:
            pr_info("in LFMOD_REGISTER_WRITE");
            if ((ioctl.reg.offset & 0x3) || (ioctl.reg.offset >= ldev->pbar.size)) {
                // check 32bit addresses or out of bounds
                pr_err("ioctl LFMOD_REGISTER_WRITE size or alignment mismatch");
                rc = -EFAULT;
                break;
            }
            BAR_WRITE_32(ldev, ioctl.reg.offset, ioctl.reg.data);
            break;

        case LFMOD_REGISTER_RMW:
            pr_info("in LFMOD_REGISTER_RMW");
            if ((ioctl.reg.offset & 0x3) || (ioctl.reg.offset >= ldev->pbar.size)) {
                // check 32bit addresses or out of bounds
                pr_err("ioctl LFMOD_REGISTER_RMW size or alignment mismatch");
                rc = -EFAULT;
                break;
            }
            tmp = BAR_READ_32(ldev, ioctl.reg.offset);
            tmp &= ~ioctl.reg.mask;
            tmp |= ioctl.reg.data & ioctl.reg.mask;
            BAR_WRITE_32(ldev, ioctl.reg.offset, tmp);
            break;

        default:
            pr_warn("unhandled ioctl number: %d\n", _IOC_NR(cmd));
            rc = -EINVAL;
            break;
    }

    // return data to user if indicated
    if (_IOC_DIR(cmd) & _IOC_READ) {
        if (copy_to_user(user_arg, &ioctl, _IOC_SIZE(cmd))) {
            pr_err("ioctl copy_to_user incomplete");
            return -EIO;
        }
    }

    return rc;
}
