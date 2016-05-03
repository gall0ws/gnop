/* $Id: text.h 27 2009-08-28 21:03:48Z gallows $
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

/*
 * Object Hierarchy:
 *
 *  Object
 *   +----Layer
 *         +----Sprite
 *               +----Text (final)
 */

#ifndef TEXT_H
#define TEXT_H

#include "sprite.h"

typedef struct _Text Text;

int  ttf_init(void);
void ttf_quit(void);

Text *text_new (const char *font_path, int ptsz, u32 fg_color);

CHECK_FMT2 int text_set_text  (Text *self, const char *fmt, ...);

#endif /* !TEXT_H */
