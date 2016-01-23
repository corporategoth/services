/* Socket and other functions for Services test suite */

#include "test.h"

/*************************************************************************/

int stricmp(const char *s1, const char *s2)
{
    while (*s1 && *s2 && tolower(*s1) == tolower(*s2)) {
	++s1; ++s2;
    }
    return *s1==*s2 ? 0 : tolower(*s1)<tolower(*s2) ? -1 : 1;
}

/*************************************************************************/

/* Create a listening socket; return its socket descriptor, and the port
 * number by side effect. */

int create_socket(int *port)
{
    struct hostent *he;
    struct sockaddr_in sin;
    int len = sizeof(sin);
    int s;

    if (!(he = gethostbyname("localhost")))
	return -1;
    memset(&sin, 0, sizeof(sin));
    memcpy(&sin.sin_addr, he->h_addr, he->h_length);
    sin.sin_family = he->h_addrtype;
    sin.sin_port = htons(0);
    if ((s = socket(he->h_addrtype, SOCK_STREAM, 0)) < 0)
	return -1;
    if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0
		|| getsockname(s, (struct sockaddr *)&sin, &len) < 0
		|| listen(s, 1) < 0) {
	close(s);
	return -1;
    }
    *port = ntohs(sin.sin_port);
    return s;
}

/*************************************************************************/

/* read_cmd - read a command from the socket and split it up into
 * arguments.  Return the number of arguments read or -1 on error. */

int read_cmd(int sock, char ***pargv)
{
    int argc = 0;
    char *s, *t;
    static char buf[1024];
    static char *argv[64];

    if (!sgets(buf, sizeof(buf), sock))
	return -1;
    if (verbose)
	fputs(buf, stdout);
    for (s = t = buf; *s; s = t) {
	if (*t == ':' && argc > 0) {
	    ++s;
	    if (!(t = strpbrk(t, "\r\n")))
		t += strlen(t);
	    *t = 0;
	} else {
	    t += strcspn(t, " \r\n");
	    if (*t) {
		*t++ = 0;
		t += strspn(t, "\r\n");
	    }
	}
	argv[argc++] = s;
	if (argc == sizeof(argv)/sizeof(*argv))
	    break;
    }
    if (pargv)
	*pargv = argv;
    return argc;
}

/*************************************************************************/

/* send_cmd - send a command through the socket. */

void send_cmd(int sock, char *source, char *fmt, ...)
{
    va_list args;
    char buf[4096];

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    if (source) {
	sockprintf(sock, ":%s %s\r\n", source, buf);
	if (verbose)
	    printf("--> :%s %s\n", source, buf);
    } else {
	sockprintf(sock, "%s\r\n", buf);
	if (verbose)
	    printf("--> %s\n", buf);
    }
}
