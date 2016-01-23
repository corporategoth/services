/* NickServ special help texts.
 *
 * Services is copyright (c) 1996-1997 Preston A. Elder.
 *     E-mail: <prez@antisocial.com>   IRC: PreZ@DarkerNet
 * This program is free but copyrighted software; see the file COPYING for
 * details.
 */

/*************************************************************************/

static const char *oper_drop_help[] = {
"Syntax: \2DROP [\37nickname\37]\2",
"",
"Without a parameter, drops your nickname from the",
"NickServ database.",
"",
"With a parameter, drops the named nick from the database.",
"This use limited to \2Services Admin\2.",
NULL
};

/*************************************************************************/

static const char *getpass_help[] = {
"Syntax: \2GETPASS \37nickname\37\2",
"",
"Returns the password for the given nickname.  \2Note\2 that",
"whenever this command is used, a message including the",
"person who issued the command and the nickname it was used",
"on will be logged and sent out as a GLOBOPS.",
"Limited to \2Services Admin\2.",
NULL
};

/*************************************************************************/

static const char *forbid_help[] = {
"Syntax: \2FORBID \37nickname\37\2",
"",
"Disallows a nickname from being registered or used by",
"anyone.  May be cancelled by dropping the nick.",
"Limited to \2Services Admin\2.",
NULL
};

/*************************************************************************/

static const char *set_ircop_help[] = {
"Syntax: \2SET IRCOP {ON|OFF}\2",
"",
"Disallows your nickname from being expired, and adds",
"\37IRC Operator\37 to your \2INFO\2 display..",
"Limited to \2IRC Operators\2.",
NULL
};

/*************************************************************************/

