#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#define DEV "/dev/subst_cipher"

#define IOC_SET_KEY _IOW(0x11, 0, char[26])

static void verify_write(int fd, const char *str);
static void verify_ioctl(int fd, const char *str);
static void verify_read(int fd, const char *str);

int main(void)
{
	const char* cipher_alphabet[4] = {
		"zyxwvutsrqponmlkjihgfedcba",
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ",
		"qrstuvwxyzabcdefghijklmnop",
		"phqgiumeaylnofdxjkrcvstzwb",
	};
	const char* plaintext[4] = {
		"hello world",
		"abcdef",
		"one two three",
		"blah",
	};
	const char* ciphertext[4][4] = {
		{"svool dliow", "zyxwvu", "lmv gdl gsivv", "yozs"},
		{"HELLO WORLD", "ABCDEF", "ONE TWO THREE", "BLAH"},
		{"xubbe mehbt", "qrstuv", "edu jme jxhuu", "rbqx"},
		{"einnd tdkng", "phqgiu", "dfi ctd cekii", "hnpe"},
	};

	int sub_cipher_fd;
	puts("testing substitution cipher");
	sub_cipher_fd = open(DEV, O_RDWR);

	if(sub_cipher_fd < 0)
		err(1, "open() error");

	for (int i = 0; i < 4; ++i) {
		verify_ioctl(sub_cipher_fd, cipher_alphabet[i]);
		for(int j = 0; j < 4; ++j) {
			verify_write(sub_cipher_fd, plaintext[j]);
			verify_read(sub_cipher_fd, ciphertext[i][j]);
		}
	}

	for (int i = 0; i < 4; ++i) {
		verify_write(sub_cipher_fd, plaintext[i]);
		for(int j = 0; j < 4; ++j) {
			verify_ioctl(sub_cipher_fd, cipher_alphabet[j]);
			verify_read(sub_cipher_fd, ciphertext[j][i]);

		}
	}

	if(close(sub_cipher_fd) < 0)
		err(1, "close() error");

	puts("testing complete");
	return 0;
}

void verify_write(int fd, const char *str)
{
	size_t length = strlen(str);
	if (write(fd, str, length) != (ssize_t)length)
		err(1, "write() error");
}

void verify_ioctl(int fd, const char *str)
{
	if (ioctl(fd, IOC_SET_KEY, str) < 0)
		err(1, "ioctl() error");
}

void verify_read(int fd, const char *str)
{
	size_t length = strlen(str);
	char buff[32];
	ssize_t ret = read(fd, buff, sizeof buff);

	if (ret < 0)
		err(1, "read() error");
	if (ret != (ssize_t)length)
		errx(1, "invalid length, Expected %zu, got %zd", length, ret);
	if (memcmp(str, buff, length) != 0)
		errx(1, "invalid output. Expected %s got %.*s", str, (int)length, buff);
}

