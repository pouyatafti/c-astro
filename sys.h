/* requires: stdio.h */

#ifndef __CASTRO_SYS_H__
#define __CASTRO_SYS_H__

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0
#endif

#define wtlog(level, ...) if (level <= DEBUG_LEVEL) fprintf(stdout, __VA_ARGS__)
#define wterror(...) fprintf(stderr, __VA_ARGS__)


#endif