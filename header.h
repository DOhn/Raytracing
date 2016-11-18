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
  float r, g, b;
} Color;

typedef struct {
  int type;

  union {
    struct {
      double width;
      double height;
    } Camera;

    struct {
      Color diffuse;
      Color specular;

      double position[3];
      double radius;

      double reflect;
      double refract;
      double ior;
    } Sphere;

    struct {
      Color diffuse;
      Color specular;

      double normal[3];
      double position[3];
    } Plane;

    struct {
      Color color;

      double theta;
      double angular_a0;
      double dirction[3];
      double radial_a0;
      double radial_a1;
      double radial_a2;
      double position[3];
    } Light;
  };
} Obj;

Obj **read_scene(char *filename);
void normalize(double *v);
double intersection_dist(V3 Ro, V3 Rd, Obj* obj);
#endif
