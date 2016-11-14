#include "header.h"

double sphere_intersection(double *Ro, double *Rd, double *cent, double r) {
  double valueA = (sqr(Rd[0]) + sqr(Rd[1]) + sqr(Rd[2]));
  double valueB = 2 * Rd[0] * (Ro[0] - cent[0]) + 2 * Rd[1] * (Ro[1] - cent[1]) + 2 * Rd[2] * (Ro[2] - cent[2]);
  double valueC = sqr(cent[0]) + sqr(cent[1]) + sqr(cent[2]) + sqr(Ro[0]) + sqr(Ro[1]) + sqr(Ro[2]) -2 * (cent[0] * Ro[0] + cent[1] * Ro[1] + cent[2] * Ro[2]) - sqr(r);
  
  double d = sqr(valueB) - 4 * valueA * valueC;
  if(d < 0)
    return -1;
  
  double t0 = (-valueB - sqrt(d)) / (2*valueA);
  if (t0 > 0)
    return t0;
  
  double t1 = (-valueB + sqrt(d)) / (2*valueA);
  if (t1 > 0)
    return t1;
  
  return -1;
}

double plane_intersection(double *Ro, double *Rd, double *pos, double *norm) {
  V3 v = malloc(sizeof(double) * 3);
  v3_subtract(Ro, pos, v);
  
  double dist = v3_dot(norm, v);
  double denom = v3_dot(norm, Rd);
  
  dist = -(dist / denom);
  if (dist > 0)
    return dist;
  
  return 0;
}

double intersection_dist(V3 Ro, V3 Rd, Obj* obj) {
  double t = INFINITY;
  
  switch (obj->type) {
    case 2:
      t = plane_intersection(Ro, Rd, obj->Plane.position, obj->Plane.normal);
      break;
    case 1:
      t = sphere_intersection(Ro, Rd, obj->Sphere.position, obj->Sphere.radius);
      break;
    case 0:
      t = INFINITY;
      break;
    case 3:
      t = INFINITY;
      break;
    default:
      // Error checking if the type if incorrect.
      fprintf(stderr, "Error: The type is incorrect, please enter a correct type.\n");
      exit(1);
  } 
  
  return t;
}

void normalize(double *v) {
  double length = sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
  v[0] /= length;
  v[1] /= length;
  v[2] /= length;
}
