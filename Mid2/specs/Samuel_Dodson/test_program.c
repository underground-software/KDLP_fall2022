#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <assert.h>

#include "bf.h"

#define IOC_SET_MODE _IO(0x11, 0)

static char *path;

static int check(int n)
{
  if (n < 0) {
    perror("error");
    exit(1);
  }
  return n;
}

static void check_err(int n) {
  if (n >= 0) {
    puts("expected error but didn't get one");
    exit(1);
  }
  perror("expected error");
}

void basic_run(const char *code, const char *input,
               const char *expected_output) {
  int fd = check(open(path, O_RDWR));
  check(write(fd, code, strlen(code)));
  check(ioctl(fd, IOC_SET_MODE, 1));
  check(write(fd, input, strlen(input)));
  char buf[1024] = {0};
  check(read(fd, buf, 1024));
  assert(strcmp(buf, expected_output) == 0);
  check(close(fd));
}

int main(int argc, char **argv) {
  (void)bf_run;
  if (argc != 2) {
    puts("Pass 1 file path as argument");
    exit(1);
  }
  path = argv[1];

  // Basic all-in-one tests
  basic_run(hello_world, "", "Hello, world!<end>");
  basic_run(three_inputs, "abc", "abc<end>");
  basic_run(three_inputs, "xyz", "xyz<end>");
  basic_run(encrypt, "hello", "Input five letters\nkbgqy<end>");
  basic_run(decrypt, "kbgqy", "Input ciphertext\nhello<end>");

  // Concurrent interpreters and lseek
  int fd1 = check(open(path, O_RDWR));
  int fd2 = check(open(path, O_RDWR));
  char buf[2048] = {0};
  check(write(fd1, encrypt, strlen(encrypt)));
  check(write(fd2, decrypt, strlen(decrypt)));
  check(ioctl(fd1, IOC_SET_MODE, 1));
  check(write(fd1, "linux", 5));
  check(lseek(fd1, strlen("Input five letters\n"), SEEK_SET));
  check(read(fd1, buf, 5));
  check(ioctl(fd2, IOC_SET_MODE, 1));
  check(write(fd2, buf, 5));
  check(read(fd2, buf, 5));
  assert(strcmp(buf, "linux") == 0);
  close(fd2);

  // Reusing fd1
  check(ioctl(fd1, IOC_SET_MODE, 0));
  check(write(fd1, hello_world, strlen(hello_world)));
  check(ioctl(fd1, IOC_SET_MODE, 1));
  check(read(fd1, buf, 2048));
  assert(strcmp(buf, "Hello, world!") == 0);

  // invalid program, and writing multiple times
  check(ioctl(fd1, IOC_SET_MODE, 0));
  check(write(fd1, "+++++", 5));
  check(write(fd1, "[", 1));
  check(ioctl(fd1, IOC_SET_MODE, 1));
  check(read(fd1, buf, 2048));
  assert(strcmp(buf, "<invalid>") == 0);

  // Expected errors
  // lseek past bounds
  check_err(lseek(fd1, 10000, SEEK_SET));

  // write user input past bounds
  check_err(write(fd1, buf, 2048));

  // read while in input mode
  check(ioctl(fd1, IOC_SET_MODE, 0));
  check_err(read(fd1, buf, 1024));

  // write program past bounds
  check_err(write(fd1, buf, 2048));

  // weird ioctl
  check_err(ioctl(fd1, 1, 0));
  check_err(ioctl(fd1, IOC_SET_MODE, 2));

  check(close(fd1));
  return 0;
}
