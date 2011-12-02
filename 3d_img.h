#ifndef __3D_IMG_CPP__
#define __3D_IMG_CPP__
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
#endif
