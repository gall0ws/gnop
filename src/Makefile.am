# $Id: Makefile.am 27 2009-08-28 21:03:48Z gallows $

bin_PROGRAMS=	gnop

SUBSYSTEMS=	video.c

if HAVE_AUDIO_SUPPORT
  SUBSYSTEMS+= audio.c
endif

gnop_SOURCES=	${SUBSYSTEMS}	\
		object.c	\
		layer.c		\
		sprite.c	\
		text.c		\
		ball.c		\
		paddle.c	\
		engine.c	\
		log.c		\
		main.c

AM_CFLAGS=	-Wall -Wno-switch -g -O2 ${sdl_CFLAGS}

DATADIR ?= 	${pkgdatadir}

AM_CPPFLAGS=	-DDATADIR=\"${DATADIR}\"
LDADD=		${sdl_LIBS}
