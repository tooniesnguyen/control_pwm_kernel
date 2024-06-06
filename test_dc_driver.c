#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h> // Include errno header

//Define device path in /dev direction
#define DEVICE_PATH "/dev/dc_driver"

//Define cmd for in out control 
#define SWERVE_MODULE_MAGIC_WORD 's'
#define SWERVE_IOCTL_SEND_TARGET_SPEED _IOW(SWERVE_MODULE_MAGIC_WORD,DEV_REG_TARGET_SPEED,uint8_t)
#define SWERVE_IOCTL_SEND_TARGET_ANGLE _IOW(SWERVE_MODULE_MAGIC_WORD,DEV_REG_TARGET_ANGLE,uint8_t)
#define SWERVE_IOCTL_READ_CURRENT_ANGLE _IOR(SWERVE_MODULE_MAGIC_WORD,17,int)

//Define an enum for module's register
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


//Define variables for target speed, target angle, and mode
uint8_t targetSpeed;
uint8_t targetAngle;

int main() {
    int fd;
    int readData;

	// Open the device
	fd = open(DEVICE_PATH, O_RDWR);
	if (fd < 0) {
		perror("Failed to open the device");
		return errno;
	}
    
	//Print the input format for user
    printf("Speed - Angle \n");
	
    while(1)
    {
        //Get and set the input value for target speed and target angle variables
        scanf("%u %u",&targetSpeed,&targetAngle);
        
		//In out control with cmd SWERVE_IOCTL_SEND_TARGET_SPEED for sending target speed to module
        if (ioctl(fd, SWERVE_IOCTL_SEND_TARGET_SPEED, &targetSpeed) < 0) {
            perror("Failed to send speed");
            close(fd);
            return errno;
        }
        
		//In out control with cmd SWERVE_IOCTL_SEND_TARGET_ANGLE for sending mode to module
        if (ioctl(fd, SWERVE_IOCTL_SEND_TARGET_ANGLE, &targetAngle) < 0) {
            perror("Failed to send angle");
            close(fd);
            return errno;
        } 
    }
    
    //In out control with cmd SWERVE_IOCTL_READ_CURRENT_ANGLE for reading current angle from module
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
