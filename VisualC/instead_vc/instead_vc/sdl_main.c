#include <libgen.h>

int SDL_main(int argc, char *argv[])
{
	int err;
	char path[100];
	getcwd(&path, 100);
	err = instead_main(argc, argv);
	return err;
}