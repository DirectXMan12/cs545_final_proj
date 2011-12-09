# Makefile for tomographic reconstruction project
# Version: December 6, 2011

EXE = tomorec
CC = g++
CFLAGS = -g
CPP_FILES = main.cpp cimg.cpp
H_FILES = 3d_img.h err_macros.h tomo_img.h cimg.h img.h
LIB = -lm

all: $(CPP_FILES) $(H_FILES)
	$(CC) $(CPP_FILES) $(H_FILES) $(CFLAGS) $(LIB) -o $(EXE)

clean:
	rm -f $(EXE)
