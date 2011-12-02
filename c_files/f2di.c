/* cs545 F97/98, file f2di.c convert floating data set to wpi image */
/* Read in straight array of floats,
   output header and array of unsigned chars */
/* Written by Michael A. Gennert */
/* 13Nov97	first version */

/* compile string:
cc f2di.c -lm -o f2di
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <float.h>

#define MIN(a,b) ((a)<(b) ? (a) : (b)) /* return the minimum value */

typedef struct wpi_image
{
  unsigned int n_dims; /* number of dimensions */
  unsigned int cols; /* digital image size */
  unsigned int rows;
  unsigned int planes;
  unsigned int colors; /* 1 for gray scale; 3 for color */
  char image_type;     /* c, i, f, or d */
  union {              /* the image array */
    unsigned char *c;
    unsigned int *i;
    float *f;
    double *d;
  } image;
  struct wpi_image *next; /* for linked list */
} wpi_image; /* newly defined type */

/* prototypes */
int write_wpi_image(wpi_image *im, char *file_name);
wpi_image *convert_f_uc(wpi_image *in);

/* Global variables */
static char *prog;

int main(int argc, char *argv[])
	{
	FILE *f; /* the file */
	wpi_image the_image;
	char *in_file, *out_file;
	int cols, rows, angles;
	int colors=1;
	long int size;
	
	prog = argv[0];

	if (argc != 6) {
	  fprintf(stderr,
		  "Usage is: %s input_file cols rows angles output_file\n",
		  prog);
	  exit (-1);
	}

	in_file = argv[1];
	cols = atoi( argv[2] );
	rows = atoi( argv[3] );
	angles = atoi( argv[4] );
	out_file = argv[5];

	if ((f = fopen(in_file,"rb"))==NULL) /* could not open file */
		{
		fprintf(stderr,
			"%s: could not open input file %s\n",
			prog, in_file);
		exit(-1);
		} /* end if(fopen()) */

	the_image.n_dims = 3;
	the_image.cols = cols;
	the_image.rows = rows;
	the_image.planes = angles;
	the_image.colors = colors;
	the_image.image_type = 'f';
	size = (long int) cols * (long int) rows * (long int) angles * (long int) colors * sizeof (float);
	if ((the_image.image.f = malloc(size)) == NULL)
		{
		fprintf(stderr,
			"%s: could not allocate memory for digital image\n",
			prog);
		exit(-1);
		}

	/* read image */
	if (fread(the_image.image.f, 1, size, f) != size)
		{
		fprintf(stderr,
			"%s: difficulty reading the image\n", prog);
		free(the_image.image.f);
		exit(-1);
		} /* end if(fread()) */
	fclose(f);

	write_wpi_image(convert_f_uc(&the_image), out_file);
	exit(0);
	} 

/* Convert an image of floats to unsigned chars */
wpi_image *convert_f_uc(wpi_image *in)
{
	wpi_image *out= malloc(sizeof(wpi_image));
	long int size;
	float *f_data;
	unsigned char *data;
	long int i;

	out->n_dims = in->n_dims;
	out->cols = in->cols;
	out->rows = in->rows;
	out->planes = in->planes;
	out->colors = in->colors;
	out->image_type = 'c';

	size = (long int) out->cols * (long int) out->rows * (long int) out->planes * (long int) out->colors;
	if ((out->image.c = malloc(size)) == NULL)
	  {
	    fprintf(stderr,
		    "%s: could not allocate memory for output image\n",
		    prog);
	    exit(-1);
	  }

	for(f_data = in->image.f, data = out->image.c, i = 0;
	    i < size;
	    f_data++, data++, i++) {
	  *data = (unsigned char) *f_data;
	}
	
	return out;
}	    

/*** write a digital image into a file ***/
int write_wpi_image(wpi_image *im, char *file_name)
	{
	FILE *f;
	unsigned char header[16] = "" ;
	unsigned int size = (long int) im->rows * (long int) im->cols * (long int) im->planes * (long int) im->colors; /* image bytes */

	if ((f = fopen(file_name,"wb")) == NULL) /* open output file */
		{
		fprintf(stderr, "%s: could not open output file %s\n",
			prog, file_name);
		exit(-1);
		} /* end if() */

	if (im == NULL || f == NULL) return 1; /* error indicator */

	header[0] = MIN((im->n_dims / 256),255); /* high order bits */ 
	header[1] = im->n_dims % 256;            /* low order bits */
	header[2] = MIN((im->cols / 256),255);   /* high order bits */ 
	header[3] = im->cols % 256;              /* low order bits */
	header[4] = MIN((im->rows / 256),255);   /* high order bits */
	header[5] = im->rows % 256;              /* low order bits */
	header[6] = MIN((im->planes / 256),255); /* high order bits */
	header[7] = im->planes % 256;            /* low order bits */
	header[8] = MIN((im->colors / 256),255); /* high order bits */
	header[9] = im->colors % 256;            /* low order bits */
	header[10] = im->image_type;
	if (fwrite(header,1,16,f) != 16) return 1; /* write the header */
	if (fwrite(im->image.c, 1, size, f) != size) return 1; /* write the image data */
	return 0; /* happy return value */
	} /* end write_wpi_image() */

/* Copyright 1997, Michael A. Gennert & Norman Wittels */

