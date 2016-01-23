/* ChanServ functions.
 *
 * Services is copyright (c) 1996-1997 Preston A. Elder.
 *     E-mail: <prez@antisocial.com>   IRC: PreZ@DarkerNet
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

#include "services.h"

/* These 3 no longer static, as they are needed in memoserv.c for the
 * news functions.
 */
ChannelInfo *chanlists[256];
ChannelInfo *cs_findchan(const char *chan);
int get_access(User *user, ChannelInfo *ci);

static def_access[] = { 0, 5, 10, 15, 20, 25, -1 };

const char s_ChanServ[] = "ChanServ";

#include "cs-help.c"

static void alpha_insert_chan(ChannelInfo *ci);
static ChannelInfo *makechan(const char *chan);
static int delchan(ChannelInfo *ci);
static int is_founder(User *user, NickInfo *ni, ChannelInfo *ci);
static int is_identified(User *user, ChannelInfo *ci);

static void do_help(const char *source);
static void do_register(const char *source);
static void do_identify(const char *source);
static void do_drop(const char *source);
static void do_set(const char *source);
static void do_set_founder(User *u, ChannelInfo *ci, char *param);
static void do_set_password(User *u, ChannelInfo *ci, char *param);
static void do_set_desc(User *u, ChannelInfo *ci, char *param);
static void do_set_topic(User *u, ChannelInfo *ci, char *param);
#if FILE_VERSION > 2
    static void do_set_url(User *u, ChannelInfo *ci, char *param);
#endif
static void do_set_mlock(User *u, ChannelInfo *ci, char *param);
static void do_set_keeptopic(User *u, ChannelInfo *ci, char *param);
static void do_set_topiclock(User *u, ChannelInfo *ci, char *param);
static void do_set_private(User *u, ChannelInfo *ci, char *param);
static void do_set_secureops(User *u, ChannelInfo *ci, char *param);
static void do_set_restricted(User *u, ChannelInfo *ci, char *param);
static void do_set_secure(User *u, ChannelInfo *ci, char *param);
static void do_access(const char *source);
static void do_akick(const char *source);
static void do_info(const char *source);
static void do_list(const char *source);
static void do_invite(const char *source);
static void do_op(const char *source);
static void do_deop(const char *source);
static void do_unban(const char *source);
static void do_clear(const char *source);
static void do_getpass(const char *source);
static void do_forbid(const char *source);


/*************************************************************************/
/*************************************************************************/

#ifndef SKELETON

/* Display total number of registered channels and info about each; or, if
 * a specific channel is given, display information about that channel
 * (like /msg ChanServ INFO <channel>).  If count_only != 0, then only
 * display the number of registered channels (the channel parameter is
 * ignored).
 */

void listchans(int count_only, const char *chan)
{
    long count = 0;
    ChannelInfo *ci;
    int i;

    if (count_only) {

	for (i = 33; i < 256; ++i)
	    for (ci = chanlists[i]; ci; ci = ci->next)
		++count;
	printf("%d channels registered.\n", count);

    } else if (chan) {

	struct tm tm;
	NickInfo *ni;
	char *t;

	if (!(ci = cs_findchan(chan))) {
	    printf("Channel %s not registered.\n", chan);
	    return;
	}
	if (ni = findnick(ci->founder))
	    t = ni->last_usermask;
	else
	    t = NULL;
	if (ci->flags & CI_VERBOTEN) {
	    printf("Channel %s is FORBIDden.\n");
	} else {
	    printf("Information about channel %s:\n", ci->name);
	    printf("        Founder: %s%s%s%s\n",
			ci->founder, t ? " (" : "", t ? t : "", t ? ")" : "");
	    printf("    Description: %s\n", ci->desc);
	    tm = *localtime(&ci->time_registered);
	    printf("     Registered: %s %2d %02d:%02d:%02d %d\n",
			month_name(tm.tm_mon+1), tm.tm_mday, tm.tm_hour,
			tm.tm_min, tm.tm_sec, tm.tm_year+1900);
	    tm = *localtime(&ci->last_used);
	    printf("      Last used: %s %2d %02d:%02d:%02d %d\n",
			month_name(tm.tm_mon+1), tm.tm_mday, tm.tm_hour,
			tm.tm_min, tm.tm_sec, tm.tm_year+1900);
	    if (ci->last_topic) {
		printf("     Last topic: %s\n", ci->last_topic);
		printf("   Topic set by: %s\n", ci->last_topic_setter);
	    }
	    printf("        Options: ");
	    if (!ci->flags) {
		printf("None\n");
	    } else {
		int need_comma = 0;
		static const char commastr[] = ", ";
		if (ci->flags & CI_PRIVATE) {
		    printf("Private");
		    need_comma = 1;
		}
		if (ci->flags & CI_KEEPTOPIC) {
		    printf("%sTopic Retention", need_comma ? commastr : "");
		    need_comma = 1;
		}
		if (ci->flags & CI_TOPICLOCK) {
		    printf("%sTopic Lock", need_comma ? commastr : "");
		    need_comma = 1;
		}
		if (ci->flags & CI_SECUREOPS) {
		    printf("%sSecure Ops", need_comma ? commastr : "");
		    need_comma = 1;
		}
		if (ci->flags & CI_RESTRICTED) {
		    printf("%sRestricted Access", need_comma ? commastr : "");
		    need_comma = 1;
		}
		if (ci->flags & CI_SECURE) {
		    printf("%sSecure", need_comma ? commastr : "");
		    need_comma = 1;
		}
	    }
	    printf("      Mode lock: ");
	    if (ci->mlock_on || ci->mlock_key || ci->mlock_limit) {
		printf("+%s%s%s%s%s%s%s%s",
			(ci->mlock_on & CMODE_I) ? "i" : "",
			(ci->mlock_key         ) ? "k" : "",
			(ci->mlock_limit       ) ? "l" : "",
			(ci->mlock_on & CMODE_M) ? "m" : "",
			(ci->mlock_on & CMODE_N) ? "n" : "",
			(ci->mlock_on & CMODE_P) ? "p" : "",
			(ci->mlock_on & CMODE_S) ? "s" : "",
			(ci->mlock_on & CMODE_T) ? "t" : "");
	     }
	    if (ci->mlock_off)
		printf("-%s%s%s%s%s%s%s%s",
			(ci->mlock_off & CMODE_I) ? "i" : "",
			(ci->mlock_off & CMODE_K) ? "k" : "",
			(ci->mlock_off & CMODE_L) ? "l" : "",
			(ci->mlock_off & CMODE_M) ? "m" : "",
			(ci->mlock_off & CMODE_N) ? "n" : "",
			(ci->mlock_off & CMODE_P) ? "p" : "",
			(ci->mlock_off & CMODE_S) ? "s" : "",
			(ci->mlock_off & CMODE_T) ? "t" : "");
	    if (ci->mlock_key)
		printf(" %s", ci->mlock_key);
	    if (ci->mlock_limit)
		printf(" %ld", ci->mlock_limit);
	    printf("\n");
	}

    } else {

	for (i = 33; i < 256; ++i) {
	    for (ci = chanlists[i]; ci; ci = ci->next) {
		printf("%-20s  %s\n", ci->name,
			ci->flags & CI_VERBOTEN ? "Disallowed (FORBID)"
			                        : ci->desc);
		++count;
	    }
	}
	printf("%d channels registered.\n", count);

    }
}

#endif	/* SKELETON */

/*************************************************************************/

/* Return information on memory use.  Assumes pointers are valid. */

void get_chanserv_stats(long *nrec, long *memuse)
{
    long count = 0, mem = 0;
    int i, j;
    ChannelInfo *ci;

    for (i = 0; i < 256; i++) {
	for (ci = chanlists[i]; ci; ci = ci->next) {
	    count++;
	    mem += sizeof(*ci);
	    if (ci->desc)
		mem += strlen(ci->desc)+1;
	    mem += ci->accesscount * sizeof(ChanAccess);
	    mem += ci->akickcount * sizeof(AutoKick);
	    for (j = 0; j < ci->akickcount; j++) {
		if (ci->akick[j].name)
		    mem += strlen(ci->akick[j].name)+1;
		if (ci->akick[j].reason)
		    mem += strlen(ci->akick[j].reason)+1;
	    }
	    if (ci->mlock_key)
		mem += strlen(ci->mlock_key)+1;
	    if (ci->last_topic)
		mem += strlen(ci->last_topic)+1;
	    if (ci->cmd_access)
		mem += sizeof(*ci->cmd_access) * CA_SIZE;
	}
    }
    *nrec = count;
    *memuse = mem;
}

/*************************************************************************/
/*************************************************************************/

/* Main ChanServ routine. */

void chanserv(const char *source, char *buf)
{
    char *cmd, *s;

    cmd = strtok(buf, " ");

    if (!cmd) {
	return;

    } else if (stricmp(cmd, "\1PING") == 0) {
	if (!(s = strtok(NULL, "")))
	    s = "\1";
	notice(s_ChanServ, source, "\1PING %s", s);

#ifdef SKELETON

    } else {
	notice(s_ChanServ, source, "%s is currently offline.", s_ChanServ);

    }

#else

    } else if (stricmp(cmd, "HELP") == 0) {
	do_help(source);

    } else if (stricmp(cmd, "REGISTER") == 0) {
	do_register(source);

    } else if (stricmp(cmd, "IDENTIFY") == 0) {
	do_identify(source);

    } else if (stricmp(cmd, "DROP") == 0) {
	do_drop(source);

    } else if (stricmp(cmd, "SET") == 0) {
	do_set(source);

    } else if (stricmp(cmd, "ACCESS") == 0) {
	do_access(source);

    } else if (stricmp(cmd, "AKICK") == 0) {
	do_akick(source);

    } else if (stricmp(cmd, "INVITE") == 0) {
	do_invite(source);

    } else if (stricmp(cmd, "UNBAN") == 0) {
	do_unban(source);

    } else if (stricmp(cmd, "INFO") == 0) {
	do_info(source);

    } else if (stricmp(cmd, "LIST") == 0) {
	do_list(source);

    } else if (stricmp(cmd, "OP") == 0) {
        do_op(source);

    } else if (stricmp(cmd, "DEOP") == 0) {
        do_deop(source);

    } else if (stricmp(cmd, "CLEAR") == 0) {
	do_clear(source);

    } else if (!is_services_op(source)) {
	notice(s_ChanServ, source,
		"Unrecognized command \2%s\2.  Type \"/msg %s HELP\" for help.",
		cmd, s_ChanServ);

    } else if (stricmp(cmd, "GETPASS") == 0) {
	do_getpass(source);

    } else if (stricmp(cmd, "FORBID") == 0) {
	do_forbid(source);

    } else {
	notice(s_ChanServ, source,
		"Unrecognized command \2%s\2.  Type \"/msg %s HELP\" for help.",
		cmd, s_ChanServ);

    }

#endif	/* SKELETON */

}

/*************************************************************************/

#ifndef SKELETON


/* Load/save data files. */

void load_cs_dbase(void)
{
    FILE *f = fopen(CHANSERV_DB, "r");
    int i, j, len;
    ChannelInfo *ci;

    if (!f) {
	log_perror("Can't read ChanServ database " CHANSERV_DB);
	return;
    }

    switch (i = get_file_version(f, CHANSERV_DB)) {

      case 3:
#if FILE_VERSION > 2

	for (i = 33; i < 256; ++i) {

	    while (fgetc(f) == 1) {

		ci = smalloc(sizeof(ChannelInfo));
		if (1 != fread(ci, sizeof(ChannelInfo), 1, f))
		    fatal_perror("Read error on %s", CHANSERV_DB);
		/* Can't guarantee the file is in a particular order...
		 * (Well, we can, but we don't have to depend on it.) */
		alpha_insert_chan(ci);
		ci->desc = read_string(f, CHANSERV_DB);
		ci->url = read_string(f, CHANSERV_DB);
		if (ci->mlock_key)
		    ci->mlock_key = read_string(f, CHANSERV_DB);
		if (ci->last_topic)
		    ci->last_topic = read_string(f, CHANSERV_DB);

		if (ci->accesscount) {
		    ChanAccess *access;
		    access = smalloc(sizeof(ChanAccess) * ci->accesscount);
		    ci->access = access;
		    if (ci->accesscount != fread(access, sizeof(ChanAccess),
							ci->accesscount, f))
			fatal_perror("Read error on ", CHANSERV_DB);
		    for (j = 0; j < ci->accesscount; ++j, ++access)
			access->name = read_string(f, CHANSERV_DB);
		    j = 0; access = ci->access;
		    /* Clear out unused entries */
		    while (j < ci->accesscount) {
			if (access->is_nick < 0) {
			    --ci->accesscount;
			    free(access->name);
			    if (j < ci->accesscount)
				bcopy(access+1, access,	sizeof(*access) *
							(ci->accesscount - j));
			} else {
			    ++j; ++access;
			}
		    }
		    if (ci->accesscount)
			ci->access = srealloc(ci->access,
					sizeof(ChanAccess) * ci->accesscount);
		    else {
			free(ci->access);
			ci->access = NULL;
		    }
		} /* if (ci->accesscount) */

		if (ci->akickcount) {
		    AutoKick *akick;
		    akick = smalloc(sizeof(AutoKick) * ci->akickcount);
		    ci->akick = akick;
		    if (ci->akickcount !=
			    fread(akick, sizeof(AutoKick), ci->akickcount, f))
			fatal_perror("Read error on %s", CHANSERV_DB);
		    for (j = 0; j < ci->akickcount; ++j, ++akick) {
			akick->name = read_string(f, CHANSERV_DB);
			if (akick->reason)
			    akick->reason = read_string(f, CHANSERV_DB);
		    }
		    j = 0; akick = ci->akick;
		    while (j < ci->akickcount) {
			if (akick->is_nick < 0) {
			    --ci->akickcount;
			    free(akick->name);
			    if (akick->reason)
				free(akick->reason);
			    if (j < ci->akickcount)
				bcopy(akick+1, akick, sizeof(*akick) *
							(ci->akickcount - j));
			} else {
			    ++j; ++akick;
			}
		    }
		    if (ci->akickcount) {
			ci->akick = srealloc(ci->akick,
					sizeof(AutoKick) * ci->akickcount);
		    } else {
			free(ci->akick);
			ci->akick = NULL;
		    }
		} /* if (ci->akickcount) */

		if (ci->cmd_access) {
		    int n_entries;
		    ci->cmd_access = smalloc(sizeof(short) * CA_SIZE);
		    n_entries = fgetc(f)<<8 | fgetc(f);
		    if (n_entries < 0)
			fatal_perror("Read error on %s", CHANSERV_DB);
		    if (n_entries <= CA_SIZE) {
			fread(ci->cmd_access, sizeof(short), n_entries, f);
		    } else {
			fread(ci->cmd_access, sizeof(short), CA_SIZE, f);
			fseek(f, sizeof(short) * (n_entries - CA_SIZE),
								SEEK_CUR);
		    }
		}

	    } /* while (fgetc(f) == 1) */

	} /* for (i) */

	break; /* case 3, etc. */

#endif
      case 2:
      case 1:

	for (i = 33; i < 256; ++i) {

	    while (fgetc(f) == 1) {

		ci = smalloc(sizeof(ChannelInfo));
		if (1 != fread(ci, sizeof(ChannelInfo), 1, f))
		    fatal_perror("Read error on %s", CHANSERV_DB);
		/* Can't guarantee the file is in a particular order...
		 * (Well, we can, but we don't have to depend on it.) */
		alpha_insert_chan(ci);
		ci->desc = read_string(f, CHANSERV_DB);
#if FILE_VERSION > 2
		ci->url = sstrdup("");
#endif
		if (ci->mlock_key)
		    ci->mlock_key = read_string(f, CHANSERV_DB);
		if (ci->last_topic)
		    ci->last_topic = read_string(f, CHANSERV_DB);

		if (ci->accesscount) {
		    ChanAccess *access;
		    access = smalloc(sizeof(ChanAccess) * ci->accesscount);
		    ci->access = access;
		    if (ci->accesscount != fread(access, sizeof(ChanAccess),
							ci->accesscount, f))
			fatal_perror("Read error on ", CHANSERV_DB);
		    for (j = 0; j < ci->accesscount; ++j, ++access)
			access->name = read_string(f, CHANSERV_DB);
		    j = 0; access = ci->access;
		    /* Clear out unused entries */
		    while (j < ci->accesscount) {
			if (access->is_nick < 0) {
			    --ci->accesscount;
			    free(access->name);
			    if (j < ci->accesscount)
				bcopy(access+1, access,	sizeof(*access) *
							(ci->accesscount - j));
			} else {
			    ++j; ++access;
			}
		    }
		    if (ci->accesscount)
			ci->access = srealloc(ci->access,
					sizeof(ChanAccess) * ci->accesscount);
		    else {
			free(ci->access);
			ci->access = NULL;
		    }
		} /* if (ci->accesscount) */

		if (ci->akickcount) {
		    AutoKick *akick;
		    akick = smalloc(sizeof(AutoKick) * ci->akickcount);
		    ci->akick = akick;
		    if (ci->akickcount !=
			    fread(akick, sizeof(AutoKick), ci->akickcount, f))
			fatal_perror("Read error on %s", CHANSERV_DB);
		    for (j = 0; j < ci->akickcount; ++j, ++akick) {
			akick->name = read_string(f, CHANSERV_DB);
			if (akick->reason)
			    akick->reason = read_string(f, CHANSERV_DB);
		    }
		    j = 0; akick = ci->akick;
		    while (j < ci->akickcount) {
			if (akick->is_nick < 0) {
			    --ci->akickcount;
			    free(akick->name);
			    if (akick->reason)
				free(akick->reason);
			    if (j < ci->akickcount)
				bcopy(akick+1, akick, sizeof(*akick) *
							(ci->akickcount - j));
			} else {
			    ++j; ++akick;
			}
		    }
		    if (ci->akickcount) {
			ci->akick = srealloc(ci->akick,
					sizeof(AutoKick) * ci->akickcount);
		    } else {
			free(ci->akick);
			ci->akick = NULL;
		    }
		} /* if (ci->akickcount) */

		if (ci->cmd_access) {
		    int n_entries;
		    ci->cmd_access = smalloc(sizeof(short) * CA_SIZE);
		    n_entries = fgetc(f)<<8 | fgetc(f);
		    if (n_entries < 0)
			fatal_perror("Read error on %s", CHANSERV_DB);
		    if (n_entries <= CA_SIZE) {
			fread(ci->cmd_access, sizeof(short), n_entries, f);
		    } else {
			fread(ci->cmd_access, sizeof(short), CA_SIZE, f);
			fseek(f, sizeof(short) * (n_entries - CA_SIZE),
								SEEK_CUR);
		    }
		}

	    } /* while (fgetc(f) == 1) */

	} /* for (i) */

	break; /* case 1, etc. */

      default:
	fatal("Unsupported version number (%d) on %s", i, CHANSERV_DB);

    } /* switch (version) */

    fclose(f);
}

/*************************************************************************/

void save_cs_dbase(void)
{
    FILE *f;
    int i, j, len;
    ChannelInfo *ci;

    remove(CHANSERV_DB ".save");
    if (rename(CHANSERV_DB, CHANSERV_DB ".save") < 0)
	fatal_perror("Can't back up %s", CHANSERV_DB);
    f = fopen(CHANSERV_DB, "w");
    if (!f) {
	log_perror("Can't write to ChanServ database %s", CHANSERV_DB);
	if (rename(CHANSERV_DB ".save", CHANSERV_DB) < 0)
	    fatal_perror("Can't restore backup copy of %s", CHANSERV_DB);
	return;
    }
    if (fchown(fileno(f), -1, file_gid) < 0)
	log_perror("Can't set group of %s to %d", CHANSERV_DB, file_gid);
    write_file_version(f, CHANSERV_DB);

    for (i = 33; i < 256; ++i) {
	for (ci = chanlists[i]; ci; ci = ci->next) {
	    fputc(1, f);
	    if (1 != fwrite(ci, sizeof(ChannelInfo), 1, f))
		fatal_perror("Write error on %s", CHANSERV_DB);
	    write_string(ci->desc ? ci->desc : "", f, CHANSERV_DB);
#if FILE_VERSION > 2
		write_string(ci->desc ? ci->url : "", f, CHANSERV_DB);
#endif
	    if (ci->mlock_key)
		write_string(ci->mlock_key, f, CHANSERV_DB);
	    if (ci->last_topic)
		write_string(ci->last_topic, f, CHANSERV_DB);

	    if (ci->accesscount) {
		ChanAccess *access = ci->access;
		if (ci->accesscount !=
			fwrite(access, sizeof(ChanAccess), ci->accesscount, f))
		    fatal_perror("Write error on %s", CHANSERV_DB);
		for (j = 0; j < ci->accesscount; ++j, ++access)
		    write_string(access->name, f, CHANSERV_DB);
	    }

	    if (ci->akickcount) {
		AutoKick *akick = ci->akick;
		if (ci->akickcount !=
			fwrite(akick, sizeof(AutoKick), ci->akickcount, f))
		    fatal_perror("Write error on %s", CHANSERV_DB);
		for (j = 0; j < ci->akickcount; ++j, ++akick) {
		    write_string(akick->name, f, CHANSERV_DB);
		    if (akick->reason)
			write_string(akick->reason, f, CHANSERV_DB);
		}
	    }

	    if (ci->cmd_access) {
		fputc(CA_SIZE >> 8, f);
		fputc(CA_SIZE & 0xFF, f);
		fwrite(ci->cmd_access, sizeof(short), CA_SIZE, f);
	    }

	} /* for (chanlists[i]) */

	fputc(0, f);

    } /* for (i) */

    fclose(f);
    remove(CHANSERV_DB ".save");
}

/*************************************************************************/

/* Check the current modes on a channel; if they conflict with a mode lock,
 * fix them. */

void check_modes(const char *chan)
{
    Channel *c = findchan(chan);
    ChannelInfo *ci = cs_findchan(chan);
    char newmodes[32], *newkey = NULL;
    long newlimit = 0;
    char *end = newmodes;
    int modes;
    int set_limit = 0, set_key = 0;

    if (!c || !ci)
	return;

    *end++ = '+';
    modes = ~c->mode & ci->mlock_on;
    if (modes & CMODE_I) {
	*end++ = 'i';
	c->mode |= CMODE_I;
    }
    if (modes & CMODE_M) {
	*end++ = 'm';
	c->mode |= CMODE_M;
    }
    if (modes & CMODE_N) {
	*end++ = 'n';
	c->mode |= CMODE_N;
    }
    if (modes & CMODE_P) {
	*end++ = 'p';
	c->mode |= CMODE_P;
    }
    if (modes & CMODE_S) {
	*end++ = 's';
	c->mode |= CMODE_S;
    }
    if (modes & CMODE_T) {
	*end++ = 't';
	c->mode |= CMODE_T;
    }

    if (!c->limit && ci->mlock_limit) {
	*end++ = 'l';
	newlimit = ci->mlock_limit;
	c->limit = newlimit;
	set_limit = 1;
    }
    if (!c->key && ci->mlock_key) {
	*end++ = 'k';
	newkey = ci->mlock_key;
	c->key = sstrdup(newkey);
	set_key = 1;
    }

    if (end[-1] == '+')
	--end;

    *end++ = '-';
    modes = c->mode & ci->mlock_off;
    if (modes & CMODE_I) {
	*end++ = 'i';
	c->mode &= ~CMODE_I;
    }
    if (modes & CMODE_M) {
	*end++ = 'm';
	c->mode &= ~CMODE_M;
    }
    if (modes & CMODE_N) {
	*end++ = 'n';
	c->mode &= ~CMODE_N;
    }
    if (modes & CMODE_P) {
	*end++ = 'p';
	c->mode &= ~CMODE_P;
    }
    if (modes & CMODE_S) {
	*end++ = 's';
	c->mode &= ~CMODE_S;
    }
    if (modes & CMODE_T) {
	*end++ = 't';
	c->mode &= ~CMODE_T;
    }

    if (c->key && (ci->mlock_off & CMODE_K)) {
	*end++ = 'k';
	newkey = sstrdup(c->key);
	free(c->key);
	c->key = NULL;
	set_key = 1;
    }
    if (c->limit && (ci->mlock_off & CMODE_L)) {
	*end++ = 'l';
	c->limit = 0;
    }

    if (end[-1] == '-')
	--end;

    if (end == newmodes)
	return;
    *end = 0;
    if (set_limit) {
	if (set_key)
	    send_cmd(s_ChanServ, "MODE %s %s %d :%s", c->name,
				newmodes, newlimit, newkey ? newkey : "");
	else
	    send_cmd(s_ChanServ, "MODE %s %s %d", c->name, newmodes, newlimit);
    } else if (set_key)
	send_cmd(s_ChanServ, "MODE %s %s :%s", c->name,
				newmodes, newkey ? newkey : "");
    else
	send_cmd(s_ChanServ, "MODE %s %s", c->name, newmodes);

    if (newkey && !c->key)
	free(newkey);
}

/*************************************************************************/

/* Check whether a user is allowed to be opped on a channel; if they
 * aren't, deop them.  If serverop is 1, the +o was done by a server.
 * Return 1 if the user is allowed to be opped, 0 otherwise. */

int check_valid_op(User *user, const char *chan, int serverop)
{
    ChannelInfo *ci = cs_findchan(chan);

    if (!ci || ci->flags & CI_LEAVEOPS)
	return 1;

    if (is_oper(user->nick) || is_services_op(user->nick))
	return 1;

    if (ci->flags & CI_VERBOTEN) {
	/* check_kick() will get them out; we needn't explain. */
	send_cmd(s_ChanServ, "MODE %s -o %s", chan, user->nick);
	return 0;
    }

    if (serverop && get_access(user, ci) < 1) {
	notice(s_ChanServ, user->nick,
		"This channel has been registered with %s.", s_ChanServ);
	send_cmd(s_ChanServ, "MODE %s -o %s", chan, user->nick);
	return 0;
    }

    if (get_access(user, ci) < (ci->flags & CI_SECUREOPS ? 1 : 0)) {
#if 0
	notice(s_ChanServ, user->nick,
		"You are not allowed chanop status on channel %s.", chan);
#endif
	send_cmd(s_ChanServ, "MODE %s -o %s", chan, user->nick);
	return 0;
    }

    return 1;
}

/*************************************************************************/

/* Check whether a user should be opped on a channel, and if so, do it.
 * Return 1 if the user was opped, 0 otherwise.  (Updates the channel's
 * last used time if the user was opped.) */

int check_should_op(User *user, const char *chan)
{
    ChannelInfo *ci = cs_findchan(chan);
    NickInfo *ni;

    if (!ci || ci->flags & CI_VERBOTEN || *chan == '+')
	return 0;

    if (ci->flags & CI_SECURE) {
	if (!(ni = findnick(user->nick)) || !(ni->flags & NI_IDENTIFIED))
	    return 0;
    }

    if (get_access(user, ci) >= 5) {
	send_cmd(s_ChanServ, "MODE %s +o %s", chan, user->nick);
	ci->last_used = time(NULL);
	return 1;
    }

    return 0;
}

/*************************************************************************/

/* Check whether a user is permitted to be on a channel.  If so, return 0;
 * else, kickban the user with an appropriate message (could be either
 * AKICK or restricted access) and return 1.
 */

int check_kick(User *user, const char *chan)
{
    ChannelInfo *ci = cs_findchan(chan);
    AutoKick *akick;
    int i, bad;
    NickInfo *ni;
    char *av[3];

    if (!ci)
	return 0;

    if (is_oper(user->nick) || is_services_op(user->nick))
	return 0;

    if (ci->flags & CI_VERBOTEN) {
	av[0] = sstrdup(chan);
	av[1] = "+b";
	av[2] = create_mask(user);
	send_cmd(s_ChanServ, "MODE %s +b %s", chan, av[2]);
	do_cmode(s_ChanServ, 3, av);
	send_cmd(s_ChanServ, "KICK %s %s :This channel may not be used.",
		chan, user->nick);
	free(av[0]);
	free(av[2]);
	return 1;
    }

    ni = findnick(user->nick);
    if ((ni == NULL) || !(ni->flags & (NI_IDENTIFIED | NI_RECOGNIZED)))
	ni = NULL;

    for (akick = ci->akick, i = 0; i < ci->akickcount; ++akick, ++i) {
	if ((akick->is_nick > 0 && ni && stricmp(akick->name, user->nick) == 0) ||
		(!akick->is_nick && match_usermask(akick->name, user))) {
	    av[0] = sstrdup(chan);
	    av[1] = "+b";
	    av[2] = akick->is_nick ? create_mask(user) : akick->name;
	    send_cmd(s_ChanServ, "MODE %s +b %s", chan, av[2]);
	    do_cmode(s_ChanServ, 3, av);
	    send_cmd(s_ChanServ,
			"KICK %s %s :%s", chan, user->nick,
			akick->reason ? akick->reason : DEF_AKICK_REASON);
	    free(av[0]);
	    if (akick->is_nick)
		free(av[2]);
	    return 1;
	}
    }

    if (!(ci->flags & CI_RESTRICTED))
	return 0;

    if (get_access(user, ci) < (ci->flags&CI_SECUREOPS ? 1 : 0)) {
	av[0] = sstrdup(chan);
	av[1] = "+b";
	av[2] = create_mask(user);
	send_cmd(s_ChanServ, "MODE %s +b %s", chan, av[2]);
	do_cmode(s_ChanServ, 3, av);
	send_cmd(s_ChanServ, "KICK %s %s :%s", chan, user->nick,
		"You are not permitted to be on this channel.");
	free(av[0]);
	free(av[2]);
	return 1;
    }

    return 0;
}

/*************************************************************************/

/* Record the current channel topic in the ChannelInfo structure. */

void record_topic(const char *chan)
{
#ifndef READONLY
    Channel *c = findchan(chan);
    ChannelInfo *ci = cs_findchan(chan);

    if (!c || !ci)
	return;
    if (ci->last_topic)
	free(ci->last_topic);
    if (c->topic)
	ci->last_topic = sstrdup(c->topic);
    else
	ci->last_topic = NULL;
    strscpy(ci->last_topic_setter, c->topic_setter, NICKMAX);
    ci->last_topic_time = c->topic_time;
#endif
}

/*************************************************************************/

/* Restore the topic in a channel when it's created, if we should. */

void restore_topic(const char *chan)
{
    Channel *c = findchan(chan);
    ChannelInfo *ci = cs_findchan(chan);

    if (!c || !ci || !(ci->flags & CI_KEEPTOPIC))
	return;
    if (c->topic)
	free(c->topic);
    if (ci->last_topic) {
	c->topic = sstrdup(ci->last_topic);
	strscpy(c->topic_setter, ci->last_topic_setter, NICKMAX);
	c->topic_time = ci->last_topic_time;
    } else {
	c->topic = NULL;
	strscpy(c->topic_setter, s_ChanServ, NICKMAX);
    }
    send_cmd(s_ChanServ, "TOPIC %s %s %lu :%s", chan,
		c->topic_setter, c->topic_time, c->topic ? c->topic : "");
}

/*************************************************************************/

/* See if the topic is locked on the given channel, and return 1 (and fix
 * the topic) if so. */

int check_topiclock(const char *chan)
{
    Channel *c = findchan(chan);
    ChannelInfo *ci = cs_findchan(chan);

    if (!c || !ci || !(ci->flags & CI_TOPICLOCK))
	return 0;
    if (c->topic)
	free(c->topic);
    if (ci->last_topic)
	c->topic = sstrdup(ci->last_topic);
    else
	c->topic = NULL;
    strscpy(c->topic_setter, ci->last_topic_setter, NICKMAX);
    c->topic_time = ci->last_topic_time;
    send_cmd(s_ChanServ, "TOPIC %s %s %lu :%s", chan,
		c->topic_setter, c->topic_time, c->topic ? c->topic : "");
}

/*************************************************************************/

/* Remove all channels which have expired. */

void expire_chans()
{
    ChannelInfo *ci;
    int i;
    const time_t expire_time = CHANNEL_EXPIRE*24*60*60;
    time_t now = time(NULL);

    for (i = 33; i < 256; ++i) {
	for (ci = chanlists[i]; ci; ci = ci->next) {
	    if (now - ci->last_used >= expire_time
					&& !(ci->flags & CI_VERBOTEN)) {
		log("Expiring channel %s", ci->name);
		delchan(ci);
	    }
	}
    }
}

/*************************************************************************/
/*********************** ChanServ private routines ***********************/
/*************************************************************************/

/* Return the ChannelInfo structure for the given channel, or NULL if the
 * channel isn't registered. */

ChannelInfo *cs_findchan(const char *chan)
{
    ChannelInfo *ci;

    for (ci = chanlists[tolower(chan[1])]; ci; ci = ci->next) {
	if (stricmp(ci->name, chan) == 0)
	    return ci;
    }
    return NULL;
}

/*************************************************************************/

/* Insert a channel alphabetically into the database. */

static void alpha_insert_chan(ChannelInfo *ci)
{
    ChannelInfo *ci2, *ci3;
    char *chan = ci->name;

    for (ci3 = NULL, ci2 = chanlists[tolower(chan[1])];
			ci2 && stricmp(ci2->name, chan) < 0;
			ci3 = ci2, ci2 = ci2->next)
	;
    ci->prev = ci3;
    ci->next = ci2;
    if (!ci3)
	chanlists[tolower(chan[1])] = ci;
    else
	ci3->next = ci;
    if (ci2)
	ci2->prev = ci;
}

/*************************************************************************/

/* Add a channel to the database.  Returns a pointer to the new ChannelInfo
 * structure if the channel was successfully registered, NULL otherwise.
 * Assumes channel does not already exist. */

static ChannelInfo *makechan(const char *chan)
{
    ChannelInfo *ci;
    Channel *c;
    char *s, *end;

    ci = scalloc(sizeof(ChannelInfo), 1);
    strscpy(ci->name, chan, CHANMAX);
    ci->time_registered = time(NULL);
    alpha_insert_chan(ci);
    return ci;
}

/*************************************************************************/

/* Remove a channel from the ChanServ database.  Return 1 on success, 0
 * otherwise. */

static int delchan(ChannelInfo *ci)
{
    int i;

    if (ci->next)
	ci->next->prev = ci->prev;
    if (ci->prev)
	ci->prev->next = ci->next;
    else
	chanlists[tolower(ci->name[1])] = ci->next;
    if (ci->desc)
	free(ci->desc);
#if FILE_VERSION > 2
    if (ci->url)
	free(ci->url);
#endif
    if (ci->mlock_key)
	free(ci->mlock_key);
    if (ci->last_topic)
	free(ci->last_topic);
    for (i = 0; i < ci->accesscount; ++i)
	free(ci->access[i].name);
    if (ci->access)
	free(ci->access);
    for (i = 0; i < ci->akickcount; ++i) {
	free(ci->akick[i].name);
	if (ci->akick[i].reason)
	    free(ci->akick[i].reason);
    }
    if (ci->akick)
	free(ci->akick);
    if (ci->cmd_access)
	free(ci->cmd_access);
    return 1;
}

/*************************************************************************/

/* Does the given user have founder access to the channel? */

static int is_founder(User *user, NickInfo *ni, ChannelInfo *ci)
{
    struct u_chaninfolist *c;

    if (ni && ((ni->flags & NI_IDENTIFIED) ||
		((ni->flags & NI_RECOGNIZED) && !(ci->flags & CI_SECURE)))
	   && stricmp(user->nick, ci->founder) == 0)
	return 1;
    return is_identified(user, ci);
}

/*************************************************************************/

/* Has the given user password-identified as founder for the channel? */

static int is_identified(User *user, ChannelInfo *ci)
{
    struct u_chaninfolist *c;

    for (c = user->founder_chans; c; c = c->next) {
	if (c->chan == ci)
	    return 1;
    }
    return 0;
}

/*************************************************************************/

/* Return the access level the given user has on the channel.  If the
 * channel doesn't exist, the user isn't on the access list, or the channel
 * is CS_SECURE and the user hasn't IDENTIFY'd with NickServ, return 0. */

int get_access(User *user, ChannelInfo *ci)
{
    NickInfo *ni = findnick(user->nick);
    ChanAccess *access;
    int i;

    if (!ci)
	return 0;
    if (is_founder(user, ni, ci))
	return 10000;
    for (access = ci->access, i = 0; i < ci->accesscount; ++access, ++i) {
	if (    (access->is_nick > 0 &&
		    ni && ((ni->flags & NI_IDENTIFIED)
			   || ((ni->flags & NI_RECOGNIZED)
			       && !(ci->flags & CI_SECURE)))
		       && stricmp(user->nick, access->name) == 0
		) ||
		(!access->is_nick && match_usermask(access->name, user))
	) {
	    return access->level;
	}
    }
    return 0;
}

/*************************************************************************/
/*********************** ChanServ command routines ***********************/
/*************************************************************************/

static void do_help(const char *source)
{
    char *cmd = strtok(NULL, "");
    char buf[256];

    if (cmd && is_oper(source)) {

	if (stricmp(cmd, "DROP") == 0) {
	    notice_list(s_ChanServ, source, oper_drop_help);
	    return;
	} else if (stricmp(cmd, "GETPASS") == 0) {
	    notice_list(s_ChanServ, source, getpass_help);
	    return;
	} else if (stricmp(cmd, "FORBID") == 0) {
	    notice_list(s_ChanServ, source, forbid_help);
	    return;
	}

    }

    sprintf(buf, "%s%s", s_ChanServ, cmd ? " " : "");
    strscpy(buf+strlen(buf), cmd ? cmd : "", sizeof(buf)-strlen(buf));
    helpserv(s_ChanServ, source, buf);
}

/*************************************************************************/

static void do_register(const char *source)
{
    char *chan = strtok(NULL, " ");
    char *pass = strtok(NULL, " ");
    char *desc = strtok(NULL, "");
    NickInfo *ni = findnick(source);
    ChannelInfo *ci;
    User *u = finduser(source);
    struct u_chaninfolist *c;

#ifdef READONLY
    notice(s_ChanServ, source,
		"Sorry, channel registration is temporarily disabled.");
    return;
#endif

    if (!desc) {

	notice(s_ChanServ, source,
		"Syntax: \2REGISTER \37channel\37 \37password\37 \37description\37\2");
	notice(s_ChanServ, source,
		"\2/msg %s HELP REGISTER\2 for more information.", s_ChanServ);

    } else if (*chan == '&') {

	notice(s_ChanServ, source, "Local channels cannot be registered.");

    } else if (chan[1] == 0) {

	notice(s_ChanServ, source, "\2%s\2 is not a valid channel name.", chan);

    } else if (!ni) {

	notice(s_ChanServ, source, "Your must register your nickname first.");
	notice(s_ChanServ, source,
		"\2/msg %s HELP\2 for information on registering nicknames.",
		s_NickServ);

    } else if (ci = cs_findchan(chan)) {

	if (ci->flags & CI_VERBOTEN) {
	    log("%s: Attempt to register FORBIDden channel %s by %s!%s@%s",
			s_ChanServ, chan, source, u->username, u->host);
	    notice(s_ChanServ, source,
			"Channel \2%s\2 may not be registered.", chan);
	} else {
	    notice(s_ChanServ, source,
			"Channel \2%s\2 already registered!", chan);
	}

    } else if (!u) {

	log("%s: Attempt to register channel %s from nonexistent nick %s",
						s_ChanServ, chan, source);
	notice(s_ChanServ, source, "Sorry, registration failed.");

    } else if (!is_chanop(source, chan)) {

	notice(s_ChanServ, source,
		"You must be a channel operator to register the channel.");

    } else {

	if (ci = makechan(chan)) {
	    Channel *c;
	    if (!(c = findchan(chan))) {
		log("%s: Channel %s not found for REGISTER", s_ChanServ, chan);
		notice(s_ChanServ, source, "Sorry, registration failed.");
		return;
	    }
	    ci->last_used = ci->time_registered;
	    strscpy(ci->founder, source, NICKMAX);
	    strscpy(ci->founderpass, pass, PASSMAX);
	    ci->desc = sstrdup(desc);
#if FILE_VERSION > 2
	    ci->url = sstrdup("");
#endif
	    if (c->topic) {
		ci->last_topic = sstrdup(c->topic);
		strscpy(ci->last_topic_setter, c->topic_setter, NICKMAX);
		ci->last_topic_time = c->topic_time;
	    }
	    ci->flags = CI_KEEPTOPIC;
	    log("%s: Channel %s registered by %s!%s@%s", s_ChanServ, chan,
			source, u->username, u->host);
	    notice(s_ChanServ, source,
			"Channel %s registered under your nickname: %s",
			chan, source);
	    notice(s_ChanServ, source,
			"Your channel password is \2%s\2 - remember it for "
			"later use.", ci->founderpass);
	} else {
	    notice(s_ChanServ, source, "Sorry, registration failed.");
	}
	c = smalloc(sizeof(*c));
	c->next = u->founder_chans;
	c->prev = NULL;
	if (u->founder_chans)
	    u->founder_chans->prev = c;
	u->founder_chans = c;
	c->chan = ci;

    }
}

/*************************************************************************/

static void do_identify(const char *source)
{
    char *chan = strtok(NULL, " ");
    char *pass = strtok(NULL, " ");
    ChannelInfo *ci;
    struct u_chaninfolist *c;
    User *u = finduser(source);

    if (!pass) {

	notice(s_ChanServ, source,
		"Syntax: \2IDENTIFY %s \37password\37\2",
		chan ? chan : "\37channel\37");
	notice(s_ChanServ, source,
		"\2/msg %s HELP IDENTIFY\2 for more information.", s_ChanServ);

    } else if (!(ci = cs_findchan(chan))) {

	notice(s_ChanServ, source, "Channel %s is not registered.", chan);

    } else if (!u) {

	log("%s: IDENTIFY from nonexistent nick %s for %s",
						s_ChanServ, source, chan);
	notice(s_ChanServ, source, "Sorry, identification failed.");

    } else {

	if (strcmp(pass, ci->founderpass) == 0) {
	    if (!is_identified(u, ci)) {
		c = smalloc(sizeof(*c));
		c->next = u->founder_chans;
		c->prev = NULL;
		if (u->founder_chans)
		    u->founder_chans->prev = c;
		u->founder_chans = c;
		c->chan = ci;
		log("%s: %s!%s@%s identified for %s", s_ChanServ,
			source, u->username, u->host, chan);
	    }
	    notice(s_ChanServ, source,
		"Password accepted - you now have founder-level access to %s.",
		chan);
	} else {
	    log("%s: Failed IDENTIFY for %s by %s!%s@%s",
			s_ChanServ, chan, source, u->username, u->host);
	    notice(s_ChanServ, source, "Password incorrect.");
	}

    }
}

/*************************************************************************/

static void do_drop(const char *source)
{
    char *chan = strtok(NULL, " ");
    ChannelInfo *ci;
    User *u = finduser(source);
    int is_servop = is_services_op(source);

#ifdef READONLY
    if (!is_servop) {
	notice(s_ChanServ, source,
		"Sorry, channel de-registration is temporarily disabled.");
	return;
    }
#endif

    if (!chan) {

	notice(s_ChanServ, source, "Syntax: \2DROP \37channel\37\2");
	notice(s_ChanServ, source,
		"\2/msg %s HELP DROP\2 for more information.", s_ChanServ);

    } else if (!(ci = cs_findchan(chan))) {

	notice(s_ChanServ, source, "Channel %s is not registered.", chan);

    } else if (!is_servop && (!u || !is_identified(u, ci))) {

	notice(s_ChanServ, source,
		"Password authentication required for that command.");
	notice(s_ChanServ, source,
		"Retry after typing \2/msg %s IDENTIFY %s <password>.",
		s_ChanServ, chan);

    } else if (is_servop && !u) {

	log("%s: DROP %s from nonexistent oper %s", s_ChanServ, chan, source);
	notice(s_ChanServ, source, "Can't find your user record!");

    } else {

#ifdef READONLY
	notice(s_ChanServ, source,
		"Warning: Services is in read-only mode; changes will not be saved.");
#endif
	delchan(ci);
	log("%s: Channel %s dropped by %s!%s@%s", s_ChanServ, chan,
			source, u->username, u->host);
	notice(s_ChanServ, source, "Channel %s has been dropped.", chan);

    }
}

/*************************************************************************/

/* Main SET routine.  Calls other routines as follows:
 *	do_set_command(User *command-sender, ChannelInfo *ci, char *param);
 * Additional parameters can be retrieved using strtok(NULL, toks).
 * (Exception is do_set_held(), which takes only source nick and channel
 * name.)
 */
static void do_set(const char *source)
{
    char *chan = strtok(NULL, " ");
    char *cmd  = strtok(NULL, " ");
    char *param;
    ChannelInfo *ci;
    User *u;
    int i;

#ifdef READONLY
    notice(s_ChanServ, source,
		"Sorry, channel option setting is temporarily disabled.");
    return;
#endif

    if (cmd) {
	if (stricmp(cmd, "DESC") == 0 || stricmp(cmd, "TOPIC") == 0)
	    param = strtok(NULL, "");
	else
	    param = strtok(NULL, " ");
    } else
	param = NULL;

    if (!param) {

	notice(s_ChanServ, source,
		"Syntax: \2SET %s \37option\37 \37parameters\37\2",
		chan ? chan : "\37channel\37");
	notice(s_ChanServ, source,
		"\2/msg %s HELP SET\2 for more information.", s_ChanServ);

    } else if (!(ci = cs_findchan(chan))) {

	notice(s_ChanServ, source, "Channel %s is not registered.", chan);

    } else if (!(u = finduser(source)) || get_access(u, ci) < 20) {

	notice(s_ChanServ, source, "Access denied.");

/*    } else if (!(u = finduser(source)) || !is_identified(u, ci)) {

	if (is_founder(u, findnick(u->nick), ci)) {
	    notice(s_ChanServ, source,
		"Password authentication required for that command.");
	    notice(s_ChanServ, source,
		"Retry after typing \2/msg ChanServ IDENTIFY %s \37password\37.",
		chan);
	} else {
	    notice(s_ChanServ, source, "Access denied.");
	} */

    } else if (stricmp(cmd, "FOUNDER") == 0) {

	if (is_founder(u, findnick(u->nick), ci)) {
	     if(is_identified(u, ci)) {
		    do_set_founder(u, ci, param);
	     } else {
		notice(s_ChanServ, source,
		    "Password authentication required for that command.");
		notice(s_ChanServ, source,
		    "Retry after typing \2/msg ChanServ IDENTIFY %s \37password\37.",
		    chan);
	     }
	} else {
	    notice(s_ChanServ, source, "Access denied.");
	}

    } else if (stricmp(cmd, "PASSWORD") == 0) {

	if (is_founder(u, findnick(u->nick), ci)) {
	     if(is_identified(u, ci)) {
		do_set_password(u, ci, param);
	     } else {
		notice(s_ChanServ, source,
		    "Password authentication required for that command.");
		notice(s_ChanServ, source,
		    "Retry after typing \2/msg ChanServ IDENTIFY %s \37password\37.",
		    chan);
	     }
	} else {
	    notice(s_ChanServ, source, "Access denied.");
	}

    } else if (stricmp(cmd, "DESC") == 0) {

	do_set_desc(u, ci, param);

#if FILE_VERSION > 2
    } else if (stricmp(cmd, "URL") == 0) {

	do_set_url(u, ci, param);
#endif

    } else if (stricmp(cmd, "TOPIC") == 0) {

	do_set_topic(u, ci, param);

    } else if (stricmp(cmd, "MLOCK") == 0) {

	do_set_mlock(u, ci, param);

    } else if (stricmp(cmd, "KEEPTOPIC") == 0) {

	do_set_keeptopic(u, ci, param);

    } else if (stricmp(cmd, "TOPICLOCK") == 0) {

	do_set_topiclock(u, ci, param);

    } else if (stricmp(cmd, "PRIVATE") == 0) {

	do_set_private(u, ci, param);

    } else if (stricmp(cmd, "SECUREOPS") == 0) {

	do_set_secureops(u, ci, param);

    } else if (stricmp(cmd, "RESTRICTED") == 0) {

	do_set_restricted(u, ci, param);

    } else if (stricmp(cmd, "SECURE") == 0) {

	do_set_secure(u, ci, param);

    } else {

	notice(s_ChanServ, source,
			"Unknown SET option \2%s\2.", strupper(cmd));

    }
}

/*************************************************************************/

static void do_set_founder(User *u, ChannelInfo *ci, char *param)
{
    NickInfo *ni = findnick(param);

    if (!ni) {
	notice(s_ChanServ, u->nick, "%s isn't a registered nickname.", param);
	return;
    }
    strscpy(ci->founder, param, NICKMAX);
    notice(s_ChanServ, u->nick,
		"Founder of %s changed to \2%s\2.", ci->name, ci->founder);
}

/*************************************************************************/

static void do_set_password(User *u, ChannelInfo *ci, char *param)
{
    strscpy(ci->founderpass, param, PASSMAX);
    notice(s_ChanServ, u->nick,
		"%s password changed to \2%s\2.", ci->name, ci->founderpass);
}

/*************************************************************************/

static void do_set_desc(User *u, ChannelInfo *ci, char *param)
{
    free(ci->desc);
    ci->desc = sstrdup(param);
    notice(s_ChanServ, u->nick,
		"Description of %s changed to \2%s\2.", ci->name, param);
}

/*************************************************************************/

#if FILE_VERSION > 2
static void do_set_url(User *u, ChannelInfo *ci, char *param)
{
    free(ci->url);
    if(stricmp(param, "NONE") == 0) {
	ci->url = sstrdup("");
	notice(s_ChanServ, u->nick,
		"World Wide Web Page (URL) of %s removed.", ci->name);
    } else {
	ci->url = sstrdup(param);
	notice(s_ChanServ, u->nick,
		"World Wide Web Page (URL) of %s changed to \2%s\2.", ci->name, param);
    }
}
#endif

/*************************************************************************/

static void do_set_topic(User *u, ChannelInfo *ci, char *param)
{
    char *source = u->nick, *chan = ci->name;
    Channel *c = findchan(chan);

    if (!c) {
	log("%s: SET TOPIC for %s from %s: channel not found!",
		s_ChanServ, chan, source);
	notice(s_ChanServ, source, "Sorry, can't set topic.");
	return;
    }
    if (ci->last_topic)
	free(ci->last_topic);
    if (*param)
	ci->last_topic = sstrdup(param);
    else
	ci->last_topic = NULL;
    if (c->topic) {
	free(c->topic);
	--c->topic_time;
    } else
	c->topic_time = time(NULL);
    if (*param)
	c->topic = sstrdup(param);
    else
	c->topic = NULL;
    strscpy(ci->last_topic_setter, source, NICKMAX);
    strscpy(c->topic_setter, source, NICKMAX);
    ci->last_topic_time = c->topic_time;
    send_cmd(s_ChanServ, "TOPIC %s %s %lu :%s",
		chan, source, c->topic_time, param);
}

/*************************************************************************/

static void do_set_mlock(User *u, ChannelInfo *ci, char *param)
{
    char *source = u->nick, *chan = ci->name;
    char *s, modebuf[32], *end, c;
    int add = -1;	/* 1 if adding, 0 if deleting, -1 if neither */

    ci->mlock_on = ci->mlock_off = ci->mlock_limit = 0;
    if (ci->mlock_key) {
	free(ci->mlock_key);
	ci->mlock_key = NULL;
    }

    while (*param) {
	if (*param != '+' && *param != '-' && add < 0) {
	    ++param;
	    continue;
	}
	switch (tolower((c = *param++))) {
	case '+':
	    add = 1; break;
	case '-':
	    add = 0; break;
	case 'i':
	    if (add)
		ci->mlock_on |= CMODE_I;
	    else
		ci->mlock_off |= CMODE_I;
	    break;
	case 'k':
	    if (add) {
		if (!(s = strtok(NULL, " "))) {
		    notice(s_ChanServ, source,
				"Parameter required for MLOCK +k.");
		    break;
		}
		ci->mlock_key = sstrdup(s);
	    } else {
		if (ci->mlock_key) {
		    free(ci->mlock_key);
		    ci->mlock_key = NULL;
		}
		ci->mlock_off |= CMODE_K;
	    }
	    break;
	case 'l':
	    if (add) {
		if (!(s = strtok(NULL, " "))) {
		    notice(s_ChanServ, source,
				"Parameter required for MLOCK +l.");
		    break;
		}
		if (atol(s) <= 0) {
		    notice(s_ChanServ, source,
			"Parameter for MLOCK +l must be a positive number.");
		    break;
		}
		ci->mlock_limit = atol(s);
	    } else
		ci->mlock_off |= CMODE_L;
	    break;
	case 'm':
	    if (add)
		ci->mlock_on |= CMODE_M;
	    else
		ci->mlock_off |= CMODE_M;
	    break;
	case 'n':
	    if (add)
		ci->mlock_on |= CMODE_N;
	    else
		ci->mlock_off |= CMODE_N;
	    break;
	case 'p':
	    if (add)
		ci->mlock_on |= CMODE_P;
	    else
		ci->mlock_off |= CMODE_P;
	    break;
	case 's':
	    if (add)
		ci->mlock_on |= CMODE_S;
	    else
		ci->mlock_off |= CMODE_S;
	    break;
	case 't':
	    if (add)
		ci->mlock_on |= CMODE_T;
	    else
		ci->mlock_off |= CMODE_T;
	    break;
	default:
	    notice(s_ChanServ, source,
			"Unknown mode character %c ignored.", c);
	} /* switch */
    } /* while (*param) */

    end = modebuf;
    *end = 0;
    if (ci->mlock_on)
	end += sprintf(end, "+%s%s%s%s%s%s%s%s",
				(ci->mlock_on & CMODE_I) ? "i" : "",
				(ci->mlock_key         ) ? "k" : "",
				(ci->mlock_limit       ) ? "l" : "",
				(ci->mlock_on & CMODE_M) ? "m" : "",
				(ci->mlock_on & CMODE_N) ? "n" : "",
				(ci->mlock_on & CMODE_P) ? "p" : "",
				(ci->mlock_on & CMODE_S) ? "s" : "",
				(ci->mlock_on & CMODE_T) ? "t" : "");
    if (ci->mlock_off)
	end += sprintf(end, "-%s%s%s%s%s%s%s%s",
				(ci->mlock_off & CMODE_I) ? "i" : "",
				(ci->mlock_off & CMODE_K) ? "k" : "",
				(ci->mlock_off & CMODE_L) ? "l" : "",
				(ci->mlock_off & CMODE_M) ? "m" : "",
				(ci->mlock_off & CMODE_N) ? "n" : "",
				(ci->mlock_off & CMODE_P) ? "p" : "",
				(ci->mlock_off & CMODE_S) ? "s" : "",
				(ci->mlock_off & CMODE_T) ? "t" : "");
    if (*modebuf)
	notice(s_ChanServ, source,
		"Mode lock on channel %s changed to \2%s\2.", chan, modebuf);
    else
	notice(s_ChanServ, source,
		"Mode lock on channel %s removed.", chan);
    check_modes(chan);
}

/*************************************************************************/

static void do_set_keeptopic(User *u, ChannelInfo *ci, char *param)
{
    char *source = u->nick, *chan = ci->name;

    if (stricmp(param, "ON") == 0) {

	ci->flags |= CI_KEEPTOPIC;
	notice(s_ChanServ, source,
		"Topic retention option is now \2ON\2.");

    } else if (stricmp(param, "OFF") == 0) {

	ci->flags &= ~CI_KEEPTOPIC;
	notice(s_ChanServ, source,
		"Topic retention option is now \2OFF\2.");

    } else {

	notice(s_ChanServ, source,
		"Syntax: \2SET %s KEEPTOPIC {ON|OFF}\2", chan);
	notice(s_ChanServ, source,
		"\2/msg %s HELP SET KEEPTOPIC\2 for more information.",
		s_ChanServ);
    }
}

/*************************************************************************/

static void do_set_topiclock(User *u, ChannelInfo *ci, char *param)
{
    char *source = u->nick, *chan = ci->name;

    if (stricmp(param, "ON") == 0) {

	ci->flags |= CI_TOPICLOCK;
	notice(s_ChanServ, source,
		"Topic lock option is now \2ON\2.");

    } else if (stricmp(param, "OFF") == 0) {

	ci->flags &= ~CI_TOPICLOCK;
	notice(s_ChanServ, source,
		"Topic lock option is now \2OFF\2.");

    } else {

	notice(s_ChanServ, source,
		"Syntax: \2SET %s TOPICLOCK {ON|OFF}\2", chan);
	notice(s_ChanServ, source,
		"\2/msg %s HELP SET TOPICLOCK\2 for more information.",
		s_ChanServ);
    }
}

/*************************************************************************/

static void do_set_private(User *u, ChannelInfo *ci, char *param)
{
    char *source = u->nick, *chan = ci->name;

    if (stricmp(param, "ON") == 0) {

	ci->flags |= CI_PRIVATE;
	notice(s_ChanServ, source,
		"Private option is now \2ON\2.");

    } else if (stricmp(param, "OFF") == 0) {

	ci->flags &= ~CI_PRIVATE;
	notice(s_ChanServ, source,
		"Private option is now \2OFF\2.");

    } else {

	notice(s_ChanServ, source,
		"Syntax: \2SET %s PRIVATE {ON|OFF}\2", chan);
	notice(s_ChanServ, source,
		"\2/msg %s HELP SET PRIVATE\2 for more information.",
		s_ChanServ);
    }
}

/*************************************************************************/

static void do_set_secureops(User *u, ChannelInfo *ci, char *param)
{
    char *source = u->nick, *chan = ci->name;

    if (stricmp(param, "ON") == 0) {

	ci->flags |= CI_SECUREOPS;
	notice(s_ChanServ, source,
		"Secure ops option is now \2ON\2.");

    } else if (stricmp(param, "OFF") == 0) {

	ci->flags &= ~CI_SECUREOPS;
	notice(s_ChanServ, source,
		"Secure ops option is now \2OFF\2.");

    } else {

	notice(s_ChanServ, source,
		"Syntax: \2SET %s SECUREOPS {ON|OFF}\2", chan);
	notice(s_ChanServ, source,
		"\2/msg %s HELP SET SECUREOPS\2 for more information.",
		s_ChanServ);
    }
}

/*************************************************************************/

static void do_set_restricted(User *u, ChannelInfo *ci, char *param)
{
    char *source = u->nick, *chan = ci->name;

    if (stricmp(param, "ON") == 0) {

	ci->flags |= CI_RESTRICTED;
	notice(s_ChanServ, source,
		"Restricted access option is now \2ON\2.");

    } else if (stricmp(param, "OFF") == 0) {

	ci->flags &= ~CI_RESTRICTED;
	notice(s_ChanServ, source,
		"Restricted access option is now \2OFF\2.");

    } else {

	notice(s_ChanServ, source,
		"Syntax: \2SET %s RESTRICTED {ON|OFF}\2", chan);
	notice(s_ChanServ, source,
		"\2/msg %s HELP SET RESTRICTED\2 for more information.",
		s_ChanServ);
    }
}

/*************************************************************************/

static void do_set_secure(User *u, ChannelInfo *ci, char *param)
{
    char *source = u->nick, *chan = ci->name;

    if (stricmp(param, "ON") == 0) {

	ChanAccess *access;
	int i;

	for (access = ci->access, i = 0; i < ci->accesscount; ++access, ++i) {
	    if (access->is_nick == 0) {
		notice(s_ChanServ, source,
			"SECURE channel access lists may only contain registered");
		notice(s_ChanServ, source,
			"nicknames, not nick!user@host masks.");
		notice(s_ChanServ, source,
			"\2/msg %s HELP SET SECURE\2 for more information.",
			s_ChanServ);
		return;
	    }
	}
	ci->flags |= CI_SECURE;
	notice(s_ChanServ, source,
		"Secure option is now \2ON\2.");

    } else if (stricmp(param, "OFF") == 0) {

	ci->flags &= ~CI_SECURE;
	notice(s_ChanServ, source,
		"Secure option is now \2OFF\2.");

    } else {

	notice(s_ChanServ, source,
		"Syntax: \2SET %s SECURE {ON|OFF}\2", chan);
	notice(s_ChanServ, source,
		"\2/msg %s HELP SET SECURE\2 for more information.",
		s_ChanServ);
    }
}

/*************************************************************************/

static void do_access(const char *source)
{
    char *chan = strtok(NULL, " ");
    char *cmd  = strtok(NULL, " ");
    char *mask = strtok(NULL, " ");
    char *s    = strtok(NULL, " ");
    ChannelInfo *ci;
    NickInfo *ni;
    User *u;
    short level = 0, ulev;
    int i;
    ChanAccess *access;
    char *nick, *user, *host;

    /* If LIST, we don't *require* any parameters, but we can take any.
     * If DEL, we require a mask and no level.
     * Else (ADD), we require a level (which implies a mask). */
    if (!cmd || ((stricmp(cmd,"LIST")==0) ? 0 :
			(stricmp(cmd,"DEL")==0) ? (!mask || !!s) : !s)) {
	notice(s_ChanServ, source,
		"Syntax: \2ACCESS %s {ADD|DEL|LIST} [\37user\37 [\37level\37]]\2",
		chan ? chan : "\37channel\37");
	notice(s_ChanServ, source,
		"\2/msg %s HELP ACCESS\2 for more information.", s_ChanServ);

    } else if (!(ci = cs_findchan(chan))) {

	notice(s_ChanServ, source, "Channel %s is not registered.", chan);

    } else if (!(u = finduser(source)) || ((ulev = get_access(u, ci)) <= 0) ||
					(s && (level = atoi(s)) >= ulev)) {
	notice(s_ChanServ, source, "Access denied.");

    } else if (stricmp(cmd, "ADD") == 0) {

#ifdef READONLY
    notice(s_ChanServ, source,
		"Sorry, channel access list modification is temporarily disabled.");
    return;
#endif

	if (level == 0) {
	    notice(s_ChanServ, source, "Access level must be non-zero.");
	    return;
	}
	ni = findnick(mask);
	if (!ni) {
	    if (ci->flags & CI_SECURE) {
		notice(s_ChanServ, source,
			"SECURE channel access lists may only contain registered");
		notice(s_ChanServ, source,
			"nicknames, not nick!user@host masks.");
		notice(s_ChanServ, source,
			"\2/msg %s HELP SET SECURE\2 for more information.",
			s_ChanServ);
		return;
	    }
	    split_usermask(mask, &nick, &user, &host);
	    s = smalloc(strlen(nick)+strlen(user)+strlen(host)+3);
	    sprintf(s, "%s!%s@%s", nick, user, host);
	    free(nick);
	    free(user);
	    free(host);
	}
	for (access = ci->access, i = 0; i < ci->accesscount; ++access, ++i) {
	    if ((access->is_nick ? stricmp(access->name,mask) :
					strcmp(access->name,mask)) == 0) {
		if (access->level >= ulev) {
		    notice(s_ChanServ, source, "Access denied.");
		    return;
		}
		if (access->level == level) {
		    notice(s_ChanServ, source,
			"Access level for \2%s\2 on %s unchanged from \2%d\2.",
			access->name, chan, level);
		    return;
		}
		access->level = level;
		notice(s_ChanServ, source,
			"Access level for \2%s\2 on %s changed to \2%d\2.",
			access->name, chan, level);
		return;
	    }
	}
	for (access = ci->access, i = 0; i < ci->accesscount; ++access, ++i) {
	    if (access->is_nick < 0)
		break;
	}
	if (i == ci->accesscount) {
	    ++ci->accesscount;
	    ci->access =
		srealloc(ci->access, sizeof(ChanAccess) * ci->accesscount);
	    access = &ci->access[ci->accesscount-1];
	}
	access->name = ni ? sstrdup(ni->nick) : s;
	access->is_nick = (ni != NULL);
	access->level = level;
	notice(s_ChanServ, source,
		"\2%s\2 added to %s access list at level \2%d\2.",
		access->name, chan, level);

    } else if (stricmp(cmd, "DEL") == 0) {

#ifdef READONLY
    notice(s_ChanServ, source,
	    "Sorry, channel access list modification is temporarily disabled.");
    return;
#endif

	/* Special case: is it a number?  Only do search if it isn't. */
	if (strspn(mask, "1234567890") == strlen(mask) &&
				(i = atoi(mask)) > 0 && i <= ci->accesscount) {
	    --i;
	    access = &ci->access[i];
	    if (access->is_nick < 0) {
		notice(s_ChanServ, source,
			"No such entry (#%d) on %s access list.", i+1, chan);
		return;
	    }
	} else {
	    ni = findnick(mask);
	    if (!ni) {
		split_usermask(mask, &nick, &user, &host);
		mask = smalloc(strlen(nick)+strlen(user)+strlen(host)+3);
		sprintf(mask, "%s!%s@%s", nick, user, host);
		free(nick);
		free(user);
		free(host);
	    }
	    /* First try for an exact match; then, a case-insensitive one. */
	    for (access = ci->access, i = 0; i < ci->accesscount;
							++access, ++i) {
		if (access->is_nick >= 0 && strcmp(access->name, mask) == 0)
		    break;
	    }
	    if (i == ci->accesscount) {
		for (access = ci->access, i = 0; i < ci->accesscount;
							++access, ++i) {
		    if (access->is_nick >= 0 && stricmp(access->name, mask) == 0)
			break;
		}
	    }
	    if (i == ci->accesscount) {
		notice(s_ChanServ, source,
			"\2%s\2 not found on %s access list.", mask, chan);
		if (!ni)
		    free(mask);
		return;
	    }
	    if (!ni)
		free(mask);
	}
	if (ulev <= access->level) {
	    notice(s_ChanServ, source, "Permission denied.");
	} else {
	    notice(s_ChanServ, source,
		"\2%s\2 deleted from %s access list.", access->name, chan);
	    free(access->name);
	    access->is_nick = -1;
	}

    } else if (stricmp(cmd, "LIST") == 0) {

	notice(s_ChanServ, source, "Access list for %s:", chan);
	notice(s_ChanServ, source, "  Num   Lev  Mask");
	for (access = ci->access, i = 0; i < ci->accesscount; ++access, ++i) {
	    if (access->is_nick < 0)
		continue;
	    if ((mask && !match_wild(mask, access->name)) ||
					(level && level != access->level))
		continue;
	    if (ni = findnick(access->name))
		s = ni->last_usermask;
	    else
		s = NULL;
	    notice(s_ChanServ, source, "  %3d %6d  %s%s%s%s",
			i+1, access->level, access->name,
			s ? " (" : "", s ? s : "", s ? ")" : "");
	}

    } else {

	notice(s_ChanServ, source,
		"Syntax: \2ACCESS %s {ADD|DEL|LIST} [\37mask\37 [\37level\37]]\2", chan);
	notice(s_ChanServ, source,
		"\2/msg %s HELP ACCESS\2 for more information.", s_ChanServ);

    }
}

/*************************************************************************/

static void do_akick(const char *source)
{
    char *chan   = strtok(NULL, " ");
    char *cmd    = strtok(NULL, " ");
    char *mask   = strtok(NULL, " ");
    char *reason = strtok(NULL, "");
    char *t;
    ChannelInfo *ci;
    NickInfo *ni;
    User *u;
    int i;
    AutoKick *akick;
    Channel *c;
    struct c_userlist *u2;

    if (!cmd || (stricmp(cmd, "LIST") != 0 && !mask)) {

	notice(s_ChanServ, source,
		"Syntax: \2AKICK %s {ADD|DEL|LIST} [\37nick-or-usermask\37]\2",
		chan ? chan : "\37channel\37");
	notice(s_ChanServ, source,
		"\2/msg %s HELP AKICK\2 for more information.", s_ChanServ);

    } else if (!(ci = cs_findchan(chan))) {

	notice(s_ChanServ, source, "Channel %s is not registered.", chan);

    } else if (!(u = finduser(source)) || get_access(u, ci) < 10) {

	notice(s_ChanServ, source, "Access denied.");

    } else if (stricmp(cmd, "ADD") == 0) {

	NickInfo *ni = findnick(mask);
	char *nick, *user, *host;

#ifdef READONLY
    notice(s_ChanServ, source,
		"Sorry, channel AutoKick list modification is temporarily disabled.");
    return;
#endif

	if (ci->akickcount > AKICK_MAX) {
	    notice(s_ChanServ, source,
		    "Sorry, you can only have %d AutoKick masks on a channel.",
		    AKICK_MAX);
	    return;
	}

	if (!ni) {
	    split_usermask(mask, &nick, &user, &host);
	    mask = smalloc(strlen(nick)+strlen(user)+strlen(host)+3);
	    sprintf(mask, "%s!%s@%s", nick, user, host);
	    free(nick);
	    free(user);
	    free(host);
	}

	for (akick = ci->akick, i = 0; i < ci->akickcount; ++akick, ++i) {
	    if ((akick->is_nick ? stricmp(akick->name,mask) :
						strcmp(akick->name,mask)) == 0) {
		notice(s_ChanServ, source,
			"\2%s\2 already exists on %s AKICK list.",
			akick->name, chan);
		return;
	    }
	}


      /* Find @*, @*.*, @*.*.*, etc. and dissalow if !>20 */
    for(i=strlen(mask)-1;mask[i]=='!' || mask[i]=='*' || mask[i]=='?' || mask[i]=='.' ;i--) ;
    if(mask[i]=='@' && (get_access(u, ci) < 20))
	notice(s_ChanServ, source, "@* AKICK's are not allowed!!");
    else {
	++ci->akickcount;
	ci->akick = srealloc(ci->akick, sizeof(AutoKick) * ci->akickcount);
	akick = &ci->akick[ci->akickcount-1];
	akick->name = ni ? sstrdup(mask) : mask;
	akick->is_nick = (ni != NULL);
	akick->pad = 0;
	if (reason)
	    akick->reason = sstrdup(reason);
	else
	    akick->reason = NULL;
	notice(s_ChanServ, source,
		"\2%s\2 added to %s AKICK list.", akick->name, chan);

	/* This call should not fail, but check just in case... */
	if (c = findchan(chan)) {
	    char *av[3];

	    if (!reason)
		reason = DEF_AKICK_REASON;
	    for (u2 = c->users; u2; u2 = u2->next) {
		if (ni ? stricmp(u2->user->nick, akick->name) :
			 match_usermask(akick->name, u2->user)) {
		    send_cmd(s_ChanServ, "KICK %s %s :%s",
					u2->user->nick, chan, reason);
		    av[0] = sstrdup(u2->user->nick);
		    av[1] = sstrdup(c->name);
		    av[2] = sstrdup(reason);
		    do_kick(s_ChanServ, 3, av);
		    free(av[0]);
		    free(av[1]);
		    free(av[2]);
		}
	    }
	}
    }
    
    } else if (stricmp(cmd, "DEL") == 0) {

#ifdef READONLY
    notice(s_ChanServ, source,
		"Sorry, channel AutoKick list modification is temporarily disabled.");
    return;
#endif

	/* Special case: is it a number?  Only do search if it isn't. */
	if (strspn(mask, "1234567890") == strlen(mask) &&
				(i = atoi(mask)) > 0 && i <= ci->akickcount) {
	    --i;
	    akick = &ci->akick[i];
	} else {
	    /* First try for an exact match; then, a case-insensitive one. */
	    for (akick = ci->akick, i = 0; i < ci->akickcount; ++akick, ++i) {
		if (strcmp(akick->name, mask) == 0)
		    break;
	    }
	    if (i == ci->akickcount) {
		for (akick = ci->akick, i = 0; i < ci->akickcount; ++akick, ++i) {
		    if (stricmp(akick->name, mask) == 0)
			break;
		}
	    }
	    if (i == ci->akickcount) {
		notice(s_ChanServ, source,
			"\2%s\2 not found on %s AKICK list.", mask, chan);
		return;
	    }
	}
	notice(s_ChanServ, source,
		"\2%s\2 deleted from %s AKICK list.", akick->name, chan);
	free(akick->name);
	if (akick->reason)
	    free(akick->reason);
	--ci->akickcount;
	if (i < ci->akickcount)
	    bcopy(akick+1, akick, sizeof(AutoKick) * (ci->akickcount-i));
	if (ci->akickcount)
	    ci->akick = srealloc(ci->akick, sizeof(AutoKick) * ci->akickcount);
	else {
	    free(ci->akick);
	    ci->akick = NULL;
	}

    } else if (stricmp(cmd, "LIST") == 0) {

	notice(s_ChanServ, source, "AKICK list for %s:", chan);
	for (akick = ci->akick, i = 0; i < ci->akickcount; ++akick, ++i) {
	    if (mask && !match_wild(mask, akick->name))
		continue;
	    if (ni = findnick(akick->name))
		t = ni->last_usermask;
	    else
		t = NULL;
	    notice(s_ChanServ, source, "  %3d %s%s%s%s%s%s%s",
			i+1, akick->name,
			t ? " (" : "", t ? t : "", t ? ")" : "",
			akick->reason ? " (" : "",
			akick->reason ? akick->reason : "",
			akick->reason ? ")" : "");
	}

    } else {

	notice(s_ChanServ, source,
		"Syntax: \2AKICK %s {ADD|DEL|LIST} [\37nick-or-usermask\37]\2",
		chan);
	notice(s_ChanServ, source,
		"\2/msg %s HELP AKICK\2 for more information.", s_ChanServ);

    }
}

/*************************************************************************/

static void do_info(const char *source)
{
    char *chan = strtok(NULL, " ");
    ChannelInfo *ci;
    NickInfo *ni;
    struct tm tm;
    char s[32], *t, *end;
    time_t curtime = time(NULL);

    if (!chan) {

	notice(s_ChanServ, source, "Syntax: \2INFO \37channel\37\2");
	notice(s_ChanServ, source,
		"\2/msg %s HELP INFO\2 for more information.", s_ChanServ);

    } else if (!(ci = cs_findchan(chan))) {

	notice(s_ChanServ, source, "Channel \2%s\2 is not registered.", chan);

    } else if (ci->flags & CI_VERBOTEN) {

    	notice(s_ChanServ, source,
    		"Channel \2%s\2 may not be registered or used.", chan);

    } else {

	notice(s_ChanServ, source, "Information for channel \2%s\2:", chan);
	if (ni = findnick(ci->founder))
	    t = ni->last_usermask;
	else
	    t = NULL;
	notice(s_ChanServ, source,
		"        Founder: %s%s%s%s",
			ci->founder, t ? " (" : "", t ? t : "", t ? ")" : "");
	notice(s_ChanServ, source,
		"    Description: %s", ci->desc);
#if FILE_VERSION > 2
	if(strlen(ci->url)>0)
	    notice(s_ChanServ, source,
		    " WWW Page (URL): %s", ci->url);
#endif
	tm = *localtime(&ci->time_registered);
	notice(s_ChanServ, source,
		"     Registered: %s %2d %02d:%02d:%02d %d",
			month_name(tm.tm_mon+1), tm.tm_mday, tm.tm_hour,
			tm.tm_min, tm.tm_sec, tm.tm_year+1900);
	tm = *localtime(&ci->last_used);
	notice(s_ChanServ, source,
		"      Last used: %s %2d %02d:%02d:%02d %d",
			month_name(tm.tm_mon+1), tm.tm_mday, tm.tm_hour,
			tm.tm_min, tm.tm_sec, tm.tm_year+1900);
        tm = *localtime(&curtime);
	notice(s_ChanServ, source,
		"   Current time: %s %2d %02d:%02d:%02d %d\n",
			month_name(tm.tm_mon+1), tm.tm_mday, tm.tm_hour,
			tm.tm_min, tm.tm_sec, tm.tm_year+1900);
	if (ci->last_topic) {
	    notice(s_ChanServ, source,
		"     Last topic: %s", ci->last_topic);
	    notice(s_ChanServ, source,
		"   Topic set by: %s\n", ci->last_topic_setter);
	}
	if (!ci->flags)
	    sprintf(s, "None\n");
	else {
	    int need_comma = 0;
	    static const char commastr[] = ", ";
	    end = s;
	    if (ci->flags & CI_PRIVATE) {
		end += sprintf(end, "Private");
		need_comma = 1;
	    }
	    if (ci->flags & CI_KEEPTOPIC) {
		end += sprintf(end, "%sTopic Retention", need_comma ? commastr : "");
		need_comma = 1;
	    }
	    if (ci->flags & CI_TOPICLOCK) {
		end += sprintf(end, "%sTopic Lock", need_comma ? commastr : "");
		need_comma = 1;
	    }
	    if (ci->flags & CI_SECUREOPS) {
		end += sprintf(end, "%sSecure Ops", need_comma ? commastr : "");
		need_comma = 1;
	    }
	    if (ci->flags & CI_RESTRICTED) {
		end += sprintf(end, "%sRestricted Access", need_comma ? commastr : "");
		need_comma = 1;
	    }
	    if (ci->flags & CI_SECURE) {
		printf("%sSecure", need_comma ? commastr : "");
		need_comma = 1;
	    }
	}
	notice(s_ChanServ, source, "        Options: %s", s);
	end = s;
	*end = 0;
	if (ci->mlock_on || ci->mlock_key || ci->mlock_limit)
	    end += sprintf(end, "+%s%s%s%s%s%s%s%s",
				(ci->mlock_on & CMODE_I) ? "i" : "",
				(ci->mlock_key         ) ? "k" : "",
				(ci->mlock_limit       ) ? "l" : "",
				(ci->mlock_on & CMODE_M) ? "m" : "",
				(ci->mlock_on & CMODE_N) ? "n" : "",
				(ci->mlock_on & CMODE_P) ? "p" : "",
				(ci->mlock_on & CMODE_S) ? "s" : "",
				(ci->mlock_on & CMODE_T) ? "t" : "");
	if (ci->mlock_off)
	    end += sprintf(end, "-%s%s%s%s%s%s%s%s",
				(ci->mlock_off & CMODE_I) ? "i" : "",
				(ci->mlock_off & CMODE_K) ? "k" : "",
				(ci->mlock_off & CMODE_L) ? "l" : "",
				(ci->mlock_off & CMODE_M) ? "m" : "",
				(ci->mlock_off & CMODE_N) ? "n" : "",
				(ci->mlock_off & CMODE_P) ? "p" : "",
				(ci->mlock_off & CMODE_S) ? "s" : "",
				(ci->mlock_off & CMODE_T) ? "t" : "");
	notice(s_ChanServ, source,
		"      Mode lock: %s", s);
    }
}

/*************************************************************************/

static void do_list(const char *source)
{
    char *chan = strtok(NULL, " ");
    ChannelInfo *ci;
    int nchans, i;
    char buf[512];

    if (!chan) {

	notice(s_ChanServ, source, "Syntax: \2LIST \37pattern\37\2");
	notice(s_ChanServ, source,
		"\2/msg %s HELP LIST\2 for more information.", s_ChanServ);

    } else {

	nchans = 0;
	notice(s_ChanServ, source, "List of entries matching \2%s\2:", chan);
	for (i = 33; i < 256; ++i) {
	    for (ci = chanlists[i]; ci; ci = ci->next) {
		if (ci->flags & (CI_PRIVATE | CI_VERBOTEN))
		    continue;
		if (strlen(ci->name)+strlen(ci->desc) > sizeof(buf))
		    continue;
		sprintf(buf, "%-20s  %s", ci->name, ci->desc);
		if (match_wild(chan, buf)) {
		    if (++nchans <= 50)
			notice(s_ChanServ, source, "    %s", buf);
		}
	    }
	}
	notice(s_ChanServ, source, "End of list - %d/%d matches shown.",
					nchans>50 ? 50 : nchans, nchans);
    }

}

/*************************************************************************/

static void do_invite(const char *source)
{
    char *chan = strtok(NULL, " ");
    User *u = finduser(source);
    ChannelInfo *ci;

    if (!chan) {

	notice(s_ChanServ, source, "Syntax: \2INVITE \37channel\37\2");
	notice(s_ChanServ, source,
		"\2/msg %s HELP INVITE\2 for more information.", s_ChanServ);

    } else if (!findchan(chan)) {

	notice(s_ChanServ, source, "Channel %s does not exist.", chan);

    } else if (!(ci = cs_findchan(chan))) {

	notice(s_ChanServ, source, "Channel %s is not registered.", chan);

    } else if (!u || get_access(u, ci) < 1) {

	notice(s_ChanServ, source, "Access denied.");

    } else {

	send_cmd(s_ChanServ, "INVITE %s %s", source, chan);

    }
}

/*************************************************************************/

static void do_op(const char *source)
{
    char *chan = strtok(NULL, " ");
    char *op_params = strtok(NULL, " ");  
    User *u = finduser(source);
    ChannelInfo *ci;
    int ulev;

    if (!chan || !op_params) {
	notice(s_ChanServ, source,
		"Syntax: \2OP\2 \037channel\037 \037nick\037");
	notice(s_ChanServ, source,
		"\2/msg ChanServ HELP OP\2 for more information.");

    } else if (!(ci = cs_findchan(chan))) {
	notice(s_ChanServ, source, "Channel %s is not registered.", chan);

    } else if (!u || (ulev = get_access(u, ci)) < 5) {
	notice(s_ChanServ, source, "Access denied.");

    } else {
	send_cmd(s_ChanServ, "MODE %s +o %s  0", chan, op_params);

    }
}

/*************************************************************************/

static void do_deop(const char *source)
{
    char *chan = strtok(NULL, " ");
    char *deop_params = strtok(NULL, " ");
    User *u = finduser(source);
    ChannelInfo *ci;
    int ulev;

    if (!chan || !deop_params) {
	notice(s_ChanServ, source,
		"Syntax: \2DEOP\2 \037channel\037 \037nick\037");
	notice(s_ChanServ, source,
		"\2/msg ChanServ HELP DEOP\2 for more information.");

    } else if (!(ci = cs_findchan(chan))) {
	notice(s_ChanServ, source, "Channel %s is not registered.", chan);

    } else if (!u || (ulev = get_access(u, ci)) < 5) {
	notice(s_ChanServ, source, "Access denied.");

    } else {
	send_cmd(s_ChanServ, "MODE %s -o %s  0", chan, deop_params);

    }
}

/*************************************************************************/

static void do_unban(const char *source)
{
    char *chan = strtok(NULL, " ");
    User *u = finduser(source);
    ChannelInfo *ci;
    Channel *c;
    int i;
    char *av[3];

    if (!chan) {

	notice(s_ChanServ, source, "Syntax: \2UNBAN \37channel\37\2");
	notice(s_ChanServ, source,
		"\2/msg %s HELP UNBAN\2 for more information.", s_ChanServ);

    } else if (!(c = findchan(chan))) {

	notice(s_ChanServ, source, "Channel %s does not exist.", chan);

    } else if (!(ci = cs_findchan(chan))) {

	notice(s_ChanServ, source, "Channel %s is not registered.", chan);

    } else if (!u || get_access(u, ci) < 1) {

	notice(s_ChanServ, source, "Access denied.");

    } else {

	av[0] = chan;
	av[1] = sstrdup("-b");
	for (i = 0; i < c->bancount; ++i) {
	    if (match_usermask(c->bans[i], u)) {
		send_cmd(s_ChanServ, "MODE %s -b %s", chan, c->bans[i]);
		av[2] = sstrdup(c->bans[i]);
		do_cmode(s_ChanServ, 3, av);
		free(av[2]);
	    }
	}
	free(av[1]);
	notice(s_ChanServ, source, "You have been unbanned from %s.", chan);

    }
}

/*************************************************************************/

static void do_clear(const char *source)
{
    char *chan = strtok(NULL, " ");
    char *what = strtok(NULL, " ");
    User *u = finduser(source);
    Channel *c;
    ChannelInfo *ci;

    if (!what) {

	notice(s_ChanServ, source,
		"Syntax: \2CLEAR \37channel\37 \37what\37\2");
	notice(s_ChanServ, source,
		"\2/msg %s HELP CLEAR\2 for more information.", s_ChanServ);

    } else if (!(c = findchan(chan))) {

	notice(s_ChanServ, source, "Channel %s does not exist.", chan);

    } else if (!(ci = cs_findchan(chan))) {

	notice(s_ChanServ, source, "Channel %s is not registered.", chan);

    } else if (!u || get_access(u, ci) < 10) {

	notice(s_ChanServ, source, "Access denied.");

    } else if (stricmp(what, "bans") == 0) {

	char *av[3];
	int i;

	for (i = 0; i < c->bancount; ++i) {
	    av[0] = sstrdup(chan);
	    av[1] = sstrdup("-b");
	    av[2] = sstrdup(c->bans[i]);
	    do_cmode(s_ChanServ, 3, av);
	    send_cmd(s_ChanServ, "MODE %s %s :%s", av[0], av[1], av[2]);
	    free(av[2]);
	    free(av[1]);
	    free(av[0]);
	}
	notice(s_ChanServ, source, "All bans on %s have been removed.", chan);

    } else if (stricmp(what, "modes") == 0) {

	char *av[3];

	av[0] = chan;
	av[1] = sstrdup("-mintpslk");
	if (c->key)
	    av[2] = sstrdup(c->key);
	else
	    av[2] = sstrdup("");
	do_cmode(s_ChanServ, 3, av);
	send_cmd(s_ChanServ, "MODE %s %s :%s", av[0], av[1], av[2]);
	free(av[2]);
	free(av[1]);
	notice(s_ChanServ, source, "All modes on %s have been reset.", chan);

    } else if (stricmp(what, "ops") == 0) {

	char *av[3];
	struct c_userlist *cu, *next;

	for (cu = c->chanops; cu; cu = next) {
	    next = cu->next;
	    av[0] = sstrdup(chan);
	    av[1] = sstrdup("-o");
	    av[2] = sstrdup(cu->user->nick);
	    do_cmode(s_ChanServ, 3, av);
	    send_cmd(s_ChanServ, "MODE %s %s :%s", av[0], av[1], av[2]);
	    free(av[2]);
	    free(av[1]);
	    free(av[0]);
	}
	notice(s_ChanServ, source, "Mode +o cleared from %s.", chan);

    } else if (stricmp(what, "voices") == 0) {

	char *av[3];
	struct c_userlist *cu, *next;

	for (cu = c->voices; cu; cu = next) {
	    next = cu->next;
	    av[0] = sstrdup(chan);
	    av[1] = sstrdup("-o");
	    av[2] = sstrdup(cu->user->nick);
	    do_cmode(s_ChanServ, 3, av);
	    send_cmd(s_ChanServ, "MODE %s %s :%s", av[0], av[1], av[2]);
	    free(av[2]);
	    free(av[1]);
	    free(av[0]);
	}
	notice(s_ChanServ, source, "Mode +v cleared from %s.", chan);

    } else if (stricmp(what, "users") == 0) {

	char *av[3];
	struct c_userlist *cu, *next_cu;
	char buf[256];

	snprintf(buf, sizeof(buf), "CLEAR USERS command from %s", source);

	for (cu = c->users; cu; cu = next_cu) {
	    next_cu = cu->next;
	    av[0] = sstrdup(chan);
	    av[1] = sstrdup(cu->user->nick);
	    av[2] = sstrdup(buf);
	    do_kick(s_ChanServ, 3, av);
	    send_cmd(s_ChanServ, "KICK %s %s :%s", av[0], av[1], av[2]);
	    free(av[2]);
	    free(av[1]);
	    free(av[0]);
	}
	notice(s_ChanServ, source, "All users kicked from %s.", chan);

    } else {

	notice(s_ChanServ, source,
		"Syntax: \2CLEAR \37channel\37 \37what\37\2");
	notice(s_ChanServ, source,
		"\2/msg %s HELP CLEAR\2 for more information.", s_ChanServ);

    }
}

/*************************************************************************/

static void do_getpass(const char *source)
{
    char *chan = strtok(NULL, " ");
    User *u = finduser(source);
    ChannelInfo *ci;

    if (!chan) {

	notice(s_ChanServ, source, "Syntax: \2GETPASS \37channel\37\2");

    } else if (!(ci = cs_findchan(chan))) {

	notice(s_ChanServ, source, "Channel %s is not registered.", chan);

    } else if (!u) {

	notice(s_ChanServ, source, "Could not find your user record!");

    } else {

	log("%s: %s!%s@%s used GETPASS on %s",
		s_ChanServ, source, u->username, u->host, chan);
	wallops("\2%s\2 used GETPASS on channel \2%s\2", source, chan);
	notice(s_ChanServ, source, "Password for %s is: \2%s\2",
		chan, ci->founderpass);

    }
}

/*************************************************************************/

static void do_forbid(const char *source)
{
    ChannelInfo *ci;
    char *chan = strtok(NULL, " ");

    if (!chan) {
	notice(s_ChanServ, source, "Syntax: \2FORBID \37channel\37\2");
	return;
    }
#ifdef READONLY
    notice(s_ChanServ, source,
	"Warning: Services is in read-only mode.  Changes will not be saved.");
#endif
    if (ci = cs_findchan(chan))
	delchan(ci);
    if (ci = makechan(chan)) {
    	log("%s: %s set FORBID for channel %s", s_ChanServ, source, chan);
	ci->flags |= CI_VERBOTEN;
	notice(s_ChanServ, source,
		"Channel \2%s\2 has been marked FORBIDden.", chan);
    } else {
    	log("%s: Valid FORBID for %s by %s failed", s_ChanServ,
    		chan, source);
	notice(s_ChanServ, source,
		"Couldn't FORBID channel \2%s\2!", chan);
    }
}

/*************************************************************************/

#endif	/* !SKELETON */
