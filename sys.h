/* requires: stdio.h */

#ifndef __CASTRO_SYS_H__
#define __CASTRO_SYS_H__

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0
#endif

#define wtlog(level, ...) if (level <= DEBUG_LEVEL) { fprintf(stdout, "%s:%d in function '%s': ", __FILE__, __LINE__, __func__); fprintf(stdout, __VA_ARGS__); fflush(stdout); }
#define wterror(...) { fprintf(stderr, "%s:%d in function '%s': ", __FILE__, __LINE__, __func__); fprintf(stderr, __VA_ARGS__); fflush(stdout); }


#endif