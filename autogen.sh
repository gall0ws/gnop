#! /bin/sh
# $Id: autogen.sh 22 2009-08-22 22:11:34Z gallows $
#
# Copyright (c) 2002  Daniel Elstner  <daniel.elstner@gmx.net>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License VERSION 2 as
# published by the Free Software Foundation.  You are not allowed to
# use any other version of the license; unless you got the explicit
# permission from the author to do so.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

useless_files="NEWS README AUTHORS ChangeLog"
dir=`echo "$0" | sed 's,[^/]*$,,'`
test "x${dir}" = "x" && dir='.'

if test "x`cd "${dir}" 2>/dev/null && pwd`" != "x`pwd`"
then
    echo "This script must be executed directly from the source directory."
    exit 1
fi

rm -f config.cache acconfig.h

touch $useless_files

#echo "- glib-gettextize."	      && \
#glib-gettextize --force --copy	      && \
echo "- aclocal."		      && \
aclocal $ACLOCAL_FLAGS	              && \
echo "- autoconf."		      && \
autoconf			      && \
echo "- autoheader."		      && \
autoheader			      && \
echo "- automake."		      && \
automake --add-missing --gnu --copy   && \
echo			  	      || exit 1

rm $useless_files
rm -rf autom4te.cache 

exit 0
