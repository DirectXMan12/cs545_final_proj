#ifndef __3D_IMG_CPP__
#define __3D_IMG_CPP__

#include <iostream>
#include <ios>
#include <fstream>

#include <stdlib.h>
#include <string.h>

#include "err_macros.h"

template <typename T> class TDImage
{
  public:
    TDImage(long cols, long rows, long colors, long depth);
    TDImage(const char* src_img);
    TDImage(TDImage* src_img);
    ~TDImage();

    T get_pixel(long col, long row, long color, long d);
    T set_pixel(long col, long row, long color, long d, T val);

    void write(const char* filename);

    long cols;
    long rows;
    long colors;
    long depth;
    long size;
    T* image;
};

template <typename T> TDImage<T>::TDImage(long cols, long rows, long colors, long depth)
{
  this->cols = cols;
  this->rows = rows;
  this->colors = colors;
  this->depth = depth;
  this->size = this->cols*this->rows*this->colors*this->depth;

  this->image = new T[size+1];
}

template <typename T> TDImage<T>::TDImage(const char* filename)
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
  this->depth = (header[6] << 8) | header[7];
  this->colors = (header[8] << 8) | header[9];
  this->size = this->cols*this->rows*this->colors*this->depth;

  this->image = new T[this->size];

  /* read image data and check for errors */
  fp.read((T*)(this->image), this->size);
  // check for errors here?

  fp.close();
  // check for close errors?
} 

template <typename T> TDImage<T>::TDImage(TDImage<T>* src_img)
{
  this->cols = src_img->cols;
  this->rows = src_img->rows;
  this->depth = src_img->depth;
  this->colors = src_img->colors;
  this->size = src_img->size;

  this->image = new T[this->size];
  memcpy(this->image, src_img->image, this->size);
}

template <typename T> TDImage<T>::~TDImage<T>()
{
  delete[] image;
}

template <typename T> T TDImage<T>::get_pixel(long col, long row, long color, long pos)
{
  return image[this->cols*(row + this->rows*(pos + color*this->depth)) + col];   
}

template <typename T> T TDImage<T>::set_pixel(long col, long row, long color, long pos, T val)
{
  return image[this->cols*(row + this->rows*(pos + color*this->depth)) + col] = val;   
}

template <typename T> void TDImage<T>::write(const char* filename)
{
  unsigned char header[16] = {0};
  std::ofstream fp;
  fp.open(filename, std::ios::out | std::ios::binary); 
  if (fp.bad() || fp.fail())
  {
    POST_ERR("Error opening file '" << filename << "' for writing -- b,f: " << fp.bad() << "," << fp.fail());
    exit (-1);
  }

  header[2] = (this->cols >> 8) & 255;
  header[3] = this->cols & 255;
  header[4] = (this->rows >> 8) & 255;
  header[5] = this->rows & 255;
  header[6] = (this->colors >> 8) & 255;
  header[7] = this->colors & 255;
  header[8] = (this->depth >> 8) & 255;
  header[9] = this->depth & 255;

  fp.write((char*)(&header), sizeof(header));
  // check for errors?
  
  fp.write((char*)(this->image), this->size);
  // check for errors?

  fp.close();
  // check for close errors?
}

#endif
