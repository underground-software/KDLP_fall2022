#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <err.h>

#define IOC_SET_ROT _IO(0x11, 0)
#define DEV_FILE "/dev/rotN"

static void read_test(int fd, char *target, ssize_t target_len, int test_num)
{
	char buf[32];
	ssize_t ret_val;

	ret_val = read(fd, buf, sizeof(buf));
	if (ret_val < 0)
		err(1, "unable to read from file");
	if (ret_val == 0)
		errx(1, "EOF from read (nothing read)");
	if (target_len != ret_val)
		errx(1, "Test %d: Wrong read length. Got %zu, expected %zu",
			test_num, ret_val, target_len);
	if (strncmp(target, buf, target_len))
		errx(1, "Test %d: Bad Rotation. Got %s, expected %s",
			test_num, buf, target);
}

int main(void)
{
	char fox[] = "quick brown fox blah blah blah";
	char hello[] = "Hello W0rld";
	char numbers[] = "867-5309";
	char symbols[] = "!@#$!#@()%?><:right?";
	char fox_rot13[] = "dhvpx oebja sbk oynu oynu oynu";
	char hello_rot13[] = "Uryyb J3eyq";
	char num_rot13[] = "190-8632";
	char sym_rot13[] = "!@#$!#@()%?><:evtug?";
	char num_rot5[] = "312-0854";
	int test = 1;
	int fd;
	puts("testing rotation driver");
	fd = open(DEV_FILE, O_RDWR);
	if (fd < 0)
		err(1, "unable to open rotN file.");
	/* Make all of the strings and their rotated equivalents */

	if (sizeof(hello) != write(fd, hello, sizeof(hello)))
		err(1, "error writing to file");
	read_test(fd, hello_rot13, sizeof(hello_rot13), test++);
	if (sizeof(fox) != write(fd, fox, sizeof(fox)))
		err(1, "error writing to file");
	read_test(fd, fox_rot13, sizeof(fox_rot13), test++);
	if (sizeof(fox_rot13) != write(fd, fox_rot13, sizeof(fox_rot13)))
		err(1, "error writing to file");
	read_test(fd, fox, sizeof(fox), test++);
	if (sizeof(numbers) != write(fd, numbers, sizeof(numbers)))
		err(1, "error writing to file");
	read_test(fd, num_rot13, sizeof(num_rot13), test++);
	if (ioctl(fd, IOC_SET_ROT, 5) < 0)
		err(1, "error performing ioctl on file");
	read_test(fd, num_rot5, sizeof(num_rot5), test++);
	if (sizeof(num_rot5) != write(fd, num_rot5, sizeof(num_rot5)))
		err(1, "error writing to file");
	read_test(fd, numbers, sizeof(numbers), test++);
	if (sizeof(symbols) != write(fd, symbols, sizeof(symbols)))
		err(1, "error writing to file");
	if (ioctl(fd, IOC_SET_ROT, 13) < 0)
		err(1, "error performing ioctl on file");
	read_test(fd, sym_rot13, sizeof(sym_rot13), test++);

	if (close(fd) < 0)
		err(1, "close error");

	puts("testing complete");
	return 0;
}
