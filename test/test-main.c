/* Test suite for Services
 *
 * Usage: test-services -h                        [list usage and all tests]
 *    or  test-services [-v] <test-number> <services-executable>  [run test]
 */

#include "test.h"

#include "test-list.c"

int verbose = 0;

/*************************************************************************/

/* Quickie little routines to get child's signal or alarm. */

static int died, timedout;
static jmp_buf jbuf;
void sigusr1(int sig_unused) { died = 1; signal(SIGUSR1, SIG_DFL); }
void sigalrm(int sig_unused) { timedout = 1; signal(SIGALRM, SIG_DFL); longjmp(jbuf, 1); }

/* run_test - start up a copy of Services and run a test.  (Always start
 * with a clean copy of Services so any earlier test failures don't affect
 * the results.)  Return 0 if passed the test, 1 if failed, 2 on other
 * error. */

int run_test(int which, char *services_exec)
{
    int port, sock, pid, res, i;
    int argc;
    char **argv;
    struct sockaddr_in sin;
    int len = sizeof(sin);

    --which;
    if (which < 0 || which >= sizeof(test_list)/sizeof(struct test)) {
	fprintf(stderr, "invalid test number: %s\n", which+1);
	return 2;
    }

    fprintf(stderr, "%-40s -- ", test_list[which].desc);
    fflush(stderr);

    /* Be ready to listen for exec() failure */
    died = 0;
    signal(SIGUSR1, sigusr1);

    /* Get the local listen-port number and socket descriptor */
    sock = create_socket(&port);
    if (sock < 0) {
	perror("create_socket()");
	return 2;
    }

    /* Start up Services on that port */
    switch (pid = fork()) {
      case -1:
	perror("fork()");
	close(sock);
	return 2;
      case 0: {
	char buf[256];
	signal(SIGUSR1, SIG_DFL);
	sprintf(buf, "127.0.0.1:%d", port);
	execl(services_exec, services_exec,
				"-remote", buf, "-dir", DATA_DIR, NULL);
	perror("exec()");
	if ((pid = getppid()) != 1)	/* don't accidentally signal init */
	    kill(pid, SIGUSR1);
	exit(2);
      }
    }

    /* Wait for Services to connect to us */
    i = accept(sock, (struct sockaddr *)&sin, &len);
    close(sock);
    sock = i;
    if (sock < 0) {
	if (!died)
	    perror("accept()");
	return 2;
    }

    send_cmd(sock, NULL, "NICK Alcan 1 8 achurch Bahamut.dragonfire.net dragonfire.esper.net :Andy Church");
    send_cmd(sock, "Alcan", "MODE Alcan +o");
    sleep(1); /* Need these sleep() calls to prevent ignore from kicking in. */
    send_cmd(sock, "Alcan", "PRIVMSG NickServ :IDENTIFY mypass");
    sleep(1);
    send_cmd(sock, "Alcan", "PRIVMSG OperServ :SET IGNORE OFF");
    while ((argc = read_cmd(sock, &argv)) > 0
                && (argc < 4 || strncmp(argv[3], "Ignore ", 7) != 0))
        ;
    if (argc <= 0)
	return 2;

    signal(SIGUSR1, SIG_DFL);

    if (!setjmp(jbuf))
	res = test_list[which].proc(sock);
    if (timedout) {
	res = 2;
    } else if (!res) {
	send_cmd(sock, "Alcan", "PRIVMSG OperServ QUIT");
	argc = read_cmd(sock, &argv);
	if (argc != 4 || stricmp(argv[1], "SQUIT") != 0)
	    res = 1;
    }
    if (res == 2)
	fprintf(stderr, "timed out\n");
    else if (res == 0)
	fprintf(stderr, "passed\n");
    else
	fprintf(stderr, "failed\n");
    close(sock);
    kill(pid, SIGKILL);
    return res;
}

/*************************************************************************/

int main(int ac, char **av)
{
    int which_test, res, i;
    char *services_exec;

    if (!((ac==4 && strcmp(av[1],"-v")==0)
	    || ac==3
	    || (ac==2 && strcmp(av[1],"-h")==0))) {
	fprintf(stderr, "\
Usage: %s [-v] <test-number> <services-executable>\n\
   or  %s -h\n", av[0], av[0]);
	return 1;
    }

    if (ac == 2) {
	printf("\
Test suite for Services\n\
\n\
Usage: %s -h\n\
   or  %s [-v] <test-number> <services-executable>\n\
\n\
-v causes all messages to and from Services to be printed.
\n\
<services-executable> is the full path to the Services executable.\n\
\n\
<test-number> is 0 to execute all tests, or one of:\n", av[0], av[0]);
	for (i = 0; i < sizeof(test_list)/sizeof(struct test); ++i) {
	    if (test_list[i].desc)
		printf("  %4d  %s\n", i+1, test_list[i].desc);
	}
	return 0;
    }

    if (ac == 4) {
	verbose = 1;
	++av;
    }
    which_test = atoi(av[1]);
    services_exec = av[2];

    if (which_test)
	res = run_test(which_test, services_exec);
    else {
	int i;
	res = 0;
	for (i = 0; i < sizeof(test_list)/sizeof(struct test); ++i) {
	    if (test_list[i].proc)
		res |= run_test(i+1, services_exec);
	}
    }

    return res;
}
