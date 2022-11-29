#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <err.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdio.h>

#define DEV "/dev/timezone"

#define IOC_SET_HOUR_AM _IO(0x11,0)
#define IOC_SET_HOUR_PM _IO(0x11,1)

enum time_zone {
	EST,
	CST,
	MST,
	PST,
	AKST,
	HST,
	NUM_ZONES,
};

static const char *zone_name[NUM_ZONES] = {
	[EST] = "EST",
	[CST] = "CST",
	[MST] = "MST",
	[PST] = "PST",
	[AKST] = "AKST",
	[HST] = "HST",
};

static const char *expected_output[NUM_ZONES][24] = {
	[EST] = {
		"1 AM", "2 AM", "3 AM", "4 AM", "5 AM", "6 AM", "7 AM", "8 AM", "9 AM", "10 AM", "11 AM", "12 AM",
		"1 PM", "2 PM", "3 PM", "4 PM", "5 PM", "6 PM", "7 PM", "8 PM", "9 PM", "10 PM", "11 PM", "12 PM",
	},
	[CST] = {
		"12 AM", "1 AM", "2 AM", "3 AM", "4 AM", "5 AM", "6 AM", "7 AM", "8 AM", "9 AM", "10 AM", "11 PM",
		"12 PM", "1 PM", "2 PM", "3 PM", "4 PM", "5 PM", "6 PM", "7 PM", "8 PM", "9 PM", "10 PM", "11 AM",
	},
	[MST] = {
		"11 PM", "12 AM", "1 AM", "2 AM", "3 AM", "4 AM", "5 AM", "6 AM", "7 AM", "8 AM", "9 AM", "10 PM",
		"11 AM", "12 PM", "1 PM", "2 PM", "3 PM", "4 PM", "5 PM", "6 PM", "7 PM", "8 PM", "9 PM", "10 AM",
	},
	[PST] = {
		"10 PM", "11 PM", "12 AM", "1 AM", "2 AM", "3 AM", "4 AM", "5 AM", "6 AM", "7 AM", "8 AM", "9 PM",
		"10 AM", "11 AM", "12 PM", "1 PM", "2 PM", "3 PM", "4 PM", "5 PM", "6 PM", "7 PM", "8 PM", "9 AM",
	},
	[AKST] = {
		"9 PM", "10 PM", "11 PM", "12 AM", "1 AM", "2 AM", "3 AM", "4 AM", "5 AM", "6 AM", "7 AM", "8 PM",
		"9 AM", "10 AM", "11 AM", "12 PM", "1 PM", "2 PM", "3 PM", "4 PM", "5 PM", "6 PM", "7 PM", "8 AM",
	},
	[HST] = {
		"8 PM", "9 PM", "10 PM", "11 PM", "12 AM", "1 AM", "2 AM", "3 AM", "4 AM", "5 AM", "6 AM", "7 PM",
		"8 AM", "9 AM", "10 AM", "11 AM", "12 PM", "1 PM", "2 PM", "3 PM", "4 PM", "5 PM", "6 PM", "7 AM",
	},
};

#define IOCTL(REQ, ARG) \
	do { \
		if (ioctl(fd, REQ, ARG) < 0) \
			err(1, "ioctl error"); \
	} while(0)

#define WRITE(STR, LEN) \
	do { \
		if (write(fd, STR, LEN) < 0) \
			err(1, "write error"); \
	} while(0)

#define READ() \
	do { \
		if ((ret = read(fd, read_buf, sizeof read_buf)) < 0) \
			err(1, "read error"); \
	} while(0)


int main(void)
{
	puts("testing time zone converter");
	int fd = open(DEV, O_RDWR);
	if (fd < 0)
		err(1, "open error");

	for (enum time_zone zone = 0; zone < NUM_ZONES; ++zone) {
		const char *name = zone_name[zone], **ptr = expected_output[zone];
		char read_buf[20];
		ssize_t ret;
		WRITE(name, strlen(name));
		for(int i = 1; i <= 12; ++i) {
			IOCTL(IOC_SET_HOUR_AM, i);
			READ();
			if (strncmp(*ptr++, read_buf, (size_t)ret) != 0)
				errx(1, "incorrect output. Expected %s got %.*s",
					ptr[-1], (int)ret, read_buf);
		}
		for(int i = 1; i <= 12; ++i) {
			IOCTL(IOC_SET_HOUR_PM, i);
			READ();
			if (strncmp(*ptr++, read_buf, (size_t)ret) != 0)
				errx(1, "incorrect output. Expected %s got %.*s",
					ptr[-1], (int)ret, read_buf);
		}
	}


	if (close(fd) < 0)
		err(1, "close error");

	puts("tests have passed");
	return 0;

}
