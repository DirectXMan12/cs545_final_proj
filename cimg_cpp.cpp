#include "cimg_cpp.h"
#include "img_cpp.h"
#include "img.h"
#include <iostream>
#include <ios>
#include <fstream>
#include <complex>

#include <stdlib.h>
#include <string.h>

#include "err_macros.h"

#define FOREACH_PIXEL(img,k,j,i) for (int i = 0; i < img->colors; i++) for (int j = 0; j < img->rows; j++) for(int k = 0; k < img->cols; k++)

ComplexImage::ComplexImage(long cols, long rows, long colors)
{
  this->cols = cols;
  this->rows = rows;
  this->colors = colors;
  this->size = this->cols*this->rows*this->colors;

  this->image = new std::complex<float>[size+1];
}

ComplexImage::ComplexImage(const char* filename)
{
  image_file_header header;
  std::ifstream fp;

  /* open file and check for errors */
  fp.open(filename, std::ios::in | std::ios::binary);
  if (fp.bad() || fp.fail())
  {
    POST_ERR("Error opening file '" << filename << "' -- b,f: " << fp.bad() << "," << fp.fail());
    exit(-1);
  }

  /* read header and check for errors */
  fp.read((char*)(&header), sizeof(header));
  if (fp.eof())
  {
    POST_ERR("Early EOF on file '" << filename << "'");
    fp.close();
    exit(-1);
  }
  // should really check for other errors here...

  /* put msl and lsb together to get true values */
  this->cols = (header.cols_msb << 8) | header.cols_lsb;
  this->rows = (header.rows_msb << 8) | header.rows_lsb;
  this->colors = (header.colors_msb << 8) | header.colors_lsb;
  this->size = this->cols*this->rows*this->colors;

  this->image = new std::complex<float>[this->size];

  /* read image data and check for errors */
  fp.read((char*)(this->image), this->size);
  // check for errors here?

  fp.close();
  // check for close errors?
} 

ComplexImage::ComplexImage(ComplexImage* src_img)
{
  this->cols = src_img->cols;
  this->rows = src_img->rows;
  this->colors = src_img->colors;
  this->size = src_img->size;

  this->image = new std::complex<float>[this->size];
  memcpy(this->image, src_img->image, this->size);
}

ComplexImage::ComplexImage(Image* in_img)
{
  this->cols = in_img->cols;
  this->rows = in_img->rows;
  this->colors = in_img->colors;
  this->size = in_img->size;

  this->image = new std::complex<float>[this->size];
  FOREACH_PIXEL(this, column, row, color)
  {
    float res = in_img->get_pixel(column, row, color);
    res *= res;
    res /= 2;
    res = sqrt(res);

    this->set_pixel(column, row, color, std::complex<float>(res, res));
  }
}

ComplexImage::~ComplexImage()
{
  delete[] image;
}

Image* ComplexImage::to_image()
{
  Image* out_img = new Image(this->cols, this->rows, this->colors);
  
  float max_val = 0;
  FOREACH_PIXEL(out_img, column, row, color)
  {
    float res = abs(this->get_pixel(column, row, color));
    if (res > max_val) max_val = res;
  }

  FOREACH_PIXEL(out_img, column, row, color)
  {
    float p = abs(this->get_pixel(column, row, color));
    out_img->set_pixel(column, row, color, (unsigned char)((p*255.0f)/max_val));
  }

  return out_img;
}

float ComplexImage::max_mag()
{
  float max_val = 0;
  FOREACH_PIXEL(this, column, row, color)
  {
    float res = abs(this->get_pixel(column, row, color));
    if ( res > max_val ) max_val = res;
  }

  return max_val;
}


std::complex<float> ComplexImage::get_pixel(long col, long row, long color)
{
  return image[color*this->cols*this->rows + row*this->cols + col];   
}

std::complex<float> ComplexImage::set_pixel(long col, long row, long color, std::complex<float> val)
{
  return image[(color*this->cols*this->rows + row*this->cols + col)] = val;
}
