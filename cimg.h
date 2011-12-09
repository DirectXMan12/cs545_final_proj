#include <complex>
#include "tomo_img.h"

// All TomoImage instances are assumed to be 2D

class ComplexImage
{
  public:
    ComplexImage(long cols, long rows, long colors);
    ComplexImage(const char* filename);
    ComplexImage(TomoImage<char>* in_img);
    ComplexImage(ComplexImage* src_img);
    ~ComplexImage();

    std::complex<float> get_pixel(long col, long row, long color);
    std::complex<float> set_pixel(long col, long row, long color, std::complex<float> val);

    float max_mag();
    TomoImage<char>* to_tomo_image();

    long cols;
    long rows;
    long size;
    long colors;
    std::complex<float>* image;
};
