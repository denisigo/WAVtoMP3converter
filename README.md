WAV to MP3 Converter
====================

Converts all WAV files to MP3 ones in the specified directory.

Currently supported:
 - Linux 64 bit
 - Windows 32 bit (using MinGW)

How to use
----------

Linux:
 1. $ make
 2. $ wavtomp3converter /path/to/wav/files/

Windows:
 1. Install MinGW and MSYS from http://www.mingw.org/
 2. Launch MSYS terminal: 
    [MinGW]/msys/msys.bat
 3. Run in MSYS:
    $ make
 4. Then launch via Windows Command Line: 
    wavtomp3converter.exe C:/path/to/wav/files/

Implementation notes
--------------------

1. main.cpp - main command line application file. 
   Parses arguments, does some preparations and starts several threads according to number of CPU cores.
2. include/encoderworker.h - contains encoderWorker function which does the main job of encoding the files.
   Uses FileProvider to get wav files for processing, reads its wav header to get sample rate and number of channels
   to use with lame.
3. include/fileprovider.h - contains FileProvider class which implements thread-safe iterator over wav files in 
   the specified directory. Every worker thread uses FileProvider::getNextFile() method to get next wav file for processing.
4. include/automutex.h - RAII (Resource Acquisition Is Initialization) implementation for safe handling of 
   phtread mutex used in FileProvider.
5. include/waveheader.h - struct which describes wav header.
6. include/utils.h - several util functions and constants.
7. libmp3lame_i686_win.a, libmp3lame_x86_64_linux.a - lame library compiled for windows 32 bit (using MinGW32) and linux 64 bit appropriately.
   They are being linked statically depending on host platform.
