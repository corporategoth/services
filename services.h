/* Main header for Services.
 *
 * Services is copyright (c) 1996-1997 Preston A. Elder.
 *     E-mail: <prez@antisocial.com>   IRC: PreZ@DarkerNet
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

#ifndef SERVICES_H
#define SERVICES_H

/*************************************************************************/

/* Pre-checks to make sure the Makefile has things set up right. */

#if defined(SKELETON) && defined(READONLY)
# error SKELETON and READONLY cannot both be defined at once!
#endif

/*************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <setjmp.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <grp.h>

#include <ctype.h>

/* We have our own versions of toupper()/tolower(). */
#undef tolower
#undef toupper
#define tolower tolower_
#define toupper toupper_
extern int toupper(char), tolower(char);

#include "config.h"

/*************************************************************************/

/* Version number for data files; if structures below change, increment
 * this.  (Otherwise -very- bad things will happen!)
 * TO DATE: 1  - Original
 *          2  - Sreamlined Auto Kill Database
 *          3  - Added URL/E-Mail to Nick Database
 *          4  - Added IGNORE (memo reject)
 */

#define FILE_VERSION	4

/*************************************************************************/

/* Nickname info structure.  Each nick structure is stored in one of 256
 * lists; the list is determined by the first character of the nick.  Nicks
 * are stored in alphabetical order within lists. */

typedef struct nickinfo_ NickInfo;
struct nickinfo_ {
    NickInfo *next, *prev;
    char nick[NICKMAX];
    char pass[PASSMAX];
#if FILE_VERSION > 2
    char *email;
    char *url;
#endif
    char *last_usermask;
    char *last_realname;
    time_t time_registered;
    time_t last_seen;
    long accesscount;	/* # of entries */
    char **access;	/* Array of strings */
#if FILE_VERSION > 3
    long ignorecount;   /* # of entries */
    char **ignore;	/* Array of strings */
#endif
    long flags;		/* See below */
    long reserved[4];	/* For future expansion -- set to 0 */
};

#define NI_KILLPROTECT	0x00000001  /* Kill others who take this nick */
#define NI_SECURE	0x00000002  /* Don't recognize unless IDENTIFY'd */
#define NI_VERBOTEN	0x00000004  /* Nick may not be registered or used */
#define NI_IRCOP	0x00000008  /* IrcOP - Nick will not expire */

#define NI_IDENTIFIED	0x80000000  /* User has IDENTIFY'd */
#define NI_RECOGNIZED	0x40000000  /* User comes from a known addy */
#define NI_KILL_HELD	0x20000000  /* Nick is being held after a kill */

/*************************************************************************/

/* Channel info structures.  Stored similarly to the nicks, except that
 * the second character of the channel name, not the first, is used to
 * determine the list.  (Hashing based on the first character of the name
 * wouldn't get very far. ;) ) */

/* Access levels for users. */
typedef struct {
    short level;
    short is_nick;	/* 1 if this is a nick, 0 if a user@host mask.  If
			 * -1, then this entry is currently unused (a hack
			 * to get numbered lists to have consistent
			 * numbering). */
    char *name;
} ChanAccess;

/* AutoKick data. */
typedef struct {
    short is_nick;
    short pad;
    char *name;
    char *reason;
} AutoKick;

typedef struct chaninfo_ ChannelInfo;
struct chaninfo_ {
    ChannelInfo *next, *prev;
    char name[CHANMAX];
    char founder[NICKMAX];		/* Always a reg'd nick */
    char founderpass[PASSMAX];
    char *desc;
#if FILE_VERSION > 2
    char *url;
#endif
    time_t time_registered;
    time_t last_used;
    long accesscount;
    ChanAccess *access;			/* List of authorized users */
    long akickcount;
    AutoKick *akick;
    short mlock_on, mlock_off;		/* See channel modes below */
    long mlock_limit;			/* 0 if no limit */
    char *mlock_key;			/* NULL if no key */
    char *last_topic;			/* Last topic on the channel */
    char last_topic_setter[NICKMAX];	/* Who set the last topic */
    time_t last_topic_time;		/* When the last topic was set */
    long flags;				/* See below */
    short *cmd_access;			/* Access levels for commands */
    long reserved[3];			/* For future expansion -- set to 0 */
};

/* Retain topic even after last person leaves channel */
#define CI_KEEPTOPIC	0x00000001
/* Don't allow non-authorized users to be opped */
#define CI_SECUREOPS	0x00000002
/* Hide channel from ChanServ LIST command */
#define CI_PRIVATE	0x00000004
/* Topic can only be changed by SET TOPIC */
#define CI_TOPICLOCK	0x00000008
/* Those not allowed ops are kickbanned */
#define CI_RESTRICTED	0x00000010
/* Don't auto-deop anyone */
#define CI_LEAVEOPS	0x00000020
/* Don't allow any privileges unless a user is IDENTIFY'd with NickServ */
#define CI_SECURE	0x00000040
/* Don't allow the channel to be registered or used */
#define CI_VERBOTEN	0x00000080

/* Indices for cmd_access[]: */
#define CA_INVITE	0
#define CA_AKICK	1
#define CA_SET		2	/* but not FOUNDER or PASSWORD */
#define CA_UNBAN	3
#define CA_AUTOOP	4
#define CA_AUTODEOP	5
#define CA_OPDEOP	6	/* ChanServ commands OP and DEOP */

#define CA_SIZE		7

/*************************************************************************/

/* MemoServ data.  Only nicks that actually have memos get records in
 * MemoServ's lists, which are stored the same way NickServ's are. */

typedef struct memo_ Memo;

struct memo_ {
    char sender[NICKMAX];
    long number;	/* Index number -- not necessarily array position! */
    time_t time;	/* When it was sent */
    char *text;
    long reserved[4];	/* For future expansion -- set to 0 */
};


typedef struct memolist_ MemoList;
typedef struct newslist_ NewsList;

struct memolist_ {
    MemoList *next, *prev;
    char nick[NICKMAX];	/* Owner of the memos */
    long n_memos;	/* Number of memos */
    Memo *memos;	/* The memos themselves */
    long reserved[4];	/* For future expansion -- set to 0 */
};

struct newslist_ {
    NewsList *next, *prev;
    char chan[CHANMAX];	/* Owner of the memos */
    long n_newss;	/* Number of memos */
    Memo *newss;	/* The memos themselves */
    long reserved[4];	/* For future expansion -- set to 0 */
};

/*************************************************************************/

/* Online user and channel data. */

typedef struct user_ User;
typedef struct channel_ Channel;

struct user_ {
    User *next, *prev;
    char nick[NICKMAX];
    char *username;
    char *host;				/* user's hostname */
    char *realname;
    char *server;			/* name of server user is on */
    time_t signon;
    time_t my_signon;			/* when did _we_ see the user? */
    short mode;				/* see below */
    struct u_chanlist {
	struct u_chanlist *next, *prev;
	Channel *chan;
    } *chans;				/* channels user has joined */
    struct u_chaninfolist {
	struct u_chaninfolist *next, *prev;
	ChannelInfo *chan;
    } *founder_chans;			/* channels user has identified for */
};

#define UMODE_O 0x0001
#define UMODE_I 0x0002
#define UMODE_S 0x0004
#define UMODE_W 0x0008
#define UMODE_G 0x0010


struct channel_ {
    Channel *next, *prev;
    char name[CHANMAX];
    time_t creation_time;		/* when channel was created */
    char *topic;
    char topic_setter[NICKMAX];		/* who set the topic */
    time_t topic_time;			/* when topic was set */
    int mode;				/* binary modes only */
    int limit;				/* 0 if none */
    char *key;				/* NULL if none */
    int bancount, bansize;
    char **bans;
    struct c_userlist {
	struct c_userlist *next, *prev;
	User *user;
    } *users, *chanops, *voices;
};

#define CMODE_I 0x01
#define CMODE_M 0x02
#define CMODE_N 0x04
#define CMODE_P 0x08
#define CMODE_S 0x10
#define CMODE_T 0x20
#define CMODE_K 0x40			/* These two used only by ChanServ */
#define CMODE_L 0x80

/*************************************************************************/

/* Ignorance list data. */

typedef struct ignore_data {
    struct ignore_data *next;
    char who[NICKMAX];
    time_t time;	/* When do we stop ignoring them? */
} IgnoreData;

/*************************************************************************/

#include "extern.h"

/*************************************************************************/

#endif	/* SERVICES_H */
