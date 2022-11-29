#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <err.h>

#define DEV "/dev/caesar_cipher"
#define IOC_SET_SHIFT _IO(0x11, 0)

int main() {
	char input1[] = "my name is nick";
	char output1[] = "qc reqi mw rmgo";
	char input2[] = "hello world";
	char output2[] = "pmttw ewztl";

        char buff[20];

        int fd;
        puts("testing caesar cipher");
        fd = open(DEV, O_RDWR);

        if (fd < 0) {
		err(1,"unable to open %s", DEV);
        }
        //the first cipher text is a four shift and second an eight
	if (write(fd, input1, sizeof(input1)) != sizeof(input1)) {
		err(1, "write error input size doesn't match output");
	}
	if (read(fd, buff, sizeof(buff)) < 0) {
		err(1, "read error");
	} else if (strcmp(buff, output1) != 0) {
		errx(1, "Cipher incorrect: %s expected but got %s\n", output1, buff);
	}
	if (ioctl(fd, IOC_SET_SHIFT, 8) < 0) {
		err(1, "ioctl error");
	}
	if (write(fd, input2, sizeof(input2)) != sizeof(input2)) {
		err(1, "write error input size doesn't match output");
	}
	if(read(fd, buff, sizeof(buff)) < 0) {
		err(1, "read error");
	} else if (strcmp(buff, output2) != 0) {
		errx(1,"Cipher incorrect: %s expected but got %s\n", output2, buff);
	}
	if (close(fd) < 0) {
		err(1, "close error");
	}
	puts("testing complete");
	return 0;
}
