/* $Id: video.c 22 2009-08-22 22:11:34Z gallows $
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

#include <errno.h>

#include <SDL.h>

#include "log.h"
#include "video.h"

static struct {
	bool	     init;
	int	     flags;
	SDL_Surface *screen;
} video;

int video_init(void)
{
	if (video.init) {
		log_warn("video seems already initialized");
		return 0;
	}

	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0) {
		log_err("could not initialize video subsystem: %s",
			  SDL_GetError());
		
		return errno ? -errno : -1;
	}
	
	video.init =  1;

	return 0;
}

INLINE void video_quit(void)
{
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

int video_set_icon(const char *bmp_path)
{
	SDL_Surface *ico;

	ico = SDL_LoadBMP(bmp_path);
	if (!ico) {
		if (!errno)
			log_warn("%s", SDL_GetError());
		else
			log_warn("%s: %s", SDL_GetError(), strerror(errno));
		return errno ? -errno : -1;
	}

	SDL_WM_SetIcon(ico, NULL);
	SDL_FreeSurface(ico);

	return 0;
}

int video_set_mode(int width, int height, int bpp)
{
	const SDL_VideoInfo *info;
	SDL_Surface *tmp;

	info = SDL_GetVideoInfo();
	
	log_info("video: hw surfaces: %s", info->hw_available ? "yes" : "no");
	if (info->hw_available) {
		video.flags |= SDL_HWSURFACE | SDL_DOUBLEBUF;
		log_info("video memory available: %dK", info->video_mem);
	}
	else {
		video.flags |= SDL_SWSURFACE;
	}
	
	if (!bpp) {
		bpp = info->vfmt->BitsPerPixel;
		log_info("video: using %d bits per pixel", bpp);
	}
	else if (bpp != info->vfmt->BitsPerPixel) {
		log_warn("video: using %d bits per pixel "
			 "(note: %d should work better!)",
			 bpp, info->vfmt->BitsPerPixel);
	}

	tmp = SDL_SetVideoMode(width, height, bpp, video.flags);
	if (!tmp) {
		log_err("video: could not set video mode: %s", SDL_GetError());
		return errno ? -errno : -1;
	}

	video.screen = tmp;

	return 0;
}

INLINE void video_set_title(const char *t)
{
	SDL_WM_SetCaption(t, t); /* 2nd argument is esoteric: don't ask */
}

INLINE void video_toggle_grab(void)
{
	int res;

	res = SDL_WM_GrabInput(SDL_GRAB_QUERY);
	res = (res == SDL_GRAB_OFF) ? SDL_GRAB_ON : SDL_GRAB_OFF;
	
	SDL_WM_GrabInput(res);
}

INLINE void video_toggle_fullscreen(void)
{
	SDL_WM_ToggleFullScreen(video.screen);    
}

INLINE void video_toggle_cursor(void)
{
	int res;

	res = SDL_ShowCursor(SDL_QUERY);
	res = (res == SDL_DISABLE) ? SDL_ENABLE : SDL_DISABLE;

	SDL_ShowCursor(res);
}

INLINE int video_flip(void)
{
	return SDL_Flip(video.screen);
}

INLINE void video_get_driver_name(char *buf, size_t bufsz)
{
	SDL_VideoDriverName(buf, bufsz);
}

INLINE u16 video_get_width(void)
{
	return video.screen->w;    
}

INLINE u16 video_get_height(void)
{
	return video.screen->h;
}

INLINE u32 video_get_flags(void)
{
	return video.flags;
}

INLINE u8 video_get_bpp(void)
{
	return video.screen->format->BitsPerPixel;
}

u32 video_get_pixel(s16 x, s16 y)
{
	u32 pixel;
	u8 r, g, b;

	if (x < 0 || x >= video.screen->w || y < 0 || y >= video.screen->h)
		return 0;

 	SDL_LockSurface(video.screen);

	pixel = *((u32 *)video.screen->pixels + y * video.screen->w + x);

 	SDL_UnlockSurface(video.screen);

	SDL_GetRGB(pixel, video.screen->format, &r, &g, &b);
	
	return (r << 16) | (g << 8) | b;
}
