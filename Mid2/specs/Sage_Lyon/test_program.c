#include <err.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define DEV "/dev/calculator"

#define IOC_OP1 _IO(0x11, 0)
#define IOC_OP2 _IO(0x11, 1)

enum operation {
	ADDITION,
	SUBTRACTION,
	MULTIPLICATION,
	DIVISION,
};

static int32_t add_(int32_t a, int32_t b){return a + b;}
static int32_t sub_(int32_t a, int32_t b){return a - b;}
static int32_t mul_(int32_t a, int32_t b){return a * b;}
static int32_t div_(int32_t a, int32_t b){return a / b;}

static int32_t (*evaluate[])(int32_t, int32_t) = {
	[ADDITION] = add_,
	[SUBTRACTION] = sub_,
	[MULTIPLICATION] = mul_,
	[DIVISION] = div_,
};

static char symbol[] = {
	[ADDITION] = '+',
	[SUBTRACTION] = '-',
	[MULTIPLICATION] = '*',
	[DIVISION] = '/',
};

#define IOCTL(REQ, ARG) \
	do { \
		if (ioctl(fd, REQ, ARG) < 0) \
			err(1, "ioctl error"); \
	} while(0)

#define WRITE(STR) \
	do { \
		if (write(fd, STR, 1) < 0) \
			err(1, "write error"); \
	} while(0)

#define READ() \
	do { \
		if ((ret = read(fd, read_buf, sizeof read_buf)) < 0) \
			err(1, "read error"); \
	} while(0)

static void test_case(int fd, int32_t a, int32_t b, enum operation op, bool valid)
{
	char read_buf[20];
	ssize_t ret;
	IOCTL(IOC_OP1, a);
	IOCTL(IOC_OP2, b);
	WRITE(&symbol[op]);
	READ();
	if (valid) {
		int32_t answer = evaluate[op](a, b);
		char ans_buf[20];
		snprintf(ans_buf, sizeof ans_buf, "%d", answer);
		if (strncmp(ans_buf, read_buf, (size_t)ret) != 0) {
			errx(1, "incorrect response. Expected %d %c %d = %d; got %.*s",
				a, symbol[op], b, answer, (int)ret, read_buf);
		}
	} else {
		if (strncmp("invalid", read_buf, (size_t)ret) != 0) {
			errx(1, "incorrect response. Execpted invalid; got %.*s",
				(int)ret, read_buf);
		}
	}
}

int main(void)
{
	puts("testing calculator");
	char read_buf[20];
	ssize_t ret;
	int fd = open(DEV, O_RDWR);
	if (fd < 0)
		err(1, "open error");

	//1 + 1 = 2 right?
	test_case(fd, 1, 1, ADDITION, true);

	//integer overflow is invalid
	test_case(fd, INT32_MAX, 1, ADDITION, false);
	test_case(fd, INT32_MIN, 1, SUBTRACTION, false);
	test_case(fd, INT32_MAX, 2, MULTIPLICATION, false);
	test_case(fd, INT32_MIN, -1, DIVISION, false);

	//division by zero is invalid
	test_case(fd, 1, 0, DIVISION, false);


	//switch operator without changing operands
	IOCTL(IOC_OP1, 5);
	IOCTL(IOC_OP2, 3);

	//ADDITION
	WRITE("+");
	READ();
	if (strncmp("8", read_buf, (size_t)ret) != 0)
		errx(1, "5 + 3 = 8. Got %.*s", (int)ret, read_buf);

	//SUBTRACTION
	WRITE("-");
	READ();
	if (strncmp("2", read_buf, (size_t)ret) != 0)
		errx(1, "5 - 3 = 2. Got %.*s", (int)ret, read_buf);

	//MULTIPLICATION
	WRITE("*");
	READ();
	if (strncmp("15", read_buf, (size_t)ret) != 0)
		errx(1, "5 * 3 = 15. Got %.*s", (int)ret, read_buf);

	//DIVISION
	WRITE("/");
	READ();
	if (strncmp("1", read_buf, (size_t)ret) != 0)
		errx(1, "5 / 3 = 1. Got %.*s", (int)ret, read_buf);

	//switch operands without changing operation
	IOCTL(IOC_OP1, 15);
	READ();
	if (strncmp("5", read_buf, (size_t)ret) != 0)
		errx(1, "15 / 3 = 5. Got %.*s", (int)ret, read_buf);

	for(int i = 0; i < 1000; ++i) {
		int32_t a = rand() - (RAND_MAX / 2);
		int32_t b = rand() - (RAND_MAX / 2);
		test_case(fd, a, b, ADDITION, true);
		test_case(fd, a, b, SUBTRACTION, true);
		test_case(fd, a, b, MULTIPLICATION, a==(int16_t)a && b==(int16_t)b);
		test_case(fd, a, b, DIVISION, b != 0);
	}

	if (close(fd) < 0)
		err(1, "close error");

	puts("testing complete");
	return 0;
}
