/* Channel-based tests */

#include "test.h"

/*************************************************************************/

/* Test JOIN command to join one channel.  Expected behavior: channel is
 * created if it does not exist, nick is added to channel list, and
 * channel is added to nick's channel list. */
/* Requires: test_nick */

int test_join_one(int sock)
{
    int argc, i, count;
    char **argv;
    char *s;

    send_cmd(sock, NULL, "NICK Alcan_ 2 17 root Redwing.airship.net airship.esper.net :Ralph Oot");

    send_cmd(sock, NULL, ":Alcan JOIN #testchan");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    for (i = 0; i < 2; ++i) {
	if ((argc = read_cmd(sock, &argv)) != 4)
	    return 1;
	if (strncmp(argv[3], "Alcan!", 6) == 0) {
	    if (strcmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan #testchan") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan") != 0)
		return 1;
	} else if (strncmp(argv[3], "Alcan_!", 7) == 0) {
	    if (strcmp(argv[3],
"Alcan_!root@Redwing.airship.net + 17 airship.esper.net :Ralph Oot") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan_") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan_") != 0)
		return 1;
	} else {
	    return 1;
	}
    }
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTCHANS");
    if ((argc = read_cmd(sock, &argv)) != 4)
	return 1;
    if (strncmp(argv[3], "#testchan ", 10) == 0) {
	if (!(s = strchr(argv[3], '+')) || strcmp(s, "+ ") != 0)
	    return 1;
	if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "#testchan Alcan"))
	    return 1;
    } else {
	return 1;
    }

    send_cmd(sock, "Alcan_", "JOIN #testchan");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    for (i = 0; i < 2; ++i) {
	if ((argc = read_cmd(sock, &argv)) != 4)
	    return 1;
	if (strncmp(argv[3], "Alcan!", 6) == 0) {
	    if (strcmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan #testchan") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan") != 0)
		return 1;
	} else if (strncmp(argv[3], "Alcan_!", 7) == 0) {
	    if (strcmp(argv[3],
"Alcan_!root@Redwing.airship.net + 17 airship.esper.net :Ralph Oot") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan_ #testchan") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan_") != 0)
		return 1;
	} else {
	    return 1;
	}
    }
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTCHANS");
    if ((argc = read_cmd(sock, &argv)) != 4)
	return 1;
    if (strncmp(argv[3], "#testchan ", 10) == 0) {
	if (!(s = strchr(argv[3], '+')) || strcmp(s, "+ ") != 0)
	    return 1;
	if ((argc = read_cmd(sock, &argv)) != 4)
	    return 1;
	s = strtok(argv[3], " ");
	count = 0;
	while (s = strtok(NULL, " ")) {
	    ++count;
	    if (strcmp(s, "Alcan") != 0 && strcmp(s, "Alcan_") != 0)
		return 1;
	}
	if (count != 2)
	    return 1;
    } else {
	return 1;
    }

    return 0;
}

/*************************************************************************/

/* Test JOIN command to join multiple channels.  Expected behavior: as for
 * test_join_one, except that the actions are performed for each channel in
 * the list. */
/* Requires: test_nick */

int test_join_multi(int sock)
{
    int argc, i, count;
    char **argv;
    char *s;

    send_cmd(sock, NULL, "NICK Alcan_ 2 17 root Redwing.airship.net airship.esper.net :Ralph Oot");

    send_cmd(sock, NULL, ":Alcan JOIN #testchan,#test2");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    for (i = 0; i < 2; ++i) {
	if ((argc = read_cmd(sock, &argv)) != 4)
	    return 1;
	if (strncmp(argv[3], "Alcan!", 6) == 0) {
	    if (strcmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4)
		return 1;
	    s = strtok(argv[3], " ");
	    count = 0;
	    while (s = strtok(NULL, " ")) {
	    	++count;
		if (strcmp(s, "#testchan") != 0 && strcmp(s, "#test2") != 0)
		    return 1;
	    }
	    if (count != 2)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan") != 0)
		return 1;
	} else if (strncmp(argv[3], "Alcan_!", 7) == 0) {
	    if (strcmp(argv[3],
"Alcan_!root@Redwing.airship.net + 17 airship.esper.net :Ralph Oot") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan_") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan_") != 0)
		return 1;
	} else {
	    return 1;
	}
    }
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTCHANS");
    for (i = 0; i < 2; ++i) {
	if ((argc = read_cmd(sock, &argv)) != 4)
	    return 1;
	if (strncmp(argv[3], "#testchan ", 10) == 0) {
	    if (!(s = strchr(argv[3], '+')) || strcmp(s, "+ ") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "#testchan Alcan"))
		return 1;
	} else if (strncmp(argv[3], "#test2 ", 7) == 0) {
	    if (!(s = strchr(argv[3], '+')) || strcmp(s, "+ ") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "#test2 Alcan") != 0)
		return 1;
	} else {
	    return 1;
	}
    }

    send_cmd(sock, "Alcan_", "JOIN #test3,#testchan,#test2");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    for (i = 0; i < 2; ++i) {
	if ((argc = read_cmd(sock, &argv)) != 4)
	    return 1;
	if (strncmp(argv[3], "Alcan!", 6) == 0) {
	    if (strcmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4)
		return 1;
	    s = strtok(argv[3], " ");
	    count = 0;
	    while (s = strtok(NULL, " ")) {
		++count;
		if (strcmp(s, "#testchan") != 0 && strcmp(s, "#test2") != 0)
		    return 1;
	    }
	    if (count != 2)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan") != 0)
		return 1;
	} else if (strncmp(argv[3], "Alcan_!", 7) == 0) {
	    if (strcmp(argv[3],
"Alcan_!root@Redwing.airship.net + 17 airship.esper.net :Ralph Oot") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4)
		return 1;
	    s = strtok(argv[3], " ");
	    count = 0;
	    while (s = strtok(NULL, " ")) {
		++count;
		if (strcmp(s, "#test3") != 0 && strcmp(s, "#testchan") != 0 && strcmp(s, "#test2") != 0)
		    return 1;
	    }
	    if (count != 3)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan_") != 0)
		return 1;
	} else {
	    return 1;
	}
    }
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTCHANS");
    for (i = 0; i < 3; ++i) {
	if ((argc = read_cmd(sock, &argv)) != 4)
	    return 1;
	if (strncmp(argv[3], "#testchan ", 10) == 0) {
	    if (!(s = strchr(argv[3], '+')) || strcmp(s, "+ ") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4)
		return 1;
	    s = strtok(argv[3], " ");
	    count = 0;
	    while (s = strtok(NULL, " ")) {
		++count;
		if (strcmp(s, "Alcan") != 0 && strcmp(s, "Alcan_") != 0)
		    return 1;
	    }
	    if (count != 2)
		return 1;
	} else if (strncmp(argv[3], "#test2 ", 7) == 0) {
	    if (!(s = strchr(argv[3], '+')) || strcmp(s, "+ ") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4)
		return 1;
	    s = strtok(argv[3], " ");
	    count = 0;
	    while (s = strtok(NULL, " ")) {
		++count;
		if (strcmp(s, "Alcan") != 0 && strcmp(s, "Alcan_") != 0)
		    return 1;
	    }
	    if (count != 2)
		return 1;
	} else if (strncmp(argv[3], "#test3 ", 7) == 0) {
	    if (!(s = strchr(argv[3], '+')) || strcmp(s, "+ ") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "#test3 Alcan_") != 0)
		return 1;
	} else {
	    return 1;
	}
    }

    return 0;
}

/*************************************************************************/

/* Test JOIN command from a bad nick.  Expected behavior: the command is
 * ignored. */
/* Requires: test_nick */

int test_join_bad(int sock)
{
    int argc, i;
    char **argv;
    char *s;

    send_cmd(sock, "Alcan2", "JOIN #testchan");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan") != 0)
	return 1;
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTCHANS");

    send_cmd(sock, NULL, ":Alcan JOIN #testchan");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan #testchan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan") != 0)
	return 1;
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTCHANS");
    if ((argc = read_cmd(sock, &argv)) != 4)
	return 1;
    if (strncmp(argv[3], "#testchan ", 10) == 0) {
	if (!(s = strchr(argv[3], '+')) || strcmp(s, "+ ") != 0)
	    return 1;
	if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "#testchan Alcan"))
	    return 1;
    } else {
	return 1;
    }

    send_cmd(sock, NULL, ":Alcan2 JOIN #testchan");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan #testchan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan") != 0)
	return 1;
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTCHANS");
    if ((argc = read_cmd(sock, &argv)) != 4)
	return 1;
    if (strncmp(argv[3], "#testchan ", 10) == 0) {
	if (!(s = strchr(argv[3], '+')) || strcmp(s, "+ ") != 0)
	    return 1;
	if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "#testchan Alcan"))
	    return 1;
    } else {
	return 1;
    }

    return 0;
}

/*************************************************************************/
