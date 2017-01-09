FILENAME := wavtomp3converter
UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
	TARGET := ${FILENAME}
	LAME_LIB := mp3lame_x86_64_linux
else
	TARGET := ${FILENAME}.exe
	LAME_LIB := mp3lame_i686_win
endif

$(TARGET) : main.cpp
	g++ -std=c++11 -I ./include -L ./ main.cpp -lpthread -static -static-libgcc -static-libstdc++ -l${LAME_LIB} -o $@

clean:
	rm -r ${TARGET}
