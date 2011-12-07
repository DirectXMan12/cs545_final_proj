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
    TomoImage(long cols, long rows, long colors, long planes, char img_type);
    TomoImage(const char* filename);
    TomoImage(TomoImage* src_img);
    ~TomoImage();

    N get_pixel(long col, long row, long color, long plane);
    N set_pixel(long col, long row, long color, long plane, N val);

    inline float get_angle(long plane);

    void write(const char* filename);

    long n_dims;
    long cols;
    long rows;
    long size;
    long planes;
    long colors;
    char image_type;
    N* image;
};

template <typename T> TomoImage<T>::TomoImage(long cols, long rows, long colors, long planes, char image_type)
{
  this->cols = cols;
  this->rows = rows;
  this->colors = colors;
  this->planes = planes;
  this->size = this->cols*this->rows*this->colors*this->planes;
  this->image_type = image_type;

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
  this->n_dims = (header[0] << 8) | header[1];
  this->cols = (header[2] << 8) | header[3];
  this->rows = (header[4] << 8) | header[5];
  this->planes = (header[6] << 8) | header[7];
  this->colors = (header[8] << 8) | header[9];
  this->image_type = header[10];
  this->size = this->cols*this->rows*this->colors*this->planes;

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
  this->planes = src_img->planes;
  this->colors = src_img->colors;
  this->size = src_img->size;

  this->image = new T[this->size];
  memcpy(this->image, src_img->image, this->size);
}

template <typename T> TomoImage<T>::~TomoImage<T>()
{
  delete[] image;
}

template <typename T> T TomoImage<T>::get_pixel(long col, long row, long color, long plane)
{
  return image[this->cols*(row + this->rows*(plane + color*this->planes)) + col];
}

template <typename T> T TomoImage<T>::set_pixel(long col, long row, long color, long plane, T val)
{
  return image[this->cols*(row + this->rows*(plane + color*this->planes)) + col] = val;
}

template <typename T> inline float TomoImage<T>::get_angle(long pos)
{
  return 360.0f/(float)this->planes*pos;
}

template <typename T> void TomoImage<T>::write(const char* filename) {
  unsigned char header[16] = {0};
  std::ofstream fp;
  fp.open(filename, std::ios::out | std::ios::binary); 
  if (fp.bad() || fp.fail())
  {
    POST_ERR("Error opening file '" << filename << "' for writing -- b,f: " << fp.bad() << "," << fp.fail());
    exit (-1);
  }

  header[0] = (this->n_dims >> 8) & 255;
  header[1] = this->n_dims % 256;
  header[2] = (this->cols >> 8) & 255;
  header[3] = this->cols % 256;
  header[4] = (this->rows >> 8) & 255;
  header[5] = this->rows % 256;
  header[6] = (this->planes >> 8) & 255;
  header[7] = this->planes % 256;
  header[8] = (this->colors >> 8) & 255;
  header[9] = this->colors & 255;
  header[10] = this->image_type;

  fp.write((char*)(&header), sizeof(header));
  // check for errors?
  
  fp.write((char*)(this->image), this->size);
  // check for errors?

  fp.close();
  // check for close errors?
}

#endif
