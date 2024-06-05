#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h> // Include errno header
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

uint8_t txBuffer[10] = {0};
uint8_t rxBuffer[10] = {0};

void PutMessage(void* data, uint8_t sizeOfData)
{
    memcpy(txBuffer, data, sizeOfData);
}


int main() {
    int fd;
    uint8_t targetSpeed = 99;
    uint8_t targetAngle = 45;
    uint8_t mode = 1;

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