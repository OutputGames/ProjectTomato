#if !defined(FILESYSTEM_H)
#define FILESYSTEM_H

#include "utils.h"

struct TMAPI tmfs
{
	static string loadFileString(string path);
	static char* loadFileBytes(string path, uint32_t& ds);
	static void writeFileString(string path, string data);
	static void copyFile(string from, string to);
	static bool fileExists(string path);
	static void copyDirectory(string from, string to);
	static string getHomeDirectory();
};


#endif // FILESYSTEM_H
