
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define DEV "/dev/number_scrabble"

#define IOC_RESET _IO(0x11, 0)

struct state {
	const char *board;
	int next_move;
};

static const struct state expected[] = {
	//player 1 win
	{
		"FREE: 123456789\n"
		"PLR1: \n"
		"PLR2: \n",
		4,
	},
	{
		"FREE: 12356789\n"
		"PLR1: 4\n"
		"PLR2: \n",
		1,
	},
	{
		"FREE: 2356789\n"
		"PLR1: 4\n"
		"PLR2: 1\n",
		5,
	},
	{
		"FREE: 236789\n"
		"PLR1: 45\n"
		"PLR2: 1\n",
		2,
	},
	{
		"FREE: 36789\n"
		"PLR1: 45\n"
		"PLR2: 12\n",
		6,
	},
	{
		"FREE: 3789\n"
		"PLR1: 456\n"
		"PLR2: 12\n"
		"Player 1 Wins (456)\n",
		0,
	},
	//player 2 win
	{
		"FREE: 123456789\n"
		"PLR1: \n"
		"PLR2: \n",
		1,
	},
	{
		"FREE: 23456789\n"
		"PLR1: 1\n"
		"PLR2: \n",
		4,
	},
	{
		"FREE: 2356789\n"
		"PLR1: 1\n"
		"PLR2: 4\n",
		2,
	},
	{
		"FREE: 356789\n"
		"PLR1: 12\n"
		"PLR2: 4\n",
		5,
	},
	{
		"FREE: 36789\n"
		"PLR1: 12\n"
		"PLR2: 45\n",
		3,
	},
	{
		"FREE: 6789\n"
		"PLR1: 123\n"
		"PLR2: 45\n",
		6,
	},
	{
		"FREE: 789\n"
		"PLR1: 123\n"
		"PLR2: 456\n"
		"Player 2 Wins (456)\n",
		0,
	},
	//tie game
	{
		"FREE: 123456789\n"
		"PLR1: \n"
		"PLR2: \n",
		1,
	},
	{
		"FREE: 23456789\n"
		"PLR1: 1\n"
		"PLR2: \n",
		9,
	},
	{
		"FREE: 2345678\n"
		"PLR1: 1\n"
		"PLR2: 9\n",
		2,
	},
	{
		"FREE: 345678\n"
		"PLR1: 12\n"
		"PLR2: 9\n",
		8,
	},
	{
		"FREE: 34567\n"
		"PLR1: 12\n"
		"PLR2: 89\n",
		3,
	},
	{
		"FREE: 4567\n"
		"PLR1: 123\n"
		"PLR2: 89\n",
		7,
	},
	{
		"FREE: 456\n"
		"PLR1: 123\n"
		"PLR2: 789\n",
		4,
	},
	{
		"FREE: 56\n"
		"PLR1: 1234\n"
		"PLR2: 789\n",
		6,
	},
	{
		"FREE: 5\n"
		"PLR1: 1234\n"
		"PLR2: 6789\n",
		5,
	},
	{
		"FREE: \n"
		"PLR1: 12345\n"
		"PLR2: 6789\n"
		"Tie Game\n",
		0,
	},
	//unfinished game
	{
		"FREE: 123456789\n"
		"PLR1: \n"
		"PLR2: \n",
		1,
	},
	{
		"FREE: 23456789\n"
		"PLR1: 1\n"
		"PLR2: \n",
		0,
	},
	{
		"FREE: 123456789\n"
		"PLR1: \n"
		"PLR2: \n",
		0,
	},
};

#define IOCTL(...) \
	do { \
		if (ioctl(fd, __VA_ARGS__) < 0) \
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

static void read_and_compare(int fd, const char *state)
{
	char read_buf[2048];
	ssize_t ret;
	READ();
	size_t length = (size_t)ret;
	if (length == sizeof read_buf)
		errx(1, "read buf too small");
	read_buf[length] = '\0';
	if (strncmp(read_buf, state, length) != 0)
		errx(1, "inccorect state. Expected:\n%sGot:\n%.*s",
			state, (int)length, read_buf);
}

int main(void)
{
	puts("testing number scrabble");
	int fd;
	fd = open(DEV, O_RDWR);
	if (fd < 0)
		err(1, "open error");

	for(size_t i = 0; i < sizeof expected / sizeof *expected; ++i) {
		read_and_compare(fd, expected[i].board);
		if(expected[i].next_move == 0)
			IOCTL(IOC_RESET);
		else
			WRITE("0123456789" + expected[i].next_move, 1);
	}
	if (write(fd, "a", 1) >= 0 || errno != EIO)
		errx(1, "write of invalid input not rejected");
	WRITE("1", 1);
	if (write(fd, "1", 1) >= 0 || errno != EIO)
		errx(1, "write of duplicate input not rejected");
	WRITE("4", 1);
	WRITE("2", 1);
	WRITE("5", 1);
	WRITE("3", 1);
	WRITE("6", 1);
	if (write(fd, "7", 1) >= 0 || errno != EIO)
		errx(1, "write after game end not rejected");

	if (close(fd) < 0)
		err(1, "close error");
	puts("testing complete");
	return 0;
}
