/**
 * test_data.dat:
 * Rows: 128
 * Cols: 128
 * Clrs: 1
 * Planes: 120
 * Size: 1966080
 */
#include <iostream>
#include <math.h>
#include <complex>

#include <stdlib.h>
#include "GL/glut.h"

#include "3d_img.h" // most of the data will be worked with in this format -- represents a series of stacked up images -- TDImage<T>
#include "tomo_img.h" // represents the data before any backprojection -- in angles -- TomoImage<T>
#include "dft.h" // contains DFT and IDFT functions, as well as some helper functions
#include "img_iters.h" // contains the image pixel iterator helpers -- FOREACH_PIXEL(img, column, row, color), FOREACH_PIXEL_3D(img, column, row, color, depth), FOREACH_PIXEL_IN_COLOR(img, column, row), FOREACH_PIXEL_TOMO(img, column, row, color, slice_num)
#include "morphology.h" // has morphological image processing operations dialate and erode

#include "err_macros.h" // contains the error macros -- POST_ERR(stream), POST_WARN(stream), and POST_INFO(stream)

using namespace std;

int se_b[9][2] = {{-1,-1},{0,-1},{1,-1}, {-1,0},{0,0},{1,0}, {-1,1},{0,1},{1,1}};
struct edgeInfo
{
  vector<int[3]> v;
  vector<int[2]> e;
};
struct edgeInfo ve;

// converts a TomoImage into a TDImage with the x axis bein omega and the y axis being theta
template <typename T> TDImage<T>* CreateSinogram(TomoImage<T>* in_img)
{
  TDImage<T>* out_img = new TDImage<T>(in_img->cols, in_img->planes, in_img->colors, in_img->rows); // depth = rows in each projection, cols = cols in projection, rows = planes in projection

  FOREACH_PIXEL_TOMO(in_img, column, row, color, slice)
  {
    out_img->set_pixel(column, slice, color, row, in_img->get_pixel(column, row, color, slice)); // column = s (aka column), row = slice number, depth = row
  }

  return out_img;
}

// takes in the dft of a sinogram, multiplies by ramp filter (abs(omega)) where omega is shifted to be from neg to pos, not 0 to pos*2
ComplexTD* DeblurDFT(ComplexTD* in_img)
{
  ComplexTD* out_img = new ComplexTD(in_img->cols, in_img->rows, in_img->colors, in_img->depth);

  FOREACH_PIXEL_3D(in_img, omega, theta, color, depth)
  {
    out_img->set_pixel(omega, theta, color, depth, (abs((float)omega/(float)in_img->cols - 0.5f))*in_img->get_pixel(omega, theta, color, depth));
  }

  return out_img;
}

template <typename T> TDImage<T>* backproject(TDImage<T>* in_img)
{
  TDImage<T>* out_img = new TDImage<T>(in_img->cols, in_img->cols, in_img->colors, in_img->depth);

  FOREACH_PIXEL_3D(out_img, column, row, color, depth)
  {
    float val = 0.0f;
    for (int theta = 0; theta < in_img->rows; theta++)
    {
      float angle = theta*3.0f;
      float c = column - out_img->cols/2;
      float r = row - out_img->rows/2;
      float s = c*sin(angle/180.0f * M_PI) + r*cos(angle/180.0f * M_PI) + in_img->cols/2;
      val += in_img->get_pixel((int)s, theta, color, depth);
    }
    val /= 12; // max theta vals
    //if(val > 255.0f) POST_ERR(val);
    //else POST_INFO(val);
    out_img->set_pixel(column, row, color, depth, (T)val);
  }

  return out_img;
}

template <typename T> TDImage<T>* resample(TDImage<T>* in_img)
{
  ComplexTD* out_img = new ComplexTD(in_img->cols, in_img->cols, in_img->colors, in_img->depth);

  FOREACH_PIXEL_3D(in_img, u, v, color, depth)
  {
    int c = in_img->cols/2;
    int omega = sqrt((u - c)*(u - c) + (v - c)*(v - c));
    int theta = 270;
    if (u - c != 0) theta = (atan((v - c)/(u - c)))*180/(3*M_PI);


    out_img->set_pixel(u,v,color,depth, in_img->get_pixel(omega, theta, color, depth));
  }

  return out_img;
}


TDImage<unsigned char>* lim_range(TDImage<unsigned char>* in_img, int plus)
{
  TDImage<unsigned char>* out_img = new TDImage<unsigned char>(in_img->cols, in_img->rows, in_img->colors, plus);

  for (int depth = 0; depth < plus; depth++)
  {
    FOREACH_PIXEL(in_img, column, row, color)
    {
      out_img->set_pixel(column, row, color, depth, in_img->get_pixel(column, row, color, 18 + depth));
    }
  }

  return out_img;
}

TDImage<unsigned char>* c_to_d(ComplexTD* in_img)
{
  TDImage<unsigned char>* out_img = new TDImage<unsigned char>(in_img->cols, in_img->rows, in_img->colors, in_img->depth);

  FOREACH_PIXEL_3D(in_img, column, row, color, depth)
  {
    out_img->set_pixel(column, row, color, depth, abs(in_img->get_pixel(column, row, color, depth)));
  }

  return out_img;
}

template <typename T> TDImage<T>* scale_vals(TDImage<T>* in_img, int tmax)
{
  TDImage<T>* out_img = new TDImage<T>(in_img->cols, in_img->rows, in_img->colors, in_img->depth);

  int max = 0;
  FOREACH_PIXEL_3D(out_img, column, row, color, depth)
  {
    int v = in_img->get_pixel(column, row, color, depth);
    if (v > max) max = v;
  }
  float scale_factor = (float)tmax/(float)max;

  FOREACH_PIXEL_3D(out_img, column, row, color, depth)
  {
    out_img->set_pixel(column, row, color, depth, (T)(scale_factor*in_img->get_pixel(column, row, color, depth)));
  }

  return out_img;
} 

// Data currently in range [0, 80]
// Can use this later to scale to [0, 255]
void scaleData(long *, long, long, long);

#define CHAIN_OP(op) tmp_img = op; delete out_img; out_img = tmp_img
#define CHAIN_OPF(op) tmpc_img = op; delete c_img; c_img = tmpc_img;

int main(int argc, char* argv[])
{
  // steps: take input slices, convert to sinograms, 1D-DFT along s -> omega, run filter, backproject, IDFT to original 3D Image
  TomoImage<unsigned char> *in_img = new TomoImage<unsigned char>(argv[1]);

  string in_name(argv[1]);
  string out_name(in_name.substr(0, in_name.rfind(".")));
  out_name += "-out.img";

  TDImage<unsigned char>* out_img = new TDImage<unsigned char>(1,1,1,1); // just a placeholder for now
  TDImage<unsigned char>* tmp_img;

  ComplexTD* c_img = new ComplexTD(1,1,1,1);
  ComplexTD* tmpc_img;

  // Part 1 -- get a 3D grid of pixels
  POST_INFO("Creating Sinograms...");
    CHAIN_OP(CreateSinogram(in_img));
    out_img->write("test_data-sinograms.img");
  //  CHAIN_OP(lim_range(out_img, 5));
  POST_INFO("1D DFT...");
    CHAIN_OPF(dft_1d_img(out_img));
  POST_INFO("Deblur...");
    CHAIN_OPF(DeblurDFT(c_img));
  //POST_INFO("Resample...");
  //  CHAIN_OPF(resample(c_img));
  POST_INFO("1D IDFT...");
    CHAIN_OP(inv_dft_1d_img(c_img));
  //POST_INFO("2D IDFT");
  //    CHAIN_OP(inv_dft_img(c_img));
  POST_INFO("Backproject");
    CHAIN_OP(backproject(out_img));
  //POST_INFO("scaling");
  //  CHAIN_OP(scale_vals(out_img, 255));
  POST_INFO("Done!");
  
  out_img->write(out_name.c_str());
/*
  // Part 2 -- prepare for output to obj file
  // open = erode then dialate
  TDImage<unsigned char>* pristine_out_img = new TDImage<unsigned char>(out_img);
  CHAIN_OP(dilate(out_img, se_b, 9));
  CHAIN_OP(subtract(out_img, pristine_out_img));
  
  delete in_img;
  delete c_img;

  out_img->write((out_name+".fc2obj").c_str());
*/
  delete out_img;

  return 0;
}

/**
 * Find the max and min values of an array of data.
 *
 * @param data The array to analyze.
 * @param n Size of the array.
 * @param max Address to store the max value.
 * @param min Address to store the min value.
 */
void findMaxAndMin(long *data, long n, long *max, long *min) {
  long px, curr;
  (*max) = (*min) = data[0];

  for(px=1; px<n; px++) {
    curr = data[px];

    if(curr > (*max)) {
      (*max) = curr;
    } else if(curr < (*min)) {
      (*min) = curr;
    }
  }
}

struct camera
{
  float x,y,z,lx,ly,lz;
  float angle;
};

struct camera cam;

void setCam()
{
  glLoadIdentity();
  gluLookAt(cam.x,cam.y,cam.z,cam.lx,cam.ly,cam.lz, 0, 1.0f, 0);
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glColor3f(1.0,1.0,1.0);

  setCam();

  glBegin(GL_LINES);
  for(int i = 0; i < ve.e.size(); i++)
  {
    int j = ve.e[i][0];
    int k = ve.e[i][1];
    glVertex3i(ve.v[j][0], ve.v[j][1], ve.v[k][2]);
    glVertex3i(ve.v[k][0], ve.v[k][1], ve.v[k][2]);
  }
  glEnd();
  glutSwapBuffers();
}

void processSpecialKeys(int key, int xx, int yy) 
{
  float fraction = 0.1f;

  switch (key)
  {
    case GLUT_KEY_LEFT :
      cam.angle -= 0.01f;
      cam.lx = sin(cam.angle);
      cam.lz = -cos(cam.angle);
      break;
    case GLUT_KEY_RIGHT :
      cam.angle += 0.01f;
      cam.lx = sin(cam.angle);
      cam.lz = -cos(cam.angle);
      break;
    case GLUT_KEY_UP :
      cam.x += cam.lx * fraction;
      cam.z += cam.lz * fraction;
      break;
    case GLUT_KEY_DOWN :
      cam.x -= cam.lx * fraction;
      cam.z -= cam.lz * fraction;
      break;
  }
}

void init3d()
{
  glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition(100,100);
  glutInitWindowSize(320,320);
  glutCreateWindow("Tomo. Wireframe Viewer");

  // register callbacks
  glutDisplayFunc(display);
  glutIdleFunc(display);
  //glutKeyboardFunc(processNormalKeys);
  glutSpecialFunc(processSpecialKeys);

  // OpenGL init
  glEnable(GL_DEPTH_TEST);

  // enter GLUT event processing cycle
  glutMainLoop();
}

/**
 * Scale all values of an array to be in the range [minOut, maxOut].
 *
 * @param data The array to scale.
 * @param n The size of the array.
 * @param minOut Desired minimum value for output (0).
 * @param maxOut Desired maximum value for output (255).
 */
void scaleData(long *data, long n, long minOut, long maxOut) {
  long maxIn, minIn;
  findMaxAndMin(data, n, &maxIn, &minIn);

  long rIn = maxIn - minIn;
  long rOut = maxOut - minOut;

  for(int px=0; px<n; px++) {
    data[px] = (data[px] - minIn) * rOut / rIn + minOut;
  }
}
