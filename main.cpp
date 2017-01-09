#include <iostream>
#include <pthread.h>

#include <encoderworker.h>

using namespace std;
using namespace wavtomp3converter;

int main(int argc, char *argv[]){
    cpu_set_t cpuset;
    int coresNumber;
    pthread_t *threads = NULL;
    EncoderWorkerArgs *encoderWorkerArgs = NULL;
    int rc;
    int i;
    FileProvider fileProvider;
    const char *dir;
    bool isError = false;

    // Show help if no args provided
    if (argc <= 1) {
        cout << "Please provide directory with wav files as the first argument." << endl;
        isError = true;
        goto theend;
    }

    // Try to open provided directory
    dir = argv[1];
    if (!fileProvider.init(dir)) {
        cout << "Unable to open provided directory: " << dir << endl;
        isError = true;
        goto theend;
    }

    // Determine number of cores and prepare resources
    coresNumber = numberOfCores();
    cout << "Number of cores: " << coresNumber << endl;
    threads = new pthread_t[coresNumber];
    encoderWorkerArgs = new EncoderWorkerArgs[coresNumber];

    for(i=0; i < coresNumber; i++){
        // Prepare thread arguments struct
        encoderWorkerArgs[i].coreNumber = i;
        encoderWorkerArgs[i].fileProvider = &fileProvider;
        rc = pthread_create(&threads[i], NULL, encoderWorker, (void *)&encoderWorkerArgs[i]);
        if (rc == 0){
            // Assign new thread to its own core
            CPU_ZERO(&cpuset);
            CPU_SET(i, &cpuset);
            pthread_setaffinity_np(threads[i], sizeof(cpu_set_t), &cpuset);
        } else {
            // We'll continue to execute even with fewer threads
            cout << "Unable to create thread #" << i << ": " << rc << endl;
        }
    }

    // Wait for all threads to complete
    for(i=0; i < coresNumber; i++)
        pthread_join(threads[i], NULL);

    theend:
        cout << (isError?"ERROR. ":"DONE. ") << "Press any key to exit" << endl;
        cin.get();
        if (threads != NULL)
            delete[] threads;
        if (encoderWorkerArgs != NULL)
            delete[] encoderWorkerArgs;
        fileProvider.close();
        exit(EXIT_SUCCESS);
}
