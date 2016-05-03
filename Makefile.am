# $Id: Makefile.am 27 2009-08-28 21:03:48Z gallows $

SUBDIRS=	src data
EXTRA_DIST=	COPYING INSTALL
MANPAGE=	gnop.6

gnopmandir=	${mandir}/man6
gnopman_DATA=	${MANPAGE}.gz

DISTCLEANFILES = ${MANPAGE}.gz

${MANPAGE}.gz:	${MANPAGE}
	cat ${MANPAGE} | gzip ${GZIP_ENV} > ${MANPAGE}.gz
