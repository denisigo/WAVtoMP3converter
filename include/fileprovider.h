#ifndef FILEPROVIDER_H_INCLUDED
#define FILEPROVIDER_H_INCLUDED

#include <stdexcept>
#include <dirent.h>
#include <automutex.h>

namespace wavtomp3converter {

/*
* A helper class which opens directory and provide wav file one by one
* by calling getNextFile() method.
*/
class FileProvider{
    public:
        FileProvider():
            mDir(NULL){
        }

        ~FileProvider(){
            close();
        };

        /*
        * Init FileProvider with directory where to search for files.
        */
        bool init(const char *dirpath){
            mDir = opendir(dirpath);
            if (mDir != NULL){
                mDirpath = dirpath;
                pthread_mutex_init(&mMutex, NULL);
            }
            return mDir != NULL;
        };

        /*
        * Close FileProvider and free all resources
        */
        void close(){
            if (mDir != NULL)
                closedir(mDir);
            mDir = NULL;
            pthread_mutex_destroy(&mMutex);
        }

        /*
        * Go through all entries and get next wav file, thread-safe.
        * Copies full file path to the filepath.
        * filepath should be prepared char array of enough capacity.
        */
        bool getNextFile(char * filepath){
            if (mDir == NULL) return false;

            // We should use mutex since readdir is not thread-safe
            AutoMutex automutex(&mMutex);

            struct dirent *dirent;
            while ((dirent = readdir(mDir)) != NULL) {
                if (isWavFile(dirent->d_name)){
                    makeFilePath(filepath, mDirpath, dirent->d_name);
                    return true;
                }
            }
            return false;
        }

    private:
        const char *mDirpath;
        DIR *mDir;
        pthread_mutex_t mMutex;
};

};
#endif // FILEPROVIDER_H_INCLUDED
