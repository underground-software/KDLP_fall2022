#include <stdio.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>

#define MESSAGE_SIZE 100
#define IOC_EAT _IO(0x11, 0)
#define IOC_EXERCISE _IO(0x11, 1)
#define open_(fd) fd = check(open("/dev/kpet", O_RDWR), "open")
#define release_(fd) check(close(fd), "close")
#define read_(fd, resp) do { \
	check(read(fd, msg, MESSAGE_SIZE), "read"); \
	if(strcmp(msg,resp)) \
		errx(1, "incorrect response. Expected %s got %s", resp, msg); \
	} while (0)
#define write_(fd, msg) check(write(fd, msg, sizeof(msg)), "write")
#define ioctl_(fd, action) check(ioctl(fd, action, 0), "ioctl")

static int check(int n, char *op_name)
{
	if (n < 0)
		err(errno, "fail to %s kpet", op_name);
	return n;
}

extern int errno;

int main(void)
{
	char msg[MESSAGE_SIZE];
	int fd;

	open_(fd);
	read_(fd, "*the pet does not respond*");
	write_(fd, "Hello.");
	read_(fd, "Hello, I am a pet in the kernel.");
	write_(fd, "How are you?");
	read_(fd, "I am alive, my stomach is 10/10 full.");
	ioctl_(fd, IOC_EXERCISE);
	read_(fd, "I am alive, my stomach is 9/10 full.");
	write_(fd, "Hello.");
	read_(fd, "Hello, I am a pet in the kernel.");
	ioctl_(fd, IOC_EAT);
	read_(fd, "Hello, I am a pet in the kernel.");
	write_(fd, "How are you?");
	read_(fd, "I am alive, my stomach is 10/10 full.");
	for(int i = 0; i < 10; ++i)
		ioctl_(fd, IOC_EXERCISE);
	read_(fd, "*the pet is dead*");
	close(fd);
	open_(fd);
	write_(fd, "How are you?");
	read_(fd, "I am alive, my stomach is 10/10 full.");
	ioctl_(fd, IOC_EAT);
	read_(fd, "*the pet is dead*");
	close(fd);

	puts("testing complete");
	return 0;
}
