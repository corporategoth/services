/* List of Services tests */

#include "test.h"
#include "test-list.h"

struct test test_list[] = {

/*************************************************************************/

/* User tests */

{ test_nick,		"NICK command (new user)"			},
{ test_nickchange,	"NICK command (nick change)"			},
{ test_badnick,		"NICK command (bad parameter count)"		},
{ test_badnickchange,	"NICK command (nonexistent source)"		},
{ test_away,		"AWAY command"					},
{ test_quit,		"QUIT command"					},
{ test_kill,		"KILL command"					},
{ test_umode,		"MODE command for a user"			},

/*************************************************************************/

/* Channel tests */

{ test_join_one,	"JOIN command (one channel)"			},
{ test_join_multi,	"JOIN command (multiple channels)"		},
{ test_join_bad,	"JOIN command (nonexistent source)"		},

/*************************************************************************/

};
