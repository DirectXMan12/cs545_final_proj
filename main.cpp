/**
 * test_data.dat:
 * Rows: 128
 * Cols: 128
 * Clrs: 1
 * Pos': 120
 * Size: 1966080
 */
#include <iostream>
#include "3d_img.h"
#include "tomo_img.h"

#include "err_macros.h"

using namespace std;

// Data currently in range [0, 80]
// Can use this later to scale to [0, 255]
void scaleData(long *, long, long, long);

int main(int argc, char* argv[]) {
  // Check args
  if(argc != 2) {
    cerr << "Usage: tomorec inputDataFile" << endl;
    return -1;
  }

  TomoImage<char> *data = new TomoImage<char>(argv[1]);
  long C = data->colors, P = data->positions;
  long M = data->cols, N = data->rows;

  for(int clr=0; clr<C; clr++) {
    for(int pos=0; pos<P; pos++) {
      for(int row=0; row<N; row++) {
        for(int col=0; col<M; col++) {

        }
      }
    }
  }

  cout << "Rows: " << data->rows << endl;
  cout << "Cols: " << data->cols << endl;
  cout << "Clrs: " << data->colors << endl;
  cout << "Pos': " << data->positions << endl;
  cout << "Size: " << data->size << endl;

  delete data;

  return 0;
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
