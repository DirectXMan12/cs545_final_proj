/**
 * test_data.dat:
 * Rows: 128
 * Cols: 128
 * Clrs: 1
 * Planes: 120
 * Size: 1966080
 */
#include <iostream>
#include <math.h>
#include <complex>

#include "3d_img.h" // most of the data will be worked with in this format -- represents a series of stacked up images -- TDImage<T>
#include "tomo_img.h" // represents the data before any backprojection -- in angles -- TomoImage<T>
#include "dft.h" // contains DFT and IDFT functions, as well as some helper functions
#include "img_iters.h" // contains the image pixel iterator helpers -- FOREACH_PIXEL(img, column, row, color), FOREACH_PIXEL_3D(img, column, row, color, depth), FOREACH_PIXEL_IN_COLOR(img, column, row), FOREACH_PIXEL_TOMO(img, column, row, color, slice_num)
#include "morphology.h" // has morphological image processing operations dialate and erode

#include "err_macros.h" // contains the error macros -- POST_ERR(stream), POST_WARN(stream), and POST_INFO(stream)

using namespace std;

int se_b[9][2] = {{-1,-1},{0,-1},{1,-1}, {-1,0},{0,0},{1,0}, {-1,1},{0,1},{1,1}};

// converts a TomoImage into a TDImage with the x axis bein omega and the y axis being theta
template <typename T> TDImage<T>* CreateSinogram(TomoImage<T>* in_img)
{
  TDImage<T>* out_img = new TDImage<T>(in_img->cols, in_img->planes, in_img->colors, in_img->rows); // depth = rows in each projection, cols = cols in projection, rows = planes in projection

  FOREACH_PIXEL_TOMO(in_img, column, row, color, slice)
  {
    out_img->set_pixel(column, slice, color, row, in_img->get_pixel(column, row, color, slice)); // column = s (aka column), row = slice number, depth = row
  }

  return out_img;
}

// takes in the dft of a sinogram, multiplies by ramp filter (abs(omega)) where omega is shifted to be from neg to pos, not 0 to pos*2
ComplexTD* DeblurDFT(ComplexTD* in_img)
{
  ComplexTD* out_img = new ComplexTD(in_img->cols, in_img->rows, in_img->colors, in_img->depth);

  FOREACH_PIXEL_3D(in_img, omega, theta, color, depth)
  {
    out_img->set_pixel(omega, theta, color, depth, (float)abs((int)(omega - in_img->cols/2))*in_img->get_pixel(omega, theta, color, depth));
  }

  return out_img;
}

// Resample an image from (w, t) space to (u, v) space
// i.e. transform from polar to Cartesian
ComplexTD* resample(ComplexTD *wtSpace);

// Data currently in range [0, 80]
// Can use this later to scale to [0, 255]
void scaleData(long *, long, long, long);

#define CHAIN_OP(op) tmp_img = op; delete out_img; out_img = tmp_img
#define CHAIN_OPF(op) tmpc_img = op; delete c_img; c_img = tmpc_img;

int main(int argc, char* argv[])
{
  // steps: take input slices, convert to sinograms, 1D-DFT along s -> omega, run filter, backproject, IDFT to original 3D Image
  TomoImage<unsigned char> *in_img = new TomoImage<unsigned char>(argv[1]);

  string in_name(argv[1]);
  string out_name(in_name.substr(0, in_name.rfind(".")));
  out_name += "-out.img";

  TDImage<unsigned char>* out_img = new TDImage<unsigned char>(1,1,1,1); // just a placeholder for now
  TDImage<unsigned char>* tmp_img;

  ComplexTD* c_img = new ComplexTD(1,1,1,1);
  ComplexTD* tmpc_img;

  // Part 1 -- get a 3D grid of pixels
  CHAIN_OP(CreateSinogram(in_img));
  CHAIN_OPF(dft_1d_img(out_img));
  CHAIN_OPF(DeblurDFT(c_img));
  CHAIN_OPF(resample(c_img));
  CHAIN_OP(inv_dft_img(c_img));
  
  out_img->write(out_name.c_str());

  // Part 2 -- prepare for output to obj file
  // open = erode then dialate
  TDImage<unsigned char>* pristine_out_img = new TDImage<unsigned char>(out_img);
  CHAIN_OP(dilate(out_img, se_b, 9));
  CHAIN_OP(subtract(out_img, pristine_out_img));
  
  delete in_img;
  delete c_img;

  out_img->write((out_name+".fc2obj").c_str());

  delete out_img;

  return 0;
}


// For now, use the method of grabbing a pixel from (w,t) space
// and placing it where it belongs in (u, v) space. This way, we
// don't have to worry about undefined values and out-of-bounds
// values.
// Also use nearest neighbor for now.
ComplexTD* resample(ComplexTD *wtSpace) {
  long cols = wtSpace->cols, rows = wtSpace->rows, colors = wtSpace->colors;
  ComplexTD *uvSpace = new ComplexTD(cols, rows, colors, wtSpace->depth);


  FOREACH_PIXEL_3D(wtSpace, column, row, color, depth)
  {
	  double u = (double)column * cos((double)row * 3.0 * M_PI / 180.0);
    double v = (double)column * sin((double)row * 3.0 * M_PI / 180.0);

    // For now, use nearest neighbor
    int u1 = (int)(u + 0.5);
    int v1 = (int)(v + 0.5);

    uvSpace->set_pixel(u1, v1, color, depth,
                                  wtSpace->get_pixel(column, row, color, depth));
  }

  return uvSpace;
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
