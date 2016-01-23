/* OperServ functions.
 *
 * Services is copyright (c) 1996-1997 Preston A. Elder.
 *     E-mail: <prez@antisocial.com>   IRC: PreZ@DarkerNet
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

/* Nick for sending global notices */
const char s_GlobalNoticer[] = "Death";

#include "services.h"

#ifdef OPERSERV

#include "os-help.c"

/* Nick for OperServ */
const char s_OperServ[] = "OperServ";

extern int mode;

#ifdef AKILL
static int nakill = 0;
static int akill_size = 0;
static struct akill {
    char *mask;
    char *reason;
    char who[NICKMAX];
    time_t time;
} *akills = NULL;
#endif

#ifdef CLONES
struct clone {
    char *host;
    long time;
};

/* List of most recent users - statically initialized to zeros */
static struct clone clonelist[CLONE_DETECT_SIZE];

/* Which hosts have we warned about, and when?  This is used to keep us
 * from sending out notices over and over for clones from the same host. */
static struct clone warnings[CLONE_DETECT_SIZE];
#endif

#ifdef AKILL
static void do_akill(const char *source, int call);
static void add_akill(const char *mask, const char *reason, const char *who, int call);
static int del_akill(const char *mask, int call);
#endif
#ifdef CLONES
static void send_clone_lists(const char *source);
static void do_set(const char *source);
#endif

/*************************************************************************/

/* Main OperServ routine. */

void operserv(const char *source, char *buf)
{
    char *cmd;
    char *s, *chan, *nick, *serv;
    int i;

    log("%s: %s: %s", s_OperServ, source, buf);
    cmd = strtok(buf, " ");

    if(mode==0 && stricmp(cmd, "ON")!=0) {
	notice(s_OperServ, source, "Sorry, Services are curently \2OFF\2.");
	return;
    }

    if (!cmd) {

	return;

    } else if (stricmp(cmd, "\1PING") == 0) {

	if (!(s = strtok(NULL, "")))
	    s = "\1";
	notice(s_OperServ, source, "\1PING %s", s);

    } else if (stricmp(cmd, "MODE") == 0) {

	Channel *c;
	User *u;
	char l[16];
	chan = strtok(NULL, " ");
	s = strtok(NULL, "");
	if (!chan)
	    return;

	if (chan[0]=='#') {
	    if (!(c = findchan(chan)))
		return;
	    else if (!s) {
		sprintf(l, " %d", c->limit);
		notice(s_OperServ, source, "%s +%s%s%s%s%s%s%s%s%s%s%s", c->name,
				(c->mode&CMODE_I) ? "i" : "",
				(c->mode&CMODE_M) ? "m" : "",
				(c->mode&CMODE_N) ? "n" : "",
				(c->mode&CMODE_P) ? "p" : "",
				(c->mode&CMODE_S) ? "s" : "",
				(c->mode&CMODE_T) ? "t" : "",
				(c->limit)        ? "l" : "",
				(c->key)          ? "k" : "",
				(c->limit)        ?  l  : "",
				(c->key)          ? " " : "",
				(c->key)          ? c->key : "");
	    } else
		send_cmd(s_OperServ, "MODE %s %s", chan, s);
	} else {
	    if (!(u = finduser(chan)))
		return;
	    else if (!s)
		notice(s_OperServ, source, "%s +%s%s%s%s%s", u->nick,
				(u->mode&UMODE_O) ? "o" : "",
				(u->mode&UMODE_I) ? "i" : "",
				(u->mode&UMODE_S) ? "s" : "",
				(u->mode&UMODE_W) ? "w" : "",
				(u->mode&UMODE_G) ? "g" : "");
#ifdef DAL_SERV
	    else if (is_services_op(source))
		send_cmd(s_OperServ, "SVSMODE %s %s", chan, s);
#endif
	}

    } else if (stricmp(cmd, "KICK") == 0) {

	chan = strtok(NULL, " ");
	nick = strtok(NULL, " ");
	s = strtok(NULL, "");
	if (!chan || !nick || !s)
	    return;
	send_cmd(s_OperServ, "KICK %s %s :%s (%s)", chan, nick, source, s);

    } else if (stricmp(cmd, "HELP") == 0) {

	cmd = strtok(NULL, " ");

	if (!cmd) {
	    notice_list(s_OperServ, source, os_help);
	    if (is_services_op(source))
		notice_list(s_OperServ, source, os_sop_help);
	    notice_list(s_OperServ, source, os_end_help);
	} else if (stricmp(cmd, "MODE") == 0)
	    notice_list(s_OperServ, source, mode_help);
	else if (stricmp(cmd, "USERLIST") == 0)
	    notice_list(s_OperServ, source, userlist_help);
	else if (stricmp(cmd, "CHANLIST") == 0)
	    notice_list(s_OperServ, source, chanlist_help);
	else if (stricmp(cmd, "CHANUSERS") == 0)
	    notice_list(s_OperServ, source, chanusers_help);
	else if (stricmp(cmd, "KICK") == 0) {
	    notice_list(s_OperServ, source, kick_help);
#ifdef AKILL
	} else if (stricmp(cmd, "AKILL") == 0) {
	    notice_list(s_OperServ, source, akill_help);
#endif
#ifdef GLOBALNOTICER
	} else if (stricmp(cmd, "GLOBAL") == 0) {
	    /* Information varies, so we need to do it manually. */
	    notice(s_OperServ, source, "Syntax: GLOBAL \37message\37");
	    notice(s_OperServ, source, "");
	    notice(s_OperServ, source,
			"Allows IRCops to send messages to all users on the");
	    notice(s_OperServ, source,
			"network.  The message will be sent from the nick");
	    notice(s_OperServ, source, "\2%s\2.", s_GlobalNoticer);
#endif
	} else if (stricmp(cmd, "STATS") == 0)
	    notice_list(s_OperServ, source, stats_help);
	else if (stricmp(cmd, "LISTSOPS") == 0)
	    notice_list(s_OperServ, source, listsops_help);
	else if (!is_services_op(source))
	    notice(s_OperServ, source,
			"No help available for command \2%s\2.", cmd);
#ifdef DAL_SERV
	else if (stricmp(cmd, "QLINE") == 0)
	    notice_list(s_OperServ, source, qline_help);
	else if (stricmp(cmd, "UNQLINE") == 0)
	    notice_list(s_OperServ, source, unqline_help);
	else if (stricmp(cmd, "NOOP") == 0)
	    notice_list(s_OperServ, source, noop_help);
	else if (stricmp(cmd, "KILL") == 0)
	    notice_list(s_OperServ, source, kill_help);
#endif
	else if (stricmp(cmd, "JUPE") == 0)
	    notice_list(s_OperServ, source, jupe_help);
#ifdef AKILL
	else if (stricmp(cmd, "PAKILL") == 0)
	    notice_list(s_OperServ, source, pakill_help);
#endif
	else if (stricmp(cmd, "UPDATE") == 0)
	    notice_list(s_OperServ, source, update_help);
	else if (stricmp(cmd, "ON") == 0)
	    notice_list(s_OperServ, source, offon_help);
	else if (stricmp(cmd, "OFF") == 0)
	    notice_list(s_OperServ, source, offon_help);
	else if (stricmp(cmd, "QUIT") == 0)
	    notice_list(s_OperServ, source, quit_help);
	else if (stricmp(cmd, "SHUTDOWN") == 0)
	    notice_list(s_OperServ, source, shutdown_help);
	else
	    notice(s_OperServ, source,
			"No help available for command \2%s\2.", cmd);

#ifdef GLOBALNOTICER
    } else if (stricmp(cmd, "GLOBAL") == 0) {

	char *msg = strtok(NULL, "");

	if (!msg) {
	    notice(s_OperServ, source, "Syntax: \2GLOBAL \37msg\37\2");
	    notice(s_OperServ, source,
			"\2/msg %s HELP GLOBAL for more information.",
			s_OperServ);
	}
	noticeall(s_GlobalNoticer, "%s", msg);
#endif /* GLOBALNOTICER */
    } else if (stricmp(cmd, "LISTSOPS") == 0) {

	    notice(s_OperServ, source, "Services OPs: \2%s\2", SERVICES_OPS);

    } else if (stricmp(cmd, "USERLIST") == 0) {

	send_user_list(source);
	
    } else if (stricmp(cmd, "CHANLIST") == 0) {

	send_channel_list(source);

    } else if (stricmp(cmd, "CHANUSERS") == 0) {

	chan = strtok(NULL, " ");
	if (chan)
	    send_channel_users(source, chan);

#ifdef AKILL
    } else if (stricmp(cmd, "AKILL") == 0) {

	do_akill(source, 0);
#endif

    } else if (stricmp(cmd, "STATS") == 0) {

	time_t uptime = time(NULL) - start_time;
	char *extra = strtok(NULL, "");

	notice(s_OperServ, source, "Current users: \2%d\2 (%d ops)",
			usercnt, opcnt);
	notice(s_OperServ, source, "Maximum users: \2%d\2", maxusercnt);
	if (uptime > 86400)
	    notice(s_OperServ, source,
	    		"Services up \2%d\2 day%s, \2%02d:%02d\2",
			uptime/86400, (uptime/86400 == 1) ? "" : "s",
			(uptime/3600) % 24, (uptime/60) % 60);
	else if (uptime > 3600)
	    notice(s_OperServ, source,
	    		"Services up \2%d hour%s, %d minute%s\2",
			uptime/3600, uptime/3600==1 ? "" : "s",
			(uptime/60) % 60, (uptime/60)%60==1 ? "" : "s");
	else
	    notice(s_OperServ, source,
	    		"Services up \2%d minute%s, %d second%s\2",
			uptime/60, uptime/60==1 ? "" : "s",
			uptime%60, uptime%60==1 ? "" : "s");

	if (extra && stricmp(extra, "ALL") == 0 && is_services_op(source)) {
	    long count, mem;
	    int i;

	    get_user_stats(&count, &mem);
	    notice(s_OperServ, source,
			"User    : \2%6d\2 records, \2%5d\2 kB",
			count, (mem+1023) / 1024);
	    get_channel_stats(&count, &mem);
	    notice(s_OperServ, source,
			"Channel : \2%6d\2 records, \2%5d\2 kB",
			count, (mem+1023) / 1024);
#ifdef NICKSERV
	    get_nickserv_stats(&count, &mem);
	    notice(s_OperServ, source,
			"NickServ: \2%6d\2 records, \2%5d\2 kB",
			count, (mem+1023) / 1024);
#endif
#ifdef CHANSERV
	    get_chanserv_stats(&count, &mem);
	    notice(s_OperServ, source,
			"ChanServ: \2%6d\2 records, \2%5d\2 kB",
			count, (mem+1023) / 1024);
#endif
#ifdef MEMOS
	    get_memoserv_stats(&count, &mem);
	    notice(s_OperServ, source,
			"MemoServ: \2%6d\2 records, \2%5d\2 kB",
			count, (mem+1023) / 1024);
#endif
#ifdef NEWS
	    get_newsserv_stats(&count, &mem);
	    notice(s_OperServ, source,
			"NewsServ: \2%6d\2 records, \2%5d\2 kB",
			count, (mem+1023) / 1024);
#endif
#ifdef CLONES
	    count = 0;
	    mem = sizeof(struct clone) * CLONE_DETECT_SIZE * 2;
	    for (i = 0; i < CLONE_DETECT_SIZE; i++) {
		if (clonelist[i].host) {
		    count++;
		    mem += strlen(clonelist[i].host)+1;
		}
		if (warnings[i].host) {
		    count++;
		    mem += strlen(warnings[i].host)+1;
		}
	    }
#endif
	    notice(s_OperServ, source,
			"OperServ: \2%6d\2 records, \2%5d\2 kB",
#ifdef AKILL
#ifdef CLONES
			nakill + count,
			(akill_size * sizeof(*akills) + mem + 1023) / 1024);
#else
			nakill, (akill_size * sizeof(*akills) + 1023) / 1024);
#endif /* CLONES */
#else
			count, (mem+1023) / 1024);
#endif /*AKILL */
	}

    /* Services ops only below this point. */

    } else if (!is_services_op(source)) {

	notice(s_OperServ, source,
		"Unrecognized command \2%s\2.  Type \"/msg %s HELP\" for help.",
		cmd, s_OperServ);

#ifdef DAL_SERV
    } else if (stricmp(cmd, "QLINE") == 0) {

	nick = strtok(NULL, " ");
	s = strtok(NULL, "");
	if(!nick)
	    return;

	if(!s) {
	    send_cmd(NULL, "SQLINE %s", nick);
	    wallops("\2%s\2 has been QUARENTINED by \2%s\2", nick, source);
	} else {
	    send_cmd(NULL, "SQLINE %s :%s", nick, s);
	    wallops("\2%s\2 has been QUARENTINED by \2%s\2 for %s", nick, source, s);
	}

    } else if (stricmp(cmd, "UNQLINE") == 0) {

	nick = strtok(NULL, " ");
	if(!nick)
	    return;

	send_cmd(NULL, "UNSQLINE %s", nick);
	wallops("\2%s\2 removed QUARENTINE for \2%s\2", source, nick);

    } else if (stricmp(cmd, "NOOP") == 0) {

	serv = strtok(NULL, " ");
	s = strtok(NULL, "");
	if (!s || (s[0]!='+' && s[0]!='-'))
	    return;

	send_cmd(s_OperServ, "SVSNOOP 1 :%s", serv, s);
	if(s[0]=='+')
	     wallops("\2%s\2 QUARENTINED OPERS on \2%s\2", source, serv);
	else
	     wallops("\2%s\2 removed QUARENTINE for OPERS on \2%s\2", source, serv);

    } else if (stricmp(cmd, "KILL") == 0) {

	nick = strtok(NULL, " ");
	s = strtok(NULL, "");
	if (!s || !finduser(nick))
	    return;

	send_cmd(s_OperServ, "SVSKILL %s :%s", nick, s);

#endif

    } else if (stricmp(cmd, "LISTIGNORE") == 0) {

	int sent_header = 0;
	IgnoreData *id;

	for (i = 0; i < 256; ++i) {
	    for (id = ignore[i]; id; id = id->next) {
		if (!sent_header) {
		    notice(s_OperServ, source, "Services ignorance list:");
		    sent_header = 1;
		}
		notice(s_OperServ, source, "%d %s", id->time, id->who);
	    }
	}
	if (!sent_header)
	    notice(s_OperServ, source, "Ignorance list is empty.");

    } else if (stricmp(cmd, "UPDATE") == 0) {

	notice(s_OperServ, source, "Updating databases.");
	save_data = 1;

#ifdef AKILL
    } else if (stricmp(cmd, "PAKILL") == 0) {

	do_akill(source, 1);
#endif

    } else if (stricmp(cmd, "SET") == 0) {

	do_set(source);

    } else if (stricmp(cmd, "JUPE") == 0) {

	char *jserver = strtok(NULL, " ");
	if (!jserver) {
	    notice(s_OperServ, source, "Syntax: \2JUPE \37servername\37\2");
	} else {
	    wallops("\2Juping\2 %s by request of \2%s\2.", jserver, source);
	    send_cmd(NULL, "SERVER %s 2 :Jupitered server", jserver);
	}

    } else if (stricmp(cmd, "OFF") == 0) {

	char *pass = strtok(NULL, " ");
	if (!pass)
	    return;
	if (stricmp(pass, SUPERPASS)==0) {
	    mode = 0;
	    notice(s_OperServ, source, "Services are now switched \2OFF\2.");
#ifdef GLOBALNOTICER
	    noticeall(s_GlobalNoticer, "Services are currently \2OFF\2 - Please do not attempt to use them!");
#endif
	} else
	    notice(s_OperServ, source, "Access Denied.");

    } else if (stricmp(cmd, "ON") == 0) {

	char *pass = strtok(NULL, " ");
	if (!pass)
	    return;
	if (stricmp(pass, SUPERPASS)==0) {
	    mode = 1;
	    notice(s_OperServ, source, "Services are now switched \2ON\2 again.");
#ifdef GLOBALNOTICER
	    noticeall(s_GlobalNoticer, "Services are back \2ON\2 again - Please use them at will.");
#endif
	} else
	    notice(s_OperServ, source, "Access Denied.");

    } else if (stricmp(cmd, "QUIT") == 0) {

	char *pass = strtok(NULL, " ");
	if (!pass)
	    return;
	if (stricmp(pass, SUPERPASS)==0) {
	    quitmsg = malloc(32 + strlen(source));
	    if (!quitmsg)
		quitmsg = "QUIT command received, but out of memory!";
	    else
		sprintf(quitmsg, "QUIT command received from %s", source);
	    quitting = 1;
	} else
	    notice(s_OperServ, source, "Access Denied.");

    } else if (stricmp(cmd, "SHUTDOWN") == 0) {

	char *pass = strtok(NULL, " ");
	if (!pass)
	    return;
	if (stricmp(pass, SUPERPASS)==0) {
	    quitmsg = malloc(32 + strlen(source));
	    save_data = 1;
	    if (!quitmsg)
		quitmsg = "SHUTDOWN command received, but out of memory!";
            else
		sprintf(quitmsg, "SHUTDOWN command received from %s", source);
	    quitting = 1;
	} else
	    notice(s_OperServ, source, "Access Denied.");

    } else if (stricmp(cmd, "RAW") == 0) {

	char *pass = strtok(NULL, " ");
	char *text = strtok(NULL, "");
	if (!pass)
	    return;
	if (stricmp(pass, SUPERPASS)==0) {
	    if (!text)
		notice(s_OperServ, source, "Syntax: \2RAW \37password command\37\2");
	    else
		send_cmd(NULL, text);
	} else
	    notice(s_OperServ, source, "Access Denied.");

    } else {

	notice(s_OperServ, source,
		"Unrecognized command \2%s\2.  Type \"/msg %s HELP\" for help.",
		cmd, s_OperServ);

    }
}

/*************************************************************************/

/* Is the given nick a Services op? */

int is_services_op(const char *nick)
{
#ifdef NICKSERV
    NickInfo *ni;
#endif
    char tmp[NICKMAX+2];

    strscpy(tmp+1, nick, NICKMAX);
    tmp[0] = ' ';
    tmp[strlen(tmp)+1] = 0;
    tmp[strlen(tmp)] = ' ';
    if (stristr(" " SERVICES_OPS " ", tmp) == NULL)
	return 0;
#ifndef NICKSERV
    return 1;
#else
    if ((ni = findnick(nick)) && (ni->flags & NI_IDENTIFIED) && is_oper(nick))
	return 1;
    return 0;
#endif
}

/*************************************************************************/
/****************************** AKILL stuff ******************************/
/*************************************************************************/

#ifdef AKILL
void load_akill()
{
    FILE *f = fopen(AKILL_DB, "r");
    int i;

    if (!f) {
	log_perror("Can't read AKILL database " AKILL_DB);
	return;
    }
    switch (i = get_file_version(f, AKILL_DB)) {
      case 4:
      case 3:
      case 2:
	nakill = fgetc(f) * 256 + fgetc(f);
	if (nakill < 8)
	    akill_size = 16;
	else
	    akill_size = 2*nakill;
	akills = smalloc(sizeof(*akills) * akill_size);
	if (!nakill) {
	    fclose(f);
	    return;
	}
	if (nakill != fread(akills, sizeof(*akills), nakill, f))
	    fatal_perror("Read error on %s", AKILL_DB);
	for (i = 0; i < nakill; ++i) {
	    akills[i].mask = read_string(f, AKILL_DB);
	    akills[i].reason = read_string(f, AKILL_DB);
	}
	break;

      case 1: {
	struct {
	    char *mask;
	    char *reason;
	    time_t time;
	} old_akill;
	nakill = fgetc(f) * 256 + fgetc(f);
	if (nakill < 8)
	    akill_size = 16;
	else
	    akill_size = 2*nakill;
	akills = smalloc(sizeof(*akills) * akill_size);
	if (!nakill) {
	    fclose(f);
	    return;
	}
	for (i = 0; i < nakill; ++i) {
	    if (1 !=fread(&old_akill, sizeof(old_akill), 1, f))
		fatal_perror("Read error on %s", AKILL_DB);
	    akills[i].time = old_akill.time;
	    akills[i].who[0] = 0;
	}
	for (i = 0; i < nakill; ++i) {
	    akills[i].mask = read_string(f, AKILL_DB);
	    akills[i].reason = read_string(f, AKILL_DB);
	}
	break;
      } /* case 1 */

      default:
	fatal("Unsupported version (%d) on %s", i, AKILL_DB);
    } /* switch (version) */
    fclose(f);
}

/*************************************************************************/

void save_akill()
{
    FILE *f;
    int i;

    remove(AKILL_DB ".save");
    if (rename(AKILL_DB, AKILL_DB ".save") < 0)
	fatal_perror("Can't back up %s", AKILL_DB);
    f = fopen(AKILL_DB, "w");
    if (!f) {
	log_perror("Can't write to AKILL database %s", AKILL_DB);
	if (rename(AKILL_DB ".save", AKILL_DB) < 0)
	    fatal_perror("Can't restore backup copy of %s", AKILL_DB);
	return;
    }
    if (fchown(fileno(f), -1, file_gid) < 0)
	log_perror("Can't change group of %s to %d", AKILL_DB, file_gid);
    write_file_version(f, AKILL_DB);

    fputc(nakill/256, f); fputc(nakill & 255, f);
    if (fwrite(akills, sizeof(*akills), nakill, f) != nakill)
	fatal_perror("Write error on %s", AKILL_DB);
    for (i = 0; i < nakill; ++i) {
	write_string(akills[i].mask, f, AKILL_DB);
	write_string(akills[i].reason, f, AKILL_DB);
    }
    fclose(f);
    remove(AKILL_DB ".save");
}

/*************************************************************************/

/* Handle an AKILL command. */

static void do_akill(const char *source, int call)
{
    char *cmd, *mask, *reason, *who, *s;
    int i;

    cmd = strtok(NULL, " ");
    if (!cmd)
	cmd = "";

    if (stricmp(cmd, "ADD") == 0) {
	if ((mask = strtok(NULL, " ")) && (reason = strtok(NULL, ""))) {
	    if (s = strchr(mask, '@')) {
		strlower(s);
	    } else {
		notice(s_OperServ, source, "Hostmask must contain an `@'
			character.");
		return;
	    }
		/* Find @*, @*.*, @*.*.*, etc. and dissalow if !SOP */
	    for(i=strlen(mask)-1;mask[i]=='*' || mask[i]=='?' || mask[i]=='.' ;i--) ;
	    if(mask[i]=='@' && !is_services_op(source))
		notice(s_OperServ, source, "@* AKILL's are not allowed!!");
	    else if(strlen(mask)<8)
		notice(s_OperServ, source, "AKILL mask too short!");
	    else {
		add_akill(mask, reason, source, call);
		notice(s_OperServ, source, "%s added to AKILL list.", mask);
	    }
	} else {
	    notice(s_OperServ, source,
			"Syntax: AKILL ADD \37mask\37 \37reason\37");
	}
#ifdef READONLY
	notice(s_OperServ, source,
		"\2Notice:\2 Changes will not be saved!  Services is in read-only mode.");
#endif

    } else if (stricmp(cmd, "DEL") == 0) {
	if (mask = strtok(NULL, " ")) {
	    if (s = strchr(mask, '@'))
		strlower(s);
	    if (del_akill(mask, call)) {
		notice(s_OperServ, source, "%s removed from AKILL list.", mask);
		if (s) {
		    *s++ = 0;
		    send_cmd(server_name, "RAKILL %s %s", s, mask);
		} else {
		    /* We lose... can't figure out what's a username and what's
		     * a hostname.  Ah well.
		     */
		}
	    } else {
		if(call==1)
		    notice(s_OperServ, source, "%s not found on AKILL list.", mask);
		else
		    notice(s_OperServ, source, "%s not found on AKILL list or is PERMINANT.", mask);
	    }
	} else {
	    notice(s_OperServ, source, "Syntax: AKILL DEL \37mask\37");
	}
#ifdef READONLY
	notice(s_OperServ, source,
		"\2Notice:\2 Changes will not be saved!  Services is in read-only mode.");
#endif

    } else if (stricmp(cmd, "LIST") == 0) {
	s = strtok(NULL, " ");
	if (!s)
	    s = "*";
	if (strchr(s, '@'))
	    strlower(strchr(s, '@'));
	notice(s_OperServ, source, "Current AKILL list:");
	for (i = 0; i < nakill; ++i) {
	    if (!s || match_wild(s, akills[i].mask)) {
		notice(s_OperServ, source, "%-32s  %s",
					akills[i].mask, akills[i].reason);
	    }
	}

    } else if (stricmp(cmd, "VIEW") == 0) {
	s = strtok(NULL, " ");
	if (!s)
	    s = "*";
	if (strchr(s, '@'))
	    strlower(strchr(s, '@'));
	notice(s_OperServ, source, "Current AKILL list:");
	for (i = 0; i < nakill; ++i) {
	    if (!s || match_wild(s, akills[i].mask)) {
	    	char timebuf[32];
	    	time_t t;
	    	struct tm tm;

	    	time(&t);
	    	tm = *localtime(&t);
	    	strftime(timebuf, sizeof(timebuf), "%d %b %Y %H:%M:%S %Z", &tm);
		notice(s_OperServ, source, "%s (by %s %s %s)",
				akills[i].mask,
				*akills[i].who ? akills[i].who : "<unknown>",
				akills[i].time ? "on" : "is",
				akills[i].time ? timebuf : "PERMINANT");
		notice(s_OperServ, source, "    %s", akills[i].reason);
	    }
	}

    } else {
	notice(s_OperServ, source,
		"Syntax: \2AKILL {ADD|DEL|LIST} [\37mask\37]\2");
	notice(s_OperServ, source,
		"For help: \2/msg %s HELP AKILL\2", s_OperServ);
    }
}

void expire_akill () {
    int i;
    const time_t expire_time = AKILL_EXPIRE*24*60*60;
    time_t tm;
    for (i = 0; i < nakill; ++i) {
	if (akills[i].time > 0) {
	    tm = time(NULL) - akills[i].time;
	    if (tm > expire_time) {
		free(akills[i].mask);
		free(akills[i].reason);
		--nakill;
		if (i < nakill)
		    bcopy(akills+i+1, akills+i, sizeof(*akills) * (nakill-i));
	    }
	}
    }
}

/*************************************************************************/

/* Does the user match any AKILLs? */

int check_akill(const char *nick, const char *username, const char *host)
{
    char buf[512];
    int i;

    strscpy(buf, username, sizeof(buf)-2);
    i = strlen(buf);
    buf[i++] = '@';
    strlower(strscpy(buf+i, host, sizeof(buf)-i));
    for (i = 0; i < nakill; ++i) {
	if (match_wild(akills[i].mask, buf)) {
	    char *av[2], nickbuf[NICKMAX];
	    send_cmd(s_OperServ,
			"KILL %s :%s (You are banned from this network)",
			nick, s_OperServ);
	    send_cmd(server_name,
			"AKILL %s %s :You are banned from this network",
			host, username);
	    av[0] = strscpy(nickbuf, nick, NICKMAX);
	    av[1] = "autokill";
	    do_kill(s_OperServ, 2, av);
	    return 1;
	}
    }
    return 0;
}

/*************************************************************************/

static void add_akill(const char *mask, const char *reason, const char *who, int call)
{
    if (nakill >= akill_size) {
	if (akill_size < 8)
	    akill_size = 8;
	else
	    akill_size *= 2;
	akills = srealloc(akills, sizeof(*akills) * akill_size);
    }
    akills[nakill].mask = sstrdup(mask);
    akills[nakill].reason = sstrdup(reason);
    if (call==1)
	akills[nakill].time = 0;
    else
	akills[nakill].time = time(NULL);
    strscpy(akills[nakill].who, who, NICKMAX);
    ++nakill;
}

/*************************************************************************/

/* Return whether the mask was found in the AKILL list. */

static int del_akill(const char *mask, int call)
{
    int i;

    for (i = 0; i < nakill && strcmp(akills[i].mask, mask) != 0; ++i)
	;
    if (i < nakill) {
	if (akills[i].time==0 && call!=1)
	    return 0;
	free(akills[i].mask);
	free(akills[i].reason);
	--nakill;
	if (i < nakill)
	    bcopy(akills+i+1, akills+i, sizeof(*akills) * (nakill-i));
	return 1;
    } else {
	return 0;
    }
}
#endif /* AKILL */

/*************************************************************************/
/**************************** Clone detection ****************************/
/*************************************************************************/

/* We just got a new user; does it look like a clone?  If so, send out a
 * GOPER to warn all IRCops.
 *
 * Note: Actually sends a GLOBOPS - GOPER isn't implemented in ircd.dal(?)
 */
#ifdef CLONES

void check_clones(User *user)
{
    int i, clone_count;
    long last_time;

#ifndef FIXED_CLONES
    return;
#endif
    if (clonelist[0].host)
	free(clonelist[0].host);
    i = CLONE_DETECT_SIZE-1;
    memmove(clonelist, clonelist+1, sizeof(struct clone) * i);
    clonelist[i].host = sstrdup(user->host);
    last_time = clonelist[i].time = time(NULL);
    clone_count = 1;
    while (--i >= 0 && clonelist[i].host) {
	if (clonelist[i].time < last_time - CLONE_MAX_DELAY)
	    break;
	if (stricmp(clonelist[i].host, user->host) == 0) {
	    ++clone_count;
	    last_time = clonelist[i].time;
	    if (clone_count >= CLONE_MIN_USERS)
		break;
	}
    }
    if (clone_count >= CLONE_MIN_USERS) {
	/* Okay, we have clones.  Check first to see if we already know
	 * about them. */
	for (i = CLONE_DETECT_SIZE-1; i >= 0; --i) {
	    if (stricmp(warnings[i].host, user->host) == 0)
		break;
	}
	if (i < 0 || warnings[i].time < user->signon - CLONE_WARNING_DELAY) {
	    /* Send out the warning, and note it. */
	    send_cmd(s_OperServ,
		"GLOBOPS :\2WARNING\2 - possible clones detected from %s",
		user->host);
	    i = CLONE_DETECT_SIZE-1;
	    if (warnings[0].host)
		free(warnings[0].host);
	    bcopy(warnings+1, warnings, sizeof(struct clone) * i);
	    warnings[i].host = sstrdup(user->host);
	    warnings[i].time = clonelist[i].time;
	}
    }
}


/* Debug: send clone arrays to given nick. */

static void send_clone_lists(const char *source)
{
    int i;

    notice(s_OperServ, source, "clonelist[]");
    for (i = 0; i < CLONE_DETECT_SIZE; ++i) {
	if (clonelist[i].host)
	    notice(s_OperServ, source, "    %10ld  %s", clonelist[i].time, clonelist[i].host ? clonelist[i].host : "(null)");
    }
    notice(s_OperServ, source, "warnings[]");
    for (i = 0; i < CLONE_DETECT_SIZE; ++i) {
	if (clonelist[i].host)
	    notice(s_OperServ, source, "    %10ld  %s", warnings[i].time, warnings[i].host ? warnings[i].host : "(null)");
    }
}
#endif /* CLONES */

/*************************************************************************/
/*************************************************************************/

/* Set various Services runtime options. */

static void do_set(const char *source)
{
    char *option = strtok(NULL, " ");
    char *setting = strtok(NULL, " ");

    if (!option || !setting) {
	notice(s_OperServ, source,
			"Syntax: \2SET \37option\37 \37setting\37\2");
    } else if (stricmp(option, "IGNORE") == 0) {
	if (stricmp(setting, "on") == 0) {
	    allow_ignore = 1;
	    notice(s_OperServ, source, "Ignore code \2will\2 be used.");
	} else if (stricmp(setting, "off") == 0) {
	    allow_ignore = 0;
	    notice(s_OperServ, source, "Ignore code \2will not\2 be used.");
	} else {
	    notice(s_OperServ, source,
			"Setting for \2IGNORE\2 must be \2ON\2 or \2OFF\2.");
	}
    } else {
	notice(s_OperServ, source, "Unknown option \2%s\2.", option);
    }
}

/*************************************************************************/
#endif /* OPERSERV */
