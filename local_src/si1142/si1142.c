//------------------------------------------------------------------------------
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
//
// Copyright (c) 2019 WAGO Kontakttechnik GmbH & Co. KG
//------------------------------------------------------------------------------
///------------------------------------------------------------------------------
/// \file    si1142.c
///
/// \brief   Monitor the 1142 motion sensor and provide the data for intrested parties
///
/// \author  Christian Hohnstädt - Wago GmbH
///------------------------------------------------------------------------------

#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define DEBUG_MSG
#ifdef DEBUG_MSG
  #define debug printf
#else
  #define debug(...) do {} while(0)
#endif

enum probe_index {
	intensity,
	intensity_ir,
	proximity_left,
	proximity_right,
	no_of_probes
};

#define MTRIGGER       "proximation"
#define MOTION_TRIGGER "/sys/kernel/config/iio/triggers/hrtimer/" MTRIGGER
#define MOTION_PATH    "/sys/bus/iio/devices/iio:device0/"
#define MOTION_SCAN_ELEMENTS MOTION_PATH "scan_elements/"
#define MOTION_BUFFER        MOTION_PATH "buffer/"
#define MOTION_DEV     "/dev/iio:device0"
#define TRIGGER_DIR    "/sys/bus/iio/devices/trigger%d/"

#define PROXIMATION    "/var/run/proximation"
#define INTENSITY      "/var/run/intensity"
#define SOCK_NAME      "/var/run/si1142"

#define PROXIMITY_THRESHOLD 600

static const char *const probes[no_of_probes] = {
	"intensity", "intensity_ir", "proximity0", "proximity1"
};

/* Tell GCC how to check the printf() parameters */
static char *read_file_buf(char *buf, size_t len, const char *fmt, ...)
				__attribute__((format(printf, 3, 4)));
static int read_file_num(const char *fmt, ...)
				__attribute__((format(printf, 1, 2)));
static int write_file(const char *val, const char *fmt, ...)
				__attribute__((format(printf, 2, 3)));


/* Remove leading and trailing whitespace */
static char *trim(char *buf)
{
	ssize_t len = strlen(buf);
	while (len > 0 && isspace(*buf)) {
		buf++;
		len--;
	}
	while (len > 0 && isspace(buf[len -1]))
		buf[--len] = 0;
	return buf;
}

/* Read into buffer from file. Filename by printf() format */
static int _read_file(char *buf, size_t len, const char *fmt, va_list ap)
{
	char fname[512];
	ssize_t ret;
	int fd;

	vsnprintf(fname, sizeof fname, fmt, ap);

	fd = open(fname, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "ERROR Opening file %s: %s\n", fname,
				strerror(errno));
		return -1;
	}
	ret = read(fd, buf, len -1);
	if (ret < 0)
		fprintf(stderr, "ERROR reading file %s: %s\n", fname,
				strerror(errno));
	close(fd);
	buf[ret] = 0;
	debug("Read file '%s',FD:%d = '%s'\n", fname, fd, trim(buf));
	return ret;
}

/* Read into buffer from file and remove trailing newline \n */
static char *read_file_buf(char *buf, size_t len, const char *fmt, ...)
{
	int ret;
	va_list ap;

	va_start(ap, fmt);
	ret = _read_file(buf, len, fmt, ap);
	va_end(ap);
	return ret == -1 ? NULL : trim(buf);
}

/* Read and return positive number from file */
static int read_file_num(const char *fmt, ...)
{
	int ret;
	char buf[64];
	va_list ap;

	va_start(ap, fmt);
	ret = _read_file(buf, sizeof buf, fmt, ap);
	va_end(ap);
	return ret == -1 ? -1 : atoi(trim(buf));
}

/* Write "val" to a file with filename in printf format */
static int write_file(const char *val, const char *fmt, ...)
{
	char fname[512];
	va_list ap;
	ssize_t ret = 0;
	int fd;

	va_start(ap, fmt);
	vsnprintf(fname, sizeof fname, fmt, ap);
	va_end(ap);

	fd = open(fname, O_WRONLY | O_CREAT, 0644);
	if (fd < 0) {
		fprintf(stderr, "ERROR opening file(%s): %s\n", fname,
				strerror(errno));
		return -1;
	}
	if (val) {
		ret = ftruncate(fd, 0);
		if (ret < 0)
			fprintf(stderr, "ERROR truncating file %s: %s\n",
					fname, strerror(errno));
		ret = write(fd, val, strlen(val));
		if (ret < 0)
			fprintf(stderr, "ERROR writing file %s: %s\n",
					fname, strerror(errno));
	}
	close(fd);
	debug("Writing '%s' to file: %s\n", val, fname);
	return ret < 0 ? ret : 0;
}


/* Find trigger: /sys/bus/iio/devices/trigger./name == MTRIGGER */
static int find_proximation_trigger()
{
	int i;
	static int index = -1;

	if (index > -1)
		return index;

	for (i=0; i<8; i++) {
		char name[128];
		if (!read_file_buf(name, sizeof name, TRIGGER_DIR "name", i))
			break;
		if (!strcmp(name, MTRIGGER)) {
			index = i;
			break;
		}
	}
	debug("Detected trigger: %d\n", index);
	return index;
}

/* Iterate over the probes and turn them on or off.
 * When turning them on, stop in case of an error
 * When turning them off, continue even in case of a failure
 *
 * The "in_<PROBE>_index" file content must be strictly monotonously rising.
 * Our order in the "enum probe_index" depends on it!
 */
static int switch_scan_probes(int on)
{
	int i, r, curr_index = -1;

	for (i = 0, r = 0; (r == 0 || !on) && i < no_of_probes; i++) {
		r = write_file(on ? "1":"0", MOTION_SCAN_ELEMENTS "in_%s_en",
					     probes[i]);
		if (!on)
			continue;

		if (r >= 0) {
			int idx = read_file_num(MOTION_SCAN_ELEMENTS "in_%s_index",
						probes[i]);
			if (idx <= curr_index) {
				fprintf(stderr, "INDEX ORDER of %s changed\n",
					probes[i]);
				exit(1);
			}
			curr_index = idx;
		}
	}
	return r;
}

static int start_motion_sampling()
{
	int ret;

	/* Create new "hrtimer" in configfs with mkdir() */
	ret = mkdir(MOTION_TRIGGER, 0755);

	/* Enable the content to be scanned */
	if (ret == 0)
		ret = switch_scan_probes(1);
	/* Use hrtimer as trigger */
	if (ret == 0)
		ret = write_file(MTRIGGER,
				 MOTION_PATH "trigger/current_trigger");
	/* Set a reasonable initial sampling frequency: 10Hz */
	if (ret == 0)
		ret = write_file("10", TRIGGER_DIR "sampling_frequency",
					find_proximation_trigger());
	/* Turn on the data buffer to receive data every 100 msecs */
	if (ret == 0)
		ret = write_file("128", MOTION_BUFFER "length");
	if (ret == 0)
		ret = write_file("1", MOTION_BUFFER "enable");

	/* Open the data channel to poll */
	if (ret == 0)
		ret = open(MOTION_DEV, O_RDONLY | O_NONBLOCK);

	return ret;
}

static void stop_motion_sampling(int fd)
{
	/* Try to do as much as possible -> No bail-out on error */
	if (fd != -1)
		close(fd);
	write_file("0", MOTION_BUFFER "enable");
	switch_scan_probes(0);
	rmdir(MOTION_TRIGGER);
}

/* Read data of all enabled probes. */
static int handle_motion_sampling(int fd, uint16_t *data)
{
	static const char *const proximation_strings[] = {
		"none", "left", "right", "front"
	};
	static uint16_t old_sane[no_of_probes];
	static int old_prox = -1;
	uint16_t sane[no_of_probes];
	ssize_t read_size;
	int i, proximation, delta;

	/* sizeof data is sizeof (void*), but "sane" is exactly
	 *  as long as the data "data" array
	 */
	read_size = read(fd, data, sizeof sane);
	if (read_size != sizeof sane) {
		if (read_size < 0)
			perror(MOTION_DEV);
		return -1;
	}
	/* Sanitize incoming data */
	for (i = 0; i < no_of_probes; i++) {
		switch (i) {
		case proximity_left:
		case proximity_right:
			sane[i] = data[i] > PROXIMITY_THRESHOLD ? 1 : 0;
			break;
		case intensity:
		case intensity_ir:
			/* Only change if |delta| > 100 */
			delta = old_sane[i] - data[i];
			sane[i] = (delta < -100 || delta > 100) ?
					data[i] : old_sane[i];
			break;
		default:
			sane[i] = data[i];
			break;
		}
	}
	/* Calculate current and previous position:
	 *  left, right, front or none */
	proximation = sane[proximity_left] + 2 * sane[proximity_right];

	if (old_prox != proximation)
		write_file(proximation_strings[proximation], PROXIMATION);

	if (old_sane[intensity] != sane[intensity] ||
	    old_sane[intensity_ir] != sane[intensity_ir])
	{
		char buf[256];
		sprintf(buf, "%d\n%d\n", sane[intensity], sane[intensity_ir]);
		write_file(buf, INTENSITY);
	}

	debug("VI: % 5d IR: % 5d - Old:%5s -> %5s (L:% 5d R:% 5d)\n",
		sane[intensity], sane[intensity_ir], proximation_strings[old_prox],
		proximation_strings[proximation],
		data[proximity_left], data[proximity_right]);
	memcpy(old_sane, sane, sizeof sane);
	old_prox = old_sane[proximity_left] + 2 * old_sane[proximity_right];
	return 1;
}

/* Poll() for incoming signals()
 * returns a file descriptor or -1 on error */
static int open_signalfd()
{
	sigset_t mask;
	int ret;

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGQUIT);
	sigaddset(&mask, SIGTERM);
	sigaddset(&mask, SIGHUP);
	sigaddset(&mask, SIGPIPE);
	sigaddset(&mask, SIGCHLD);

	ret = sigprocmask(SIG_BLOCK, &mask, NULL);
	if (ret == 0)
		ret = signalfd(-1, &mask, 0);
	if (ret == -1)
		perror("sigprocmask() / signalfd()");
	return ret;
}

static int handle_signalfd(int fd, uint16_t *data)
{
	int ret = -1;
	struct signalfd_siginfo fdsi;
	(void)data;

	if (read(fd, &fdsi, sizeof fdsi) != sizeof fdsi) {
		perror("read(signalfd");
		return -1;
	}
	switch (fdsi.ssi_signo) {
	case SIGPIPE:
		printf("Ignore SIGPIPE\n");
		return 1;
	case SIGQUIT:
	case SIGTERM:
		ret = 0;
		/* fallthrough */
	default:
		printf("Terminating on signal %d: %s\n", fdsi.ssi_signo,
			strsignal(fdsi.ssi_signo));
	}
	return ret;
}

/* Opens the "raw data" socket.
 * Returns a listening unix file descriptor or -1 on error */
static int open_socket()
{
	int new = socket(AF_UNIX, SOCK_STREAM, 0);
	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
		.sun_path = SOCK_NAME
	};

	if (new == -1) {
		perror("socket(AF_UNIX, SOCK_STREAM, 0)");
		return -1;
	}
	unlink(SOCK_NAME);
	if (bind(new, (void*)&addr, sizeof addr) == -1) {
		perror("bind("SOCK_NAME")");
		close(new);
		return -1;
	}
	if (listen(new, 10) == -1) {
		perror("listen("SOCK_NAME")");
		close(new);
		return -1;
	}
	return new;
}

/* Handle any connect()s "kurz und schmerzlos" (aka fire & forget):
 * accept() the new connection, write the data, close connection.
 * Returns -1 on error and 1 on success (ignoring a possible write() error).
 */
static int handle_socket(int sock, uint16_t *data)
{
	int i, fd;
	char b[512], *p = b;

	for (i=0; i<no_of_probes && p < b + sizeof b; i++) {
		p += snprintf(p, sizeof b - (b-p),
				"%s=%d\n", probes[i], data[i]);
	}
	fd = accept(sock, NULL, NULL);
	if (fd == -1) {
		perror("accept("SOCK_NAME")");
		return -1;
	}
	if (write(fd, b, p -b) != p -b)
		perror("write("SOCK_NAME")");
	close(fd);
	debug("Socket send: '%s'\n", b);
	return 1;
}

static int usage(const char *me)
{
	printf(
"Usage:\n"
"  %s\n"
"for testing or for productive use:\n"
"  daemonize %s\n\n"
"This application manages the si1142 motion sensor and is\n"
"intended to run in the background and serve others with\n"
"data. See the 'si_client' application.\n\n"
"It does not take any arguments.\n\n",
	me, realpath(me, NULL));
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	int motfd, sigfd, ret = -1, sockfd;
	uint16_t data[no_of_probes];

	if (argc > 1)
		return usage(argv[0]);

	motfd = start_motion_sampling();
	sigfd = open_signalfd();
	sockfd = open_socket();

	memset(data, 0, sizeof data);

	while (motfd >= 0 && sigfd >= 0 && sockfd >= 0) {
		struct pollfd pollfds[] = {
			{ motfd, POLLIN, 0 },
			{ sigfd, POLLIN, 0 },
			{ sockfd, POLLIN, 0 }
		};
		/* A list of event handlers for each pollfds[] entry
		 * handle_*() return:
		 *   -1 for EXIT_FAILURE
		 *    0 for EXIT_SUCCESS
		 *   >0 for continue;
		 */
		int (*handler[]) (int, uint16_t*) = {
			handle_motion_sampling,
			handle_signalfd,
			handle_socket,
		};
		size_t i, len = sizeof pollfds / sizeof pollfds[0];

		/* After 3 seconds we expect the motion sensor to respond */
		ret = poll(pollfds, len, 3000);

		if (ret == -1) {
			perror("poll()");
			break;
		}
		if (ret == 0) {
			fprintf(stderr, "Timeout waiting for "MOTION_DEV"\n");
			ret = -1;
			break;
		}
		for (i=0; i<len; i++) {
			/* Check all file descriptors and call the handler */
			if (pollfds[i].revents & pollfds[i].events) {
				ret = handler[i](pollfds[i].fd, data);
				if (ret <= 0)
					break;
			}
		}
		if (ret <= 0)
			break;
	}
	stop_motion_sampling(motfd);
	return ret < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
