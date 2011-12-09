/**
 * test_data.dat:
 * Rows: 128
 * Cols: 128
 * Clrs: 1
 * Planes: 120
 * Size: 1966080
 */
#include <iostream>
#include "3d_img.h"
#include "tomo_img.h"
#include "cimg.h"
#include <math.h>

#include "err_macros.h"

using namespace std;

// Resample an image from (w, t) space to (u, v) space
// i.e. transform from polar to Cartesian
ComplexImage *resample(ComplexImage *wtSpace);

// Data currently in range [0, 80]
// Can use this later to scale to [0, 255]
void scaleData(long *, long, long, long);

int main(int argc, char* argv[]) {
  int row = 0;

  // Check args
  if(argc != 3) {
    cerr << "Usage: tomorec inputDataFile outputSliceFile" << endl;
    return -1;
  }

  TomoImage<char> *data = new TomoImage<char>(argv[1]);
  long C = data->colors, P = data->planes;
  long M = data->cols, N = data->rows;

  TomoImage<char> *slice = new TomoImage<char>(M, P, C, 1, data->image_type);
  slice->n_dims = 2;

  for(int clr=0; clr<C; clr++) {
    for(int pln=0; pln<P; pln++) {
      for(int col=0; col<M; col++) {
        slice->set_pixel(col, pln, clr, 0,
          data->get_pixel(col, row, clr, pln));
      }
    }
  }

  slice->write(argv[2]);

  delete data;
  delete slice;

  return 0;
}

// For now, use the method of grabbing a pixel from (w,t) space
// and placing it where it belongs in (u, v) space. This way, we
// don't have to worry about undefined values and out-of-bounds
// values.
// Also use nearest neighbor for now.
ComplexImage *resample(ComplexImage *wtSpace) {
  long M = wtSpace->cols, N = wtSpace->rows, C = wtSpace->colors;
  ComplexImage *uvSpace = new ComplexImage(M, N, C);

  for(int c=0; c<C; c++) {
    for(int t=0; t<N; t++) {
      for(int w=0; w<M; w++) {
	double u = (double)w * cos((double)t * 3.0 * M_PI / 180.0);
        double v = (double)w * sin((double)t * 3.0 * M_PI / 180.0);

        // For now, use nearest neighbor
	int u1 = (int)(u + 0.5);
	int v1 = (int)(v + 0.5);

	uvSpace->set_pixel(u, v, c,
          wtSpace->get_pixel(w, t, c));
      }
    }
  }
}

/**
 * Find the max and min values of an array of data.
 *
 * @param data The array to analyze.
 * @param n Size of the array.
 * @param max Address to store the max value.
 * @param min Address to store the min value.
 */
void findMaxAndMin(long *data, long n, long *max, long *min) {
  long px, curr;
  (*max) = (*min) = data[0];

  for(px=1; px<n; px++) {
    curr = data[px];

    if(curr > (*max)) {
      (*max) = curr;
    } else if(curr < (*min)) {
      (*min) = curr;
    }
  }
}

/**
 * Scale all values of an array to be in the range [minOut, maxOut].
 *
 * @param data The array to scale.
 * @param n The size of the array.
 * @param minOut Desired minimum value for output (0).
 * @param maxOut Desired maximum value for output (255).
 */
void scaleData(long *data, long n, long minOut, long maxOut) {
  long maxIn, minIn;
  findMaxAndMin(data, n, &maxIn, &minIn);

  long rIn = maxIn - minIn;
  long rOut = maxOut - minOut;

  for(int px=0; px<n; px++) {
    data[px] = (data[px] - minIn) * rOut / rIn + minOut;
  }
}
