/* General tests */

#include "test.h"

/*************************************************************************/

/* Test NICK command to add a user.  Expected behavior: user record is
 * created. */

int test_nick(int sock)
{
    int argc, i;
    char **argv;

    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;

    send_cmd(sock, NULL, "NICK Alcan_ 2 17 root Redwing.airship.net airship.esper.net :Ralph Oot");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    for (i = 0; i < 2; ++i) {
	if ((argc = read_cmd(sock, &argv)) != 4)
	    return 1;
	if (strncmp(argv[3], "Alcan!", 6) == 0) {
	    if (strcmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[3], "Alcan") != 0)
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
	} else
	    return 1;
    }

    return 0;
}

/*************************************************************************/

/* Test NICK command to change a nickname.  Expected behavior: old user
 * record is replaced by a new record with the new nickname and all other
 * information identical. */
/* Requires: test_nick */

int test_nickchange(int sock)
{
    int argc, i;
    char **argv;

    send_cmd(sock, NULL, "NICK Alcan- 2 17 root Redwing.airship.net airship.esper.net :Ralph Oot");
    send_cmd(sock, NULL, "NICK Alcan_ 3 42 dragon notfoo.res.cmu.edu chocobo.esper.net :Dragon");

    send_cmd(sock, "Alcan-", "NICK _Alcan 70");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    for (i = 0; i < 3; ++i) {
	if ((argc = read_cmd(sock, &argv)) != 4)
	    return 1;
	if (strncmp(argv[3], "Alcan!", 6) == 0) {
	    if (strcmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
		return 1;
	} else if (strncmp(argv[3], "_Alcan!", 7) == 0) {
	    if (strcmp(argv[3],
"_Alcan!root@Redwing.airship.net + 70 airship.esper.net :Ralph Oot") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "_Alcan") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "_Alcan") != 0)
		return 1;
	} else if (strncmp(argv[3], "Alcan_!", 7) == 0) {
	    if (strcmp(argv[3],
"Alcan_!dragon@notfoo.res.cmu.edu + 42 chocobo.esper.net :Dragon") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan_") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan_") != 0)
		return 1;
	} else
	    return 1;
    }

    send_cmd(sock, "_Alcan", "NICK Alcan- 80");
    send_cmd(sock, "Alcan_", "NICK _Alcan 81");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    for (i = 0; i < 3; ++i) {
	if ((argc = read_cmd(sock, &argv)) != 4)
	    return 1;
	if (strncmp(argv[3], "Alcan!", 6) == 0) {
	    if (strcmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
		return 1;
	} else if (strncmp(argv[3], "Alcan-!", 7) == 0) {
	    if (strcmp(argv[3],
"Alcan-!root@Redwing.airship.net + 80 airship.esper.net :Ralph Oot") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan-") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan-") != 0)
		return 1;
	} else if (strncmp(argv[3], "_Alcan!", 7) == 0) {
	    if (strcmp(argv[3],
"_Alcan!dragon@notfoo.res.cmu.edu + 81 chocobo.esper.net :Dragon") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "_Alcan") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "_Alcan") != 0)
		return 1;
	} else
	    return 1;
    }

    return 0;
}

/*************************************************************************/

/* Test NICK command with a bad parameter count.  Expected behavior: the
 * command is ignored. */

int test_badnick(int sock)
{
    int argc;
    char **argv;

    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;

    send_cmd(sock, NULL, "NICK Alcan_ 2 17 root Redwing.airship.net airship.esper.net");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;

    send_cmd(sock, NULL, "NICK Alcan_ 2 17 root Redwing.airship.net airship.esper.net Ralph Oot");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;

    send_cmd(sock, "Alcan", "NICK Alcan1");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;

    send_cmd(sock, "Alcan", "NICK Alcan2 99 blah");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;

    return 0;
}

/*************************************************************************/

/* Test the NICK command for nick changes coming from a nonexistent nick.
 * Expected behavior: the command is ignored. */

int test_badnickchange(int sock)
{
    int argc;
    char **argv;

    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;

    send_cmd(sock, "Alcan2", "NICK Alcan3 99");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;

    return 0;
}

/*************************************************************************/

/* Test the AWAY command.  Expected behavior: if the nick exists, MemoServ
 * sends a NOTICE of available memos, if any, upon an AWAY with no message
 * or a null mesasge; if the nick does not exist, the command is ignored. */
/* Requires: test_nick */

int test_away(int sock)
{
    int argc;
    char **argv;

    send_cmd(sock, "Alcan", "AWAY Boink.");
    send_cmd(sock, "Alcan", "AWAY");
    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[0], ":MemoServ") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[0], ":MemoServ") != 0)
	return 1;

    send_cmd(sock, "Alcan", "AWAY Boink.");
    send_cmd(sock, "Alcan", "AWAY :");
    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[0], ":MemoServ") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || strcmp(argv[0], ":MemoServ") != 0)
	return 1;

    send_cmd(sock, "Alcan2", "AWAY Boink.");
    send_cmd(sock, "Alcan2", "AWAY");

    return 0;
}

/*************************************************************************/

/* Test the QUIT command.  Expected behavior: if the nick exists, it is
 * removed from the user list; if the nick does not exist, the command is
 * ignored. */
/* Requires: test_nick */

int test_quit(int sock)
{
    int argc, i;
    char **argv;

    send_cmd(sock, NULL, "NICK Alcan- 2 17 root Redwing.airship.net airship.esper.net :Ralph Oot");
    send_cmd(sock, NULL, "NICK Alcan_ 3 42 dragon notfoo.res.cmu.edu chocobo.esper.net :Dragon");

    send_cmd(sock, "Alcan_", "QUIT :");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    for (i = 0; i < 2; ++i) {
	if ((argc = read_cmd(sock, &argv)) != 4)
	    return 1;
	if (strncmp(argv[3], "Alcan!", 6) == 0) {
	    if (strcmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
		return 1;
	} else if (strncmp(argv[3], "Alcan-!", 7) == 0) {
	    if (strcmp(argv[3],
"Alcan-!root@Redwing.airship.net + 17 airship.esper.net :Ralph Oot") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan-") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan-") != 0)
		return 1;
	} else
	    return 1;
    }

    send_cmd(sock, NULL, "NICK Alcan_ 3 42 dragon notfoo.res.cmu.edu chocobo.esper.net :Dragon");
    send_cmd(sock, "Alcan-", "QUIT :");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    for (i = 0; i < 2; ++i) {
	if ((argc = read_cmd(sock, &argv)) != 4)
	    return 1;
	if (strncmp(argv[3], "Alcan!", 6) == 0) {
	    if (strcmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
		return 1;
	} else if (strncmp(argv[3], "Alcan_!", 7) == 0) {
	    if (strcmp(argv[3],
"Alcan_!dragon@notfoo.res.cmu.edu + 42 chocobo.esper.net :Dragon") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan_") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan_") != 0)
		return 1;
	} else
	    return 1;
    }

    send_cmd(sock, "Alcan-", "QUIT :");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    for (i = 0; i < 2; ++i) {
	if ((argc = read_cmd(sock, &argv)) != 4)
	    return 1;
	if (strncmp(argv[3], "Alcan!", 6) == 0) {
	    if (strcmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
		return 1;
	} else if (strncmp(argv[3], "Alcan_!", 7) == 0) {
	    if (strcmp(argv[3],
"Alcan_!dragon@notfoo.res.cmu.edu + 42 chocobo.esper.net :Dragon") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan_") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan_") != 0)
		return 1;
	} else
	    return 1;
    }

    return 0;
}

/*************************************************************************/

/* Test the KILL command.  Expected behavior: same as QUIT, except that the
 * nick in question is not the source, but the specified target nick. */
/* Requires: test_nick */

int test_kill(int sock)
{
    int argc, i;
    char **argv;

    send_cmd(sock, NULL, "NICK Alcan- 2 17 root Redwing.airship.net airship.esper.net :Ralph Oot");
    send_cmd(sock, NULL, "NICK Alcan_ 3 42 dragon notfoo.res.cmu.edu chocobo.esper.net :Dragon");

    send_cmd(sock, "Alcan", "KILL Alcan_ :");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    for (i = 0; i < 2; ++i) {
	if ((argc = read_cmd(sock, &argv)) != 4)
	    return 1;
	if (strncmp(argv[3], "Alcan!", 6) == 0) {
	    if (strcmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
		return 1;
	} else if (strncmp(argv[3], "Alcan-!", 7) == 0) {
	    if (strcmp(argv[3],
"Alcan-!root@Redwing.airship.net + 17 airship.esper.net :Ralph Oot") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan-") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan-") != 0)
		return 1;
	} else
	    return 1;
    }

    send_cmd(sock, NULL, "NICK Alcan_ 3 42 dragon notfoo.res.cmu.edu chocobo.esper.net :Dragon");
    send_cmd(sock, "Alcan", "KILL Alcan- :");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    for (i = 0; i < 2; ++i) {
	if ((argc = read_cmd(sock, &argv)) != 4)
	    return 1;
	if (strncmp(argv[3], "Alcan!", 6) == 0) {
	    if (strcmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
		return 1;
	} else if (strncmp(argv[3], "Alcan_!", 7) == 0) {
	    if (strcmp(argv[3],
"Alcan_!dragon@notfoo.res.cmu.edu + 42 chocobo.esper.net :Dragon") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan_") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan_") != 0)
		return 1;
	} else
	    return 1;
    }

    send_cmd(sock, "Alcan", "KILL Alcan- :");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    for (i = 0; i < 2; ++i) {
	if ((argc = read_cmd(sock, &argv)) != 4)
	    return 1;
	if (strncmp(argv[3], "Alcan!", 6) == 0) {
	    if (strcmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +o 8 dragonfire.esper.net :Andy Church") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
		return 1;
	} else if (strncmp(argv[3], "Alcan_!", 7) == 0) {
	    if (strcmp(argv[3],
"Alcan_!dragon@notfoo.res.cmu.edu + 42 chocobo.esper.net :Dragon") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan_") != 0)
		return 1;
	    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan_") != 0)
		return 1;
	} else
	    return 1;
    }

    return 0;
}

/*************************************************************************/

/* Test the MODE command for users.  Expected behavior: if the target nick
 * exists and is the same as the source nick, the nick's modes are changed
 * as specified; if the target nick and source nick are different, a
 * GLOBOPS is sent and the command is dropped; otherwise, the command is
 * ignored. */
/* Requires: test_nick */

int test_umode(int sock)
{
    int argc;
    char **argv;

    send_cmd(sock, "Alcan", "MODE Alcan +wg");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +gow 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;

    send_cmd(sock, "Alcan", "MODE Alcan -w");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +go 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;

    send_cmd(sock, "Alcan", "MODE Alcan +gi");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +gio 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;

    send_cmd(sock, "Alcan", "MODE Alcan -wi");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +go 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;

    send_cmd(sock, "Alcan", "MODE Alcan +w-g");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +ow 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;

    send_cmd(sock, "Alcan", "MODE Alcan -w+g");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +go 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;

    send_cmd(sock, "Alcan2", "MODE Alcan2 +wg");
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +go 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;

    send_cmd(sock, "Alcan2", "MODE Alcan +wg");
    if ((argc = read_cmd(sock, &argv)) != 3 || stricmp(argv[1], "GLOBOPS") != 0)
	return 1;
    send_cmd(sock, "Alcan", "PRIVMSG OperServ LISTUSERS");
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3],
"Alcan!achurch@Bahamut.dragonfire.net +go 8 dragonfire.esper.net :Andy Church") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;
    if ((argc = read_cmd(sock, &argv)) != 4 || stricmp(argv[3], "Alcan") != 0)
	return 1;

    return 0;
}

/*************************************************************************/
