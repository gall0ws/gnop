/* $Id: audio.c 27 2009-08-28 21:03:48Z gallows $
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

#include <errno.h>
#include <stdio.h>
#include <limits.h>

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include "log.h"
#include "audio.h"
#include "engine.h"

#define BOUNCE_FILE	"bounce.raw"
#define SCORED_FILE	"scored.raw"
#define GAMEOVER_FILE	"gameover.raw"

#define AUDIO_FREQ	44100
#define AUDIO_FMT	AUDIO_S16SYS
#define AUDIO_CHANNELS	1
#define CHUNKSZ		2048

#define VOLUME_START	104	/* ~80% */
#define VOLUME_STEP	8	/*  ~5% */

#define VOLUME_PERC(X)	(X * 100 / MIX_MAX_VOLUME)

static struct {
	Mix_Chunk *chunk[AUDIO_SOUND_NO];
	u8	   vol;		/* [0-128] */
	bool       inited;
	bool       mute;
} audio;

/*
 * Get file size of the given stream.
 */
INLINE static size_t get_file_size(FILE *stream)
{
	size_t b;
	long ofs;

	ofs = ftell(stream);
	fseek(stream, 0, SEEK_END);
	b = ftell(stream);
 	fseek(stream, ofs, SEEK_SET);

	return b;
}

/*
 * Load a sound.
 */
static int load_snd(AudioSound snd, const char *path)
{
	FILE *fp;
	size_t sz;
	u8 *buf;

	fp = fopen(path, "rb");
	if (!fp) {
		log_err("audio_init: could not open `%s': %s", 
			path, strerror(errno));
		return -errno;
	}

	sz = get_file_size(fp);

	buf = malloc(sz);
	fread(buf, sz, 1, fp);
	fclose(fp);

	audio.chunk[snd] = Mix_QuickLoad_RAW(buf, sz);
	if (!audio.chunk[snd]) {
		log_err("could not load chunk: %s", SDL_GetError());
		return -1;
	}

	audio.chunk[snd]->volume = VOLUME_START;

	return 0;
}

/*
 * Initialize audio subsystem.
 */
int audio_init(const char *datadir)
{
	char buf[PATH_MAX];

	if (audio.inited) {
		log_warn("audio: subsystem seems already initialized");
		return 1;
	}

	if (Mix_OpenAudio(AUDIO_FREQ, AUDIO_FMT, AUDIO_CHANNELS, CHUNKSZ)) {
		log_err("could not open audio: %s", Mix_GetError());
		return -1;
	}

	Mix_AllocateChannels(4);

	SDL_AudioDriverName(buf, sizeof(buf));
	log_info("driver audio: %s", buf);

	audio.vol = VOLUME_START;

	if (!datadir)
		datadir = DATADIR;

	join_path(datadir, BOUNCE_FILE, buf);
	load_snd(AUDIO_BOUNCE, buf);
	
	join_path(datadir, SCORED_FILE, buf);
	load_snd(AUDIO_SCORED, buf);

	join_path(datadir, GAMEOVER_FILE, buf);
	load_snd(AUDIO_GAMEOVER, buf);

	audio.inited = 1;

	return 0;
}

/*
 * Close audio subsystem.
 */
void audio_quit(void)
{
	int i;

	if (audio.inited) {
		for (i=0; i<AUDIO_SOUND_NO; ++i) {
			free(audio.chunk[i]->abuf);
			Mix_FreeChunk(audio.chunk[i]);
		}

		Mix_CloseAudio();
		audio.inited = 0;
	}
}

/*
 * Play given sound.
 */
INLINE void audio_play(AudioSound snd)
{
	if (!audio.mute && snd < AUDIO_SOUND_NO)
		if (Mix_PlayChannel(-1, audio.chunk[snd], 0) == -1)
			log_err("Mix_PlayChannel: %s", Mix_GetError());
}

/*
 * Enable/disable mute.
 */
INLINE bool audio_toggle_mute(void)
{
	log_info("audio mute: %s", 
		 (audio.mute = !audio.mute) ? "off" : "on");

	return audio.mute;
}

/*
 * Increase volume.
 */
int audio_volume_up(void)
{
	int i;

	if (audio.vol + VOLUME_STEP <= MIX_MAX_VOLUME) {
		audio.vol += VOLUME_STEP;
		for (i=0; i<AUDIO_SOUND_NO; ++i)
			audio.chunk[i]->volume = audio.vol;
	}

/* 	log_debug("real: %d\t\tperc: %d",  */
/* 		  audio.vol, VOLUME_PERC(audio.vol)); */

	return VOLUME_PERC(audio.vol);
}

/*
 * Decrease volume.
 */
int audio_volume_down(void)
{
	int i;

	if (audio.vol - VOLUME_STEP >= 0) {
		audio.vol -= VOLUME_STEP;
		for (i=0; i<AUDIO_SOUND_NO; ++i)
			audio.chunk[i]->volume = audio.vol;
	}

/* 	log_debug("real: %d\t\tperc: %d",  */
/* 		  audio.vol, VOLUME_PERC(audio.vol)); */

	return VOLUME_PERC(audio.vol);
}
