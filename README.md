#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DRIVER_NAME "mpu6050_driver"
#define CLASS_NAME "mpu6050"
#define DEVICE_NAME "mpu6050"

#define MPU6050_REG_ACCEL_XOUT_H 0x3B
#define MPU6050_REG_PWR_MGMT_1 0x6B

#define SWERVE_REG_SPEED 0x22

// IOCTL commands
#define MPU6050_IOCTL_MAGIC 'm'
#define MPU6050_IOCTL_READ_X _IOR(MPU6050_IOCTL_MAGIC, 1, int)
#define MPU6050_IOCTL_READ_Y _IOR(MPU6050_IOCTL_MAGIC, 2, int)
#define MPU6050_IOCTL_READ_Z _IOR(MPU6050_IOCTL_MAGIC, 3, int)


MODULE_LICENSE("GPL");
MODULE_AUTHOR("EMBEDDED 4 GNU/Linux");
MODULE_DESCRIPTION("A simple I2C Device driver with driver_data");

static struct i2c_client *mpu6050_client;
static struct class* mpu6050_class = NULL;
static struct device* mpu6050_device = NULL;
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
#define SWERVE_IOCTL_READ_CURRENT_ANGLE _IOR(SWERVE_MODULE_MAGIC_WORD,17,u8)

static struct i2c_device_id my_ids[] = {
	{"mpu6050", 0},
    {"swerve", 0},
    {},
};
MODULE_DEVICE_TABLE(i2c, my_ids);

static int mpu6050_read_axis(struct i2c_client *client, int axis)
{
    u8 buf[6];
    s16 accel_data[3];

    if (i2c_smbus_read_i2c_block_data(client, MPU6050_REG_ACCEL_XOUT_H, sizeof(buf), buf) < 0) {
        printk(KERN_ERR "Failed to read accelerometer data\n");
        return -EIO;
    }

    // Combine high and low bytes to form 16-bit values
    accel_data[0] = (buf[0] << 8) | buf[1]; // X axis
    accel_data[1] = (buf[2] << 8) | buf[3]; // Y axis
    accel_data[2] = (buf[4] << 8) | buf[5]; // Z axis

    return accel_data[axis];
}

static u8 swerve_received_data(struct i2c_client *client, u8 reg)
{
    u8 value;
    value = i2c_smbus_read_byte_data(client, reg);
    printk("Value read: %u from reg: %u",value, reg);
    return value;
}

static int swerve_send_data(struct i2c_client *client, u8 data, u8 reg)
{
    if(i2c_smbus_write_byte_data(client, reg, data) < 0){
            printk("Failed to send data");
            return -EIO;
    }
    
    return 1;
}

static long swerve_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
    int data;
    u8 value;
    u8 receiveData;
    switch (cmd) {
        case SWERVE_IOCTL_SEND_TARGET_SPEED:
            if(copy_from_user(&value,(u8 __user *)arg,sizeof(value))){
                printk("Data write from user error");
            }
            printk("Target speed: %u", value);
            data = swerve_send_data(mpu6050_client, value, DEV_REG_TARGET_SPEED);
            break;
        case SWERVE_IOCTL_SEND_TARGET_ANGLE:
            if(copy_from_user(&value,(u8 __user *)arg,sizeof(value))){
                printk("Data write from user error");
            }
            printk("Target angle: %u", value);
            data = swerve_send_data(mpu6050_client, value, DEV_REG_TARGET_ANGLE);
            break;
        case SWERVE_IOCTL_SET_MODE:
            if(copy_from_user(&value,(u8 __user *)arg,sizeof(value))){
                printk("Data write from user error");
            }
            printk("Mode set: %u", value);
            data = swerve_send_data(mpu6050_client, value, DEV_REG_MODE);
            break;
        case SWERVE_IOCTL_READ_CURRENT_ANGLE:
            receiveData = swerve_received_data(mpu6050_client, DEV_REG_TARGET_ANGLE);
            if (copy_to_user((u8 __user *)arg, &receiveData, sizeof(receiveData))) {
                return -EFAULT;
            }
            printk("Receive data angle: %u", receiveData);
            break;
        default:
            return -EINVAL;
    }

    

    return 0;
}    

static long mpu6050_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int data;

    switch (cmd) {
        case MPU6050_IOCTL_READ_X:
            data = mpu6050_read_axis(mpu6050_client, 0);
            break;
        case MPU6050_IOCTL_READ_Y:
            data = mpu6050_read_axis(mpu6050_client, 1);
            break;
        case MPU6050_IOCTL_READ_Z:
            data = mpu6050_read_axis(mpu6050_client, 2);
            break;
        default:
            return -EINVAL;
    }

    if (copy_to_user((int __user *)arg, &data, sizeof(data))) {
        return -EFAULT;
    }

    return 0;
}

static int mpu6050_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "MPU6050 device opened\n");
    return 0;
}

static int mpu6050_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "MPU6050 device closed\n");
    return 0;
}

static struct file_operations fops = {
    .open = mpu6050_open,
    .unlocked_ioctl = swerve_ioctl,
    .release = mpu6050_release,
};

static int my_probe(struct i2c_client *client, const struct i2c_device_id *id)
{	
    /*  
    This function to check 
    */
    mpu6050_client = client;
    printk("mpu6050_driver - Now I am in the probe function!\n");
    
    // Create a char device
    // https://archive.kernel.org/oldlinux/htmldocs/kernel-api/API---register-chrdev.html
    // Positive: the number of major that successful registered
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ERR "Failed to register a major number\n");
        return major_number;
    }

    // https://www.kernel.org/doc/html/latest/driver-api/infrastructure.html?highlight=class_create
    // THIS_MODULE là tiêu chuẩn để địa diện cho module hiện tại
    // This is used to create a struct class pointer that can then be used in calls to device_create().
    mpu6050_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(mpu6050_class)) {
        // https://manpages.debian.org/testing/linux-manual-4.8/__unregister_chrdev.9
        // Unregister and destroy the cdev occupying the region described by major, baseminor and count. This function undoes what __register_chrdev did.
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to register device class\n");
        return PTR_ERR(mpu6050_class);
    }

    // https://www.kernel.org/doc/html/latest/driver-api/infrastructure.html?highlight=device_create
    // A struct device will be created in sysfs, registered to the specified class.
    // MKDEV(int major, int minor) gộp số major và minor để tạo thành device number
    mpu6050_device = device_create(mpu6050_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(mpu6050_device)) {
        class_destroy(mpu6050_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to create the device\n");
        return PTR_ERR(mpu6050_device);
    }

    printk(KERN_INFO "MPU6050 driver installed\n");

	return 0;
}

static void my_remove(struct i2c_client *client)
{
    device_destroy(mpu6050_class, MKDEV(major_number, 0));
    class_unregister(mpu6050_class);
    class_destroy(mpu6050_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk("my_i2c_driver - Removing device\n");
}

static const struct of_device_id mpu6050_of_match[] = {
    { .compatible = "invensense,mpu6050", },
    { },
};
MODULE_DEVICE_TABLE(of, mpu6050_of_match);

static struct i2c_driver my_driver= {
	.probe = my_probe,
	.remove = my_remove,
	.id_table = my_ids,
	.driver = {
		.name = DRIVER_NAME,
		.of_match_table = of_match_ptr(mpu6050_of_match),
	}
};

module_i2c_driver(my_driver);
