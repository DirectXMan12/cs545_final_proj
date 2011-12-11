#ifndef __DFT_UTILS__
#define __DFT_UTILS__

#define complexf complex<float>

#define ComplexTD ComplexImage<complexf>

#ifndef FOREACH_PIXEL_3D
#define FOREACH_PIXEL_3D(img,k,j,i,d) for (int d = 0; d < img->depth; d++) for (int i = 0; i < img->colors; i++) for (int j = 0; j < img->rows; j++) for(int k = 0; k < img->cols; k++)
#endif

#ifndef FOREACH_PIXEL_IN_COLOR
#define FOREACH_PIXEL_IN_COLOR(img,k,j) for (int j = 0; j < img->rows; j++) for (int k = 0; k < img->cols; k++)
#endif

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

template <typename T> ComplexTD* dft_img(TDImage<T>* in_img)
{
  ComplexTD* complex_img = new ComplexTD(in_img->cols, in_img->rows, in_img->colors, in_img->depth);

  FOREACH_PIXEL_3D(in_img, u, v, color, depth)
  {
    complex<float> res = _I(0.0);
    FOREACH_PIXEL_IN_COLOR(in_img, x, y)
    {
      res += dft_kernel(x, y, u, v, in_img->cols, in_img->rows)*((float)(in_img->get_pixel(x, y, color,depth))*checkerboard(x,y));
    }
    complex_img->set_pixel(u, v, color, depth, res);
  }

  return complex_img;
}

template <typename T> TDImage<T>* inv_dft_img(ComplexTD* in_img)
{

  TDImage<float>* tmp_img = new TDImage<float>(in_img->cols, in_img->rows, in_img->colors, in_img->depth);

  FOREACH_PIXEL_3D(in_img, column, row, color, depth)
  {
    float res = 0.0f;
    FOREACH_PIXEL_IN_COLOR(in_img, x, y)
    {
      res += idft_kernel(x, y, column, row, in_img->cols, in_img->rows)*(in_img->get_pixel(x, y, color, depth));
    }
    tmp_img->set_pixel(column, row, color, checkerboard(column, row)*res/((float)in_img->rows*(float)in_img->cols));
  }

  TDImage<T>* out_img = new TDImage<T>(in_img->cols, in_img->rows, in_img->colors, in_img->depth);

  FOREACH_PIXEL_3D(out_img, column, row, color, depth)
  {
    out_img->set_pixel(column, row, color, depth, tmp_img->get_pixel(column, row, color, depth));  
  }

  delete tmp_img;
  return out_img;
}

#endif
