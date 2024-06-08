#if !defined(FILESYSTEM_H)
#define FILESYSTEM_H

#include "utils.h"

struct tmfs
{
	static string loadFileString(string path);
	static char* loadFileBytes(string path, int& ds);
};


#endif // FILESYSTEM_H
