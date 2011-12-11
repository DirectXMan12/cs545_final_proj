# Makefile for tomographic reconstruction project
# Version: December 6, 2011

EXE = tomorec
CC = g++
CFLAGS = -g -std=c++0x
CPP_FILES = main.cpp
LIB = -lm

all: $(CPP_FILES) $(H_FILES)
	$(CC) $(CPP_FILES) -I . $(CFLAGS) $(LIB) -o $(EXE) -Wall

clean:
	rm -f $(EXE)
