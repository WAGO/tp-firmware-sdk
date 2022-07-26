//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019-2022 WAGO GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
/// \file    si1142_client.c
///
/// \brief   client si1142
///
/// \author  Ralf Gliese, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/sendfile.h>
#include <sys/inotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/socket.h>
#include <sys/un.h>

int raw_data()
{
	char b[1024];
	ssize_t s;
	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
		.sun_path = "/var/run/si1142"
	};
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (fd == -1) {
		perror("socket(AF_UNIX, SOCK_STREAM, 0)");
		return EXIT_FAILURE;
	}
	if (connect(fd, (void*)&addr, sizeof addr) == -1) {
		perror("connect()");
		close(fd);
		return EXIT_FAILURE;
	}
	s = read(fd, b, sizeof b);
	if (s == -1) {
		perror("read(socket)");
		close(fd);
		return EXIT_FAILURE;
	}
	write(1, b, s);
	close(fd);
	return EXIT_SUCCESS;
}

void cat(const char* filename)
{
	int ret, fd, i;
	char buf[4096];

	printf("%s: '", filename);

	ret = fd = open(filename, O_RDONLY);
	if (fd != -1)
		ret = read(fd, buf, sizeof buf -1);

	for (i=0; i<ret; i++) {
		if (buf[i] == '\n')
			fputs("\\n", stdout);
		else
			putchar(buf[i]);
	}
	fputs("'\n", stdout);
	if (ret == -1)
		perror(filename);
	if (fd != -1)
		close(fd);
}

struct {
	const char *fname;
	int wd;
} watches[] = {
	{ "/var/run/proximation", 0 },
	{ "/var/run/intensity", 0 }
};
#define NO_OF_WATCHES ((int)(sizeof watches / sizeof watches[0]))

int watch_proximity()
{
	int fd, i;
	struct inotify_event ev;

	fd = inotify_init();
	for (i = 0; i < NO_OF_WATCHES; i++) {
		cat(watches[i].fname);
		watches[i].wd = inotify_add_watch(fd,
					watches[i].fname, IN_CLOSE_WRITE);
	}
	while (read(fd, &ev, sizeof ev) > 0) {
		for (i = 0; i < NO_OF_WATCHES; i++) {
			if (ev.wd == watches[i].wd) {
				raw_data();
				cat(watches[i].fname);
			}
		}
	}
	return 0;
}

int usage(const char *me)
{
	int i;

	printf(
"Usage: %s [-f]\n\n"

"  Print current sampling data of the SI1142 sensor of the touchpanel.\n\n"

"Options:\n"
"  -f:  Monitor changes of the files:", me);

	for (i = 0; i < NO_OF_WATCHES; i++)
		printf(" %s", watches[i].fname);

	puts("");
	return 0;
}

int main(int argc, char *argv[])
{
	int follow = 0, i;

	if (argc > 1) {
		if (!strcmp(argv[1], "-f"))
			follow = 1;
		else
			return usage(argv[0]);
	}
	raw_data();
	if (follow)
		return watch_proximity();

	for (i = 0; i < NO_OF_WATCHES; i++)
		cat(watches[i].fname);

	return 0;
}
