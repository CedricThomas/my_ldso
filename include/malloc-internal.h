/* used only in malloc.c implementation, do not edit */

#define USE_LOCKS 0
#define HAVE_MORECORE 0
#define NO_MALLOC_STATS 1

#define MALLOC_FAILURE_ACTION 
#define LACKS_TIME_H 1

#define ABORT _exit(1)
