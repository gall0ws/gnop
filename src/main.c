/* $Id: main.c 26 2009-08-24 22:38:58Z gallows $
 *
 * Copyright (c) 2008,2009 Sergio Perticone <g4ll0ws@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <getopt.h>

#include "log.h"
#include "engine.h"

#define USAGE_FMT	\
	"gnop (%s)\n\n"							\
	"Usage: %s [OPTION]...\n"					\
	"\nDisplay Options:\n"						\
	"  -c, --fg-color=COLOR\t set foreground color\n"		\
	"  -C, --bg-color=COLOR\t set background color\n"		\
	"  -f, --fullscreen\t enable fullscreen mode\n"			\
	"  --display=DISPLAY\t X display to use\n"			\
	"\nMisc Options:\n"						\
	"  -d, --datadir=DIR\t load game data from DIR\n"		\
	"                   \t (default: %s)\n"				\
	"  -m, --mute\t\t disable sounds\n"				\
	"  --help\t\t display this help and exit\n\n"

enum {
	OPT_DISPLAY,
	OPT_HELP,
};

static struct option long_options[] = {
	{ "fg-color", required_argument, NULL, 'c' },
	{ "bg-color", required_argument, NULL, 'C' },
	{ "fullscreen", no_argument, NULL, 'f' },
	{ "datadir", required_argument, NULL, 'd' },
#if HAVE_LIBSDL_MIXER
	{ "mute", no_argument, NULL, 'm' },
#endif
	{ "display", required_argument, NULL, OPT_DISPLAY },
	{ "help", no_argument, NULL, OPT_HELP },
	{ NULL },
};

int main(int ac, char *av[])
{
	int c;
	char *p, *datadir;
	u32 fg, bg;
	u8 opts;

	datadir = NULL;
	opts = 0;
	fg = ENGINE_FG_COLOR;
	bg = ENGINE_BG_COLOR;

	for (;;) {
		c = getopt_long(ac, av, "c:C:fd:"
#if HAVE_LIBSDL_MIXER
				"m"
#endif
				, long_options, NULL);
		if (c == -1)
			break;

		switch (c) {
		case OPT_HELP:
			printf(USAGE_FMT, VERSION, *av, DATADIR);
			return 0;

		case OPT_DISPLAY:
 			setenv("DISPLAY", optarg, 1);
			break;

		case 'c':
			fg = strtol(optarg, &p, 16);
			if (*p) {
				log_err("invalid fg color: %s", optarg);
				return 1;
			}
			break;

		case 'C':
			bg = strtol(optarg, &p, 16);
			if (*p) {
				log_err("invalid bg color: %s", optarg);
				return 1;
			}
			break;

		case 'f':
			opts |= ENGINE_OPTION_FS;
			break;

		case 'd':
			datadir = optarg;
			break;
#if HAVE_LIBSDL_MIXER
		case 'm':
			opts |= ENGINE_OPTION_MUTE;
			break;
#endif
		case '?':
			printf("Try `%s --help' for more information\n", 
			       av[0]);
			return 1;
		}
	}

	if (engine_init(opts, datadir, fg, bg) != 0)
		return 1;

	engine_loop();
	engine_quit();

	return 0;
}
