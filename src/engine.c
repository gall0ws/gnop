/* $Id: engine.c 27 2009-08-28 21:03:48Z gallows $
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

#include <errno.h>
#include <limits.h>
#include <time.h>

#if HAVE_ALLOCA_H
# include <alloca.h>
#endif

#include <SDL_timer.h>
#include <SDL_events.h>

#if HAVE_LIBSDL_MIXER
# include "audio.h"
# define PLAY_SND(SND) if (gnop.have_audio) audio_play(SND)
#else
# define PLAY_SND(SND)	/* nothing */
#endif

#include "video.h"

#include "ball.h"
#include "paddle.h"
#include "text.h"

#include "engine.h"

#define VIDEO_WIDTH	512
#define VIDEO_HEIGHT	400

#define IDLE_MS		20

#define TIME_PREGAME	500
#define TIME_SCORED	800
#define TIME_GAME_OVER	3000

#define FONT_BASENAME	"DejaVuSans.ttf"
#define FONT_SCORE_PTSZ	48
#define FONT_WON_PTSZ	24

#define PANEL_ALPHA	92

#define SCORE_TXT_Y	16
#define SCORE_LIMIT	10
#define WON_TXT_Y	(VIDEO_HEIGHT - 64)

/*
 * Place score text for player P.
 */
#define AUTO_SET_X_SCORE_TXT(P) do {					\
	if (P == 0)							\
		layer_set_x(gnop.score_txt[P], VIDEO_WIDTH / 3 +	\
			    - layer_get_width(gnop.score_txt[P]) / 2);	\
	else								\
		layer_set_x(gnop.score_txt[P], VIDEO_WIDTH * 2/3 +	\
			    - layer_get_width(gnop.score_txt[P]) / 2);	\
} while (0)

/*
 * Place won text for player P.
 */
#define AUTO_SET_X_WON_TXT(P) do {					\
	if (P == 0)							\
		layer_set_x(gnop.won_txt, VIDEO_WIDTH / 4 +		\
			    - layer_get_width(gnop.won_txt) / 2);	\
	else								\
		layer_set_x(gnop.won_txt, VIDEO_WIDTH * 3/4 +		\
			    - layer_get_width(gnop.won_txt) / 2);	\
} while (0)

/*
 * Set engine status.
 */ 
#define SET_STATE(STATE) do {						\
	gnop.prev_state = gnop.state;					\
	gnop.state = STATE;						\
} while (0)


/*
 * gnop states:
 */
enum {
	STATE_PREGAME,
	STATE_INGAME,
	STATE_GAMEOVER,
	STATE_IDLE,
};

/*
 * gnop's Engine variables:
 */
static struct Engine {
	u8    state, prev_state;
	bool  running;
	bool  first_run;
	bool  paused;
	bool  todraw_panel;
	bool  have_audio;

	bool  key_up_pressed;
	bool  key_down_pressed;

	time_t  tstart;

	u8   score[2];

	Sprite *bg;
	Sprite *panel;
	Ball *ball;
	Paddle *paddle[2];
	Text *score_txt[2];
	Text *won_txt;

	s16     paddle_x[2];

	/*
	 * gnop.scored stores:
	 *	0	no one scored
	 *	1	player1 scored
	 *	2	player2 scored
	 */
	u8	scored;

	char *datadir;
} gnop;

static void init_sprites  (u32 fg_color, u32 bg_color);
static void draw          (void);
static void handle_input  (void);
static void handle_ai     (void);
static void handle_ball   (void);
static void go_idle       (int ms);

/*
 * Initialize gnop's engine.
 */
int engine_init(u8 opts, const char *datadir, u32 fg_color, u32 bg_color)
{
	if (video_init() != 0)
		return -1;

	video_set_title("gnop");
	video_toggle_cursor();

	if (video_set_mode(VIDEO_WIDTH, VIDEO_HEIGHT, 0) != 0) {
		video_quit();
		return -1;
	}

	ttf_init();

	if (opts & ENGINE_OPTION_FS)
		video_toggle_fullscreen();

	if (video_get_bpp() != 8) {
		gnop.panel = sprite_new(VIDEO_WIDTH, VIDEO_HEIGHT);
		sprite_set_alpha(gnop.panel, PANEL_ALPHA);
	}

	gnop.datadir = datadir ? strdup(datadir) : strdup(DATADIR);
	init_sprites(fg_color, bg_color);

#if HAVE_LIBSDL_MIXER
	if (!(opts & ENGINE_OPTION_MUTE)) {
		audio_init(gnop.datadir);
		gnop.have_audio = 1;
	}
#endif

	time(&gnop.tstart);

	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, 
			    SDL_DEFAULT_REPEAT_INTERVAL);

	return 0;
}

/*
 * Quit gnop's engine.
 */
void engine_quit(void)
{
	time_t ticks;
	div_t t60;

	time(&ticks);
	ticks -= gnop.tstart;

	if (ticks) {
		printf("\nAddicted to gnop for ");
		t60 = div(ticks, 60);
		if (t60.quot > 0) {
			ticks = t60.rem;
			printf("%d minute%s%s", 
			       t60.quot, t60.quot == 1 ? "" : "s",
			       ticks ? " and " : "\n\n");
		}
		
		if (ticks)
			printf("%lu second%s\n\n", 
			       ticks, ticks == 1 ? "" : "s");
	}

	objects_free(gnop.bg, gnop.ball, gnop.score_txt[0], gnop.score_txt[1],
		     gnop.won_txt, gnop.paddle[0], gnop.paddle[1], gnop.panel,
		     NULL);

	free(gnop.datadir);

#if HAVE_LIBSDL_MIXER
	if (gnop.have_audio)
		audio_quit();
#endif
	ttf_quit();
	video_quit();
}

/*
 * Gnop main loop.
 */
int engine_loop(void)
{
	int p;

	if (gnop.running) {
		log_warn("main loop is already runnning");
		return 1;
	}

	gnop.running = 1;
	gnop.first_run = 1;
	gnop.state = STATE_PREGAME;
	srand(time(NULL));

	do {
		if (gnop.state == STATE_PREGAME) {
			if (gnop.first_run) {
				gnop.first_run = 0;
			}
			else {  /* reset scores and paddles position */
				gnop.score[0] = gnop.score[1] = 0;
				text_set_text(gnop.score_txt[0], "0");
				text_set_text(gnop.score_txt[1], "0");
				AUTO_SET_X_SCORE_TXT(0);
				AUTO_SET_X_SCORE_TXT(1);
				paddle_reset_pos(gnop.paddle[0]);
				paddle_reset_pos(gnop.paddle[1]);
			}

 			go_idle(TIME_PREGAME);
			SET_STATE(STATE_INGAME);

			ball_start(gnop.ball);
		}
		else if (gnop.scored) {
			/* someone scored (player gnop.scored-1) */
			p = gnop.scored - 1;
			text_set_text(gnop.score_txt[p], 
				      "%d", gnop.score[p]);

			AUTO_SET_X_SCORE_TXT(p);

			if (gnop.score[p] >= SCORE_LIMIT &&
			    gnop.score[p] > gnop.score[!p] + 1) {
				/* it was a match ball.. */
				SET_STATE(STATE_GAMEOVER);
				AUTO_SET_X_WON_TXT(p);

				log_info("Player %d won: %d - %d",
					 gnop.score[0] > gnop.score[1] ? 1 : 2,
					 gnop.score[0], gnop.score[1]);
				PLAY_SND(AUDIO_GAMEOVER);

				go_idle(TIME_GAME_OVER);
				SET_STATE(STATE_PREGAME);
			}
			else {
				go_idle(TIME_SCORED);
				ball_start(gnop.ball);
			}

			gnop.scored = 0;
		}

		handle_input();
 		handle_ai();

		if (gnop.state == STATE_INGAME)
			handle_ball();

		draw();

		SDL_Delay(IDLE_MS);
	} while (gnop.running);
    
	return 0;
}

/*
 * Absolutely non-portable utility to join path `a' with `b'.
 */
char *join_path(const char *a, const char *b, char *buf)
{
	char *joined_path;
	char *retp;
	size_t sz;

	sz = snprintf(NULL, 0, "%s/%s", a, b);

#if HAVE_ALLOCA
	joined_path = alloca(sz+1);
#else
	joined_path = malloc(sz+1);
#endif

	sprintf(joined_path, "%s/%s", a, b);
	errno = 0;
	retp = realpath(joined_path, buf);
	if (!retp)
		log_err("join_path(%s): %s", joined_path, strerror(errno));

#if !HAVE_ALLOCA
	free(joined_path);
#endif

	return retp;
}

/*
 * Create and initialize some gnop's sprites.
 */
static void init_sprites(u32 fg_color, u32 bg_color)
{
	char path[PATH_MAX];
	s16 i;

	gnop.bg = sprite_new(VIDEO_WIDTH, VIDEO_HEIGHT);
  	sprite_fill(gnop.bg, bg_color);
	sprite_set_accel(gnop.bg, bg_color);

	for (i=13; i<VIDEO_HEIGHT; ++i) { /* separator */
		if (!(i % 13))
			sprite_fill_region(gnop.bg, VIDEO_WIDTH / 2 - 2, 
					   i-4, 4, 6, fg_color);

		/* I know, too many magic numbers... */
	}

	gnop.ball = ball_new(fg_color);

	gnop.paddle[0] = paddle_new(fg_color, PADDLE_POS_LEFT);
	gnop.paddle[1] = paddle_new(fg_color, PADDLE_POS_RIGHT);
	gnop.paddle_x[0] = layer_get_x(gnop.paddle[0]);
	gnop.paddle_x[1] = layer_get_x(gnop.paddle[1]);
	
	join_path(gnop.datadir, FONT_BASENAME, path);

	gnop.score_txt[0] = text_new(path, FONT_SCORE_PTSZ, fg_color);
	gnop.score_txt[1] = text_new(path, FONT_SCORE_PTSZ, fg_color);
	text_set_text(gnop.score_txt[0], "0");
	text_set_text(gnop.score_txt[1], "0");
	layer_set_y(gnop.score_txt[0], SCORE_TXT_Y);
	layer_set_y(gnop.score_txt[1], SCORE_TXT_Y);
	AUTO_SET_X_SCORE_TXT(0);
	AUTO_SET_X_SCORE_TXT(1);

	gnop.won_txt = text_new(path, FONT_WON_PTSZ, fg_color);
	text_set_text(gnop.won_txt, "won");
	layer_set_y(gnop.won_txt,  WON_TXT_Y);
}

/*
 * Performs sprites blit then update screen.
 */
static void draw(void)
{
	if (gnop.paused && !gnop.todraw_panel)
		return;

	objects_blit(gnop.bg, gnop.score_txt[0], gnop.score_txt[1],
		     gnop.paddle[0], gnop.paddle[1], NULL);

	if (gnop.state != STATE_IDLE)
		object_blit(gnop.ball);

	if (gnop.state == STATE_IDLE && gnop.prev_state == STATE_GAMEOVER)
		object_blit(gnop.won_txt);

	if (gnop.paused) {
		object_blit(gnop.panel);
		gnop.todraw_panel = 0;
	}

	video_flip();
}

/*
 * Handle user input.
 */
static void handle_input(void)
{
	SDL_Event event;
	
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				gnop.running = 0;
				break;

			case SDLK_UP:
				gnop.key_up_pressed = 1;
				break;

			case SDLK_DOWN:
				gnop.key_down_pressed = 1;
				break;

			case SDLK_F2:
				gnop.state = STATE_PREGAME;
				log_info("match restarted");
				break;

			case SDLK_f:
				video_toggle_fullscreen();
				break;
#if HAVE_LIBSDL_MIXER
			case SDLK_m:
				if (gnop.have_audio) audio_toggle_mute();
				break;
#endif
			case SDLK_p:
				gnop.paused = !gnop.paused;
				if (gnop.paused)
					gnop.todraw_panel = 1;
				break;
#if HAVE_LIBSDL_MIXER
			case SDLK_9:
				if (gnop.have_audio) audio_volume_down();
				break;

			case SDLK_0:
				if (gnop.have_audio) audio_volume_up();
				break;
#endif
			}
			break;

		case SDL_KEYUP:
			switch (event.key.keysym.sym) {
			case SDLK_UP:
				gnop.key_up_pressed = 0;
				break;
				
			case SDLK_DOWN:
				gnop.key_down_pressed = 0;
				break;
			}
			break;

		case SDL_QUIT:
			gnop.running = 0;
			return;
		}
	}

	if (gnop.key_up_pressed)
		paddle_move(gnop.paddle[0], PADDLE_MOVE_UP);
	else if (gnop.key_down_pressed)
		paddle_move(gnop.paddle[0], PADDLE_MOVE_DOWN);
}

/*
 * Handle CPU player.
 */
static void handle_ai(void)
{
	u16 pbar, pbar_human;
	s16 bx, by;
	s8 vect_x, vect_y;

	pbar = layer_get_y(gnop.paddle[1]) + PADDLE_HEIGHT / 2;

	ball_get_vectors(gnop.ball, &vect_x, &vect_y);
	bx = layer_get_x(gnop.ball);
	by = layer_get_y(gnop.ball);

	if (gnop.state == STATE_IDLE || gnop.state == STATE_PREGAME) {
		if (pbar < VIDEO_HEIGHT / 2)
			paddle_move(gnop.paddle[1], PADDLE_MOVE_DOWN);
		else if (pbar > VIDEO_HEIGHT / 2)
			paddle_move(gnop.paddle[1], PADDLE_MOVE_UP);

		return;
	}
	
	if (vect_x < 0 && bx + BALL_WIDTH <= VIDEO_WIDTH * 2/3) {
		if (pbar <= VIDEO_HEIGHT * 2/5)
			paddle_move(gnop.paddle[1], PADDLE_MOVE_DOWN);
		else if (pbar >= VIDEO_HEIGHT * 3/5)
			paddle_move(gnop.paddle[1], PADDLE_MOVE_UP);
		
		if (gnop.state == STATE_PREGAME)
			return;
	}

	if (vect_x > 0 && bx + BALL_WIDTH + vect_x >= gnop.paddle_x[1]) {
		pbar_human = layer_get_y(gnop.paddle[0]) + PADDLE_HEIGHT / 2;
		if (pbar_human > VIDEO_HEIGHT / 2)
			paddle_move(gnop.paddle[1], PADDLE_MOVE_UP);
		else
			paddle_move(gnop.paddle[1], PADDLE_MOVE_DOWN);
	}

	if (vect_x < 0 && abs(vect_y) > vect_x && bx < VIDEO_WIDTH * 2/3)
		return;

	if ((vect_x < 0 && bx + BALL_WIDTH / 2 <= VIDEO_WIDTH * 2/3) ||
	    (vect_x < 10 && bx + BALL_WIDTH / 2 <= VIDEO_WIDTH * 3/5))
		return;
	
	if (pbar + 5 < by + BALL_HEIGHT / 2)
		paddle_move(gnop.paddle[1], PADDLE_MOVE_DOWN);
	else if (pbar - 5 > by + BALL_HEIGHT / 2)
		paddle_move(gnop.paddle[1], PADDLE_MOVE_UP);
}

/*
 * Handle ball activities.
 */
static void handle_ball(void)
{
	s16 x, y;
	s8 vect_x, lm;
	int i, j, p;
	bool hit;

	x = layer_get_x(gnop.ball);
	y = layer_get_y(gnop.ball);

	p = -1;
	if (x + BALL_WIDTH < 0) 
		p = 1; /* player1 scored */
	else if (x >= VIDEO_WIDTH)
		p = 0; /* player0 scored */

	if (p > -1) { /* update score */
		++gnop.score[p];
		gnop.scored = p+1;
		PLAY_SND(AUDIO_SCORED);
		return;
	}

	/* Check for bouncing against horizontal walls:  */
	if (y <= 0 || y + BALL_HEIGHT >= VIDEO_HEIGHT) {
		ball_bounce(gnop.ball, BALL_BOUNCE_V, 0);
		PLAY_SND(AUDIO_BOUNCE);

		/* fix ball position: */
		if (y < 0)
			layer_set_y(gnop.ball, 0);
		else if (y + BALL_HEIGHT > VIDEO_HEIGHT)
			layer_set_y(gnop.ball, VIDEO_HEIGHT - BALL_HEIGHT);

		return;
	}

	if (x <= gnop.paddle_x[0] + PADDLE_WIDTH)
		p = 0; /* player0 could have hit the ball */
	else if (x + BALL_WIDTH >= gnop.paddle_x[1])
		p = 1; /* player1 could have hit the ball */
	else
		return;

	ball_get_vectors(gnop.ball, &vect_x, NULL);
	if ((!p && vect_x > 0) || (p && vect_x < 0))
		return;

	/* Check for paddle-ball collisions: */
	for (i=0, hit=0; i<BALL_WIDTH && !hit; ++i) {
		for (j=0; j<BALL_HEIGHT && !hit; ++j) {
			if (layer_own(gnop.paddle[p], x+i, y+j)) {
				/*
				 * Player(p) hit the ball, now we have to fix
				 * ball position then perform bouncing.
				 */

				if (p) {
					layer_set_x(gnop.ball, 
						    gnop.paddle_x[1] +
						    - BALL_WIDTH);
				}
				else {
					layer_set_x(gnop.ball,
						    gnop.paddle_x[0] + 
						    PADDLE_WIDTH);
				}

				lm = paddle_last_move(gnop.paddle[p]);
				ball_bounce(gnop.ball, BALL_BOUNCE_H, lm);
				PLAY_SND(AUDIO_BOUNCE);
				hit = 1;
			}
		}
	}
}

/*
 * Set the game in idle mode for `ms' milliseconds.
 */
static void go_idle(int ms)
{
	u32 ticks;

	gnop.prev_state = gnop.state;
	gnop.state = STATE_IDLE;

	ticks = SDL_GetTicks();
	while (ticks + ms > SDL_GetTicks() && gnop.running) {
		handle_input();

		if (gnop.prev_state != STATE_GAMEOVER)
			handle_ai();

		draw();

		SDL_Delay(IDLE_MS);
	}

	gnop.state ^= gnop.prev_state;
	gnop.prev_state ^= gnop.state;
	gnop.state ^= gnop.prev_state;
}
