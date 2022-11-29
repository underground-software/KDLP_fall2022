#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <err.h>

struct vigenere_key {
	const char *data;
	size_t length;
};

#define IOC_SET_KEY _IOW(0x11, 0, struct vigenere_key)
#define DEV_FILE "/dev/vigenere-cipher"

static void read_test(int fd, char *target, ssize_t target_len, int test_num)
{
	char buf[256];
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
		errx(1, "Test %d: Bad Cipher. Expected %s, got %.*s",
			test_num, target, (int)target_len, buf);
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
	char hello[] = "hello world";
	char fox[] = "the quick brown fox jumped over the lazy dog";
	char hello_default[] = "cmrpb afvgl";
	char fox_default[] = "opk uhmto wzuaa jfb ecstrh fzzz zlr prdt luk";
	char hello_key[] = "rijvs uyvjn";
	char fox_key[] = "dlc aygmo zbsux jmh nswtcn stov rri jkdw nse";

	int test = 1;
	int fd;
	puts("testing vigenere cipher");
	fd = open(DEV_FILE, O_RDWR);

	if (fd < 0)
		err(1, "open error");
	TEST(hello, default);
	TEST(fox, default);
	if(ioctl(fd, IOC_SET_KEY, &(struct vigenere_key){.data = "key", .length = 3}) < 0)
		err(1, "ioctl error");
	TEST(hello, key);
	TEST(fox, key);
	puts("testing complete");
	return 0;
}
