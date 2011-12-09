#include <complex>
#include "img_cpp.h"

class ComplexImage
{
  public:
    ComplexImage(long cols, long rows, long colors);
    ComplexImage(const char* filename);
    ComplexImage(Image* in_img);
    ComplexImage(ComplexImage* src_img);
    ~ComplexImage();

    std::complex<float> get_pixel(long col, long row, long color);
    std::complex<float> set_pixel(long col, long row, long color, std::complex<float> val);

    float max_mag();
    Image* to_image();
    
    long cols;
    long rows;
    long size;
    long colors;
    std::complex<float>* image;
};
