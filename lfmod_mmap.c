#define pr_fmt(fmt) KBUILD_MODNAME ": [%s] " fmt, __func__

#include "lfmod.h"

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/types.h>

int lfmod_mmap(struct file *fp, struct vm_area_struct *vma)
{
    struct lf_device *ldev = (struct lf_device *)fp->private_data;
    unsigned long base = pci_resource_start(ldev->pdev, 0);
    unsigned long size = pci_resource_len(ldev->pdev, 0);

    // some sanity checks
    if ((vma->vm_pgoff << PAGE_SHIFT) + (vma->vm_end - vma->vm_start) > PAGE_ALIGN(size)) {
        return -EINVAL;
    }

    vma->vm_flags |= VM_IO;
    vma->vm_flags |= VM_DONTEXPAND;
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

    return remap_pfn_range(vma, vma->vm_start, base, vma->vm_end - vma->vm_start, vma->vm_page_prot);
}
