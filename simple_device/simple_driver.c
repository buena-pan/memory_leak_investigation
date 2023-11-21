#include <linux/module.h>       // Needed by all modules
#include <linux/kernel.h>       // Needed for KERN_INFO
#include <linux/init.h>         // Needed for the macro
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/dma-mapping.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple Hello World module");

dev_t dev_num;        // Will hold the major number that the kernel gives
int major_number;     // Stores the major number extracted from dev_num
int ret;              // Will hold return values of functions; this is true throughout the code
struct cdev *mcdev;   // This will hold a character device driver descriptor

static struct class *simple_class;
static struct device *simple_device;

static void *dma_buf; 
static dma_addr_t dma_handle;
static size_t size = 4 * 1024 * 1024;

int device_open(struct inode *inode, struct file *filep) {
    printk(KERN_INFO "simple_device: Device successfully opened\n");
    return 0;
}

int device_close(struct inode *inode, struct file *filep) {
    printk(KERN_INFO "simple_device: Device successfully closed\n");
    return 0;
}

static void drxpd_vma_open(struct vm_area_struct *vma) {
    printk(KERN_INFO "simple_device: VMA Open\n");
}

static void drxpd_vma_close(struct vm_area_struct *vma) {
    // DMA unregister
    if (dma_buf){
            dma_free_coherent(simple_device, size, dma_buf, dma_handle);}
    //private data removal
    if(vma->vm_private_data){
	    vma->vm_private_data = NULL;
    }
    printk(KERN_INFO "simple_device: VMA Close\n");
}

static struct vm_operations_struct drxpd_vm_ops = {
    .open = drxpd_vma_open,
    .close = drxpd_vma_close,
};


static int dev_mmap(struct file *filp, struct vm_area_struct *vma)
{
    // DMA allocation
    if (dma_set_coherent_mask(simple_device, DMA_BIT_MASK(63))) {
            dev_warn(simple_device, "simple_device: No suitable DMA available\n");
                printk("simple_device: bad DMA initialization");

    }
    printk("Allocating %zu bytes\n",size);
    dma_buf = dma_alloc_coherent(simple_device, size, &dma_handle, GFP_ATOMIC);
    if (!dma_buf) {
	    printk(KERN_ALERT "Failed to allocate DMA buffer\n");
	    return -ENOMEM;

    }

    printk("simple_device: Lets mmap this");
    unsigned long vsize = vma->vm_end - vma->vm_start ;
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

    if (remap_pfn_range(vma, vma->vm_start, dma_handle >> PAGE_SHIFT, vsize, vma->vm_page_prot))
    	{
		printk("simple_device: Bad mmap()");
    		return -EAGAIN;
	}
    vma->vm_ops = &drxpd_vm_ops;
    vma->vm_private_data = filp->private_data;
    printk("simple_device: Successful mmap() ");
    return 0;
}

struct file_operations fops =
{
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_close,
    .mmap = dev_mmap
};

static int __init simple_start(void) {
    // Minimal initialization
    printk(KERN_INFO "simple_device: Registering a character device...\n");
    
    ret = alloc_chrdev_region(&dev_num, 0, 1, "simple_device");
    if (ret < 0) {
        printk(KERN_ALERT "simple_device: Failed to allocate a major number\n");
        return ret;
    }
    major_number = MAJOR(dev_num);
    printk(KERN_INFO "simple_device: major number is %d\n", major_number);

    mcdev = cdev_alloc();
    mcdev->ops = &fops;
    mcdev->owner = THIS_MODULE;

    ret = cdev_add(mcdev, dev_num, 1);
    if (ret < 0) {
        printk(KERN_ALERT "simple_device: Unable to add cdev to kernel\n");
        return ret;
    }

    simple_class = class_create(THIS_MODULE, "simple_class");
    if (IS_ERR(simple_class)) {
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(simple_class);
    }

    simple_device = device_create(simple_class, NULL, dev_num, NULL, "simple_device");
    if (IS_ERR(simple_device)) {
        class_destroy(simple_class);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(simple_device);
    }

    printk(KERN_INFO "simple_device: Device node created successfully\n");

    return 0;
}


static void __exit simple_end(void) {	
    // First, check if mcdev is not NULL before attempting to delete it
    if (mcdev) {
        cdev_del(mcdev);
    }

    // Destroy the device only if it has been created
    if (simple_device) {
        device_destroy(simple_class, dev_num);
    }

    // Destroy the class only if it has been created
    if (simple_class) {
        class_destroy(simple_class);
    }

    // Unregister the device number
    unregister_chrdev_region(dev_num, 1);
    
    printk(KERN_INFO " Device removed successfuly\n");
}


module_init(simple_start);
module_exit(simple_end)
