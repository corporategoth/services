/* Services configuration.
 *
 * Services is copyright (c) 1996-1997 Preston A. Elder.
 *     E-mail: <prez@antisocial.com>   IRC: PreZ@DarkerNet
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

#ifndef CONFIG_H
#define CONFIG_H


/******* General configuration *******/

/* The remote server and port to use, and the password for the link. */
#define REMOTE_SERVER	"srealm.darker.net"
#define REMOTE_PORT	9666
#define PASSWORD	""	/* Not runtime-configurable */

/* Information about us as a server. */
#define SERVER_NAME	"hell.darker.net"
#define SERVER_DESC	"DarkerNet's IRC services"
#define SERVICES_USER	"reaper"
#define SERVICES_HOST	"darker.net"

/* Pick n choose */
#define	NICKSERV_ON
#define	CHANSERV_ON
#define	HELPSERV_ON
#define	IRCIIHELP_ON
#define	MEMOSERV_ON
#define OPERSERV_ON
#define	DEVNULL_ON
#define	GLOBALNOTICER_ON

/* Directory for data files */
#if 0	/* this is now handled by configure / sysconf.h */
# define SERVICES_DIR	"/usr/local/lib/services"
#endif

/*** End of runtime-configurable options. ***/

/* Log filename in services directory */
#define LOG_FILENAME	"services.log"

/* File for Message of the Dat (/motd) */
#define MOTD_FILENAME	"services.motd"

/* File for message to user upon logon */
#define LOGON_MSG	"services.msg"

/* Database filenames */
#define NICKSERV_DB	"nick.db"
#define CHANSERV_DB	"chan.db"
#define MEMOSERV_DB	"memo.db"
#define NEWSSERV_DB	"news.db"
#define AKILL_DB	"akill.db"

/* File containing process ID */
#define PID_FILE	"services.pid"

/* Subdirectory for help files */
#define HELPSERV_DIR	"helpfiles"

/* Delay (in seconds) between database updates.  (Incidentally, this is
 * also how often we check for nick/channel expiration.) */
#define UPDATE_TIMEOUT	300

/* Delay (in seconds) before we time out on a read and do other stuff,
 * like checking NickServ timeouts. */
#define READ_TIMEOUT	15

/* What timezone is Services in?  (Yes, there is a better way to do
 * this.  No, I can't think of it offhand, and don't really feel like
 * bothering.)
 *      Note that this is only used descriptively, so it doesn't really
 * affect anything critical (though it might confuse users a bit if it's
 * set wrong).
 */
#define TIMEZONE	"EST"



/******* ChanServ configuration *******/

/* Number of days before a channel expires */
#define CHANNEL_EXPIRE	14

/* Maximum number of AKICKs on a single channel. */
#define AKICK_MAX	32

/* Default reason for AKICK if none is given. */
#define DEF_AKICK_REASON "User has been banned from the channel"



/******* NickServ configuration *******/

/* Number of days before a nick registration expires */
#define NICK_EXPIRE	30

/* Delay (in seconds) before a NickServ-collided nick is released. */
#define RELEASE_TIMEOUT	60



/******* MemoServ configuration *******/

/* Number of days before news items expire */
#define NEWS_EXPIRE	21



/******* OperServ configuration *******/

/* Who are the Services ops? (space-separated list of NICKNAMES ONLY)
 * Note that to use commands limited to Services ops, a user must both:
 *	- Have a nickname in the list below
 *	- Identify with NickServ (therefore, the nick must be registered)
 */
#define SERVICES_OPS "PreZ Lord_Striker"

/* How big a hostname list do we keep for clone detection? */
#define CLONE_DETECT_SIZE 16

/* How many successive connects do we have to see before we consider the
 * possibility of clones? */
#define CLONE_MIN_USERS	5

/* How much time (in seconds) has to elapse between successive users
 * before we decide they're not clones? */
#define CLONE_MAX_DELAY	10

/* How long do we wait (in seconds) between successive warnings for
 * clones between the same host? */
#define CLONE_WARNING_DELAY 30



/******* Miscellaneous - it should be save to leave these untouched *******/

/* Extra warning: if you change these, your data files will be unusable! */

/* Size of input buffer */
#define BUFSIZE		1024

/* Maximum length of a channel name */
#define CHANMAX		64

/* Maximum length of a nickname */
#define NICKMAX		32

/* Maximum length of a password */
#define PASSMAX		32



/**************************************************************************/

/* System-specific defines */
#include "sysconf.h"

#if !HAVE_STRICMP && HAVE_STRCASECMP
# define stricmp strcasecmp
# define strnicmp strncasecmp
#endif

#endif	/* CONFIG_H */
