/* $Id: ball.c 27 2009-08-28 21:03:48Z gallows $ 
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

#include <math.h>	/* sqrt() */

#include "video.h"

#include "sprite_impl.h"
#include "ball.h"

#define GEN_VECTOR(s8_VECT) sqrt(BALL_SPEED * BALL_SPEED - s8_VECT * s8_VECT)

struct _Ball {
	Sprite parent;

	s16 x, y;
	s8 vector_x, vector_y;
	u16 video_w, video_h;
};

INLINE static int ball_blit(Ball *self)
{
	int retv = sprite_blit(SPRITE(self));

	if (retv == 0) {
		self->x += self->vector_x;
		self->y += self->vector_y;

		sprite_set_xy(SPRITE(self), self->x, self->y);
	}

	return retv;
}

INLINE_METHOD static void ball_set_x(Ball *self, s16 x)
{
	self->x = x;
}

INLINE_METHOD static void ball_set_y(Ball *self, s16 y)
{
	self->y = y;
}

INLINE_METHOD static void ball_set_xy(Ball *self, s16 x, s16 y)
{
	self->x = x;
	self->y = y;
}

INLINE_METHOD static s16 ball_get_x(const Ball *self)
{
	return self->x;
}

INLINE_METHOD static s16 ball_get_y(const Ball *self)
{
	return self->y;
}

Ball *ball_new(u32 color)
{
	Sprite *parent;
	Ball *self;

	parent = sprite_new(BALL_WIDTH, BALL_HEIGHT);
	sprite_fill(parent, color);
	sprite_set_accel(parent, color);

	self = realloc(parent, sizeof(Ball));
	
	self->x = self->y = 0;
	self->video_w = video_get_width();
	self->video_h = video_get_height();
	self->vector_x = self->vector_y = 0;

	OBJECT(self)->vtable.blit = (pfBlit)ball_blit;
	LAYER(self)->vtable.get_x = (pfGet)ball_get_x;
	LAYER(self)->vtable.get_y = (pfGet)ball_get_y;
	LAYER(self)->vtable.set_x = (pfSet)ball_set_x;
	LAYER(self)->vtable.set_y = (pfSet)ball_set_y;
	LAYER(self)->vtable.set_xy = (pfSet2)ball_set_xy;
	
	return self;
}

void ball_start(Ball *self)
{
	self->x = self->video_w / 2 - BALL_WIDTH / 2;
	self->y = self->video_h / 2 - BALL_HEIGHT / 2;

	sprite_set_xy(SPRITE(self), self->x, self->y);

    	self->vector_y = rand() % 3;
	self->vector_x = GEN_VECTOR(self->vector_y);

 	if ((rand() % 2) == 0) 
 		self->vector_y *= -1;

 	if ((rand() % 2) == 0)
		self->vector_x *= -1;
}

void ball_bounce(Ball *self, BallBounce bounce, BallPush p)
{
	int i, tmp;

	if (bounce == BALL_BOUNCE_V)
		self->vector_y *= -1;
	else
		self->vector_x *= -1;

	tmp = self->vector_x;

	for (i=0; i<2; ++i)
		if (p == BALL_PUSH_DOWN && self->vector_y - 1 > -BALL_SPEED)
			self->vector_y -= 1;
		else if (p == BALL_PUSH_UP && self->vector_y + 1 < BALL_SPEED)
			self->vector_y += 1;
		else
			return;

	self->vector_x = GEN_VECTOR(self->vector_y);

	if (tmp < 0)
		self->vector_x *= -1;
}

INLINE_METHOD void ball_get_vectors(const Ball *self, s8 *x, s8 *y)
{
	if (x) *x = self->vector_x;
	if (y) *y = self->vector_y;
}
