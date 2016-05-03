/* $Id: sprite.h 27 2009-08-28 21:03:48Z gallows $
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
 */

#ifndef SPRITE_H
#define SPRITE_H

#include <SDL_video.h>

#include "layer.h"

#define SPRITE(OBJ)	((Sprite *)(OBJ))

#define SPRITE_COLORKEY 0xff00ff

typedef struct _Sprite Sprite;

Sprite *sprite_new           (u16 width, u16 height);
Sprite *sprite_new_from_file (const char *bmp_path);
Sprite *sprite_new_from_sdl  (SDL_Surface *surface);

int  sprite_fill        (Sprite *self, u32 color);
int  sprite_fill_region (Sprite *self, s16 x, s16 y, u16 w, u16 h, u32 color);
int  sprite_fill_pixel  (Sprite *self, s16 x, s16 y, u32 color);

int  sprite_set_alpha	 (Sprite *self, u8 alpha);
int  sprite_set_colorkey (Sprite *self, u32 color);
int  sprite_set_accel    (Sprite *self, u32 color);	

#define sprite_enable_colorkey(SELF) sprite_set_colorkey(SELF, SPRITE_COLORKEY)

#endif /* !SPRITE_H */
