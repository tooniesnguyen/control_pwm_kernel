# Driver Control PWM Motor

### Hardware requirements
- [Raspberry Pi 3 Model B+](https://raspberrypi.vn/san-pham/raspberry-pi-3-model-b)
- [PID Driver for DC motor](http://www.roboconshop.com/San-Pham/%C4%90ien-tu/Driver-and-controller/PID-Driver-for-DC-motor.aspx)
- [STM32 F4]()

### Connect

|H Bridge Driver pin| STM32F1 | Board pin | Physical RPi pin | RPi pin name | 
|-------------------|---------|-----------|------------------|--------------| 
|                   | SDA     | PB7       | 3                | SDA.1        |
|                   | SCK     | PB6       | 5                | SCL.1        | 
|DIR                | DIR     | PA0       |                  |              | 
|PWM                | PWM     | PA2       |                  |              |
|                   | ENA     | PA6       |                  |              | 
|                   | ENB     | PA7       |                  |              |
|GND                | GND     | GND       | 9                | GND          |
|5V                 | 5V      | 5V        |                  |              |
|24V                |         |           |                  |              |
|0V                 |         |           |                  |              |
|MA                 |         |           |                  |              |
|MB                 |         |           |                  |              |

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
#define DEVICE_PATH "/dev/dc_driver"
#define SWERVE_MODULE_MAGIC_WORD 's'
#define SWERVE_IOCTL_SEND_TARGET_SPEED _IOW(SWERVE_MODULE_MAGIC_WORD,DEV_REG_TARGET_SPEED,uint8_t)
#define SWERVE_IOCTL_SEND_TARGET_ANGLE _IOW(SWERVE_MODULE_MAGIC_WORD,DEV_REG_TARGET_ANGLE,uint8_t)
#define SWERVE_IOCTL_SET_MODE _IOW(SWERVE_MODULE_MAGIC_WORD,DEV_REG_MODE,uint8_t)
#define SWERVE_IOCTL_READ_CURRENT_ANGLE _IOR(SWERVE_MODULE_MAGIC_WORD,17,int)
/*CODE END DEFINE */

/*START INITIALIZING VARIABLES*/
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

uint8_t txBuffer[10] = {0};
uint8_t rxBuffer[10] = {0};

uint8_t targetSpeed;
uint8_t targetAngle;
uint8_t mode_dc;
uint8_t stage = 0;
/*END INITIALIZING VARIABLES*/


void PutMessage(void* data, uint8_t sizeOfData)
{
    memcpy(txBuffer, data, sizeOfData);
}

void inputBegin(void)
{
    printf("Mode 0: Disable motor \n");
    printf("Mode 1: PID Speed motor \n");
    printf("Mode 2: PID Position motor \n");
    printf("Please select mode: \n");
    
}



int main() {
    int fd;
    int readData;

        // Open the device
    fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("Failed to open the device");
        return errno;
    }
           
    printf("Speed - Angle \n");
    while(1)
    {
        //scanf("%u %u %u",&mode_dc ,&targetSpeed,&targetAngle);
        scanf("%u %u",&targetSpeed,&targetAngle);
        
        if (ioctl(fd, SWERVE_IOCTL_SEND_TARGET_SPEED, &targetSpeed) < 0) {
            perror("Failed to send speed");
            close(fd);
            return errno;
        }
        
        if (ioctl(fd, SWERVE_IOCTL_SET_MODE, &mode_dc) < 0) {
            perror("Failed to set mode");
            close(fd);
            return errno;
        }
        
        if (ioctl(fd, SWERVE_IOCTL_SEND_TARGET_ANGLE, &targetAngle) < 0) {
            perror("Failed to send angle");
            close(fd);
            return errno;
        }
        
        
    
        
    }
    
    
    /*
    // Read angle from DEV_REG_TARGET_ANGLE
    if (ioctl(fd, SWERVE_IOCTL_READ_CURRENT_ANGLE, &readData) < 0) {
        perror("Failed to read angle");
        close(fd);
        return errno;
    }
        printf("Read Angle + 1: %d\n",readData);
    */
    
    printf("Out of while loop");
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

