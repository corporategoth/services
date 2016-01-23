/* Main include file for Services test suite */

#ifndef TEST_H
#define TEST_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>

/* Directory containing test data */
#ifndef DATA_DIR
# error Please define DATA_DIR
#endif

/*************************************************************************/

/* Data for each test routine */

struct test {
    int (*proc)(int sock);
    char *desc;
};

/*************************************************************************/

#define E extern

/**** test-main.c ****/

E int verbose;


/**** test-misc.c ****/

E int stricmp(const char *s1, const char *s2);
E int create_socket(int *port);
E int read_cmd(int sock, char ***pargv);
E void send_cmd(int sock, char *source, char *fmt, ...);


/**** ../sockutil.c ****/

E int sgetc(int s);
E char *sgets(char *buf, unsigned int len, int s);
E int sputs(char *str, int s);
E int sockprintf(int s, char *fmt,...);
E int conn(char *host, int port);
E void disconn(int s);

/*************************************************************************/

#endif	/* TEST_H */
