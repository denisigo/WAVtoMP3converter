#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <cctype>
#include <cstring>

namespace wavtomp3converter {

#if defined(WIN32) || defined(_WIN32)

#include <stdlib.h>

#define PATH_SEPARATOR "\\"
#define PATH_MAX_LENGTH MAX_PATH

#else

#include <unistd.h>
#include <linux/limits.h>

#define PATH_SEPARATOR "/"
#define PATH_MAX_LENGTH PATH_MAX

#endif

/*
* Swaps file extension from wav to mp3
*/
void wavToMp3Extension(char *filename){
    size_t len = strlen(filename);
    filename[len-3] = 'm';
    filename[len-2] = 'p';
    filename[len-1] = '3';
}

/*
* Checks whether provided filename has wav extension (case insensitive).
*/
bool isWavFile(const char * filename){
    char ext[4];
    size_t len = strlen(filename);
    if (len < 5) return false;
    if (filename[len-4] != '.') return false;
    ext[0] = tolower(filename[len-3]);
    ext[1] = tolower(filename[len-2]);
    ext[2] = tolower(filename[len-1]);
    ext[3] = '\0';
    return !strcmp(ext, "wav");
}

/*
* Composes file path of directory and filename.
*/
void makeFilePath(char *destination, const char *dir, const char *filename){
    strcpy(destination, dir);
    strcat(destination, PATH_SEPARATOR);
    strcat(destination, filename);
}

/*
* Returns number of CPU cores available
*/
int numberOfCores(){
    #if defined(WIN32) || defined(_WIN32)
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return sysinfo.dwNumberOfProcessors;
    #else
        return sysconf(_SC_NPROCESSORS_ONLN);
    #endif
}

};

#endif // UTILS_H_INCLUDED
