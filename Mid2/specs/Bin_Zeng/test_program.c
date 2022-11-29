#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <err.h>

#define IOC_SET_KEY _IO(0x11, 0)
#define DEV_FILE "/dev/xor-cipher"

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
	if (memcmp(target, buf, target_len)) {
		fputs("Expected:\t", stderr);
		for(ssize_t i = 0; i < target_len; ++i)
			fprintf(stderr, "\\x%02"SCNx8, target[i]);
		fputs("\nGot:\t\t", stderr);
		for(ssize_t i = 0; i < ret_val; ++i)
			fprintf(stderr, "\\x%02"SCNx8, buf[i]);
		fputc('\n', stderr);
		errx(1, "Test %d: Bad Cipher.", test_num);
	}
}

#define TEST(LABEL, SUFFIX) \
	do { \
		if (sizeof(LABEL)-1 != write(fd, LABEL, sizeof(LABEL)-1)) \
			err(1, "error writing to file"); \
		read_test(fd, LABEL##_##SUFFIX, sizeof(LABEL##_##SUFFIX)-1, test++); \
	} while(0)


int main(void)
{
	char fox[] = "quick brown fox blah blah blah";
	char hello[] = "Hello W0rld";
	char cipher[] = "\xe2\xcf\xc6\xc6\xc5\x8a\xfd\xc5\xd8\xc6\xce\x9b\x98\x99";
	char numbers[] = "867-5309";
	char symbols[] = "!@#$!#@()%?><:right?";

	char fox_0xAA[] = "\xdb\xdf\xc3\xc9\xc1\x8a\xc8\xd8\xc5\xdd\xc4\x8a\xcc\xc5\xd2\x8a\xc8\xc6\xcb\xc2\x8a\xc8\xc6\xcb\xc2\x8a\xc8\xc6\xcb\xc2";
	char hello_0xAA[] = "\xe2\xcf\xc6\xc6\xc5\x8a\xfd\x9a\xd8\xc6\xce";
	char cipher_0xAA[] = "Hello World123";
	char numbers_0xAA[] = "\x92\x9c\x9d\x87\x9f\x99\x9a\x93";
	char symbols_0xAA[] = "\x8b\xea\x89\x8e\x8b\x89\xea\x82\x83\x8f\x95\x94\x96\x90\xd8\xc3\xcd\xc2\xde\x95";

	char fox_0x11[] = "\x60\x64\x78\x72\x7a\x31\x73\x63\x7e\x66\x7f\x31\x77\x7e\x69\x31\x73\x7d\x70\x79\x31\x73\x7d\x70\x79\x31\x73\x7d\x70\x79";
	char hello_0x11[] = "\x59\x74\x7d\x7d\x7e\x31\x46\x21\x63\x7d\x75";
	char cipher_0x11[] = "\xf3\xde\xd7\xd7\xd4\x9b\xec\xd4\xc9\xd7\xdf\x8a\x89\x88";
	char numbers_0x11[] = "\x29\x27\x26\x3c\x24\x22\x21\x28";
	char symbols_0x11[] = "\x30\x51\x32\x35\x30\x32\x51\x39\x38\x34\x2e\x2f\x2d\x2b\x63\x78\x76\x79\x65\x2e";

	int test = 1;
	int fd;
	puts("testing xor cipher");
	fd = open(DEV_FILE, O_RDWR);

	if (fd < 0)
		err(1, "open error");
	TEST(fox, 0xAA);
	TEST(hello, 0xAA);
	TEST(cipher, 0xAA);
	TEST(numbers, 0xAA);
	TEST(symbols, 0xAA);
	if(ioctl(fd, IOC_SET_KEY, 0x11) < 0)
		err(1, "ioctl error");
	TEST(fox, 0x11);
	TEST(hello, 0x11);
	TEST(cipher, 0x11);
	TEST(numbers, 0x11);
	TEST(symbols, 0x11);
	puts("testing complete");
	return 0;
}
