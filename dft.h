#include <math.h>
#include <complex>

#include "3d_img.h"

#ifndef __DFT_UTILS__
#define __DFT_UTILS__

#define complexf std::complex<float>

#define ComplexTD TDImage< complexf >

#ifndef FOREACH_PIXEL_3D
#define FOREACH_PIXEL_3D(img,k,j,i,d) for (int d = 0; d < img->depth; d++) for (int i = 0; i < img->colors; i++) for (int j = 0; j < img->rows; j++) for(int k = 0; k < img->cols; k++)
#endif

#ifndef FOREACH_PIXEL_IN_COLOR
#define FOREACH_PIXEL_IN_COLOR(img,k,j) for (int j = 0; j < img->rows; j++) for (int k = 0; k < img->cols; k++)
#endif

inline float checkerboard(int x, int y)
{
  if ( (x + y) % 2 == 0) return 1.0f;
  else return -1.0f;
}

inline std::complex<float> _I()
{
  return (std::complex<float>(0,1));
}

inline std::complex<float> _I(float d)
{
  return (std::complex<float>(0,d));
}

inline std::complex<float> dft_kernel(float x, float y, float u, float v, float M, float N)
{
  return exp(complexf(0,-1.0) * 2.0f * (float)M_PI * ((u/M)*x + (v/N)*y));
}

inline std::complex<float> idft_kernel(float x, float y, float u, float v, float M, float N)
{
  return exp(complexf(0,1.0) * 2.0f * (float)M_PI * ((u/M)*x + (v/N)*y));
}

template <typename T> ComplexTD* dft_img(TDImage<T>* in_img)
{
  ComplexTD* complex_img = new ComplexTD(in_img->cols, in_img->rows, in_img->colors, in_img->depth);

  FOREACH_PIXEL_3D(in_img, u, v, color, depth)
  {
    std::complex<float> res = _I(0.0);
    FOREACH_PIXEL_IN_COLOR(in_img, x, y)
    {
      res += dft_kernel(x, y, u, v, in_img->cols, in_img->rows)*((float)(in_img->get_pixel(x, y, color,depth))*checkerboard(x,y));
    }
    complex_img->set_pixel(u, v, color, depth, res);
  }

  return complex_img;
}

inline std::complex<float> dft_1d_kernel(float s, float omega, float M)
{
  return exp(complexf(0,-1.0) * 2.0f * (float)M_PI * (omega/M)*s);
}

inline std::complex<float> idft_1d_kernel(float s, float omega, float M)
{
  return exp(complexf(0,1.0) * 2.0f * (float)M_PI * (omega/M)*s);
}

template <typename T> ComplexTD* dft_1d_img(TDImage<T>* in_img)
{
  ComplexTD* complex_img = new ComplexTD(in_img->cols, in_img->rows, in_img->colors, in_img->depth);

  FOREACH_PIXEL_3D(in_img, omega, theta, color, depth)
  {
    std::complex<float> res = _I(0.0);
    for (int s = 0; s < in_img->cols; s++) // just FT over s, not theta for now
    {
      res += dft_1d_kernel(s, omega, in_img->cols)*((float)(in_img->get_pixel(s, theta, color, depth))*checkerboard(s,theta));
    }
    complex_img->set_pixel(omega, theta, color, depth, res);
  }

  return complex_img;
}

TDImage<unsigned char>* inv_dft_img(ComplexTD* in_img)
{

  ComplexTD* tmp_img = new ComplexTD(in_img->cols, in_img->rows, in_img->colors, in_img->depth);
  TDImage<float>* tmp2_img = new TDImage<float>(in_img->cols, in_img->rows, in_img->colors, in_img->depth);

  // Columns
  FOREACH_PIXEL_3D(in_img, column, row, color, depth) {
    std::complex<float> res = _I(0.0);
    for(int s=0; s<in_img->cols; s++) {
      res += (idft_1d_kernel(s, column, in_img->cols)*(in_img->get_pixel(s, row, color, depth)));
    }
    tmp_img->set_pixel(column, row, color, depth, res);
  }

  // Rows
  FOREACH_PIXEL_3D(in_img, column, row, color, depth) {
    float res = 0.0;
    for(int s=0; s<in_img->rows; s++) {
      res += (idft_1d_kernel(s, row, in_img->rows)*(tmp_img->get_pixel(column, s, color, depth))).real();
    }
    tmp2_img->set_pixel(column, row, color, depth, checkerboard(column, row)*res/((float)in_img->rows*(float)in_img->cols));
  }

  // We had floats...we want not floats
  TDImage<unsigned char>* out_img = new TDImage<unsigned char>(in_img->cols, in_img->rows, in_img->colors, in_img->depth);

  FOREACH_PIXEL_3D(out_img, column, row, color, depth)
  {
    out_img->set_pixel(column, row, color, depth, tmp2_img->get_pixel(column, row, color, depth));  
  }

  delete tmp_img;
  delete tmp2_img;
  return out_img;
}

/*

// for some reason, this refuses to work as a template function
TDImage<unsigned char>* inv_dft_img(ComplexTD* in_img)
{

  TDImage<float>* tmp_img = new TDImage<float>(in_img->cols, in_img->rows, in_img->colors, in_img->depth);

  FOREACH_PIXEL_3D(in_img, column, row, color, depth)
  {
    float res = 0.0f;
    FOREACH_PIXEL_IN_COLOR(in_img, x, y)
    {
      res += (idft_kernel(x, y, column, row, in_img->cols, in_img->rows)*(in_img->get_pixel(x, y, color, depth))).real();
    }
    tmp_img->set_pixel(column, row, color, depth, checkerboard(column, row)*res/((float)in_img->rows*(float)in_img->cols));
  }

  TDImage<unsigned char>* out_img = new TDImage<unsigned char>(in_img->cols, in_img->rows, in_img->colors, in_img->depth);

  FOREACH_PIXEL_3D(out_img, column, row, color, depth)
  {
    out_img->set_pixel(column, row, color, depth, tmp_img->get_pixel(column, row, color, depth));  
  }

  delete tmp_img;
  return out_img;
}
*/
#endif
