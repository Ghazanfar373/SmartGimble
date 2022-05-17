

## Serial Port Configuration
sudo usermod -a -G dialout $USER
ls -l /dev/ttyS* /dev/ttyACM* /dev/ttyUSB*
ls -ltrh /dev/ttyS* /dev/ttyACM* /dev/ttyUSB*

## Enable uart on Vim3 pro
overlays=uart3 pwm_ao_a pwm_f i2c0
reboot


## Compilation of cpp tracker with lnker libs
g++ tracker.cpp -0 tracker -I/usr/local/include/opencv -L/usr/local/lib -lopencv_core -lopencv_tracking -lopencvimgproc -libopencv_highgui -lopencv_videoio -L/usr/include/libserial -lserial

## Test opencv linked and installed
To check cv2.so link file         > ldd cv2.so
unlink is a similar command to rm > unlink cv2.so

## Debug SerialPort on Linux 
1. sudo apt install screen    
2. screen /dev/ttyS3  115200
