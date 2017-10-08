#include "dirent.h"

int dir_exists(char *path)
{
	DIR* dir = opendir(path);
	if (dir)
	{
		/* Directory exists. */
		closedir(dir);
		return 1;
	}
	return 0;
}

int create_dir_if_needed(char *path)
{
	DIR* dir = opendir(path);
	if (dir)
	{
		/* Directory exists. */
		closedir(dir);
		return 0;
	}
	else if (ENOENT == errno)
	{
		/* Directory does not exist. */
		_mkdir(path);
		return 1;
	}
	else
	{
		/* opendir() failed for some other reason. */
		return 2;
	}
}
