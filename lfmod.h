#ifndef LFMOD_H_
#define LFMOD_H_

#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/types.h>

#define LF_MODULE_NAME "lfmod"

// pci BAR
struct lf_bar {
    uint32_t __iomem *base;
    size_t size;
};

#define BAR_READ_32(ldev, addr)        ioread32((ldev)->pbar.base + (addr))
#define BAR_WRITE_32(ldev, addr, data) iowrite32(data, (ldev)->pbar.base + (addr))

// device handle
struct lf_device {
    // mapped BAR (for register access)
    struct lf_bar pbar;
    // device number for filenode
    dev_t device_number;
    // device in sysfs
    struct device *sysfs_device;
    // cdev
    struct cdev chrdev;
};

#endif // LFMOD_H_
