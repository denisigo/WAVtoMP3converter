#ifndef AUTOMUTEX_H_INCLUDED
#define AUTOMUTEX_H_INCLUDED

#include <stdexcept>
#include <pthread.h>

namespace wavtomp3converter {

/*
* A helper class for implementing RAII for pthread mutexes.
*/
class AutoMutex {
    public:
        AutoMutex(pthread_mutex_t  *mutex)
            : mMutex(mutex){
            if (pthread_mutex_lock(mMutex))
                throw std::runtime_error("Could not lock the mutex.");
        }

        ~AutoMutex(){
            pthread_mutex_unlock(mMutex);
        }

    private:
        pthread_mutex_t *mMutex;
};

};
#endif // AUTOMUTEX_H_INCLUDED
