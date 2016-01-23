/* Version information for Services.
 *
 * Services is copyright (c) 1996-1997 Preston A. Elder.
 *     E-mail: <prez@antisocial.com>   IRC: PreZ@DarkerNet
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

#define BUILD	"1"

const char version_number[] = "3.0.3";
const char version_build[] =
	"build #" BUILD ", compiled " __DATE__ " " __TIME__
#ifdef SKELETON
	" (skeleton version)"
#elifdef READONLY
	" (read-only mode)"
#endif
	;
