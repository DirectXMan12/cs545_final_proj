/* cs545 F97/98, transform a gray-scale or color digital image into a tiff image */
/* Written by Norman Wittels */
/* 20Mar97	first version */
/* 28Mar97	Updated to TIFF v6, fixed a problem with a tag, added all "required" tags */
/* 30Apr97	Updated to include 24-bit rgb color */
/* 14Nov97 Updated to use new WPI format - Michael A. Gennert */

/* compile string:
cc wpi2tif.c -lm -o wpi2tif
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <time.h>
#include <float.h>

#define WHITE 255
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
wpi_image *read_wpi_image(char *file_name);
wpi_image *create_wpi_image(int cols, int rows, int planes, int colors, char image_type);
void destroy_wpi_image(wpi_image *img);
void erase_wpi_image(wpi_image *img, unsigned char *color_ptr);

/* Global variables */
static char *prog;

int main(int argc, char *argv[])
{
	FILE *f; /* the file */
	unsigned char header[16]={0}; /* the header information */
	char *in_file, *out_file;
	int n_dims, cols, planes, rows, colors;
	char image_type;
	wpi_image *in_image;
	long int size, strip;
	unsigned char *p2im; /* pointer to the image data */

	prog = argv[0];

	if (argc != 3) {
	  fprintf(stderr, "Usage is: %s input_file output_file\n", prog);
	  exit (-1);
	}

	in_file = argv[1]; out_file = argv[2];

	in_image = read_wpi_image(in_file);
	n_dims = in_image->n_dims;
	cols = in_image->cols;
	rows = in_image->rows;
	colors = in_image->colors;
	image_type = in_image->image_type;

	if (n_dims != 2) {
	  fprintf(stderr, "%s problem with %d dimensions - must be 2\n",
		  prog, n_dims);
	  exit(-1);
	}
	if (colors != 1 && colors != 3) {
	  fprintf(stderr,
		  "%s: problem with %d colors - must be 1 or 3\n",
		  prog, colors);
	  exit(-1);
	}

	if (image_type != 'c') {
	  fprintf(stderr,
		  "%s: problem with image type %c - must be c = char\n",
		  prog, image_type);
	  exit(-1);
	}

	size = cols * rows * colors; /* the image size (number of pixels) */

	if ((f = fopen(out_file,"wb"))==NULL) {
	  fprintf(stderr,
		  "%s: could not open output file %s\n",
		  prog, out_file);
	  exit(-1);
	} 

	/* write the tiff header */
	fputc('M',f); fputc('M',f);  /* byte order */
	fputc(0,f); fputc(42,f); /* "version" number */
	fputc(0,f); fputc(0,f); fputc(0,f); fputc(0x10,f); /* ifd offset */
	
	/* write the ifd */
	fseek(f,0x10,SEEK_SET); /* beginning of ifd */
	fputc(0,f); fputc(14,f); /* number of entries */

	fputc(0x00,f); fputc(0xfe,f); /* tag 254 - NEW SUBFILE TYPE */
	fputc(0x00,f); fputc(0x04,f); /* type LONG */
	fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
	fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); /* the usual */

	fputc(0x01,f); fputc(0x00,f); /* tag 256 - I_WIDTH */
	fputc(0x00,f); fputc(0x04,f); /* type LONG */
	fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
	fputc(0x00,f); fputc(0x00,f); fputc(cols / 256 % 256,f); fputc(cols % 256,f); /* the data */

	fputc(0x01,f); fputc(0x01,f); /* tag 257 - I_LENGTH */
	fputc(0x00,f); fputc(0x04,f); /* type LONG */
	fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
	fputc(0x00,f); fputc(0x00,f); fputc(rows / 256 % 256,f); fputc(rows % 256,f); /* the data */
	
	fputc(0x01,f); fputc(0x02,f); /* tag 258 - BITS_PER_SAMPLE */
	fputc(0x00,f); fputc(0x03,f); /* type SHORT */
	if (colors == 1) /* 8-bit grayscale */
		{
		fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
		fputc(0x00,f); fputc(0x08,f); fputc(0x00,f); fputc(0x00,f); /* the data - one byte per pixel */
		} /* end if(colors == 1) */
	if (colors == 3) /* 24-bit color */
		{
		fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x03,f); /* three of them */
		fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); fputc(0x20,f); /* location of the data */
		} /* end if(colors == 1) */
	
	fputc(0x01,f); fputc(0x03,f); /* tag 259 - COMPRESSION */
	fputc(0x00,f); fputc(0x03,f); /* type SHORT */
	fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
	fputc(0x00,f); fputc(0x01,f); fputc(0x00,f); fputc(0x00,f); /* no compression */
	
	fputc(0x01,f); fputc(0x06,f); /* tag 262 - PHOTOMETRIC INTERPRETATION */
	fputc(0x00,f); fputc(0x03,f); /* type SHORT */
	fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
	if (colors == 1)
		{
		fputc(0x00,f); fputc(0x01,f); fputc(0x00,f); fputc(0x00,f); /* grayscale level; Black is zero */
		} /* end if(colors) */
	if (colors == 3)
		{
		fputc(0x00,f); fputc(0x02,f); fputc(0x00,f); fputc(0x00,f); /* 24-bit rgb color; Black is zero */
		} /* end if(colors) */
	
	fputc(0x01,f); fputc(0X11,f); /* tag 273 - STRIP OFFSETS */
	fputc(0x00,f); fputc(0x04,f); /* type LONG */
	if (colors == 1)
		{
		fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
		fputc(0x00,f); fputc(0x00,f); fputc(0x03,f); fputc(0x00,f); /* beginning of first (and only) strip */
		} /* end if(colors) */
	if (colors == 3)
		{
		fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x03,f); /* 3 of them */
		fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); fputc(0x30,f); /* offset of first strip offset */
		} /* end if(colors) */

	fputc(0x01,f); fputc(0x15,f); /* tag 277 - SAMPLES PER PIXEL */
	fputc(0x00,f); fputc(0x03,f); /* type SHORT */
	fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
	if (colors == 1) /* 8-bit grayscale */
		{
		fputc(0x00,f); fputc(0x01,f); fputc(0x00,f); fputc(0x00,f); /* one sample per pixel */
		} /* end if(colors) */
	if (colors == 3) /* 24-bit rgb color */
		{
		fputc(0x00,f); fputc(0x03,f); fputc(0x00,f); fputc(0x00,f); /* three samples per pixel */
		} /* end if(colors) */
	
	fputc(0x01,f); fputc(0x16,f); /* tag 278 - ROWS PER STRIP */
	fputc(0x00,f); fputc(0x04,f); /* type LONG */
	fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
	fputc(0x00,f); fputc(0x00,f); fputc(rows /256 % 256,f); fputc(rows % 256,f); /* the count */
	
	fputc(0x01,f); fputc(0x17,f); /* tag 279 - STRIP BYTE COUNTS */
	fputc(0x00,f); fputc(0x04,f); /* type LONG */
	if (colors == 1)
		{
		fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
		fputc(size / 256 / 256 / 256 % 256, f);
		fputc(size / 256 / 256 % 256, f);
		fputc(size / 256 % 256,f);
		fputc(size % 256,f); /* the count */
		} /* end if(color) */
	if (colors == 3)
		{
		fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x03,f); /* three of them */
		fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); fputc(0x60,f); /* location of first item */
		} /* end if(color) */
	
	fputc(0x01,f); fputc(0x1A,f); /* tag 282 - X_RESOLUtiON */
	fputc(0x00,f); fputc(0x05,f); /* type RATIONAL */
	fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
	fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); fputc(0x00,f); /* location of the data */
	
	fputc(0x01,f); fputc(0x1B,f); /* tag 283 - Y_RESOLUTION */
	fputc(0x00,f); fputc(0x05,f); /* type RATIONAL */
	fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
	fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); fputc(0x10,f); /* location of the data */
	
	fputc(0x01,f); fputc(0x1C,f); /* tag 284 - PLANAR_CONFIGURATION */
	fputc(0x00,f); fputc(0x03,f); /* type SHORT */
	fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
	fputc(0x00,f); fputc(0x02,f); fputc(0x00,f); fputc(0x00,f); /* planar format */
	
	fputc(0x01,f); fputc(0x28,f); /* tag 296 - RESOLUTION_UNIT */
	fputc(0x00,f); fputc(0x03,f); /* type SHORT */
	fputc(0x00,f); fputc(0x00,f); fputc(0x00,f); fputc(0x01,f); /* only 1 of them */
	fputc(0x00,f); fputc(0x02,f); fputc(0x00,f); fputc(0x00,f); /* default value - inches */
	
	fputc(0,f); fputc(0,f); fputc(0,f); fputc(0,f); /* there is no other ifd */

	fseek(f,0x100,SEEK_SET); /* X_RESOLUTION data */	
	fputc(0,f); fputc(0,f); fputc(0,f); fputc(72,f); /* 72/inch is default */
	fputc(0,f); fputc(0,f); fputc(0,f); fputc(1,f);
	fseek(f,0x110,SEEK_SET); /* Y_RESOLUTION data */	
	fputc(0,f); fputc(0,f); fputc(0,f); fputc(72,f); /* 72/inch is default */
	fputc(0,f); fputc(0,f); fputc(0,f); fputc(1,f);
	fseek(f,0x120,SEEK_SET); /* BITS_PER_SAMPLE data */	
	fputc(0x00,f); fputc(0x08,f); /* one byte per color */
	fputc(0x00,f); fputc(0x08,f);
	fputc(0x00,f); fputc(0x08,f);
	fseek(f,0x130,SEEK_SET); /* STRIP_OFFSETS data */	
	strip = 0x300; /* beginning of red strip */
	fputc(strip / 256 / 256 / 256 % 256, f);
	fputc(strip / 256 / 256 % 256, f);
	fputc(strip / 256 % 256,f);
	fputc(strip % 256,f);
	strip = size / 3 + 0x300; /* beginning of green strip */
	fputc(strip / 256 / 256 / 256 % 256, f);
	fputc(strip / 256 / 256 % 256, f);
	fputc(strip / 256 % 256,f);
	fputc(strip % 256,f);
	strip = 2 * size / 3 + 0x300; /* beginning of blue strip */
	fputc(strip / 256 / 256 / 256 % 256, f);
	fputc(strip / 256 / 256 % 256, f);
	fputc(strip / 256 % 256,f);
	fputc(strip % 256,f);
	fseek(f,0x160,SEEK_SET); /* STRIP_BYTE_COUNTS data */	
	fputc(size / 3 / 256 / 256 / 256 % 256, f); /* red */
	fputc(size / 3 / 256 / 256 % 256, f);
	fputc(size / 3 / 256 % 256,f);
	fputc(size / 3 % 256,f);
	fputc(size / 3 / 256 / 256 / 256 % 256, f); /* green */
	fputc(size / 3 / 256 / 256 % 256, f);
	fputc(size / 3 / 256 % 256,f);
	fputc(size / 3 % 256,f);
	fputc(size / 3 / 256 / 256 / 256 % 256, f); /* blue */
	fputc(size / 3 / 256 / 256 % 256, f);
	fputc(size / 3 / 256 % 256,f);
	fputc(size / 3 % 256,f);

	fseek(f,0x300,SEEK_SET); /* where to begin writing data */	
	fwrite(in_image->image.c,1,size,f); /* write the data */
	destroy_wpi_image(in_image); /* clean up */
	} /* end main() */

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

	the_image->n_dims = n_dims;

	size = (long int) cols * (long int) rows * (long int) planes * (long int) colors; /* the image size (number of pixels) */

	if (fread(the_image->image.c,1,size,f) != size) /* read the image data */
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

/*** destroy a wpi_image ***/
void destroy_wpi_image(wpi_image *img)
	{
	if (img->image.c != NULL) free(img->image.c);
	if (img->next != NULL) free(img->next);
	free (img);
	return;
	} /* end destroy_wpi_image() */

 /*** set all pixels to a color ***/
void erase_wpi_image(wpi_image *img, unsigned char *color_ptr)
	{
	int size = img->cols * img->rows;
	int pixel = 0; int color = 0; /* loop counters */
	unsigned char *image_ptr = img->image.c; /* pointer to pixel data */
	for (color = 0; color < img->colors; color++) /* loop through color planes */
		{
		for (pixel=0;pixel<size;pixel++) /* loop through pixels within a color plane */
			*(image_ptr++) = *color_ptr; /* write a pixel */
		color_ptr++; /* go to next color */
		} /* end for(color) */
	} /* end erase_wpi_image() */

/* Copyright 1997, Michael A. Gennert & Norman Wittels */

