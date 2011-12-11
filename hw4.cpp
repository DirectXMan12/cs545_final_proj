#include <iostream>
#include <complex>
#include "img_cpp.h"
#include "cimg_cpp.h"
#include "err_macros.h"
#include <stdlib.h>

#define FOREACH_PIXEL(img,k,j,i) for (int i = 0; i < img->colors; i++) for (int j = 0; j < img->rows; j++) for(int k = 0; k < img->cols; k++)
#define FOREACH_PIXEL_IN_COLOR(img,k,j) for (int j = 0; j < img->rows; j++) for (int k = 0; k < img->cols; k++)

#define CHAIN_OP(op) tmp_img = op; delete out_img; out_img = tmp_img
#define CHAIN_OPF(op) tmpc_img = op; delete c_img; c_img = tmpc_img;

using namespace std;

const float PI = 2*acos(0.0);

#define complexf complex<float>

/*complex<long double> operator "" _i(long double d)
{
  return complex<long double>(0, d);
}*/ // C++11 (C++0x) User-defined Literals don't work in GCC yet...

inline complex<float> _I()
{
  return (complex<float>(0,1));
}

inline complex<float> _I(float d)
{
  return (complex<float>(0,d));
}

inline complex<float> dft_kernel(float x, float y, float u, float v, float M, float N)
{
  return exp(complexf(0,-1.0) * 2.0f * PI * ((u/M)*x + (v/N)*y));
}

inline complex<float> idft_kernel(float x, float y, float u, float v, float M, float N)
{
  return exp(complexf(0,1.0) * 2.0f * PI * ((u/M)*x + (v/N)*y));
}

inline float checkerboard(int x, int y)
{
  if ( (x + y) % 2 == 0) return 1.0f;
  else if (x + y == 0) return 1.0f;
  else return -1.0f;
  //return 1.0f;
}

ComplexImage* log_scale(ComplexImage* in_img) // scale by 0.0001 (to maximize pop of real details) and then take the logarithm of the magnitude
{
  ComplexImage* out_img = new ComplexImage(in_img->cols, in_img->rows, in_img->colors);
  
  float c = 1.0f/log(1.0f + in_img->max_mag());
  FOREACH_PIXEL(out_img, column, row, color)
  {
    complexf src_pixel = in_img->get_pixel(column, row, color);
    src_pixel *= 0.0001;
    float src_mag = abs(src_pixel) + 1.0f;
    src_mag = log(src_mag);
    src_mag *= c;
    src_mag /= 2.0f; // this line and the next make it so that abs(complexf(src_mag, src_mag)) == src_mag_from_the_previous_line (i.e. the actual desired magnitude)
    src_mag = sqrt(src_mag);
    out_img->set_pixel(column, row, color, complexf(src_mag, src_mag));
    if (src_mag >= 1.0f) POST_WARN("val to big in log_scale: " << src_mag);
  }

  return out_img;
}

ComplexImage* dft_img(Image* in_img)
{
  ComplexImage* complex_img = new ComplexImage(in_img->cols, in_img->rows, in_img->colors);

  FOREACH_PIXEL(in_img, u, v, color)
  {
    complex<float> res = _I(0.0);
    FOREACH_PIXEL_IN_COLOR(in_img, x, y)
    {
      res += dft_kernel(x, y, u, v, in_img->cols, in_img->rows)*((float)(in_img->get_pixel(x, y, color))*checkerboard(x,y));
    }
    complex_img->set_pixel(u, v, color, res);
  }

  return complex_img;
}

Image* double_bar(Image* in_img)
{
  Image* out_img = new Image(40,40,1);
  FOREACH_PIXEL(out_img, column, row, color)
  {
    if (column % 4 == 0 || (column - 1) % 4 == 0) out_img->set_pixel(column, row, color, 0xff);
    else out_img->set_pixel(column, row, color, 0x00);
  }

  return out_img;
}

Image* inv_dft_img(ComplexImage* in_img)
{

  ComplexImage* complex_img = new ComplexImage(in_img->cols, in_img->rows, in_img->colors);

  FOREACH_PIXEL(in_img, column, row, color)
  {
    complex<float> res = 0.0f + _I(0.0);
    FOREACH_PIXEL_IN_COLOR(in_img, x, y)
    {
      res += idft_kernel(x, y, column, row, in_img->cols, in_img->rows)*(in_img->get_pixel(x, y, color));
    }
    complex_img->set_pixel(column, row, color, res/((float)in_img->rows*(float)in_img->cols));
  }

  Image* out_img = complex_img->to_image(); 

  FOREACH_PIXEL(out_img, column, row, color)
  {
    out_img->set_pixel(column, row, color, out_img->get_pixel(column, row, color)*checkerboard(column, row));  
  }

  delete complex_img;
  return out_img;
}

int main(int argc, char* argv[])
{
  Image *in_img = new Image(argv[1]);

  string in_name(argv[1]);
  string out_name(in_name.substr(0, in_name.rfind(".")));
  out_name += "-out.img";

  POST_INFO("in_img(cols x rows x colors): " << in_img->cols << " x " << in_img->rows << " x " << in_img->colors);
  Image *out_img = new Image(in_img);
  Image *tmp_img;

  bool exited_fourier_space = true;
  ComplexImage *c_img = new ComplexImage(in_img);
  ComplexImage *tmpc_img;

  for (int i = 0; i < argc-2; i++)
  {
    string curr_op = argv[i+2];
    if (curr_op == "dft")
    {
      CHAIN_OPF(dft_img(out_img));
      exited_fourier_space = false;
    }
    else if (curr_op == "inv_dft")
    {
      if (!exited_fourier_space)
      {
        CHAIN_OP(inv_dft_img(c_img));
        exited_fourier_space = true;
      }
      else
      {
        c_img = new ComplexImage(out_img);
        CHAIN_OP(inv_dft_img(c_img));
        delete c_img;
      }
    }
    else if (curr_op == "db")
    {
      CHAIN_OP(double_bar(out_img));
    }
    else if (curr_op == "log_scale")
    {
      if (!exited_fourier_space)
      {
        CHAIN_OPF(log_scale(c_img));
      }
      else POST_WARN("log_scale only works in Fourier space");
    }
    else
    {
      POST_WARN("Skipping unrecognized operation '" << curr_op << "'!");
    }
  }

  if (exited_fourier_space) out_img->write(out_name.c_str());
  else c_img->to_image()->write(out_name.c_str());

  POST_INFO("in_img(cols x rows x colors): " << in_img->cols << " x " << in_img->rows << " x " << in_img->colors << ", out_img(cols x rows x colors): " << out_img->cols << " x " << out_img->rows << " x " << out_img->colors);


  delete in_img;
  delete out_img;
  delete c_img;
  exit(0);
}
