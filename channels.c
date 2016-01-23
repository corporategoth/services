/* Channel-handling routines.
 *
 * Services is copyright (c) 1996-1997 Preston A. Elder.
 *     E-mail: <prez@antisocial.com>   IRC: PreZ@DarkerNet
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

#include "services.h"

static Channel *chanlist = NULL;

/*************************************************************************/

/* Return statistics.  Pointers are assumed to be valid. */

void get_channel_stats(long *nrec, long *memuse)
{
    long count = 0, mem = 0;
    Channel *chan;
    struct c_userlist *cu;
    int i;

    for (chan = chanlist; chan; chan = chan->next) {
	count++;
	mem += sizeof(*chan);
	if (chan->topic)
	    mem += strlen(chan->topic)+1;
	if (chan->key)
	    mem += strlen(chan->key)+1;
	mem += sizeof(char *) * chan->bansize;
	for (i = 0; i < chan->bancount; i++) {
	    if (chan->bans[i])
		mem += strlen(chan->bans[i])+1;
	}
	for (cu = chan->users; cu; cu = cu->next)
	    mem += sizeof(*cu);
	for (cu = chan->chanops; cu; cu = cu->next)
	    mem += sizeof(*cu);
	for (cu = chan->voices; cu; cu = cu->next)
	    mem += sizeof(*cu);
    }
    *nrec = count;
    *memuse = mem;
}

/*************************************************************************/

/* Send the current list of channels to the named user. */

void send_channel_list(const char *user)
{
    Channel *c = chanlist;
    char s[16], buf[512], *end;
    struct c_userlist *u, *u2;
    int isop, isvoice;

    while (c) {
	sprintf(s, " %d", c->limit);
	notice(s_OperServ, user, "%s %lu +%s%s%s%s%s%s%s%s%s%s%s %s", c->name,
				c->creation_time,
				(c->mode&CMODE_I) ? "i" : "",
				(c->mode&CMODE_M) ? "m" : "",
				(c->mode&CMODE_N) ? "n" : "",
				(c->mode&CMODE_P) ? "p" : "",
				(c->mode&CMODE_S) ? "s" : "",
				(c->mode&CMODE_T) ? "t" : "",
				(c->limit)        ? "l" : "",
				(c->key)          ? "k" : "",
				(c->limit)        ?  s  : "",
				(c->key)          ? " " : "",
				(c->key)          ? c->key : "",
				c->topic ? c->topic : "");
	end = buf;
	end += sprintf(end, "%s", c->name);
	for (u = c->users; u; u = u->next) {
	    isop = isvoice = 0;
	    for (u2 = c->chanops; u2; u2 = u2->next) {
		if (u2->user == u->user) {
		    isop = 1; break;
		}
	    }
	    for (u2 = c->voices; u2; u2 = u2->next) {
		if (u2->user == u->user) {
		    isvoice = 1; break;
		}
	    }
	    end += sprintf(end, " %s%s%s", isvoice ? "+" : "",
					isop ? "@" : "", u->user->nick);
	}
	notice(s_OperServ, user, buf);
	c = c->next;
    }
}

/* Send list of users on a single channel. */

void send_channel_users(const char *user, const char *chan)
{
    Channel *c = findchan(chan);
    struct c_userlist *u;

    if (!c) {
	notice(s_OperServ, user, "Channel %s not found!",
		chan ? chan : "(null)");
	return;
    }
    notice(s_OperServ, user, "Channel %s users:", chan);
    for (u = c->users; u; u = u->next)
	notice(s_OperServ, user, "%s", u->user->nick);
    notice(s_OperServ, user, "Channel %s chanops:", chan);
    for (u = c->chanops; u; u = u->next)
	notice(s_OperServ, user, "%s", u->user->nick);
    notice(s_OperServ, user, "Channel %s voices:", chan);
    for (u = c->voices; u; u = u->next)
	notice(s_OperServ, user, "%s", u->user->nick);
}

/*************************************************************************/

/* Return the Channel structure corresponding to the named channel, or NULL
 * if the channel was not found. */

Channel *findchan(const char *chan)
{
    Channel *c = chanlist;

    while (c) {
	if (stricmp(c->name, chan) == 0)
	    return c;
	c = c->next;
    }
    return NULL;
}

/*************************************************************************/

/* Add/remove a user to/from a channel, creating or deleting the channel as
 * necessary. */

void chan_adduser(User *user, const char *chan)
{
    Channel *c = findchan(chan);
    int newchan = !c;
    struct c_userlist *u;

    if (newchan) {
	/* Allocate pre-cleared memory */
	c = scalloc(sizeof(Channel), 1);
	c->next = chanlist;
	if (chanlist)
	    chanlist->prev = c;
	chanlist = c;
	strscpy(c->name, chan, sizeof(c->name));
	c->creation_time = time(NULL);
#ifndef SKELETON
	check_modes(chan);
	restore_topic(chan);
#endif
    }
#ifndef SKELETON
    if (check_should_op(user, chan)) {
	u = smalloc(sizeof(struct c_userlist));
	u->next = c->chanops;
	u->prev = NULL;
	if (c->chanops)
	    c->chanops->prev = u;
	c->chanops = u;
	u->user = user;
    }
#endif
    u = smalloc(sizeof(struct c_userlist));
    u->next = c->users;
    u->prev = NULL;
    if (c->users)
	c->users->prev = u;
    c->users = u;
    u->user = user;
}

void chan_deluser(User *user, Channel *c)
{
    struct c_userlist *u;

    for (u = c->users; u && u->user != user; u = u->next)
	;
    if (!u)
	return;
    if (u->next)
	u->next->prev = u->prev;
    if (u->prev)
	u->prev->next = u->next;
    else
	c->users = u->next;
    free(u);
    for (u = c->chanops; u && u->user != user; u = u->next)
	;
    if (u) {
	if (u->next)
	    u->next->prev = u->prev;
	if (u->prev)
	    u->prev->next = u->next;
	else
	    c->chanops = u->next;
	free(u);
    }
    for (u = c->voices; u && u->user != user; u = u->next)
	;
    if (u) {
	if (u->next)
	    u->next->prev = u->prev;
	if (u->prev)
	    u->prev->next = u->next;
	else
	    c->voices = u->next;
	free(u);
    }
    if (!c->users) {
	if (c->topic)
	    free(c->topic);
	if (c->key)
	    free(c->key);
	if (c->bancount) {
	    int i;
	    for (i = 0; i < c->bancount; ++i)
		free(c->bans[i]);
	}
	if (c->bansize)
	    free(c->bans);
	if (c->chanops || c->voices)
	    log("channel: Memory leak freeing %s: %s%s%s %s non-NULL!",
			c->name,
			c->chanops ? "c->chanops" : "",
			c->chanops && c->voices ? " and " : "",
			c->voices ? "c->voices" : "",
			c->chanops && c->voices ? "are" : "is");
	if (c->next)
	    c->next->prev = c->prev;
	if (c->prev)
	    c->prev->next = c->next;
	else
	    chanlist = c->next;
	free(c);
    }
}

/*************************************************************************/

/* Handle a channel MODE command. */

void do_cmode(const char *source, int ac, char **av)
{
    Channel *chan;
    struct c_userlist *u;
    User *user;
    char *s, *nick;
    int add = 1;		/* 1 if adding modes, 0 if deleting */
    char *modestr = av[1];

    chan = findchan(av[0]);
    if (!chan) {
	log("channel: MODE %s for nonexistent channel %s",
					merge_args(ac-1, av+1), av[0]);
	return;
    }
    s = modestr;
    ac -= 2; av += 2;

    while (*s) {

	switch (*s++) {

	case '+':
	    add = 1; break;

	case '-':
	    add = 0; break;

	case 'i':
	    if (add)
		chan->mode |= CMODE_I;
	    else
		chan->mode &= ~CMODE_I;
	    break;

	case 'm':
	    if (add)
		chan->mode |= CMODE_M;
	    else
		chan->mode &= ~CMODE_M;
	    break;

	case 'n':
	    if (add)
		chan->mode |= CMODE_N;
	    else
		chan->mode &= ~CMODE_N;
	    break;

	case 'p':
	    if (add)
		chan->mode |= CMODE_P;
	    else
		chan->mode &= ~CMODE_P;
	    break;

	case 's':
	    if (add)
		chan->mode |= CMODE_S;
	    else
		chan->mode &= ~CMODE_S;
	    break;

	case 't':
	    if (add)
		chan->mode |= CMODE_T;
	    else
		chan->mode &= ~CMODE_T;
	    break;

	case 'k':
	    if (--ac < 0) {
		log("channel: MODE %s %s: missing parameter for %ck",
					chan->name, modestr, add ? '+' : '-');
		break;
	    }
	    if (chan->key) {
		free(chan->key);
		chan->key = NULL;
	    }
	    if (add)
		chan->key = sstrdup(*av++);
	    break;

	case 'l':
	    if (add) {
		if (--ac < 0) {
		    log("channel: MODE %s %s: missing parameter for +l",
							chan->name, modestr);
		    break;
		}
		chan->limit = atoi(*av++);
	    } else {
		chan->limit = 0;
	    }
	    break;

	case 'b':
	    if (--ac < 0) {
		log("channel: MODE %s %s: missing parameter for %cb",
					chan->name, modestr, add ? '+' : '-');
		break;
	    }
	    if (add) {
		if (chan->bancount >= chan->bansize) {
		    chan->bansize += 8;
		    chan->bans = srealloc(chan->bans,
					sizeof(char *) * chan->bansize);
		}
		chan->bans[chan->bancount++] = sstrdup(*av++);
	    } else {
		char **s = chan->bans;
		int i = 0;
		while (i < chan->bancount && strcmp(*s, *av) != 0) {
		    ++i; ++s;
		}
		if (i < chan->bancount) {
		    --chan->bancount;
		    if (i < chan->bancount)
			bcopy(s+1, s, sizeof(char *) * (chan->bancount-i));
		}
		++av;
	    }
	    break;

	case 'o':
	    if (--ac < 0) {
		log("channel: MODE %s %s: missing parameter for %co",
					chan->name, modestr, add ? '+' : '-');
		break;
	    }
	    nick = *av++;
	    if (add) {
		for (u = chan->chanops; u && stricmp(u->user->nick, nick) != 0;
								u = u->next)
		    ;
		if (u)
		    break;
		user = finduser(nick);
		if (!user) {
		    log("channel: MODE %s +o for nonexistent user %s",
							chan->name, nick);
		    break;
		}
		if (debug)
		    log("debug: Setting +o on %s for %s", chan->name, nick);
#ifndef SKELETON
		if (!check_valid_op(user, chan->name, !!strchr(source, '.')))
		    break;
#endif
		u = smalloc(sizeof(*u));
		u->next = chan->chanops;
		u->prev = NULL;
		if (chan->chanops)
		    chan->chanops->prev = u;
		chan->chanops = u;
		u->user = user;
	    } else {
		for (u = chan->chanops; u && stricmp(u->user->nick, nick);
								u = u->next)
		    ;
		if (!u)
		    break;
		if (u->next)
		    u->next->prev = u->prev;
		if (u->prev)
		    u->prev->next = u->next;
		else
		    chan->chanops = u->next;
		free(u);
	    }
	    break;

	case 'v':
	    if (--ac < 0) {
		log("channel: MODE %s %s: missing parameter for %cv",
					chan->name, modestr, add ? '+' : '-');
		break;
	    }
	    nick = *av++;
	    if (add) {
		for (u = chan->voices; u && stricmp(u->user->nick, nick);
								u = u->next)
		    ;
		if (u)
		    break;
		user = finduser(nick);
		if (!user) {
		    log("channe: MODE %s +v for nonexistent user %s",
							chan->name, nick);
		    break;
		}
		if (debug)
		    log("debug: Setting +v on %s for %s", chan->name, nick);
		u = smalloc(sizeof(*u));
		u->next = chan->voices;
		u->prev = NULL;
		if (chan->voices)
		    chan->voices->prev = u;
		chan->voices = u;
		u->user = user;
	    } else {
		for (u = chan->voices; u && stricmp(u->user->nick, nick);
								u = u->next)
		    ;
		if (!u)
		    break;
		if (u->next)
		    u->next->prev = u->prev;
		if (u->prev)
		    u->prev->next = u->next;
		else
		    chan->voices = u->next;
		free(u);
	    }
	    break;

	} /* switch */

    } /* while (*s) */

#ifndef SKELETON
    /* Check modes against ChanServ mode lock */
    check_modes(chan->name);
#endif
}

/*************************************************************************/

/* Handle a TOPIC command. */

void do_topic(const char *source, int ac, char **av)
{
    Channel *c = findchan(av[0]);

    if (!c) {
	log("channel: TOPIC %s for nonexistent channel %s",
						merge_args(ac-1, av+1), av[0]);
	return;
    }
#ifndef SKELETON
    if (check_topiclock(av[0]))
	return;
#endif
    strscpy(c->topic_setter, av[1], sizeof(c->topic_setter));
    c->topic_time = atol(av[2]);
    if (c->topic) {
	free(c->topic);
	c->topic = NULL;
    }
    if (ac > 3 && *av[3])
	c->topic = sstrdup(av[3]);
#ifndef SKELETON
    record_topic(av[0]);
#endif
}

/*************************************************************************/
