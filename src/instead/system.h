#ifndef __INSTEAD_EXTERNAL_H_
#define __INSTEAD_EXTERNAL_H_

#include "SDL_platform.h"  // __WINRT__ define

#ifdef _WIN32_WCE
 #define PATH_MAX 255
 #define errno 0
 #define strerror(a) ""
 #define putenv(a) ;
 #define setlocale(a, b) ;
#elif defined(__WINRT__)
// #define PATH_MAX 255
#define putenv(a) ;
#endif

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>

#include <limits.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <dirent.h>
#include <time.h>
#ifndef S60
 #include <libgen.h>
#endif
#ifdef S60
 #include "snprintf.h"
 typedef long ssize_t;
#endif
#include <math.h>

#ifndef PATH_MAX
#define PATH_MAX 	4096
#endif

#endif
