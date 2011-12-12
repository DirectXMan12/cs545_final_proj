#ifndef __MORPHO_OPS__
#define __MORPHO_OPS__
#include <vector>
#include <algorithm>


// Perform erosion or dilation depending on whether the passed function
// combine represents either std::min_element or std::max_element (respectively)
// Uses  the structuring element B={(-1,0), (0,0), (1,0), (0,-1), (0,1)}
template <typename T> TDImage<T>* erode(TDImage<T>* in_img, int se[][2], int len)
{
  TDImage<T>* transformed = new TDImage<T>(in_img->cols, in_img->rows, in_img->colors, in_img->depth);
  FOREACH_PIXEL_3D(in_img, column, row, color, depth)
  {
    T pv = (T)255; // min pixel value
    for (int i = 0; i < len; i++)
    {
      // cap the x and y vals -- assume that outside borders, pixels take the same value as the border
      // value for that column and row
      int px = column + se[i][0];
      int py = row + se[i][1];
      if (px < 0) px = 0;
      else if (px >= in_img->cols) px = in_img->cols - 1;
      if (py < 0) py = 0;
      else if (py >= in_img->rows) py = in_img->rows - 1;

      if (in_img->get_pixel(px, py, depth, color) < pv) pv = in_img->get_pixel(px,py,color,depth);
    }

    transformed->set_pixel(column, row, color, depth, pv);
  }
  return transformed;
}

// Perform erosion or dilation depending on whether the passed function
// combine represents either std::min_element or std::max_element (respectively)
// Uses  the structuring element B={(-1,0), (0,0), (1,0), (0,-1), (0,1)}
template <typename T> TDImage<T>* dilate(TDImage<T>* in_img, int se[][2], int len)
{
  TDImage<T>* transformed = new TDImage<T>(in_img->cols, in_img->rows, in_img->colors, in_img->depth);
  FOREACH_PIXEL_3D(in_img, column, row, color, depth)
  {
    T pv = (T)0; // max pixel value
    for (int i = 0; i < len; i++)
    {
      // cap the x and y vals -- assume that outside borders, pixels take the same value as the border
      // value for that column and row
      int px = column + se[i][0];
      int py = row + se[i][1];
      if (px < 0) px = 0;
      else if (px >= in_img->cols) px = in_img->cols - 1;
      if (py < 0) py = 0;
      else if (py >= in_img->rows) py = in_img->rows - 1;

      if (in_img->get_pixel(px, py, color, depth) > pv) pv = in_img->get_pixel(px,py,color,depth);
    }

    transformed->set_pixel(column, row, color, depth, pv);
  }
  return transformed;
}

// computes A - B
template <typename T> TDImage<T>* subtract(TDImage<T>* in_img_a, TDImage<T>* in_img_b)
{
  TDImage<T>* out_img = new TDImage<T>(in_img_a->cols, in_img_a->rows, in_img_a->colors, in_img_a->depth);

  FOREACH_PIXEL_3D(out_img, column, row, color, depth)
  {
    out_img->set_pixel(column, row, color, depth, in_img_a->get_pixel(column, row, color, depth) - in_img_b->get_pixel(column, row, color, depth));
  }

  return out_img;
}


// Perform open operation by dilating eroded image
/*template <typename T> TDImage<T>* open(TDImage<T> *orig_img)
{
  return dilate(erode(orig_img));
}*/ // -- commented out b/c leaks memory -- that's why I have CHAIN_OP macro
#endif
