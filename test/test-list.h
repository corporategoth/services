/* Declarations of test functions */

#ifndef TEST_LIST_H
#define TEST_LIST_H

#define F(n)	extern int n(int sock)

/* Users */
F(test_nick);
F(test_nickchange);
F(test_badnick);
F(test_badnickchange);
F(test_away);
F(test_quit);
F(test_kill);
F(test_umode);

/* Channels */
F(test_join_one);
F(test_join_multi);
F(test_join_bad);

#undef F

#endif	/* TEST_LIST_H */
