#ifndef __TOMO_IMG_CPP__
#define __TOMO_IMG_CPP__

#include "tomo_img.h"
#include <iostream>
#include <ios>
#include <fstream>

#include <stdlib.h>
#include <string.h>

#include "err_macros.h"

template <typename N> class TomoImage
{
  public:
    TomoImage(long cols, long rows, long colors, long positions);
    TomoImage(const char* filename);
    TomoImage(TomoImage* src_img);
    ~TomoImage();

    N get_pixel(long col, long row, long color, long pos);
    N set_pixel(long col, long row, long color, long pos, N val);

    inline float get_angle(long pos);

    long cols;
    long rows;
    long size;
    long positions;
    long colors;
    char image_type;
    N* image;
};

template <typename T> TomoImage<T>::TomoImage(long cols, long rows, long colors, long positions)
{
  this->cols = cols;
  this->rows = rows;
  this->colors = colors;
  this->positions = positions;
  this->size = this->cols*this->rows*this->colors*this->positions;

  this->image = new T[size+1];
}

template <typename T> TomoImage<T>::TomoImage(const char* filename)
{
  unsigned char header[16];
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
  this->cols = (header[2] << 8) | header[3];
  this->rows = (header[4] << 8) | header[5];
  this->positions = (header[6] << 8) | header[7];
  this->colors = (header[8] << 8) | header[9];
  this->size = this->cols*this->rows*this->colors*this->positions;

  this->image = new T[this->size];

  /* read image data and check for errors */
  fp.read((char*)(this->image), this->size);
  // check for errors here?

  fp.close();
  // check for close errors?
} 

template <typename T> TomoImage<T>::TomoImage(TomoImage<T>* src_img)
{
  this->cols = src_img->cols;
  this->rows = src_img->rows;
  this->positions = src_img->positions;
  this->colors = src_img->colors;
  this->size = src_img->size;

  this->image = new T[this->size];
  memcpy(this->image, src_img->image, this->size);
}

template <typename T> TomoImage<T>::~TomoImage<T>()
{
  delete[] image;
}

template <typename T> T TomoImage<T>::get_pixel(long col, long row, long color, long pos)
{
  return image[this->cols*(row + this->rows*(pos + color*this->positions)) + col];   
}

template <typename T> T TomoImage<T>::set_pixel(long col, long row, long color, long pos, T val)
{
  return image[this->cols*(row + this->rows*(pos + color*this->positions)) + col] = val;   
}

template <typename T> inline float TomoImage<T>::get_angle(long pos)
{
  return 360.0f/(float)this->positions*pos;
}

#endif
