/* file img.h - header file for cs545 image format */

/* 12Aug97 First version */
/* 0Sep97  Revised macros, fixed create_char_image() prototype */
/* 13Sep02 MAG cleaned it up */

#ifndef _IMG_H_
#define _IMG_H_

/*** image macros ***/
#define IMG_FILE_NAME_LENGTH 60 /* maximum file name length */
#define IMG_HDR_LENGTH       6
#define IMG_MAX_COLS         2048
#define IMG_MAX_ROWS         2048
#define IMG_MAX_COLORS       3
#define IMG_MAX_IMAGE_SIZE   (IMG_MAX_COLS * IMG_MAX_ROWS * IMG_MAX_COLORS)
#define IMG_WHITE            0xFF
#define IMG_BLACK            0x00


/*** utility macros ***/
#define ABS(a)   ((a)<0 ? -(a) : (a))
#define MIN(a,b) ((a)<(b) ? (a) : (b))
#define MAX(a,b) ((a)>(b) ? (a) : (b))
#define SQR(a)   ((a)*(a)) 
#define SIGN(a)  ((a)<0 ? -1 : ((a)>0 ? 1 : 0 ) ) 


/*** type definitions ***/
typedef struct {
  long cols;
  long rows;
  long colors;
  long size;     /* cols * rows * colors */
  unsigned char *image;
} img;

typedef struct {
  long cols;
  long rows;
  long colors;
  long size;     /* cols * rows * colors */
  double *image;
} double_image;

typedef struct {
  unsigned char cols_msb;
  unsigned char cols_lsb;
  unsigned char rows_msb;
  unsigned char rows_lsb;
  unsigned char colors_msb;
  unsigned char colors_lsb;
} image_file_header;


/*** function prototypes ***/
/* all prototypes now take prog_name as 1st arg */
img *read_img (char *, char *);                    /* read image file */
img *read_user_specified_img_file (char *);        /* query and read image */
void write_img (char *, img *, char *);            /* write image file */
void write_user_specified_img_file (char *, img *);/* query and write image */
img *copy_img (char *, img *);                     /* duplicate image struct */
img *create_img (char *, long, long, long);           /* create image struct */
void destroy_img (char *, img *);                  /* destroy image struct */

/* Copyright 1997 - 2002 Norman Wittels and Michael A. Gennert */

#endif
