/* Help text for OperServ */

static const char *os_help[] = {
"OperServ commands:",
"   \2MODE \37channel\37 \37modes\37\2 -- Change a channel's modes",
"   \2KICK \37channel\37 \37nick\37 \37reason\37\2 -- Kick a user from a channel",
"   \2AKILL {ADD|DEL|LIST} [\37mask\37 [\37reason\37]]\2 -- Manipulate the AKILL list",
"   \2GLOBAL \37message\37\2 -- Send a message to all users",
"   \2STATS\2 -- status of Services and network",
"   \2LISTSOPS\2 -- Show hardcoded Service Operators",
"",
"\2Notice:\2 All commands sent to OperServ are logged!",
NULL
};

/*************************************************************************/

static const char *mode_help[] = {
"Syntax: MODE \37channel\37 \37modes\37",
"",
"Allows IRCops to set channel modes for any channel.",
"Parameters are the same as for the standard /MODE",
"command.",
NULL
};

/*************************************************************************/

static const char *kick_help[] = {
"Syntax: KICK \37channel\37 \37user\37 \37reason\37",
" ",
"Allows IRCops to kick a user from any channel.",
"Parameters are the same as for the standard /KICK",
"command.  The kick message will have the nickname of the",
"IRCop sending the KICK command prepended; for example:",
"",
"*** SpamMan has been kicked off channel #my_channel by OperServ (PreZ (Flood))",
NULL
};

/*************************************************************************/

static const char *akill_help[] = {
"Syntax: AKILL ADD \37mask\37 \37reason\37",
"        AKILL DEL \37mask\37",
"        AKILL LIST [\37mask\37]",
"        AKILL VIEW [\37mask\37]",
"",
"Allows IRCops to manipulate the AKILL list.  If a user",
"matching an AKILL mask attempts to connect, Services will",
"issue a KILL for that user.",
" ",
"AKILL ADD adds the given user@host mask to the AKILL",
"list for the given reason (which \2must\2 be given).",
"AKILL DEL removes the given mask from the AKILL list if it",
"is present.  AKILL LIST shows all current AKILLs; if the",
"optional mask is given, the list is limited to those",
"AKILLs matching the mask.  AKILL VIEW is a more verbose",
"version of AKILL LIST, and will show who added an AKILL as",
"well as the user@host mask and reason.",
NULL
};

/*************************************************************************/

static const char *stats_help[] = {
"Syntax: \2STATS [ALL]\2",
"",
"Shows the current number of users and IRCops online",
"(excluding Services), the highest number of users online",
"since Services was started, and the length of time",
"Services has been running.",
"",
"The \2ALL\2 option is available only to Services admins, and",
"displays information on Services' memory usage.  Using this",
"option can freeze Services for a short period of time on",
"large networks, so don't overuse it!",
NULL
};

/*************************************************************************/

static const char *listsops_help[] = {
"Syntax: \2LISTSOPS\2",
"",
"Shows the current Service Operators hardcoded into",
"services.",
NULL
};

/*************************************************************************/
static const char *update_help[] = {
"Syntax: \2UPDATE\2",
"",
"Forces Services to update the channel, nick, and memo",
"databases as soon as you send the command.  Useful",
"before shutdowns.",
"Limited to \2Services Admin\2.",
NULL
};
/************************************************************************/
static const char *quit_help[] = {
"Syntax: \2QUIT\2",
"",
"Causes Services to do an immediate shutdown.  It is",
"a good idea to update the databases using the",
"\2UPDATE\2 command beforehand.",
"\2NOTE:\2 This command is NOT to be toyed with--using",
"it without good reason can disrupt network operations,",
"especially if a person with access to restart Services",
"is not around.",
"Limited to \2Services Admin\2.",
NULL
};
/************************************************************************/
static const char *shutdown_help[] = {
"Syntax: \2SHUTDOWN\2",
"",
"Tells Services to shut down, but save databases.  It is",
"a \"clean\" alternative to \2QUIT\2.",
"\2NOTE:\2 This command is NOT to be toyed with--using",
"it without good reason can disrupt network operations,",
"especially if a person with access to restart Services",
"is not around.",
"Limited to \2Services Admin\2.",
NULL
}; 
/************************************************************************/
static const char *jupe_help[] = {
"Syntax: \2JUPE \37server\37\2",
"",
"Tells Services to jupiter a server -- that is, to create",
"a fake \"server\" connected to Services which prevents",
"the real server of that name from connecting.  The jupe",
"may be removed using a standard \2SQUIT\2."
"To be used \2only\2 in a situation where a server is",
"disrupting the network and must be juped.",
"Limited to \2Services Admin\2.",
NULL
};
