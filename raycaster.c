#include "header.h"
#include "vector_math.h"

double sphere_intersection(double *Ro, double *Rd, Obj* obj) {
  double r = obj->Sphere.radius;
  double* pos = obj->Sphere.position;

  double valueA = (sqr(Rd[0]) + sqr(Rd[1]) + sqr(Rd[2]));
  double valueB = 2 * Rd[0] * (Ro[0] - pos[0]) + 2 * Rd[1] * (Ro[1] - pos[1]) + 2 * Rd[2] * (Ro[2] - pos[2]);
  double valueC = sqr(pos[0]) + sqr(pos[1]) + sqr(pos[2]) + sqr(Ro[0]) + sqr(Ro[1]) + sqr(Ro[2]) -2 * (pos[0] * Ro[0] + pos[1] * Ro[1] + pos[2] * Ro[2]) - sqr(r);

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

double plane_intersection(double *Ro, double *Rd, Obj* obj) {
  double *pos = obj->Plane.position;
  double *norm = obj->Plane.normal;

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
      t = plane_intersection(Ro, Rd, obj);
      break;
    case 1:
      t = sphere_intersection(Ro, Rd, obj);
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


void get_sphere_normal(Obj* obj, double* val_intersect, double* norm){
    v3_subtract(val_intersect, obj->Sphere.position, norm);
    normalize(norm);
}

void get_plane_normal(Obj* obj, double* norm) {
   memcpy(norm, obj->Plane.normal, sizeof(double)*3);
   normalize(norm);
}

void diffuse(double* totalDiffuse, double* norm, double* lightDirect, Obj* obj, double* lightCol) {
  double dot = v3_dot(lightDirect, norm);

  if(dot <= 0) {
    dot = 0;
  } else {
    //printf("%lf\n", dot);
  }

  v3_scale(obj->diffuse, dot, totalDiffuse);
  v3_multi(lightCol, totalDiffuse, totalDiffuse);
  //printf("%lf %lf %lf\n", totalDiffuse[0], totalDiffuse[1], totalDiffuse[2]);

}

void specular(double* specColor, double* norm, double* lightDirect, Obj* obj, double* lightCol, double* Rd) {
  double ns = 20.0;
  double reflectLightDirect[3] = {0, 0, 0};
  double negRd[3] = {0, 0, 0};
  double dot = 0;

  reflection_vector(lightDirect, norm, reflectLightDirect);
  v3_scale(Rd, 1, negRd);

  //Returns the bigger value of the two (makes vector non-negative)
  dot = maxVal(v3_dot(reflectLightDirect, negRd), 0);
  dot = pow(dot, ns);
  v3_scale(lightCol, dot, specColor);
  v3_multi(specColor, obj->specular, specColor);
}

void normalize(double *v) {
  double length = sqrt(sqr(v[0]) + sqr(v[1]) + sqr(v[2]));
  v[0] /= length;
  v[1] /= length;
  v[2] /= length;
}
