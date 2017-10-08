#ifndef _WINRT_EXTERNAL_H
#define _WINRT_EXTERNAL_H

#ifndef PATH_MAX
#define PATH_MAX 	4096
#endif
#define putenv(a) ;

/* must be implemented as extern "C" in winrt cpp code */
#ifdef __cplusplus_winrt
extern "C"
{
#else
extern
#endif
void getAppTempDir(char *lpPathBuffer);
#ifdef __cplusplus_winrt
}
#endif

#endif
