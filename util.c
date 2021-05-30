#include "util.h"

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

#else

#include <sys/stat.h>
#include <sys/types.h>

#endif

bool GetLastWriteTime(const char* filename, long long* time) {
#ifdef _WIN32
    FILETIME c, a, w;
    HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, 0, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    if (!GetFileTime(hFile, &c, &a, &w)) {
        return false;
    }

    *time = ((long long)w.dwHighDateTime << 32 | w.dwLowDateTime);

    CloseHandle(hFile);

    return true;
#else
    struct stat buf;

    int r = stat(filename, &buf);

    if (r < 0) {
        return false;
    }

    *time = buf.st_mtim.tv_sec;

    return true;
#endif
}
