#ifndef H_MAIN
#define H_MAIN

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

// #define G_ERRORCHECK_MUTEXES

#include <pthread.h>

#include <gtk/gtk.h>

extern const char szAppTitle[];
extern const char szAppName[];
extern const char szVersion[];

#ifndef VERSION
#define VERSION "unknown"
#endif

#define _(a) a

#define true  1
#define false 0

typedef enum { rcOk=0, rcNomem=1, rcIO=2, rcBounds=3, rcUnknown=-1 } TResult ;

#define DEBUG_FRAMEWORK 1
#define DEBUG_WAVE      2
#define DEBUG_SELECTION 3
#define DEBUG_WAVEVIEW  4
#define DEBUG_RECORDER  5

#define SETZERO(me)  memset(me,0,sizeof(*me))

void debug_printf(int debug_class,const char *szFormat, ...);

#endif

