/* requires: stdio.h */

#ifndef __CASTRO_TEST_H__
#define __CASTRO_TEST_H__

#define test_begin() int testsuccess = 1, testsuccessall = 1
#define test_end() return (testsuccessall ? 0 : 1)

#define tprint(cond, msg, ...) { \
	fprintf(stdout, "%s:%d in function '%s': testing %s... ", __FILE__, __LINE__, __func__, (msg)); \
	fflush(stdout); \
	if ((testsuccess = (cond))) fprintf(stdout, "OK\n"); \
	else {\
		fprintf(stdout, "FAILED "); \
		fprintf(stdout, __VA_ARGS__); \
		fprintf(stdout, "\n"); \
	} \
	testsuccessall *= testsuccess; \
}

#define TESTFMT_int "%d"
#define TESTFMT_double "%2.6f"

#define testrel(type, rel, lhs, rhs) { \
	type testrel_l=(lhs), testrel_r=(rhs); \
	tprint(testrel_l rel testrel_r, #lhs " " #rel " " #rhs, TESTFMT_##type " " #rel " " TESTFMT_##type, testrel_l, testrel_r); \
}

#endif