#include "header.h"

float clamp(float v) {
  if (v < 0.0) {
    return 0;
  }
  else if (v > 1) {
    return 1.0;
  }
  else {
    return v;
  }
}

// Creates the output.ppm file.
void ppmMaker(Color** buff, int x, int y, FILE *output) {
  fprintf(output, "P%i\n%i %i\n%i\n", 6, x, y, 255);
  int i;
  for(i = 0; i < (x*y); i++) {
    Color* buffer = buff[i];
    fprintf(output, "%i\n%i\n%i\n", buffer->r, buffer->g, buffer->b);
  }
}

void currentIntersect(double* intersect, double* Ro, double* Rd, double t) {
  intersect[0] = t*Rd[0] + Ro[0];
  intersect[1] = t*Rd[1] + Ro[1];
  intersect[2] = t*Rd[2] + Ro[2];
}

Obj* rayCast(double* t, Obj** objs, Obj* skip, double* Ro, double* Rd) {

  double tNew = INFINITY, tVal;
  Obj* newObj = NULL;
  int i;

  for(i = 0; objs[i] != NULL; i++) {
    if(objs[i] == skip) {
      continue;
    } else if(objs[i]->type == 2) {
      tVal = plane_intersection(Ro, Rd, objs[i]);
    } else if(objs[i]->type == 1) {
      tVal = sphere_intersection(Ro, Rd, objs[i]);
    } else if(objs[i]->type == 0 || objs[i]->type == 3) {
      continue;
    } else {
      fprintf(stderr, "Error: Encountered an Unknown Type.\n");
      exit(1);
    }

    if(tVal < tNew && tVal != -1) {
     tNew = tVal;
     newObj = objs[i];
    }
  }
    if(tNew == INFINITY) {
      tNew = -1;
    }
   *t = tNew;
    return newObj;
}

void reflection(double** reflectColor, double* reflectObjNorm, int dp, double* Ro, double* Rd,
  Obj** objs, Obj** light, double t) {
    double reflectObj[3];
    double tempRo[3];

    currentIntersect(tempRo, Ro, Rd, t - 0.00001);
    reflection_vector(Rd, reflectObjNorm, reflectObj);
    *reflectColor = renderColor(dp - 1, tempRo, reflectObj, objs, light);

}

void refraction(double ior, double** refractColor, double* refractNorm, Obj* check, int dp,
  double* Ro, double* Rd, Obj** objs,  Obj** light, double t) {

  // Used to store new values of Ro, Rd, and t
  double tempRo[3] = {0};
  double tempRd[3] = {0};
  double tNew;

  currentIntersect(tempRo, Ro, Rd, t + 0.00001);
  refraction_vector(Rd, refractNorm, tempRd, ior);

  if(check->type == 2) {
    tNew = plane_intersection(tempRo, tempRd, check);
  } else if(check->type == 1) {
    tNew = sphere_intersection(tempRo, tempRd, check);
  } else {
    fprintf(stderr, "Error: Something went wrong with Refraction.\n");
    exit(1);
  }

  if(tNew != -1 && tNew != INFINITY) {
    currentIntersect(tempRo, tempRo, tempRd, tNew + 0.00001);
    v3_subtract(check->Sphere.position, tempRo, refractNorm);
    normalize(refractNorm);
    refraction_vector(tempRd, refractNorm, tempRd, (1.0 / ior));
  }

  *refractColor = renderColor(dp - 1, tempRo, tempRd, objs, light);
}

double* renderColor(int dp, double* Ro, double* Rd, Obj** objs, Obj** light) {
  double t = 0;

  // Initializing variables and color array
  double* col = malloc(sizeof(double)*3);
  col[0] = 0;
  col[1] = 0;
  col[2] = 0;

  Obj* obj = rayCast(&t, objs, NULL, Ro, Rd);

  if(t == -1) {
    return col;
    }
  double intersect[3] = {0, 0, 0};
  double Norm[3] = {0, 0 ,0};
  currentIntersect(intersect, Ro, Rd, t);

  if(obj->type == 1) {
    get_sphere_normal(obj, intersect, Norm);
  } else if(obj->type == 2) {
    get_plane_normal(obj, Norm);
  }

  // Setting up the lighting for diffuse and specular
  double diffColor[3] = {0, 0, 0};
  double totalDiff[3] = {0, 0, 0};
  double specColor[3] = {0, 0, 0};
  double totalSpec[3] = {0, 0, 0};
  int i;

  for(i = 0; light[i] !=  NULL; i++) {
    double lightDirect[3] = {0, 0, 0};
    double lightColor[3] = {0, 0, 0};
    double lightObj[3] = {0, 0, 0};

    v3_subtract(light[i]->Light.position, intersect, lightDirect);
    double mag = sqrt(sqr(lightDirect[0]) + sqr(lightDirect[1]) + sqr(lightDirect[2]));
    normalize(lightDirect);

    // Testing the shadows
    Obj* temp = rayCast(&t, objs, obj, intersect, lightDirect);

    if(t >= 0 && t < mag && temp->type != 2) {
      continue;
    }
    v3_scale(lightDirect, -1, lightObj);
    if(light[i]->Light.direct != NULL) {
      double dot = v3_dot(lightObj, light[i]->Light.direct);
      if((dot) < sin(light[i]->Light.theta*M_PI/180)) {
        continue;
      }
      v3_scale(lightColor, pow(dot, light[i]->Light.radial_a0), lightColor);
    }
    // Less light as it is farther away
    double radA = 1/(sqr(mag)*(light[i]->Light.radial_a2) + (light[i]->Light.radial_a1)*mag + light[i]->Light.radial_a0);
    v3_scale(light[i]->diffuse, radA, lightColor);

    // Getting the diffuse color
    diffuse(diffColor, Norm, lightDirect, obj, lightColor);
    v3_add(diffColor, totalDiff, totalDiff);

    // Getting the specular color
    specular(specColor, Norm, lightDirect, obj, lightColor, Rd);
    v3_add(specColor, totalSpec, totalSpec);

  }

  v3_add(col, totalDiff, col);
  v3_add(col, totalSpec, col);


  if(dp <= 0) {
    return col;
  }

  double refractivity = obj->refractivity;
  double reflectivity = obj->reflectivity;
  double refracIndex = obj->refracIndex;
  double* newCol;

  v3_scale(col, 1.0 - (reflectivity + refractivity), col);



  // Getting the reflection
  reflection(&newCol, Norm, dp, Ro, Rd, objs, light, t);
  v3_scale(newCol, reflectivity, newCol);
  v3_add(col, newCol, col);

  // Getting the refraction
  refraction(refracIndex, &newCol, Norm, obj, dp, Ro, Rd, objs, light, t);
  v3_scale(newCol, refractivity, newCol);
  v3_add(col, newCol, col);

  return col;
}

Color** sceneMaker(Obj** objs, Obj** light, int height, int width) {
  // Coordinates of the camera
  double cx = 0;
  double cy = 0;

  // Getting the color width and height
  double h = objs[0]->Camera.height;
  double w = objs[0]->Camera.width;
  double* col;

  int M = height;
  int N = width;

  Color** buff = malloc(M * N * sizeof(Color*));

  double imgH = h / M;
  double imgW = w / N;


  int y, x;
  for (y = 0; y < M; y++) {
    for (x = 0; x < N; x++) {
      double Ro[3] = {0, 0, 0};
      double Rd[3] = {cx - (w / 2) + imgW * (x + 0.5),
                      -(cy - (h / 2) + imgH * (y + 0.5)), 1};
      normalize(Rd);
      col = renderColor(7, Ro, Rd, objs, light);

      // Setting the color and getting it's values
      Color* color = malloc(sizeof(Color));
      color->r = (unsigned char)clamp(col[0]*255);
      color->g = (unsigned char)clamp(col[1]*255);
      color->b = (unsigned char)clamp(col[2]*255);
      buff[y*N + x] = color;
    }
  }
  return buff;
}

int main(int argc, char *argv[]) {

  // Error checking the proper amount of arguments
  if (argc < 5) {
    fprintf(stderr, "Error: Need 4 arguments for this project.\n");
    exit(1);
  }

  // Getting height and width values from the arguments.
  int imgW = strtol(argv[1], (char **)NULL, 10);
  int imgH = strtol(argv[2], (char **)NULL, 10);

  char *fput = argv[3];

  // Error checking for the output file.
  FILE *output = fopen(argv[4], "wb");
  if (!output) {
    fprintf(stderr, "Error: Failed to open file %s\n", argv[4]);
    fclose(output);
    return -1;
  }

  Color** buff;
  Obj** objs;
  Obj** light = malloc(sizeof(Obj*)*128);

  objs = read_scene(argv[3], light);
  buff = sceneMaker(objs, light, imgH, imgW);
  printf("We made it here.\n");
  // Creates the PPM picture in the output file
  ppmMaker(buff, imgH, imgW, output);
  return 0;
}
