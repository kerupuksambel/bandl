CC=g++
BUILD_DIR=bin
EXECUTABLE=bandl
CFLAGS=-c -Wall
MODE=DEBUG

all: main.o
	$(CC) -o ./$(BUILD_DIR)/$(EXECUTABLE) $< -Llib/ -lyaml-cpp -llibcurl

main.o: main.cpp
	$(CC) -Iinclude/  $(CFLAGS) $<

tes: tes.cpp
	$(CC) -Iinclude/ $< -o tes -Llib/ -lyaml-cpp
	
debug: main.cpp
	$(CC) -g  $< -o mdebug

.PHONY: debug clean
clean: 
	@echo Cleaning all obj file..
	cmd /c del *.o
