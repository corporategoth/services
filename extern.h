/* Prototypes and external variable declarations.
 *
 * Services is copyright (c) 1996-1997 Preston A. Elder.
 *     E-mail: <prez@antisocial.com>   IRC: PreZ@DarkerNet
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

#ifndef EXTERN_H
#define EXTERN_H


#define E extern


/**** channels.c ****/

E Channel *chanlist;

E void get_channel_stats(long *nrec, long *memuse);
#ifdef OPERSERV
E void send_channel_list(const char *user, const char *s);
E void send_channel_users(const char *user, const char *chan);
#endif
E Channel *findchan(const char *chan);
E void chan_adduser(User *user, const char *chan);
E void chan_deluser(User *user, Channel *c);
E void do_cmode(const char *source, int ac, char **av);
E void do_topic(const char *source, int ac, char **av);
#ifdef CHANSERV
E void do_chcmode(const char *who, const char *chan, const char *mode, const char *pram);
#endif

/**** chanserv.c ****/

#ifdef CHANSERV
E const char s_ChanServ[];
E int def_access[];
#ifdef NEWS
E ChannelInfo *chanlists[256];
#endif

E void listchans(int count_only, const char *chan);
E void get_chanserv_stats(long *nrec, long *memuse);

E int get_access(User *user, ChannelInfo *ci);
E int is_founder(User *user, NickInfo *ni, ChannelInfo *ci);
E void chanserv(const char *source, char *buf);
E void load_cs_dbase(void);
E void save_cs_dbase(void);
E void check_modes(const char *chan);
E int check_valid_op(User *user, const char *chan, int newchan);
E int check_should_op(User *user, const char *chan);
E int check_akick(User *user, const char *chan);
E void record_topic(const char *chan);
E void restore_topic(const char *chan);
E int check_topiclock(const char *chan);
E void expire_chans(void);
E ChannelInfo *cs_findchan(const char *chan);
#endif

/**** helpserv.c ****/

E const char s_HelpServ[];

E void helpserv(const char *whoami, const char *source, char *buf);


/**** main.c ****/

E char *remote_server;
E int remote_port;
E char *server_name;
E char *server_desc;
E char *services_user;
E char *services_host;
E char *services_dir;
E char *log_filename;
E char *time_zone;
E int update_timeout;
E int debug;
E int services_level;

E int mode;
E char *offreason;
E int quitting;
E int terminating;
E char *quitmsg;
E char inbuf[BUFSIZE];
E int servsock;
E int save_data;
E int got_alarm;
E gid_t file_gid;
E time_t start_time;

E void log(const char *fmt,...);
E void log_perror(const char *fmt,...);
E void fatal(const char *fmt,...);
E void fatal_perror(const char *fmt,...);
E void check_file_version(FILE *f, const char *filename);
E void write_file_version(FILE *f, const char *filename);
E int is_services_nick(const char *nick);
E void introduce_users(const char *user);
E int is_server(const char *nick);

/**** memoserv.c ****/

#ifdef MEMOSERV
E const char s_MemoServ[];

E void get_memoserv_stats(long *nrec, long *memuse);

E void memoserv(const char *source, char *buf);
# ifdef MEMOS
E MemoList *memolists[256];
E void load_ms_dbase(void);
E void save_ms_dbase(void);
E void check_memos(const char *nick);
E MemoList *find_memolist(const char *nick);
# endif
# ifdef NEWS
E NewsList *newslists[256];
E void load_news_dbase(void);
E void save_news_dbase(void);
E void check_newss(const char *chan, const char *source);
E void expire_news(void);
E NewsList *find_newslist(const char *chan);
# endif
#endif

/**** misc.c ****/

E char *sgets2(char *buf, long size, int sock);
E char *strscpy(char *d, const char *s, int len);
#ifdef NEED_STRICMP
E int stricmp(const char *s1, const char *s2);
E int strnicmp(const char *s1, const char *s2, int len);
#endif
#ifdef NEED_STRDUP
E char *strdup(const char *s);
#endif
#ifdef NEED_STRSPN
E size_t strspn(const char *s, const char *accept);
#endif
E char *stristr(char *s1, char *s2);
#ifdef NEED_STRERROR
E char *strerror(int errnum);
#endif
E void *smalloc(long size);
E void *scalloc(long elsize, long els);
E void *srealloc(void *oldptr, long newsize);
E char *sstrdup(const char *s);
E char *merge_args(int argc, char **argv);
E int match_wild(const char *pattern, const char *str);
E char *month_name(int month);
E char *strupper(char *s);
E char *strlower(char *s);
E char *read_string(FILE *f, const char *filename);
E char *write_string(const char *s, FILE *f, const char *filename);


/**** nickserv.c ****/

#ifdef NICKSERV
E const char s_NickServ[];
E NickInfo *nicklists[256];
E Timeout *timeouts;

E void listnicks(int count_only, const char *nick);
E void get_nickserv_stats(long *nrec, long *memuse);

E void nickserv(const char *source, char *buf);
E void load_ns_dbase(void);
E void save_ns_dbase(void);
E int validate_user(User *u);
E void cancel_user(User *u);
E void check_timeouts(void);
E void expire_nicks(void);
E NickInfo *findnick(const char *nick);
#if (FILE_VERSION > 3) && defined(MEMOS)
    E int is_on_ignore(const char *source, char *target);
#endif
#endif

/**** operserv.c ****/

#ifdef GLOBALNOTICER
E const char s_GlobalNoticer[];
#endif
#ifdef OPERSERV
E const char s_OperServ[];

E void operserv(const char *source, char *buf);
#ifdef AKILL
E Akill *akills;
E int nakill;
E int akill_size;
E void load_akill(void);
E void save_akill(void);
E int check_akill(const char *nick, const char *username, const char *host);
E void expire_akill(void);
#endif
#ifdef CLONES
E Clone *clonelist;
E Allow *clones;
E int nclone;
E int clone_size;
E void clone_add(const char *nick, const char *host);
E void clone_del(const char *host);
#endif
#endif

/**** process.c ****/

E int allow_ignore;
E IgnoreData *ignore[];

E void add_ignore(const char *nick, time_t delta);
E IgnoreData *get_ignore(const char *nick);
E int split_buf(char *buf, char ***argv, int colon_special);
E void process(void);


/**** send.c ****/

E void send_cmd(const char *source, const char *fmt, ...);
E void vsend_cmd(const char *source, const char *fmt, va_list args);
E void wallops(const char *fmt, ...);
E void notice(const char *source, const char *dest, const char *fmt, ...);
E void notice_list(const char *source, const char *dest, const char **text);
E void privmsg(const char *source, const char *dest, const char *fmt, ...);


/**** sockutil.c ****/

E int sgetc(int s);
E char *sgets(char *buf, unsigned int len, int s);
E int sputs(char *str, int s);
E int sockprintf(int s, char *fmt,...);
E int conn(char *host, int port);
E void disconn(int s);


/**** users.c ****/

E int usercnt, opcnt, maxusercnt;
E User *userlist;

#ifdef OPERSERV
E void send_user_list(const char *user, const char *s);
#endif
E void get_user_stats(long *nusers, long *memuse);
E User *finduser(const char *nick);
E int findakill(const char *mask, const char *reason);

E void change_user_nick(User *u, const char *nick);
E void do_nick(const char *source, int ac, char **av);
E void do_join(const char *source, int ac, char **av);
E void do_part(const char *source, int ac, char **av);
E void do_kick(const char *source, int ac, char **av);
E void do_umode(const char *source, int ac, char **av);
E void do_quit(const char *source, int ac, char **av);
E void do_kill(const char *source, int ac, char **av);

E int is_services_op(const char *nick);
E int is_oper(const char *nick);
E int is_on_chan(const char *nick, const char *chan);
E int is_chanop(const char *nick, const char *chan);
E int is_voiced(const char *nick, const char *chan);

E int match_usermask(const char *mask, User *user);
E void split_usermask(const char *mask, char **nick, char **user, char **host);
E char *create_mask(User *u);


#endif	/* EXTERN_H */
