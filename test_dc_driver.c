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
