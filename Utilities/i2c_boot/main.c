/* 
 * File:   main.c
 * Author: Jason Lefley
 *
 * Created on March 4, 2015, 3:20 PM
 */

#include <stdlib.h>
#include <stdio.h> 
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <stdint.h>

#define I2C_ADDRESS 0x29

// From twiboot
#define CMD_READ_VERSION	0x01
#define CMD_READ_CHECKSUM   0x03
#define CMD_SWITCH_APPLICATION	CMD_READ_VERSION
#define BOOTTYPE_APPLICATION	0x80

int i2c_open(unsigned char bus, unsigned char addr) {
    int file;
    char filename[16];
    sprintf(filename, "/dev/i2c-%d", bus);
    if ((file = open(filename, O_RDWR)) < 0) {
        fprintf(stderr, "i2c_open open error: %s\n", strerror(errno));
        return (file);
    }
    if (ioctl(file, I2C_SLAVE, addr) < 0) {
        fprintf(stderr, "i2c_open ioctl error: %s\n", strerror(errno));
        return (-1);
    }
    return (file);
}

int i2c_close(int handle) {
    if ((close(handle)) != 0) {
        fprintf(stderr, "i2c_close error: %s\n", strerror(errno));
        return (-1);
    }
    return (0);
}

int i2c_write(int handle, unsigned char* buf, unsigned int length) {
    if (write(handle, buf, length) != length) {
        fprintf(stderr, "i2c_write error: %s\n", strerror(errno));
        return (-1);
    }
    return (length);
}

int i2c_read(int handle, unsigned char* buf, unsigned int length) {
    if (read(handle, buf, length) != length) {
        fprintf(stderr, "i2c_read error: %s\n", strerror(errno));
        return (-1);
    }
    return (length);
}

int switch_application(int i2c_fd) {
    unsigned char cmd[] = {CMD_SWITCH_APPLICATION, BOOTTYPE_APPLICATION};

    if(i2c_write(i2c_fd, cmd, sizeof (cmd)) == -1)
        return -1;

    return 0;
}

int read_version(int i2c_fd, unsigned char* buffer, int length) {
    unsigned char cmd[] = {CMD_READ_VERSION};

    if (i2c_write(i2c_fd, cmd, sizeof (cmd)) == -1)
        return -1;

    memset(buffer, 0, length);

    if (i2c_read(i2c_fd, buffer, length) == -1)
        return -1;

    int i;
    for (i = 0; i < length; i++)
        buffer[i] &= ~0x80;

    return 0;
}

int read_checksum(int i2c_fd, unsigned char* buffer, int length) {
    unsigned char cmd[] = {CMD_READ_CHECKSUM};

    if(i2c_write(i2c_fd, cmd, sizeof (cmd)) == -1)
        return -1;
   
    memset(buffer, 0, length);
    
    if (i2c_read(i2c_fd, buffer, length) == -1)
        return -1;

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Must specify I2C bus number and command\n");
        return -1;
    }
   
    int returnVal = -1;
    int fd = i2c_open(atoi(argv[1]), I2C_ADDRESS);

    if (fd == -1)
        return -1;

    if (strcmp(argv[2], "s") == 0) {
        
        returnVal = switch_application(fd);
        
    } else if (strcmp(argv[2], "v") == 0) {

        unsigned char version[18];
        version[17] = '\0';

        returnVal = read_version(fd, version, sizeof (version) - 1);

        if (returnVal != -1)
            printf("%s\n", version);

    } else if (strcmp(argv[2], "c") == 0) {
        unsigned char buffer[2];

        returnVal = read_checksum(fd, buffer, sizeof(buffer));

        if (returnVal != -1) {
            printf("%02X %02X\n", buffer[0], buffer[1]);
        }
    } else {
        fprintf(stderr, "Unknown command\n");
    }

    i2c_close(fd);
    return returnVal;
}

