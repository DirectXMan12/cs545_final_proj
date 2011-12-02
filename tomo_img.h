#ifndef __TOMO_IMG_CPP__
#define __TOMO_IMG_CPP__
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
#endif
