/* cs545 F97/98, read and print out the header of a raw digital image */
/* Written by Norman Wittels */
/* 08Mar97 - first version */
/* 02Sep97 - changed around includes and definitions */
/* 14Nov97 - Revised to use new WPI image format */


/* compile string:
cc wpi_prt_head.c -lm -o wpi_prt_head
*/

/* include eveything */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <float.h>

/* standard definitions */
#define FILE_NAME_LENGTH 51 /* maximum length of a file name */

/* Global variables */
static char *prog;

int main(int argc, char *argv[])
	{
	FILE *f; /* the file */
	unsigned char header[16]={0}; /* the header information */
	char *file_name;
	int n_dims, cols, rows, planes, colors;
	char image_type;
	int bytes_read;

	prog = argv[0];

	if (argc != 2) {
	  fprintf(stderr,
		  "Usage is: %s file_name\n",
		  prog);
	  exit (-1);
	}

	file_name = argv[1];

	if ((f = fopen(file_name,"rb"))==NULL) /* could not open file */
		{
		fprintf(stderr, "%s: could not open file %s\n",
			prog, file_name);
		exit(-1);
		} /* end if(fopen()) */

	bytes_read = fread(header,1,16,f);
	if (bytes_read != 6)
	  fprintf(stderr, "%s: only read %d bytes\n", prog, bytes_read);
	n_dims = 256 * header[0] + header[1];
	cols = 256 * header[2] + header[3];
	rows = 256 * header[4] + header[5];
	planes = 256 * header[6] + header[7];
	colors = 256 * header[8] + header[9];
	image_type = header[10];

	printf("n_dims = %d\n", n_dims);
	printf("cols   = %d (width)\n", cols);
	printf("rows   = %d (height)\n", rows);
	printf("planes = %d (depth)\n", planes);
	printf("colors = %d\n", colors);
	printf("type   = %c\n", image_type);
	fclose(f);
	} /* end main() */

/* Copyright 1997, Michael A. Gennert & Norman Wittels */

