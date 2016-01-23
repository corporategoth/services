/* Services -- main source file.
 * Copyright (c) 1996-97 Preston A. Elder <prez@antisocial.com>  PreZ@DarkerNet
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, write to the
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "services.h"


/******** Global variables! ********/

/* These can all be set by options. */
char *remote_server = REMOTE_SERVER;	/* -remote server[:port] */
int remote_port = REMOTE_PORT;
char *server_name = SERVER_NAME;	/* -name servername */
char *server_desc = SERVER_DESC;	/* -desc serverdesc */
char *services_user = SERVICES_USER;	/* -user username */
char *services_host = SERVICES_HOST;	/* -host hostname */
char *services_dir = SERVICES_DIR;	/* -dir directory */
char *log_filename = LOG_FILENAME;	/* -log filename */
char *time_zone = TIMEZONE;		/* -tz timezone */
int update_timeout = UPDATE_TIMEOUT;	/* -update secs */
int debug = 0;				/* -debug */

/* What gid should we give to all files?  (-1 = don't set) */
gid_t file_gid = -1;

/* Set to 1 if we are to quit */
int quitting = 0;

/* Contains a message as to why services is terminating */
char *quitmsg = NULL;

/* Input buffer - global, so we can dump it if something goes wrong */
char inbuf[BUFSIZE];

/* Socket for talking to server */
int servsock = -1;

/* Should we update the databases now? */
int save_data = 0;

/* At what time were we started? */
time_t start_time;


/******** Local variables! ********/

/* Set to 1 if we are waiting for input */
static int waiting = 0;

/* Set to 1 after we've set everything up */
static int started = 0;

/* If we get a signal, use this to jump out of the main loop. */
static jmp_buf panic_jmp;

/*************************************************************************/

/* If we get a weird signal, come here. */

static void sighandler(int signum)
{
    if (started) {
    	if (signum == SIGHUP) {  /* SIGHUP = save databases and quit */
    	    save_data = -1;
    	    signal(SIGHUP, SIG_IGN);
    	    return;
	} else if (!waiting) {
	    log("PANIC! buffer = %s", inbuf);
	    /* Cut off if this would make IRC command >510 characters. */
	    if (strlen(inbuf) > 448) {
		inbuf[446] = '>';
		inbuf[447] = '>';
		inbuf[448] = 0;
	    }
	    wallops("PANIC! buffer = %s\r\n", inbuf);
	} else if (waiting < 0) {
	    if (waiting == -1) {
		log("PANIC! in timed_update");
		wallops("PANIC! in timed_update");
	    } else {
		log("PANIC! waiting=%d", waiting);
		wallops("PANIC! waiting=%d", waiting);
	    }
	}
    }
    if (signum == SIGUSR1 || !(quitmsg = malloc(256))) {
	quitmsg = "Out of memory!";
	quitting = 1;
    } else {
#if HAVE_STRSIGNAL
	sprintf(quitmsg, "Services terminating: %s", strsignal(signum));
#else
	sprintf(quitmsg, "Services terminating on signal %d", signum);
#endif
	quitting = 1;
    }
    if (started)
	longjmp(panic_jmp, 1);
    else {
	log("%s", quitmsg);
	exit(1);
    }
}

/*************************************************************************/

/* Log stuff to stderr with a datestamp. */

void log(const char *fmt,...)
{
    va_list args;
    time_t t;
    struct tm tm;
    char buf[256];

    va_start(args, fmt);
    time(&t);
    tm = *localtime(&t);
    strftime(buf, sizeof(buf)-1, "[%b %d %H:%M:%S %Y] ", &tm);
    fputs(buf, stderr);
    vfprintf(stderr, fmt, args);
    fputc('\n', stderr);
    fflush(stderr);
}

/* Like log(), but tack a ": " and a system error message (as returned by
 * strerror() onto the end.
 */

void log_perror(const char *fmt,...)
{
    va_list args;
    time_t t;
    struct tm tm;
    char buf[256];

    va_start(args, fmt);
    time(&t);
    tm = *localtime(&t);
    strftime(buf, sizeof(buf)-1, "[%b %d %H:%M:%S %Y] ", &tm);
    fputs(buf, stderr);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, ": %s\n", strerror(errno));
    fflush(stderr);
}

/*************************************************************************/

/* We've hit something we can't recover from.  Let people know what
 * happened, then go down.
 */

void fatal(const char *fmt,...)
{
    va_list args;
    time_t t;
    struct tm tm;
    char buf[256], buf2[4096];

    va_start(args, fmt);
    time(&t);
    tm = *localtime(&t);
    strftime(buf, sizeof(buf)-1, "[%b %d %H:%M:%S %Y]", &tm);
    vsprintf(buf2, fmt, args);
    fprintf(stderr, "%s FATAL: %s\n", buf, buf2);
    fflush(stderr);
    if (servsock >= 0)
	wallops("FATAL ERROR!  %s", buf2);
    exit(1);
}

/* Same thing, but do it like perror().
 */

void fatal_perror(const char *fmt,...)
{
    va_list args;
    time_t t;
    struct tm tm;
    char buf[256], buf2[4096];

    va_start(args, fmt);
    time(&t);
    tm = *localtime(&t);
    strftime(buf, sizeof(buf)-1, "[%b %d %H:%M:%S %Y]", &tm);
    vsprintf(buf2, fmt, args);
    fprintf(stderr, "%s FATAL: %s: %s\n", buf, buf2, strerror(errno));
    fflush(stderr);
    if (servsock >= 0)
	wallops("FATAL ERROR!  %s", buf2);
    exit(1);
}

/*************************************************************************/

int get_file_version(FILE *f, const char *filename)
{
    int version = fgetc(f)<<24 | fgetc(f)<<16 | fgetc(f)<<8 | fgetc(f);
    if (ferror(f) || feof(f))
	fatal_perror("Error reading version number on %s", filename);
    else if (version > FILE_VERSION || version < 1)
	fatal("Invalid version number (%d) on %s", version, filename);
    return version;
}

void write_file_version(FILE *f, const char *filename)
{
    if (
	fputc(FILE_VERSION>>24 & 0xFF, f) < 0 ||
	fputc(FILE_VERSION>>16 & 0xFF, f) < 0 ||
	fputc(FILE_VERSION>> 8 & 0xFF, f) < 0 ||
	fputc(FILE_VERSION     & 0xFF, f) < 0
    )
	fatal_perror("Error writing version number on %s", filename);
}

/*************************************************************************/

/* Send a NICK command for the given pseudo-client.  If `user' is NULL,
 * send NICK commands for all the pseudo-clients. */

#if defined(IRC_DALNET)
# define NICK(nick,name) \
    do { \
	send_cmd(NULL, "NICK %s 1 %lu %s %s %s :%s", (nick), time(NULL), \
		services_user, services_host, server_name, (name)); \
    } while (0)
#elif defined(IRC_UNDERNET)
# define NICK(nick,name) \
    do { \
	send_cmd(server_name, "NICK %s 1 %lu %s %s %s :%s", (nick), time(NULL),\
		services_user, services_host, server_name, (name)); \
    } while (0)
#elif defined(IRC_TS8)
# define NICK(nick,name) \
    do { \
	send_cmd(NULL, "NICK %s :1", (nick)); \
	send_cmd((nick), "USER %ld %s %s %s :%s", time(NULL), \
		services_user, services_host, server_name, (name)); \
    } while (0)
#else
# define NICK(nick,name) \
    do { \
	send_cmd(NULL, "NICK %s :1", (nick)); \
	send_cmd((nick), "USER %ld %s %s %s :%s", \
		services_user, services_host, server_name, (name)); \
    } while (0)
#endif

void introduce_user(const char *user)
{
#ifdef NICKSERV_ON
    if (!user || stricmp(user, s_NickServ) == 0) {
	NICK(s_NickServ, "Nickname Server");
    }
#endif
#ifdef CHANSERV_ON
    if (!user || stricmp(user, s_ChanServ) == 0) {
	NICK(s_ChanServ, "Channel Server");
    }
#endif
#ifdef HELPSERV_ON
    if (!user || stricmp(user, s_HelpServ) == 0) {
	NICK(s_HelpServ, "Help Server");
    }
#endif
#ifdef IRCIIHELP_ON
    if (!user || stricmp(user, "IrcIIHelp") == 0) {
	NICK("IrcIIHelp", "ircII Help Server");
    }
#endif
#ifdef MEMOSERV_ON
    if (!user || stricmp(user, s_MemoServ) == 0) {
	NICK(s_MemoServ, "Memo Server");
    }
#endif
#ifdef OPERSERV_ON
    if (!user || stricmp(user, s_OperServ) == 0) {
	NICK(s_OperServ, "Operator Server");
	send_cmd(s_OperServ, "MODE %s +i", s_OperServ);
    }
#endif
#ifdef DEVNULL_ON
    if (!user || stricmp(user, "DevNull") == 0) {
	NICK("DevNull", "/dev/null -- message sink");
	send_cmd(NULL, ":DevNull MODE DevNull +i");
    }
#endif
#ifdef GLOBALNOTICER_ON
    if (!user || stricmp(user, s_GlobalNoticer) == 0) {
	NICK(s_GlobalNoticer, "Global Noticer");
	send_cmd(s_GlobalNoticer, "MODE %s +io", s_GlobalNoticer);
    }
#endif
}

#undef NICK

/*************************************************************************/

/* Remove our PID file.  Done at exit. */
void remove_pidfile()
{
    remove(PID_FILE);
}

/*************************************************************************/

/* Main routine.  (What does it look like? :-) ) */
/* (Yes, big and ugly, I know...) */

int main(int ac, char **av)
{
    time_t last_update;	/* When did we last update the databases? */
#ifndef SKELETON
    time_t last_check;	/* When did we last check NickServ timeouts? */
#endif
    int i;
    char *progname;
    char *s, *t;
#ifdef RUNGROUP
    struct group *gr;
    char *errmsg = NULL;
#endif
    FILE *pidfile;


#ifdef DEFUMASK
    umask(DEFUMASK);
#endif

#ifdef RUNGROUP
    setgrent();
    while (gr = getgrent()) {
	if (strcmp(gr->gr_name, RUNGROUP) == 0)
	    break;
    }
    endgrent();
    if (gr)
	file_gid = gr->gr_gid;
    else
	errmsg = "Group not known";
#endif

    /* Find program name. */
    if (progname = strrchr(av[0], '/'))
	++progname;
    else
	progname = av[0];


    /* Parse -dir ahead of time. */
    for (i = 1; i < ac; ++i) {
	s = av[i];
	if (*s == '-') {
	    ++s;
	    if (strcmp(s, "dir") == 0) {
		if (++i >= ac) {
		    log("-dir requires a parameter");
		    break;
		}
		services_dir = av[i];
	    }
	}
    }

    /* Chdir to services directory. */

    if (chdir(services_dir) < 0) {
	perror(services_dir);
	return 20;
    }


#ifndef SKELETON

    /* Check for program name == "listnicks" or "listchans" and do
     * appropriate special stuff if so. */

    if (strcmp(progname, "listnicks") == 0) {

	int count = 0;	/* Count only rather than display? */
	int usage = 0;	/* Display command usage?  (>0 also indicates error) */
	int i;

#ifdef RUNGROUP
	if (errmsg)
	    log("%s: %s", errmsg, RUNGROUP);
#endif

	i = 1;
	while (i < ac) {
	    if (av[i][0] == '-') {
		switch (av[i][1]) {
		case 'h':
		    usage = -1; break;
		case 'c':
		    if (i > 1)
			usage = 1;
		    count =  1; break;
		default :
		    usage =  1; break;
		}
		--ac;
		if (i < ac)
		bcopy(av+i+1, av+i, sizeof(char *) * ac-i);
	    } else {
		if (count)
		    usage = 1;
		++i;
	    }
	}
	if (usage) {
	    fprintf(stderr, "\
\n\
Usage: listnicks [-c] [nick [nick...]]\n\
     -c: display count of registered nicks only\n\
            (cannot be combined with nicks)\n\
   nick: nickname(s) to display information for\n\
\n\
If no nicks are given, the entire nickname database is printed out in\n\
compact format followed by the number of registered nicks (with -c, the\n\
list is suppressed and only the count is printed).  If one or more nicks\n\
are given, detailed information about those nicks is displayed.\n\
\n");
	    return usage>0 ? 1 : 0;
	}
	load_ns_dbase();
	if (ac > 1) {
	    for (i = 1; i < ac; ++i)
		listnicks(0, av[i]);
	} else {
	    listnicks(count, NULL);
	}
	return 0;


    } else if (strcmp(progname, "listchans") == 0) {

	int count = 0;	/* Count only rather than display? */
	int usage = 0;	/* Display command usage?  (>0 also indicates error) */
	int i;

#ifdef RUNGROUP
	if (errmsg)
	    log("%s: %s", errmsg, RUNGROUP);
#endif

	i = 1;
	while (i < ac) {
	    if (av[i][0] == '-') {
		switch (av[i][1]) {
		case 'h':
		    usage = -1; break;
		case 'c':
		    if (i > 1)
			usage = 1;
		    count =  1; break;
		default :
		    usage =  1; break;
		}
		--ac;
		if (i < ac)
		bcopy(av+i+1, av+i, sizeof(char *) * ac-i);
	    } else {
		if (count)
		    usage = 1;
		++i;
	    }
	}
	if (usage) {
	    fprintf(stderr, "\
\n\
Usage: listchans [-c] [chan [chan...]]\n\
     -c: display count of registered channels only\n\
            (cannot be combined with channels)\n\
   chan: channel(s) to display information for\n\
\n\
If no channels are given, the entire channel database is printed out in\n\
compact format followed by the number of registered channelss (with -c, the\n\
list is suppressed and only the count is printed).  If one or more channels\n\
are given, detailed information about those channels is displayed.\n\
\n");
	    return usage>0 ? 1 : 0;
	}
	load_ns_dbase();
	load_cs_dbase();
	if (ac > 1) {
	    for (i = 1; i < ac; ++i)
		listchans(0, av[i]);
	} else {
	    listchans(count, NULL);
	}
	return 0;

    }

#endif	/* !SKELETON */


    for (i = 1; i < ac; ++i) {
	s = av[i];
	if (*s == '-') {
	    ++s;
	    if (strcmp(s, "remote") == 0) {
		if (++i >= ac) {
		    log("-remote requires hostname[:port]");
		    break;
		}
		s = av[i];
		if (t = strchr(s, ':')) {
		    *t++ = 0;
		    if (((int) strtol((  t  ), (char **) ((void *)0) , 10) )  > 0)
			remote_port = ((int) strtol((  t  ), (char **) ((void *)0) , 10) ) ;
		    else
			log("-remote: port number must be a positive integer.  Using default.");
		}
		remote_server = s;
	    } else if (strcmp(s, "name") == 0) {
		if (++i >= ac) {
		    log("-name requires a parameter");
		    break;
		}
		server_name = av[i];
	    } else if (strcmp(s, "desc") == 0) {
		if (++i >= ac) {
		    log("-desc requires a parameter");
		    break;
		}
		server_desc = av[i];
	    } else if (strcmp(s, "user") == 0) {
		if (++i >= ac) {
		    log("-user requires a parameter");
		    break;
		}
		services_user = av[i];
	    } else if (strcmp(s, "host") == 0) {
		if (++i >= ac) {
		    log("-host requires a parameter");
		    break;
		}
		services_host = av[i];
	    } else if (strcmp(s, "dir") == 0) {
		if (++i >= ac) {
		    log("-dir requires a parameter");
		    break;
		}
		/* already handled, but we needed to ++i */
	    } else if (strcmp(s, "log") == 0) {
		if (++i >= ac) {
		    log("-log requires a parameter");
		    break;
		}
		log_filename = av[i];
	    } else if (strcmp(s, "tz") == 0) {
		if (++i >= ac) {
		    log("-tz requires a parameter");
		    break;
		}
		time_zone = av[i];
	    } else if (strcmp(s, "update") == 0) {
		if (++i >= ac) {
		    log("-update requires a parameter");
		    break;
		}
		s = av[i];
		if (atoi(s) <= 0)
		    log("-update: number of seconds must be positive");
		else
		    update_timeout = (atoi(s));
	    } else if (strcmp(s, "debug") == 0) {
		debug = 1;
	    } else {
		log("Unknown option -%s", s);
	    }
	} else {
	    log("Non-option arguments not allowed");
	}
    }


    /* Write our PID to the PID file. */

    pidfile = fopen(PID_FILE, "w");
    if (pidfile) {
    	fprintf(pidfile, "%d\n", getpid());
    	fclose(pidfile);
    	atexit(remove_pidfile);
    } else {
    	log_perror("Warning: cannot write to PID file %s", PID_FILE);
    }
    

    /* Redirect stderr to logfile. */

    if (!freopen(log_filename, "a", stderr)) {
	perror(log_filename);
	return 20;
    }

#ifdef RUNGROUP
    if (errmsg)
	log("%s: %s", errmsg, RUNGROUP);
#endif


    /* Detach ourselves */

    if ((i = fork()) < 0) {
	log_perror("fork()");
	return 1;
    } else if (i != 0)
	return 0;
    if (setpgrp(0, 0) < 0) {
	log_perror("setpgrp()");
	return 1;
    }


#ifdef SKELETON
    log("Services (skeleton version) starting up");
#else
    log("Services starting up");
#endif
    start_time = time(NULL);


    /* Set signal handlers */

    signal(SIGINT, sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGPIPE, SIG_IGN);		/* We don't care about broken pipes */
    signal(SIGQUIT, sighandler);
    signal(SIGSEGV, sighandler);
    signal(SIGBUS, sighandler);
    signal(SIGQUIT, sighandler);
    signal(SIGHUP, sighandler);
    signal(SIGILL, sighandler);
    signal(SIGTRAP, sighandler);
    signal(SIGIOT, sighandler);
    signal(SIGFPE, sighandler);
    signal(SIGALRM, SIG_IGN);		/* Used by sgets() for read timeout */
    signal(SIGUSR2, SIG_IGN);
    signal(SIGCHLD, SIG_IGN);
    signal(SIGWINCH, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);

    signal(SIGUSR1, sighandler);	/* This is our "out-of-memory" panic switch */


    /* Load up databases. */

#ifndef SKELETON
    load_ns_dbase();
    load_cs_dbase();
    load_ms_dbase();
    load_news_dbase();
#endif
    load_akill();


    /* Connect to the remote server */

    servsock = conn(remote_server, remote_port);
    if (servsock < 0) {
	log_perror("Can't connect to server");
	return 20;
    }
    send_cmd(NULL, "PASS :%s", PASSWORD);
    send_cmd(NULL, "SERVER %s 1 :%s", server_name, server_desc);
    sgets2(inbuf, sizeof(inbuf), servsock);
    if (strnicmp(inbuf, "ERROR", 5) == 0) {
	log("Remote server returned: %s", inbuf);
	return 20;
    }

    /* Bring in our pseudo-clients. */
    introduce_user(NULL);


    /*** Main loop. ***/

    /* We have a line left over from earlier, so process it first. */
    process();

    /* The signal handler routine will drop back here with quitting != 0
     * if it gets called. */
    setjmp(panic_jmp);
    started = 1;

    last_update = time(NULL);

    while (!quitting) {
	time_t t = time(NULL);
	waiting = -3;
	if (save_data || t-last_update >= update_timeout) {
#if !defined(SKELETON) && !defined(READONLY)
	    /* First check for expired nicks/channels */
	    waiting = -22;
	    expire_nicks();
	    expire_chans();
#endif
	    /* Now actually save stuff */
	    waiting = -2;
#if !defined(SKELETON) && !defined(READONLY)
	    save_ns_dbase();
	    save_cs_dbase();
	    save_ms_dbase();
	    save_news_dbase();
#endif
#ifndef READONLY
	    save_akill();
#endif
	    if (save_data < 0)
		break;	/* out of main loop */

	    save_data = 0;
	    last_update = t;
	}
	waiting = -1;
#ifndef SKELETON
	/* Ew... hardcoded constant.  Somebody want to come up with a good
	 * name for this? */
	if (t-last_check >= 3)
	    check_timeouts();	/* Nick kill stuff */
#endif
	waiting = 1;
	i = (int)sgets2(inbuf, sizeof(inbuf), servsock);
	waiting = 0;
	if (i > 0) {
	    process();
	} else if (i == 0) {
	    if (quitmsg = malloc(512))
		sprintf(quitmsg, "Read error from server: %s", strerror(errno));
	    else
		quitmsg = "Read error from server";
	    quitting = 1;
	}
	waiting = -4;
    }


    /* Disconnect and exit */

    if (!quitmsg)
	quitmsg = "Terminating, reason unknown";
    log("%s", quitmsg);
    if (started)
	send_cmd(server_name, "SQUIT %s :%s", server_name, quitmsg);
    disconn(servsock);
    return 0;
}
