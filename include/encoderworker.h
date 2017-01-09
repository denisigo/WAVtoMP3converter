#ifndef ENCODERWORKER_H_INCLUDED
#define ENCODERWORKER_H_INCLUDED

#include <iostream>

#include <lame.h>
#include <wavheader.h>
#include <utils.h>
#include <fileprovider.h>

namespace wavtomp3converter {

using namespace std;

// How many chunks (left+right samples for stereo or one sample for mono) to read from wav file at once
const unsigned int WAV_CHUNKS_COUNT = 8192;
// How many bytes of mp3 data to read from lame at once (by formula from lame.h: 1.25*num_samples + 7200)
const unsigned int MP3_BUFFER_SIZE = 17440;
// Lame encoding settings
const unsigned int LAME_QUALITY = 5; // good
const vbr_mode LAME_VBR_MODE = vbr_default;

/*
* A struct used to provide worker with some arguments
*/
struct EncoderWorkerArgs{
    int coreNumber;
    FileProvider *fileProvider;
};

/*
* Main worker function to be running within a thread.
* Gets wav files from FileProvider and encodes them to mp3.
*/
void *encoderWorker(void *args){
    const size_t WAV_SAMPLE_SIZE = sizeof(short int);
    short int wavBuffer[WAV_CHUNKS_COUNT*2]; // for simplicity, allocate buffer for stereo
    unsigned char mp3Buffer[MP3_BUFFER_SIZE];
    wav_hdr_t wavHeader;
    int read, encoded;
    int sampleRate, channelsNumber;
    char wavFilename[PATH_MAX_LENGTH];
    char mp3Filename[PATH_MAX_LENGTH];
    FILE *wavFile, *mp3File;
    lame_t lame;
    bool eof;
    struct EncoderWorkerArgs *encoderWorkerArgs = (struct EncoderWorkerArgs *) args;

    // Iterate over all available wav files until there is no one
    while (encoderWorkerArgs->fileProvider->getNextFile(wavFilename)){
        wavFile = fopen(wavFilename, "rb");
        if (wavFile == NULL){
            cout << "Unable to open file: " << wavFilename << endl;
            goto error;
        }

        // Read WAV header to figure out sample rate and channels
        read = fread(&wavHeader, WAV_HEADER_SIZE, 1, wavFile);
        if (read == 0){
            cout << "Unable to read wav header of file: " << wavFilename << endl;
            goto error;
        }

        // Extra check for RIFF magic header
        if (!(wavHeader.RIFF[0] == 'R' && wavHeader.RIFF[1] == 'I' &&
              wavHeader.RIFF[2] == 'F' && wavHeader.RIFF[3] == 'F')){
            cout << "Invalid magic header (" << wavHeader.RIFF << ") in file: " << wavFilename << endl;
            goto error;
        }

        // Extra check for either mono or stereo audio
        if (wavHeader.NumOfChan != 1 && wavHeader.NumOfChan != 2){
            cout << "Invalid number of channels (" << wavHeader.NumOfChan << ") in file: " << wavFilename << endl;
            goto error;
        }

        sampleRate = wavHeader.SamplesPerSec;
        channelsNumber = wavHeader.NumOfChan;

        // Seek wav file to the start to not confuse lame
        fseek(wavFile, 0, SEEK_SET);

        // Prepare and open file for mp3 output
        strcpy(mp3Filename, wavFilename);
        wavToMp3Extension(mp3Filename);
        mp3File = fopen(mp3Filename, "wb");
        if (mp3File == NULL){
            cout << "Unable to open mp3 file: " << mp3File << endl;
            goto error;
        }

        // Init lame
        lame = lame_init();
        if (lame == NULL){
            cout << "Unable to init lame for file: " << wavFilename << endl;
            goto error;
        }

        lame_set_in_samplerate(lame, sampleRate);
        lame_set_num_channels(lame, channelsNumber);
        lame_set_VBR(lame, LAME_VBR_MODE);
        lame_set_quality(lame, LAME_QUALITY);
        if (lame_init_params(lame) == -1){
            cout << "Unable to set lame params for file: " << wavFilename << endl;
            goto error;
        }

        // Now read data from wav file and encode it to mp3
        eof = false;
        do {
            read = fread(wavBuffer, channelsNumber*WAV_SAMPLE_SIZE, WAV_CHUNKS_COUNT, wavFile);
            // If chunks were read don't match count we provided - it means either EOF or error
            if (read != WAV_CHUNKS_COUNT){
                if (feof(wavFile)){
                    encoded = lame_encode_flush(lame, mp3Buffer, MP3_BUFFER_SIZE);
                    eof = true;
                } else {
                    cout << "Error while reading file: " << wavFilename << " ferror: " << ferror(wavFile) << endl;
                    goto error;
                }
            } else {
                // For stereo use interleaved method, for mono - pass only left channel data
                if (channelsNumber == 2)
                    encoded = lame_encode_buffer_interleaved(lame, wavBuffer, read, mp3Buffer, MP3_BUFFER_SIZE);
                else
                    encoded = lame_encode_buffer(lame, wavBuffer, NULL, read, mp3Buffer, MP3_BUFFER_SIZE);
            }
            if (encoded < 0){
                cout << "Error while encoding file: " << wavFilename << " lame returned: " << encoded << endl;
                goto error;
            } else {
                if (fwrite(mp3Buffer, encoded, 1, mp3File) != 1){
                    cout << "Error while writing to file: " << mp3Filename << " lame returned: " << encoded << endl;
                    goto error;
                }
            }
        } while (!eof);

        if (eof)
            cout << "Core #" << encoderWorkerArgs->coreNumber << ", encoded file: " << wavFilename << ", to: " << mp3Filename << ", sample rate: " << sampleRate << ", channels: " << channelsNumber << endl;

        error:
            lame_close(lame);
            fclose(mp3File);
            fclose(wavFile);
    }

    pthread_exit(NULL);
}

};

#endif // ENCODERWORKER_H_INCLUDED
