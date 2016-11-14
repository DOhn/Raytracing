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
void ppmMaker(Color *buff, int x, int y, FILE *output) {
  fprintf(output, "P6\n");
  fprintf(output, "%d %d\n", x, y);
  fprintf(output, "255\n");

  int i;
  for (i = 0; i < x * y; i++) {
    fputc(buff[i].r, output);
    fputc(buff[i].g, output);
    fputc(buff[i].b, output);
  }
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
  
  // Declaring color objects
  Color color = {.r = 0, .g = 0, .b = 0};
  Color bgColor = {.r = 0, .g = 0, .b = 0};
  
  // Obj pointers used to iterate through the input file.
  Obj **objs;
  objs = read_scene(fput);
  
  double h = 2;
  double w = 2;

  int cams = 0;
  int objNum = 64;
  int i, j, x, y, k;
  
  // Getting the camera values
  for (i = 0; objs[i] != 0; i++) {
    if (objs[i]->type == 0) {
      w = objs[i]->Camera.width;
      h = objs[i]->Camera.height;
      cams++;
    }
  }
  
  // Error checking for amount of cameras
  if (cams != 1) {
    fprintf(stderr, "Error: There are too many cameras within the input.json file. There are %d cameras.\n", cams);
  }
  
  // Getting the values of the lights
  size_t lightNum = 0;
  Obj **lightArray = malloc(objNum*sizeof(Obj));
  for (i = 0; objs[i] != 0; i++) {
    if (objs[i]->type == 3) {
      lightArray[lightNum] = objs[i];
      lightNum++;
    }
  }
  lightArray[lightNum] = NULL;
  
  // Declaring variables
  double cx = 0;
  double cy = 0;
  int N = imgW;
  int M = imgH;
  
  // Creating the image buffer.
  Color *buffer = malloc(M * N * sizeof(Color));
  
  double pixheight = h / M;
  double pixwidth = w / N;
  
  // Begins to process the image
  for (y = 0; y < M; y += 1) {
    for (x = 0; x < N; x += 1) {
      double Ro[3] = {0, 0, 0};
      double Rd[3] = {cx - (w / 2) + pixwidth * (x + 0.5),
        -(cy - (h / 2) + pixheight * (y + 0.5)), 1};
      normalize(Rd);
      
      double closest_t = INFINITY;
      size_t closest_obj = 0;
      
      // Getting value of closest distance
      for (i = 0; objs[i] != 0; i++) {
        double t = intersection_dist(Ro, Rd, objs[i]);
        if (t > 0 && t < closest_t) {
          closest_t = t;
          closest_obj = i;
        }
      }
      Obj* closest_object = objs[closest_obj];
      
      // Getting vector from surface objects
      for (j=0; j < lightNum; j+=1) {
        // Origin of the vector
        V3 Lo = malloc(3*sizeof(double));
        closest_t = intersection_dist(Ro, Rd, closest_object);
        v3_scale(Rd, closest_t-0.1, Lo);
        
        // Point to the surface from the light position
        V3 Ld = malloc(3*sizeof(double));
        v3_subtract(lightArray[j]->Light.position, Lo, Ld);
        
        // Begins working on the shading and shadows
        double lightDist = v3_magnitude(Ld);
        Obj* closetShadow = NULL;
        double closetShadow_t = INFINITY;
        
        for (k = 0; objs[k] != 0; k++) {
          double t = intersection_dist(Lo, Ld, objs[k]);
          if (t > 0 && t < closest_t) {
            closetShadow_t = t;
            closetShadow = objs[k];
          }
        }
        
        // Begins to compute colors for the shadows
        if (closetShadow == NULL) {
          normalize(Rd);
          normalize(Ld);

          Color diffuse, specular;

          // Getting diffuse value of Sphere/Plane
          if (closest_object->type == 1) {
            diffuse = closest_object->Sphere.diffuse;
          }
          else {
            diffuse = closest_object->Plane.diffuse;
          }

          // Getting specular value of Sphere/Plane
          if (closest_object->type == 1) {
            specular = closest_object->Sphere.specular;
          }
          else {
            specular = closest_object->Plane.specular;
          }

          // Surface normal value calculated
          V3 normal;
          if (closest_object->type == 2) {
            normal = closest_object->Plane.normal;
          }
          else {
            normal = malloc(3*sizeof(double));
            v3_subtract(Lo, closest_object->Sphere.position, normal);
          }
          
          normalize(normal);
          // Vector aims towards lightArray.
          V3 L = Ld;
          
          // Reflecting value from the normal
          V3 R = malloc(3*sizeof(double));
          v3_scale(normal, 2*v3_dot(L, normal), R);
          v3_subtract(L, R, R);
          normalize(R);
          
          // Getting the angle values and computing them.
          double diffCoeff = v3_dot(L, normal);
          // specCoeff acts as ns variable
          double specCoeff = pow(v3_dot(R, Rd), 60);
          double v1 = .125;
          double v2 = .125;
          double v3 = .125;
          
          color.r = (diffCoeff*diffuse.r*lightArray[j]->Light.color.r) + specCoeff*specular.r*lightArray[j]->Light.color.r;
          color.g = (diffCoeff*diffuse.g*lightArray[j]->Light.color.g) + specCoeff*specular.g*lightArray[j]->Light.color.g;
          color.b = (diffCoeff*diffuse.b*lightArray[j]->Light.color.b) + specCoeff*specular.b*lightArray[j]->Light.color.b;
          
          if(lightDist < 0 && lightDist >= INFINITY){
            lightDist = 1;
          }

          color.r /= (pow(v1, 2)*lightDist + v2*lightDist+v3);
          color.g /= (pow(v1, 2)*lightDist + v2*lightDist+v3);
          color.b /= (pow(v1, 2)*lightDist + v2*lightDist+v3);
        }
        else{
          color = bgColor;
        }
      }
      // Getting final color within the buffer
        buffer[(y*N)+x].r = (uint8_t)(255*clamp(color.r));
        buffer[(y*N)+x].g = (uint8_t)(255*clamp(color.g));
        buffer[(y*N)+x].b = (uint8_t)(255*clamp(color.b));
    }
  } 

  // Creates the PPM picture in the output file
  ppmMaker(buffer, N, M, output);
  return 0;
}
