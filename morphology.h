#ifndef __MORPHO_OPS__
#define __MORPHO_OPS__
#include <vector>
#include <algorithm>

// Perform erosion or dilation depending on whether the passed function
// combine represents either std::min_element or std::max_element (respectively)
// Uses  the structuring element B={(-1,0), (0,0), (1,0), (0,-1), (0,1)}
template <typename T> TDImage<T>* erode(TDImage<T>* in_img)
{
  TDImage<T>* transformed = new TDImage<T>(in_img->cols, in_img->rows, in_img->colors, in_img->depth);
  FOREACH_PIXEL_3D(in_img, column, row, color, depth)
  {
    std::vector<T> struct_elt;

    struct_elt.push_back(in_img->get_pixel(column, row, color, depth));
    if(row != 0) struct_elt.push_back(in_img->get_pixel(column, row-1, color, depth));
    if(row != in_img->rows - 1) struct_elt.push_back(in_img->get_pixel(column, row+1, color, depth));
    if(column != 0) struct_elt.push_back(in_img->get_pixel(column-1, row, color, depth));
    if(column != in_img->cols - 1) struct_elt.push_back(in_img->get_pixel(column+1, row, color, depth));

    transformed->set_pixel(column, row, color, depth, *std::min_element(struct_elt.begin(), struct_elt.end()));
  }
  return transformed;
}

// Perform erosion or dilation depending on whether the passed function
// combine represents either std::min_element or std::max_element (respectively)
// Uses  the structuring element B={(-1,0), (0,0), (1,0), (0,-1), (0,1)}
template <typename T> TDImage<T>* dilate(TDImage<T>* in_img)
{
  TDImage<T>* transformed = new TDImage<T>(in_img->cols, in_img->rows, in_img->colors, in_img->depth);
  FOREACH_PIXEL_3D(in_img, column, row, color, depth)
  {
    std::vector<T> struct_elt;

    struct_elt.push_back(in_img->get_pixel(column, row, color, depth));
    if(row != 0) struct_elt.push_back(in_img->get_pixel(column, row-1, color, depth));
    if(row != in_img->rows - 1) struct_elt.push_back(in_img->get_pixel(column, row+1, color, depth));
    if(column != 0) struct_elt.push_back(in_img->get_pixel(column-1, row, color, depth));
    if(column != in_img->cols - 1) struct_elt.push_back(in_img->get_pixel(column+1, row, color, depth));

    transformed->set_pixel(column, row, color, depth, *std::min_element(struct_elt.begin(), struct_elt.end()));
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
