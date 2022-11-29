#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <err.h>
#include <string.h>
#include <sys/ioctl.h>

#define DEV "/dev/text_filter"

struct filter {
	const char *data;
	size_t length;
};

#define IOC_SET_FILTER _IOW(0x11, 0, struct filter)

int main(void)
{
	int fd;
	char test_string1[] = "the eternal return is the being of becoming";
	char test_string2[] = "aaaa aaaa aaaa aaaa";
	char buf[128];
	if((fd = open(DEV, O_RDWR)) < 0) err(1, "failed to open driver");
	if(ioctl(fd, IOC_SET_FILTER, &(struct filter){"being", 5}) < 0) err(1, "failed to ioctlize the driver");
	if((unsigned)write(fd, test_string1, sizeof test_string1) < sizeof(test_string1)) err(1, "failed to write to driver");
	if((unsigned)read(fd, buf, sizeof buf) < (sizeof test_string1)) err(1, "failed to read from driver");
	if(strcmp("the eternal return is the ***** of becoming", buf) != 0) errx(2, "test 1: output doesn't look right. Got %s", buf);

	if(ioctl(fd, IOC_SET_FILTER, &(struct filter){"a", 1}) < 0) err(1, "failed to ioctlize the driver");
	if((unsigned)write(fd, test_string2, sizeof test_string2) < sizeof(test_string2)) err(1, "failed to write to driver");
	if((unsigned)read(fd, buf, sizeof buf) < (sizeof test_string2)) err(1, "failed to read from driver");
	if(strcmp("**** **** **** ****", buf) != 0) errx(2, "test 2: output doesn't look right. Got %s", buf);

	if(ioctl(fd, IOC_SET_FILTER, &(struct filter){"aaa", 3}) < 0) err(1, "failed to ioctlize the driver");
	if((unsigned)write(fd, test_string2, sizeof test_string2) < sizeof(test_string2)) err(1, "failed to write to driver");
	if((unsigned)read(fd, buf, sizeof buf) < (sizeof test_string2)) err(1, "failed to read from driver");
	if(strcmp("***a ***a ***a ***a", buf) != 0) errx(2, "test 3: output doesn't look right. Got %s", buf);
	if(close(fd) < 0) err(1, "failed to close driver");
	return 0;
}
