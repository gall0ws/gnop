/* $Id: ball.h 27 2009-08-28 21:03:48Z gallows $ 
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

/*
 * Object Hierarchy:
 *
 *  Object
 *   +----Layer
 *         +----Sprite
 *               +----Ball (final)
 */

#ifndef BALL_H
#define BALL_H

#include "sprite.h"

#define BALL_SPEED	13
#define BALL_WIDTH	14
#define BALL_HEIGHT	BALL_WIDTH

typedef enum {
	BALL_BOUNCE_H,
	BALL_BOUNCE_V
} BallBounce;

typedef enum {
	BALL_PUSH_NONE =  0,
	BALL_PUSH_DOWN = -1,
	BALL_PUSH_UP   =  1,
} BallPush;

typedef struct _Ball Ball;

Ball  *ball_new     (u32 color);
void   ball_start   (Ball *self);
void   ball_bounce  (Ball *self, BallBounce bounce, BallPush push);

void   ball_get_vectors (const Ball *self, s8 *x, s8 *y);

#endif /* !BALL_H */
