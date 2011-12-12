# Makefile for tomographic reconstruction project
# Version: December 11, 2011

EXE_TOMO = tomorec
EXE_TD = 3d_2_tif
CC = g++
CFLAGS = -g -std=c++0x
CPP_FILES_TOMO = main.cpp
CPP_FILES_TD = 3d_2_tif.cpp
LIB = -lm -lglut -lGLU -lGL

all:
	make tomorec
	make 3d_2_tif

tomorec: $(CPP_FILES_TOMO) $(H_FILES)
	$(CC) $(CPP_FILES_TOMO) -I . $(CFLAGS) $(LIB) -o $(EXE_TOMO) -Wall

3d_2_tif: $(CPP_FILES_TD)
	$(CC) $(CPP_FILES_TD) $(CFLAGS) -o $(EXE_TD)

clean:
	rm -f $(EXE_TOMO) $(EXE_TD)
