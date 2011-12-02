TOMOGRAPHIC DATA
We provide a sample data set using wpi_image format.  This data set is
structured as

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

Example programs that read/write this format are:
f2di.c          -- read in array of floats and write wpi_image format
wpi_prt_head.c  -- print the header
wpi2tif.c       -- convert to tiff

Data set is organized as columns, rows, planes.  Each plane is a
complete tomographic projection taken at a different angle.  3 degree
spacing between angles.  Original object is a SPECT simulation of a
human torso.  It appears to be upside down, i.e., lower numbered rows
should be at the bottom, but they come out on top.

Data set has maximum intensity of 80 or so, which does not show up at
all.  Must scale brightness to view.  Do not scale brightness to
reconstruct!

Program that extracts data:
slice_at_row.c    -- get all data for a given row
                     (called a"sinogram")

Project goal is to reconstruct original object using any method you
choose.  If possible, generating an animated gif of the
unreconstructed dataset would make a nice presentation.  Should scale
before viewing.  Reconstructed dataset should allows slices along x,
y, or z directions.  Generating an animated gif from slices also makes
for a nice presentation.

Your project should describe the approaches considered and taken,
specific algorithms, literature citations where appropriate, and
demonstrations of your results.

