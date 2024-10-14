#include <stdio.h>
#include <stdlib.h>
#include <CoreAudio/CoreAudio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>

int main(int argc, char *argv[]) {
    // Check command-line arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <serial_port> <baud_rate>\n", argv[0]);
        fprintf(stderr, "Example: %s /dev/tty.usbserial 9600\n", argv[0]);
        return 1;
    }

    // Get serial port and baud rate from command-line arguments
    char *serial_port = argv[1];
    int baud_rate_input = atoi(argv[2]);

    // Map integer baud rate to termios baud rate constants
    speed_t baud_rate;
    switch (baud_rate_input) {
        case 1200: baud_rate = B1200; break;
        case 1800: baud_rate = B1800; break;
        case 2400: baud_rate = B2400; break;
        case 4800: baud_rate = B4800; break;
        case 9600: baud_rate = B9600; break;
        case 19200: baud_rate = B19200; break;
        case 38400: baud_rate = B38400; break;
        case 57600: baud_rate = B57600; break;
        case 115200: baud_rate = B115200; break;
        case 230400: baud_rate = B230400; break;
        default:
            fprintf(stderr, "Unsupported baud rate: %d\n", baud_rate_input);
            return 1;
    }

    // Open the serial port
    int serial_fd = open(serial_port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_fd == -1) {
        perror("Unable to open serial port");
        return 1;
    }

    // Configure the serial port
    struct termios options;
    tcgetattr(serial_fd, &options);

    cfsetispeed(&options, baud_rate);
    cfsetospeed(&options, baud_rate);

    options.c_cflag |= (CLOCAL | CREAD);  // Enable receiver, set local mode
    options.c_cflag &= ~PARENB;           // No parity
    options.c_cflag &= ~CSTOPB;           // 1 stop bit
    options.c_cflag &= ~CSIZE;            // Mask data size
    options.c_cflag |= CS8;               // 8 data bits
    options.c_cflag &= ~CRTSCTS;          // No hardware flow control

    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // Raw input
    options.c_iflag &= ~(IXON | IXOFF | IXANY);         // No software flow control
    options.c_oflag &= ~OPOST;                          // Raw output

    tcsetattr(serial_fd, TCSANOW, &options);

    // Prepare for microphone status check
    AudioDeviceID deviceID = 0;
    UInt32 size = sizeof(deviceID);
    AudioObjectPropertyAddress defaultInputDevicePropertyAddress = {
        kAudioHardwarePropertyDefaultInputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain // Updated to ElementMain
    };
    OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                                 &defaultInputDevicePropertyAddress,
                                                 0, NULL, &size, &deviceID);
    if (status != noErr) {
        fprintf(stderr, "Error getting default input device\n");
        close(serial_fd);
        return 1;
    }

    UInt32 isRunning = 0;
    size = sizeof(isRunning);
    AudioObjectPropertyAddress isRunningPropertyAddress = {
        kAudioDevicePropertyDeviceIsRunningSomewhere,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain // Updated to ElementMain
    };

    // Main loop
    while (1) {
        status = AudioObjectGetPropertyData(deviceID,
                                            &isRunningPropertyAddress,
                                            0, NULL, &size, &isRunning);
        if (status != noErr) {
            fprintf(stderr, "Error getting device running status\n");
            close(serial_fd);
            return 1;
        }

        if (isRunning) {
            // Microphone is in use
            // printf("Microphone is in use\n");
            if (write(serial_fd, "1", 1) == -1) {
                perror("Error writing to serial port");
            }
        } else {
            // Microphone is not in use
            // printf("Microphone is not in use\n");
            if (write(serial_fd, "0", 1) == -1) {
                perror("Error writing to serial port");
            }
        }

        sleep(1); // Check every second
    }

    close(serial_fd);

    return 0;
}
