#ifndef LFMOD_H_
#define LFMOD_H_

#define LF_MODULE_NAME "lfmod"

// pci BAR
struct lf_bar {
    uint32_t __iomem *base;
    size_t size;
};

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
