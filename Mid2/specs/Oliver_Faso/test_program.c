
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define DEV "/dev/menu"

#define IOC_RESTAURANTS _IO(0x11, 0)
#define IOC_ORDER(ITEM) _IOWR(0x11, (ITEM), int)

#define MIN_MENUS 2
#define MIN_ITEMS 3
#define MAX_ITEMS 20

struct menu_item {
	char *name;
	int price_in_cents;
};

struct menu {
	size_t num_items;
	struct menu_item items[MAX_ITEMS];
};

static struct menu menus[MIN_MENUS];

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

static void read_menu(int fd, struct menu *menu)
{
	char read_buf[2048];
	ssize_t ret;
	READ();
	size_t length = (size_t)ret;
	if (length == sizeof read_buf)
		errx(1, "read buf too small");
	read_buf[length] = '\0';

	char *end_of_header = strchr(read_buf, '\n');
	if (!end_of_header)
		errx(1, "menu missing header");
	struct menu_item *ptr = menu->items;
	for(size_t num_items = 0, offset = (end_of_header - read_buf)+1;
		num_items < MAX_ITEMS && offset < length; ++num_items) {
		int dollars, cents;
		char *name;
		ptrdiff_t count;
		//__extension__ is a slightly cludgy way to allow m modifier for allocating memory for a string
		//read with scanf without disabling -Wpedantic or the entire -Wformat option
		if (__extension__ sscanf(read_buf + offset, "(%*d) \"%m[^\"]\" %d.%d %tn", &name, &dollars,
			&cents, &count) != 3 || dollars < 0 || cents < 0 || cents > 99) {
			errx(1, "invalid menu item");
		}
		*ptr++ = (struct menu_item) {
			.name = name,
			.price_in_cents = 100*dollars + cents
		};
		offset += count;
	}
	menu->num_items = ptr - menu->items;
	if (menu->num_items < MIN_ITEMS)
		errx(1, "not enough items on the menu");
}

int main(void)
{
	puts("testing menu");
	char read_buf[2048];
	ssize_t ret;
	int fd;
	fd = open(DEV, O_RDWR);
	if (fd < 0)
		err(1, "open error");
	char *names[MIN_MENUS];
	READ();
	size_t length = (size_t)ret;
	if (length == sizeof read_buf)
		errx(1, "read buf too small");
	read_buf[length] = '\0';
	for(size_t i = 0, offset = 0; i < MIN_MENUS; ++i) {
		if(offset >= length)
			errx(1, "not enough menu options");
		ptrdiff_t count;
		//__extension__ is a slightly cludgy way to allow m modifier for allocating memory for a string
		//read with scanf without disabling -Wpedantic or the entire -Wformat option
		if (__extension__ sscanf(read_buf + offset, "%m[^\n] %tn", &names[i], &count) != 1)
			errx(1, "invalid restaurant name");
		offset += count;
	}
	for(size_t i = 0; i < MIN_MENUS; ++i) {
		WRITE(names[i], strlen(names[i]));
		read_menu(fd, &menus[i]);
	}
	for(size_t i = 0; i < MIN_MENUS; ++i) {
		WRITE(names[i], strlen(names[i]));
		int money = 0;
		for(size_t j = 0; j < menus[i].num_items; ++j) {
			money += menus[i].items[j].price_in_cents;
		}
		for(size_t j = 0; j < menus[i].num_items; ++j) {
			int money_before = money;
			IOCTL(IOC_ORDER(j + 1), &money);
			if (money + menus[i].items[j].price_in_cents != money_before)
				errx(1, "incorrect charge. Expected %d, got %d",
					menus[i].items[j].price_in_cents, money_before - money);

		}
		//ordering the first item again if I have no money shold fail.
		if (ioctl(fd, IOC_ORDER(1), &money) >= 0 || errno != EINVAL)
			errx(1, "shouldn't be able to order with zero dollars");

	}
	IOCTL(IOC_RESTAURANTS);
	char new_buf[2048];
	ssize_t new_ret;
	if ((new_ret = read(fd, new_buf, sizeof new_buf)) < 0)
		err(1, "read error");
	if (new_ret != ret || memcmp(read_buf, new_buf, length) != 0)
		errx(1, "ioctl 0 did not switch back to restaurant list");

	if (close(fd) < 0)
		err(1, "close error");
	puts("testing complete");
	for(size_t i = 0; i < MIN_MENUS; ++i) {
		free(names[i]);
		for(size_t j = 0; j < menus[i].num_items; ++j)
			free(menus[i].items[j].name);
	}
	return 0;
}
