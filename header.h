#ifndef _header_
#define _header_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include "vector_math.h"


// const uint8_t CAMERA = 0;
// const uint8_t SPHERE = 1;
// const uint8_t PLANE = 2;
// const uint8_t LIGHT = 3;

typedef struct {
  unsigned char r, g, b;
} Color;

typedef struct {
  int type;
  double* diffuse;
  double* specular;
  double refractivity;
  double reflectivity;
  double refracIndex;

  union {
    struct {
      double width;
      double height;
    } Camera;

    struct {
      double* position;
      double radius;
    } Sphere;

    struct {
      double* normal;
      double* position;
    } Plane;

    struct {
      double theta;
      double angular_a0;
      double* direct;
      double radial_a0;
      double radial_a1;
      double radial_a2;
      double* position;
    } Light;
  };
} Obj;

static double* renderColor(int dp, double* Ro, double* Rd, Obj** objs, Obj** light);
Obj **read_scene(char *, Obj** light);
void normalize(double *v);
double intersection_dist(V3 Ro, V3 Rd, Obj* obj);
void specular(double* specColor, double* norm, double* lightDirect, Obj* obj, double* lightCol, double* Rd);
void diffuse(double* totalDiffuse, double* norm, double* lightDirect, Obj* obj, double* lightCol);
#endif
