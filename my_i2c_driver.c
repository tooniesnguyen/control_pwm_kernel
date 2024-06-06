#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DRIVER_NAME "dc_driver"
#define CLASS_NAME "dc_driver"
#define DEVICE_NAME "dc_driver"



MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huynh Anh Duy");
MODULE_DESCRIPTION("I2C driver for control DC motor");

static struct i2c_client *dcDriver_client;
static struct class* dcDriver_class = NULL;
static struct device* dcDriver_device = NULL;
static int major_number;

typedef enum SwerveParamRegister{
	DEV_REG_SPEED_KP = 1,
	DEV_REG_SPEED_KI,
	DEV_REG_SPEED_KD,

	DEV_REG_ANGLE_KP,
	DEV_REG_ANGLE_KI,
	DEV_REG_ANGLE_KD,

	DEV_REG_TARGET_SPEED,
	DEV_REG_TARGET_ANGLE,
	DEV_REG_MODE,
}SwerveParamRegister;

#define SWERVE_MODULE_MAGIC_WORD 's'
#define SWERVE_IOCTL_SEND_TARGET_SPEED _IOW(SWERVE_MODULE_MAGIC_WORD,DEV_REG_TARGET_SPEED,u8)
#define SWERVE_IOCTL_SEND_TARGET_ANGLE _IOW(SWERVE_MODULE_MAGIC_WORD,DEV_REG_TARGET_ANGLE,u8)
#define SWERVE_IOCTL_SET_MODE _IOW(SWERVE_MODULE_MAGIC_WORD,DEV_REG_MODE,u8)
#define SWERVE_IOCTL_READ_CURRENT_ANGLE _IOR(SWERVE_MODULE_MAGIC_WORD,17,int)

static struct i2c_device_id my_ids[] = {
	{"dc_driver", 0},
    {},
};
MODULE_DEVICE_TABLE(i2c, my_ids);

static int dcDriver_read(struct i2c_client *client)
{
    u8 buf[1];

    if (i2c_smbus_read_i2c_block_data(client, DEV_REG_TARGET_ANGLE, sizeof(buf), buf) < 0) {
        printk(KERN_ERR "Failed to read Target Angle data\n");
        return -EIO;
    }

    return buf[0];
}

static u8 dcDriver_received_data(struct i2c_client *client, u8 reg)
{
    u8 value;
    value = i2c_smbus_read_byte_data(client, reg);
    printk("Value read: %u from reg: %u",value, reg);
    return value;
}

static int dcDriver_send_data(struct i2c_client *client, u8 data, u8 reg)
{
    if(i2c_smbus_write_byte_data(client, reg, data) < 0){
            printk("Failed to send data");
            return -EIO;
    }
    
    return 1;
}

static long dcDriver_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    int data;
    u8 value;
    int receiveData;
    switch (cmd) {
        case SWERVE_IOCTL_SEND_TARGET_SPEED:
            if(copy_from_user(&value,(u8 __user *)arg,sizeof(value))){
                printk("Data write from user error");
            }
            printk("Target speed: %u", value);
            data = dcDriver_send_data(dcDriver_client, value, DEV_REG_TARGET_SPEED);
            break;
        case SWERVE_IOCTL_SEND_TARGET_ANGLE:
            if(copy_from_user(&value,(u8 __user *)arg,sizeof(value))){
                printk("Data write from user error");
            }
            printk("Target angle: %u", value);
            data = dcDriver_send_data(dcDriver_client, value, DEV_REG_TARGET_ANGLE);
            break;
        case SWERVE_IOCTL_SET_MODE:
            if(copy_from_user(&value,(u8 __user *)arg,sizeof(value))){
                printk("Data write from user error");
            }
            printk("Mode set: %u", value);
            data = dcDriver_send_data(dcDriver_client, value, DEV_REG_MODE);
            break;
        case SWERVE_IOCTL_READ_CURRENT_ANGLE:
            receiveData = dcDriver_read(dcDriver_client);
            if (copy_to_user((int __user *)arg, &receiveData, sizeof(receiveData))) {
                return -EFAULT;
            }
            printk("Receive data angle: %u", receiveData);
            break;
        default:
            return -EINVAL;
    }

    

    return 0;
}    


static int dcDriver_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "dc_driver device opened\n");
    return 0;
}

static int dcDriver_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "dc_driver device closed\n");
    return 0;
}

static struct file_operations fops = {
    .open = dcDriver_open,
    .unlocked_ioctl = dcDriver_ioctl,
    .release = dcDriver_release,
};

static int my_probe(struct i2c_client *client, const struct i2c_device_id *id)
{	
    dcDriver_client = client;
    printk("dc_driver - Now I am in the probe function!\n");
    
    // Create a char device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ERR "Failed to register a major number\n");
        return major_number;
    }

    dcDriver_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(dcDriver_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to register device class\n");
        return PTR_ERR(dcDriver_class);
    }

    dcDriver_device = device_create(dcDriver_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(dcDriver_device)) {
        class_destroy(dcDriver_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to create the device\n");
        return PTR_ERR(dcDriver_device);
    }

    printk(KERN_INFO "dc_driver installed\n");
    

	return 0;
}

static void my_remove(struct i2c_client *client)
{
    device_destroy(dcDriver_class, MKDEV(major_number, 0));
    class_unregister(dcDriver_class);
    class_destroy(dcDriver_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk("my_i2c_driver - Removing device\n");
}

static const struct of_device_id dcDriver_of_match[] = {
    { .compatible = "SPK-Turbo,dc_driver", },
    { },
};
MODULE_DEVICE_TABLE(of, dcDriver_of_match);

static struct i2c_driver my_driver= {
	.probe = my_probe,
	.remove = my_remove,
	.id_table = my_ids,
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = of_match_ptr(dcDriver_of_match),
	}
};

module_i2c_driver(my_driver);
