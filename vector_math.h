#ifndef _math_
#define _math_

#include <math.h>
#include "header.h"

typedef double* V3;
void normalize(double *v);

static inline double sqr(double v) {
  return v * v;
}

static inline void v3_add(V3 a, V3 b, V3 c) {
  c[0] = a[0] + b[0];
  c[1] = a[1] + b[1];
  c[2] = a[2] + b[2];
}

static inline double v3_magnitude(V3 v){
  return sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
}

static inline void v3_subtract(V3 a, V3 b, V3 c) {
  c[0] = a[0] - b[0];
  c[1] = a[1] - b[1];
  c[2] = a[2] - b[2];
}

static inline void v3_scale(V3 a, double s, V3 c) {
  c[0] = s * a[0];
  c[1] = s * a[1];
  c[2] = s * a[2];
}

static inline double v3_dot(V3 a, V3 b) {
  return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

static inline void v3_cross(V3 a, V3 b, V3 c) {
  c[0] = a[1] * b[2] - a[2] * b[1];
  c[1] = a[2] * b[0] - a[0] * b[2];
  c[2] = a[0] * b[1] - a[1] * b[0];
}

static inline void v3_multi(double* a, double* b, double* c) {
  c[0] = a[0] * b[0];
  c[1] = a[1] * b[1];
  c[2] = a[2] * b[2];
}

static inline void v3_cpy(V3 to, V3 from) {
  to[0] = from[0];
  to[1] = from[1];
  to[2] = from[2];
}

static inline double maxVal(double a, double b) {
  if (b > a) {
    return b;
  }
  return a;
}

static inline void reflection_vector(double* a, double* b, double* c) {
  v3_scale(b, 2 * v3_dot(a, b), c);
  v3_subtract(a, c, c);
}

static inline void refraction_vector(double* Rd, double* norm, double* val, double refracIndex) {
  double a[3] = {0};
  double b[3] = {0};
  double sinPi;
  double cosPi;

  v3_cross(norm, Rd, a);
  normalize(a);
  v3_cross(a, norm, b);
  normalize(b);

  sinPi = refracIndex * v3_dot(Rd, b);
  cosPi = sqrt(1 - sqr(sinPi));

  v3_scale(norm, -1 * cosPi, norm);
  v3_scale(b, sinPi, b);
  v3_add(b, norm, val);
}

#endif
