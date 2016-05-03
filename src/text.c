/* $Id: text.c 27 2009-08-28 21:03:48Z gallows $
 *
 * Copyright (c) 2009 Sergio Perticone <g4ll0ws@gmail.com>
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

#define _SPRITE_CHILD

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>
#include <stdarg.h>

#if HAVE_ALLOCA_H
# include <alloca.h>
#endif

#include <SDL_ttf.h>

#include "sprite_impl.h"
#include "text.h"

struct _Text {
	Sprite         parent;
	TTF_Font      *font;
	u32            color;
};

INLINE_METHOD static void text_free(Text *self)
{
	TTF_CloseFont(self->font);
	sprite_free(SPRITE(self));
}

int ttf_init(void)
{
	if (!TTF_WasInit()) {
		if (TTF_Init() != 0) {
			log_err("could not inizialize TTF subsystem: %s",
				 TTF_GetError());
			return -1;
		}
	}
			
	return 0;
}

INLINE void ttf_quit(void)
{
	TTF_Quit();
}

Text *text_new(const char *fnt, int ptsz, u32 color)
{
	Sprite *parent;
	Text *self;
	TTF_Font *font;

	font = TTF_OpenFont(fnt, ptsz);
	if (!font) {
		log_err("could not load given font: %s", SDL_GetError());
		return NULL;
	}

	parent = sprite_new(0, 0);
	self = realloc(parent, sizeof(Text));

	OBJECT(self)->vtable.dtor = (pfDtor)text_free;

	self->color = color;
	self->font = font;

	return self;
}

int text_set_text(Text *self, const char *fmt, ...)
{
	va_list ap;
	SDL_Color color;
	SDL_Surface *tmp, *surface;
	char *buf;
	int sz;

	va_start(ap, fmt);

	sz = vsnprintf(NULL, 0, fmt, ap);

#if HAVE_ALLOCA
	buf = alloca(sz+1);
#else
	buf = malloc(sz+1);
#endif

	vsprintf(buf, fmt, ap);
	va_end(ap);

	color.r = self->color >> 16;
	color.g = (self->color >> 8) & 0xff;
	color.b = self->color & 0xff;

	tmp = TTF_RenderText_Blended(self->font, buf, color);

#if !HAVE_ALLOCA
	free(buf);
#endif

	surface = SDL_DisplayFormatAlpha(tmp);
	SDL_FreeSurface(tmp);

	if (!surface) {
		log_err("could not create surface: %s", SDL_GetError());
		return errno ? -errno : -1;
	}

	if (SPRITE(self)->surface)
		SDL_FreeSurface(SPRITE(self)->surface);

	SPRITE(self)->surface = surface;

	layer_set_width(self, surface->w);
	layer_set_height(self, surface->h);

	return 0;
}
