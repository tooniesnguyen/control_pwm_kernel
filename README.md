# Driver Control PWM Motor

### Hardware requirements
- [Raspberry Pi 3 Model B+](https://raspberrypi.vn/san-pham/raspberry-pi-3-model-b)
- [PID Driver for DC motor](http://www.roboconshop.com/San-Pham/%C4%90ien-tu/Driver-and-controller/PID-Driver-for-DC-motor.aspx)
- [STM32 F4]()

### Connect

| STM32F4 | Board pin | Physical RPi pin | RPi pin name | Beaglebone Black pin name |
|----------------|-----------|------------------|--------------| --------------------------|
| SDA            | 1         | 24               | GPIO8, CE0   | P9\_17, SPI0\_CS0         |
| SCK            | 2         | 23               | GPIO11, SCKL | P9\_22, SPI0\_SCLK        |
| MOSI           | 3         | 19               | GPIO10, MOSI | P9\_18, SPI0\_D1          |
| MISO           | 4         | 21               | GPIO9, MISO  | P9\_21, SPI0\_D0          |
| IRQ            | 5         | 18               | GPIO24       | P9\_15, GPIO\_48          |
| GND            | 6         | 6, 9, 20, 25     | Ground       | Ground                    |
| RST            | 7         | 22               | GPIO25       | P9\_23, GPIO\_49          |
| 3.3V           | 8         | 1,17             | 3V3          | VDD\_3V3                  |

### Clone project
```
git clone https://github.com/tooniesnguyen/pwm_driver

cd pwm_driver
```

### Building and Installing the Module
```
sudo echo 0x17 > /sys/bus/i2c/devices/i2c-1/new_device

make

sudo insmod my_i2c_driver.ko
```

### Uninstalling the Module

```
sudo rmmod my_i2c_driver.
```



### User Interface Code
```
/*CODE BEGIN Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h> // Include errno header
/*CODE END Includes  */

/*CODE BEGIN DEFINE */
#define DEVICE_PATH "/dev/mpu6050"
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
#define SWERVE_IOCTL_SEND_TARGET_SPEED _IOW(SWERVE_MODULE_MAGIC_WORD,DEV_REG_TARGET_SPEED,uint8_t)
#define SWERVE_IOCTL_SEND_TARGET_ANGLE _IOW(SWERVE_MODULE_MAGIC_WORD,DEV_REG_TARGET_ANGLE,uint8_t)
#define SWERVE_IOCTL_READ_CURRENT_ANGLE _IOR(SWERVE_MODULE_MAGIC_WORD,17,uint8_t)
#define SWERVE_IOCTL_SET_MODE _IOW(SWERVE_MODULE_MAGIC_WORD,DEV_REG_MODE,uint8_t)
/*CODE END DEFINE */

/*CODE BEGIN SET ARRAY*/
uint8_t txBuffer[10] = {0};
uint8_t rxBuffer[10] = {0};
/*CODE END SET ARRAY*/

void PutMessage(void* data, uint8_t sizeOfData)
{
    memcpy(txBuffer, data, sizeOfData);
}


int main() {
    int fd;

    /* USER BEGIN CONFIG CHANGE VALUE*/
    uint8_t targetSpeed = 99;
    uint8_t targetAngle = 45;
    uint8_t mode = 1;
    /* USER END CONFIG CHANGE VALUE*/

    // Open the device
    fd = open(DEVICE_PATH, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open the device");
        return errno;
    }

    // Write speed to DEV_REG_TARGET_SPEED
    if (ioctl(fd, SWERVE_IOCTL_SEND_TARGET_SPEED, &targetSpeed) < 0) {
        perror("Failed to send speed");
        close(fd);
        return errno;
    }
    
    // Write angle to DEV_REG_TARGET_ANGLE
    if (ioctl(fd, SWERVE_IOCTL_SEND_TARGET_ANGLE, &targetAngle) < 0) {
        perror("Failed to send angle");
        close(fd);
        return errno;
    }
    // Set mode to DEV_REG_MODE
    if (ioctl(fd, SWERVE_IOCTL_SET_MODE, &mode) < 0) {
        perror("Failed to set mode");
        close(fd);
        return errno;
    }
    
    /*
    // Read angle from DEV_REG_TARGET_ANGLE
    if (ioctl(fd, SWERVE_IOCTL_READ_CURRENT_ANGLE, &targetAngle) < 0) {
        perror("Failed to read angle");
        close(fd);
        return errno;
    }
    else {
        printf("Read Angle + 1: %u\n",targetAngle);
    }
    
*/

    // Close the device
    close(fd);
    return 0;
}

```



### Tips
Show the kernel log to check the install state or instantiated:
```
dmesg | tail 
```

To scan I2C bus 1 for connected I2C devices:
```
i2cdetect -y 1
```

To list all module installed:

```
ls /dev
```

