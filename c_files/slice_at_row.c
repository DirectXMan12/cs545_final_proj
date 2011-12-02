/* cs545 F97/98, file slice_at_row.c 
/* Produce slices for a given row -- each slice is col, angle */
/* Written by Michael A. Gennert */
/* 13Nov97	first version */

/* compile string:
cc slice_at_row.c -lm -o slice_at_row
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
#define WHITE 255

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
wpi_image *read_wpi_image(char *file_name);
wpi_image *create_wpi_image(int cols, int rows, int planes, int colors, char image_type);
int write_wpi_image(wpi_image *im, char *file_name);

/* Global variables */
static char *prog;

int main(int argc, char *argv[])
{
	FILE *f; /* the file */
	wpi_image *in_image, *out_image;
	char *in_file, *out_file;
	long int cols, rows, planes, colors;
	long int col, row, plane, color;
	
	prog = argv[0];

	if (argc != 4) {
	  fprintf(stderr,
		  "Usage is: %s input_file row output_file\n",
		  prog);
	  exit (-1);
	}

	in_file = argv[1];
	row = atoi( argv[2] );
	out_file = argv[3];

	in_image = read_wpi_image(in_file);

	cols = in_image->cols; rows = in_image->rows;
	planes = in_image->planes; colors = in_image->colors;

	out_image = create_wpi_image(cols, planes, 1, colors,
				     in_image->image_type);
	out_image->n_dims = 2;

	for(color = 0; color < colors; color++)
	  for(plane = 0; plane < planes; plane++)
	    for(col = 0; col < cols; col++)
	      out_image->image.c[col + cols * (plane + color * planes)] =
		in_image->image.c[col + cols * (row + rows * (plane + color * planes))];
	      
	write_wpi_image(out_image, out_file);
	
	exit(0);
}

/* Read wpi image from a file */
wpi_image *read_wpi_image(char *file_name)
{
        FILE *f;
	unsigned char header[16];
	unsigned int n_dims, cols, rows, planes, colors;
	char image_type;
	long int size;
	wpi_image *the_image;

	if ((f = fopen(file_name,"rb"))==NULL) /* could not open file */
		{
		fprintf(stderr, "%s: could not open input file %s\n",
			prog, file_name);
		exit(-1);
		} /* end if(fopen()) */

	if (fread(header,1,16,f) != 16)
		{
		fprintf(stderr, "%s: could not read file header %s\n",
			prog, file_name);
		fclose(f);
		exit(-1);
		}
	n_dims = 256 * header[0] + header[1];
	cols = 256 * header[2] + header[3];
	rows = 256 * header[4] + header[5];
	planes = 256 * header[6] + header[7];
	colors = 256 * header[8] + header[9];
	image_type = header[10];

	if (colors != 1 && colors != 3)
		{
		fprintf(stderr,
			"%s: problem with color number %d - must be 1 or 3\n",
			prog, colors);
		exit(-1);
		} /* end if(colors) */

	if (image_type != 'c') {
	  fprintf(stderr, "%s: image type %c not supported - must be type c\n",
		  prog, image_type);
	  exit(-1);
	}

	if ((the_image = create_wpi_image(cols,rows,planes,colors,image_type)) == NULL)
		{
		fprintf(stderr, "%s: could not allocate image\n", prog);
		fclose(f);
		exit(-1);
		} /* end if(the_image) */

	size = (long int) cols * (long int) rows * (long int) planes * (long int) colors; /* the image size (number of pixels) */

	if (fread(the_image->image,1,size,f) != size) /* read the image data */
		{
		fprintf(stderr, "%s: problem reading image %s\n",
			prog, file_name);
		fclose(f);
		exit(-1);
		} /* end if(fread()) */
	fclose(f);
	return(the_image);
}

/* allocate and initialize a wpi_image */
wpi_image *create_wpi_image(int cols, int rows, int planes, int colors, char image_type)
	{
	wpi_image *image;
	unsigned char *white; /* array to hold the color table for white */
	int n_color;
	/* create color table and set it to black */
	if ((white = malloc(colors)) == NULL) return NULL; /* could not allocate */
	for(n_color = 0; n_color < colors; n_color++) *(white + n_color) = WHITE;
	if ((image = malloc(sizeof(wpi_image))) == NULL) /* could not allocate */
		{
		free (white);
		return NULL;
		}
	if ((image->image.c = malloc((long int) cols * (long int) rows * (long int) planes * (long int) colors)) == NULL) /* could not allocate */
		{
		free (white);
		free (image);
		return NULL;
		}
	image->n_dims = 3;
	image->cols = cols;
	image->rows = rows;
	image->planes = planes;
	image->colors = colors;
	image->image_type = image_type;
	image->next = NULL;
	free (white);
	return image; 
	} /* end create_wpi_image() */

/*** write a wpi image into a file ***/
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

