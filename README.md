# Driver Control PWM Motor

### Hardware requirements
- [Raspberry Pi 3 Model B+](https://raspberrypi.vn/san-pham/raspberry-pi-3-model-b)
- [PID Driver for DC motor](http://www.roboconshop.com/San-Pham/%C4%90ien-tu/Driver-and-controller/PID-Driver-for-DC-motor.aspx)
- [STM32 F103C8T6]()

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
sudo su

cd /sys/bus/i2c/devices/i2c-1/

echo dc_driver 0x17 > new_device

exit

make

sudo insmod my_i2c_driver.ko
```

### Uninstalling the Module

```
sudo rmmod my_i2c_driver.
```



### Run User Interface Code

```
gcc -o test_dc_driver test_dc_driver.c -lm

sudo ./test_dc_driver
```

### Example send speed mode
```
pi@raspberrypi:~/Desktop/Nhan_CDT/temp1/pwm_driver $ sudo ./test_dc_driver
Speed - Angle
90 0
```

> :warning: Warning: Khi chuyển từ mode vận tốc sang góc hoặc ngược lại cần truyền trước tham sô `0 0`
```
pi@raspberrypi:~/Desktop/Nhan_CDT/temp1/pwm_driver $ sudo ./test_dc_driver
Speed - Angle 
90 0
0  0
0 40
```

### Example send Angle mode
```
pi@raspberrypi:~/Desktop/Nhan_CDT/temp1/pwm_driver $ sudo ./test_dc_driver
Speed - Angle
0 120
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

