#ifndef _WINRT_EXTERNAL_H
#define _WINRT_EXTERNAL_H

#define PATH_MAX 255
#define putenv(a) ;
#ifdef __cplusplus_winrt
extern "C"
{
#endif
	int dir_exists(char *path);
    int create_dir_if_needed(char *path);
    void rotate_landscape(void);
    void rotate_portrait(void);
    void unlock_rotation(void);
	void getAppTempDir(char *lpPathBuffer);
#ifdef __cplusplus_winrt
}
#endif
#endif
