/* $Id: paddle.c 27 2009-08-28 21:03:48Z gallows $
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

#define _SPRITE_CHILD

#include "video.h"

#include "sprite_impl.h"
#include "paddle.h"

struct _Paddle {
	Sprite parent;

	s16 pos, lim;
	s8  move, last_move;
};

static int paddle_blit(Paddle *self)
{
	if (self->move) {
		self->pos += self->move;
		self->last_move = self->move > 0 ? 1 : -1;
		self->move = 0;
		sprite_set_y(SPRITE(self),
			    self->pos + video_get_height()/2 - PADDLE_HEIGHT/2);
	}
	else {
		self->last_move = 0;
	}

	return sprite_blit(SPRITE(self));
}

Paddle *paddle_new(u32 color, PaddleType type)
{
	Sprite *parent;
	Paddle *self;
	u16 w, h;

	parent = sprite_new(PADDLE_WIDTH, PADDLE_HEIGHT);
	sprite_fill(parent, color);
	sprite_set_accel(parent, color);

	self = realloc(parent, sizeof(Paddle));

	self->pos = 0;
	self->move = 0;
	self->last_move = 0;

	w = video_get_width();
	h = video_get_height();

	if (type == PADDLE_POS_LEFT)
		sprite_set_x(SPRITE(self), PADDLE_DISTANCE);
	else
		sprite_set_x(SPRITE(self), w - PADDLE_DISTANCE - PADDLE_WIDTH);

	sprite_set_y(SPRITE(self), h / 2 - PADDLE_HEIGHT / 2);

	self->lim = h / 2 - PADDLE_HEIGHT / 2;

	OBJECT(self)->vtable.blit = (pfBlit)paddle_blit;

	return self;
}

INLINE_METHOD bool paddle_move(Paddle *self, PaddleMove way)
{
	if (abs(self->pos + way) < self->lim) {
		self->move = way;
		return 1;
	}

	return 0;
}

INLINE_METHOD void paddle_reset_pos(Paddle *self)
{
	sprite_set_y(SPRITE(self), video_get_height()/2 - PADDLE_HEIGHT/2);
	self->pos = 0;
}

INLINE_METHOD s8 paddle_last_move(const Paddle *self)
{
	return self->last_move;
}
