#!/bin/sh
#
# Increment Services build number

VERSION=3.0.0

if [ -f version.h ] ; then
	BUILD=`fgrep '#define BUILD' version.h | sed 's/^#define BUILD.*"\([0-9]*\)".*$/\1/'`
	BUILD=`expr $BUILD + 1`
else
	BUILD=1
fi
cat >version.h <<EOF
/* Version information for Services.
 *
 * Services is copyright (c) 1996-1997 Preston A. Elder.
 *     E-mail: <prez@antisocial.com>   IRC: PreZ@DarkerNet
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

#define BUILD	"$BUILD"

const char version_number[] = "$VERSION";
const char version_build[] =
	"build #" BUILD ", compiled " __DATE__ " " __TIME__
#ifdef SKELETON
	" (skeleton version)"
#elifdef READONLY
	" (read-only mode)"
#endif
	;
EOF
