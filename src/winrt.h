#ifndef _WINRT_EXTERNAL_H
#define _WINRT_EXTERNAL_H

#define PATH_MAX 255
#define putenv(a) ;
#ifdef __cplusplus_winrt
extern "C"
{
#endif
    int create_dir_if_needed(char *path);
#ifdef __cplusplus_winrt
}
#endif
#ifdef __cplusplus_winrt
extern "C"
{
#endif	
	void getAppTempDir(char *lpPathBuffer);
#ifdef __cplusplus_winrt
}
#endif
#endif
