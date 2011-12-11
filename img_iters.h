#ifndef FOREACH_PIXEL
#define FOREACH_PIXEL(img,k,j,i) for (int i = 0; i < img->colors; i++) for (int j = 0; j < img->rows; j++) for(int k = 0; k < img->cols; k++)
#endif

#ifndef FOREACH_PIXEL_3D
#define FOREACH_PIXEL_3D(img,k,j,i,d) for (int d = 0; d < img->depth; d++) for (int i = 0; i < img->colors; i++) for (int j = 0; j < img->rows; j++) for(int k = 0; k < img->cols; k++)
#endif

#ifndef FOREACH_PIXEL_IN_COLOR
#define FOREACH_PIXEL_IN_COLOR(img,k,j) for (int j = 0; j < img->rows; j++) for (int k = 0; k < img->cols; k++)
#endif

#ifndef FOREACH_PIXEL_TOMO
#define FOREACH_PIXEL_TOMO(img,k,j,i,d) for (int d = 0; d < img->slices; d++) for (int i = 0; i < img->colors; i++) for (int j = 0; j < img->rows; j++) for(int k = 0; k < img->cols; k++)
#endif
