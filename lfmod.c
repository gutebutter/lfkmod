#define pr_fmt(fmt) KBUILD_MODNAME ": [%s] " fmt,__func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/cdev.h>

#include "lfmod.h"
#include "lfmod_file.h"
#include "lfmod_ioctl.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("rage");
MODULE_DESCRIPTION("litefury pcie driver module.");
MODULE_VERSION("0.1");

// class creation handle
static struct class *lf_class;

// device id table lists the devices that we're interested in
static const struct pci_device_id device_ids[] = {
    { PCI_DEVICE(0x1337, 0x1000) }, // litefury device
    { 0 }
};
MODULE_DEVICE_TABLE(pci, device_ids);

// entry functions called from pci subsystem
int lf_pci_probe(struct pci_dev *dev, const struct pci_device_id *id);
void lf_pci_remove(struct pci_dev *dev);

// we're a pci driver
static struct pci_driver lfmod_pci = {
    .name = LF_MODULE_NAME,
    .id_table = device_ids,
    .probe = lf_pci_probe,
    .remove = lf_pci_remove
};

// fileops of the registered file in sysfs
static const struct file_operations lfmod_fops = {
    .owner = THIS_MODULE,
    .open = lfmod_open,
    .release = lfmod_release,
    .unlocked_ioctl = lfmod_ioctl
};

// map the specified bar
static int lf_map_bar(struct pci_dev *dev, int bar) {
    struct lf_device *ldev = pci_get_drvdata(dev);

    // get the BAR base address
    ldev->pbar.base = pci_iomap(dev, bar, 0);
    if (ldev->pbar.base == NULL) {
        // resource mapping failed
        return -EIO;
    }

    // get the BAR size
    ldev->pbar.size = pci_resource_len(dev, bar);

    return 0;
}

// unmap the specified bar
static int lf_unmap_bar(struct pci_dev *dev) {
    struct lf_device *ldev = pci_get_drvdata(dev);

    // free resource
    if (ldev->pbar.base) {
        pci_iounmap(dev, ldev->pbar.base);
        ldev->pbar.base = NULL;
        ldev->pbar.size = 0;
    }

    return 0;
}

// create a filenode for the given device
static int lf_create_filenode(struct pci_dev *dev) {
    struct lf_device *ldev = pci_get_drvdata(dev);
    dev_t num;
    struct device *d;

    // get a free major number with the first minor from the system
    int rc = alloc_chrdev_region(&num, 0, 1, LF_MODULE_NAME);
    if (rc < 0) {
        pr_err("alloc_chrdev_region failed: %d\n", rc);
        return rc;
    }
    ldev->device_number = num;

    // create a filenode in /dev
    d = device_create(lf_class, NULL, num, NULL, "litefury");
    if (d == NULL) {
        pr_err("device_create failed\n");
        return -1;
    }
    ldev->sysfs_device = d;

    // associate cdev with fileops
    cdev_init(&ldev->chrdev, &lfmod_fops);

    // make device available
    rc = cdev_add(&ldev->chrdev, num, 1);
    if (rc < 0) {
        pr_err("cdev_add failed: %d\n", rc);
        return rc;
    }

    return 0;
}

// device probing. called from pci subsystem when a device for us has been detected.
int lf_pci_probe(struct pci_dev *dev, const struct pci_device_id *id) {
    int rc = 0;

    // build the modules' device handle
    struct lf_device *ldev = kzalloc(sizeof(struct lf_device), GFP_KERNEL);
    if (ldev == NULL) {
        pr_err("kzalloc for the module device handle failed\n");
        return -ENOMEM;
    }
    
    // associate the local device handle with the kernel pci device handle
    pci_set_drvdata(dev, ldev);

    // enable the device
    rc = pci_enable_device(dev);
    if (rc < 0) {
        pr_err("pci_enable_device failed: %d\n", rc);
        goto err_enable_device;
    }

    // associate the resources of the device with this module
    rc = pci_request_regions(dev, LF_MODULE_NAME);
    if (rc < 0) {
        pr_err("pci_request_regions failed: %d\n", rc);
        goto err_request_regions;
    }

    // map BAR0 to the module
    rc = lf_map_bar(dev, 0);
    if (rc < 0) {
        pr_err("lf_map_bar failed: %d\n", rc);
        goto err_map_bar;
    }

    // create a file node for the device
    rc = lf_create_filenode(dev);
    if (rc < 0) {
        pr_err("lf_create_filenode failed: %d\n", rc);
        goto err_create_filenode;
    }

    // success
    return 0;

    // cleanup chain
err_create_filenode:
    lf_unmap_bar(dev);
err_map_bar:
    pci_release_regions(dev);
err_request_regions:
    pci_disable_device(dev);
err_enable_device:
    kfree(ldev);
    return rc;
}

// release one of our devices.
void lf_pci_remove(struct pci_dev *dev) {
    struct lf_device *ldev = pci_get_drvdata(dev);

    // cleanup device stuff
    lf_unmap_bar(dev);
    pci_release_regions(dev);
    pci_disable_device(dev);
    pci_disable_device(dev);

    // cleanup device handle
    kfree(ldev);
}

static int __init lfmod_init(void) {

    // register driver with the pci subsystem
    int rc = pci_register_driver(&lfmod_pci);
    if (rc < 0) {
        pr_err("pci_register_driver failed: %d\n", rc);
        return rc;
    }

    // register class for filenodes
    lf_class = class_create(THIS_MODULE, LF_MODULE_NAME);
    if (lf_class == NULL) {
        pr_err("class_create failed.\n");
        pci_unregister_driver(&lfmod_pci);
        return -1;
    }

    return 0;
}

static void __exit lfmod_exit(void) {

    // cleanup from pci subsystem
    pci_unregister_driver(&lfmod_pci);

}

module_init(lfmod_init);
module_exit(lfmod_exit);
