#include <stdio.h>
#include <stdlib.h>
#include <CoreAudio/CoreAudio.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <dirent.h>

void restart_program(char *argv[]) {
    execv(argv[0], argv);
    perror("Error restarting program");
    exit(1);
}

char *find_default_serial_port() {
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir("/dev")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strncmp(ent->d_name, "tty.usbmodem", 11) == 0) {
                char *path = malloc(strlen("/dev/") + strlen(ent->d_name) + 1);
                if (path != NULL) {
                    strcpy(path, "/dev/");
                    strcat(path, ent->d_name);
                    closedir(dir);
                    return path;
                }
            }
        }
        closedir(dir);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    // Get serial port and baud rate from command-line arguments or use defaults
    char *serial_port;
    int baud_rate_input;

    if (argc == 3) {
        serial_port = argv[1];
        baud_rate_input = atoi(argv[2]);
    } else {
        serial_port = find_default_serial_port();
        if (serial_port == NULL) {
            fprintf(stderr, "No default serial port found\n");
            return 1;
        }
        baud_rate_input = 9600;
        fprintf(stderr, "Using default serial port: %s with baud rate: %d\n", serial_port, baud_rate_input);
    }

    if (baud_rate_input <= 0) {
        fprintf(stderr, "Invalid baud rate: %d\n", baud_rate_input);
        restart_program(argv);
    }

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
            restart_program(argv);
    }

    // Open the serial port
    int serial_fd = open(serial_port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial_fd == -1) {
        perror("Unable to open serial port");
        restart_program(argv);
    }

    // Set the file status flags to blocking mode
    if (fcntl(serial_fd, F_SETFL, 0) == -1) {
        perror("Unable to set serial port to blocking mode");
        close(serial_fd);
        restart_program(argv);
    }

    // Configure the serial port
    struct termios options;
    if (tcgetattr(serial_fd, &options) == -1) {
        perror("Error getting serial port attributes");
        close(serial_fd);
        restart_program(argv);
    }

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

    if (tcsetattr(serial_fd, TCSANOW, &options) == -1) {
        perror("Error setting serial port attributes");
        close(serial_fd);
        restart_program(argv);
    }

    // Prepare for microphone status check
    AudioDeviceID deviceID = 0;
    UInt32 size = sizeof(deviceID);
    AudioObjectPropertyAddress defaultInputDevicePropertyAddress = {
        kAudioHardwarePropertyDefaultInputDevice,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };
    OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject,
                                                 &defaultInputDevicePropertyAddress,
                                                 0, NULL, &size, &deviceID);
    if (status != noErr) {
        fprintf(stderr, "Error getting default input device\n");
        close(serial_fd);
        restart_program(argv);
    }

    UInt32 isRunning = 0;
    size = sizeof(isRunning);
    AudioObjectPropertyAddress isRunningPropertyAddress = {
        kAudioDevicePropertyDeviceIsRunningSomewhere,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };

    // Main loop
    while (1) {
        status = AudioObjectGetPropertyData(deviceID,
                                            &isRunningPropertyAddress,
                                            0, NULL, &size, &isRunning);
        if (status != noErr) {
            fprintf(stderr, "Error getting device running status\n");
            close(serial_fd);
            restart_program(argv);
        }

        ssize_t bytes_written;
        if (isRunning) {
            // Microphone is in use
            bytes_written = write(serial_fd, "1", 1);
        } else {
            // Microphone is not in use
            bytes_written = write(serial_fd, "0", 1);
        }

        if (bytes_written == -1) {
            perror("Error writing to serial port");
            close(serial_fd);
            restart_program(argv);
        }

        sleep(1); // Check every second
    }

    if (close(serial_fd) == -1) {
        perror("Error closing serial port");
        restart_program(argv);
    }

    return 0;
}
