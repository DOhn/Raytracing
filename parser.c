#include "header.h"

int line = 1;

// next_c() wraps the getc() function and provides error checking and line
// number maintenance

int next_c(FILE *fput) {
  int c = fgetc(fput);
#ifdef DEBUG
  printf("next_c: '%c'\n", c);
#endif
  if (c == '\n') {
    line += 1;
  }
  if (c == EOF) {
    fprintf(stderr, "Error: Unexpected end of file on line number %d.\n", line);
    exit(1);
  }
  return c;
}

// expect_c() checks that the next character is d.  If it is not it emits
// an error.
void expect_c(FILE *fput, int d) {
  int c = next_c(fput);
  if (c == d)
    return;
  fprintf(stderr, "Error: Expected '%c' on line %d.\n", d, line);
  exit(1);
}

// skip_ws() skips white space in the file.
void skip_ws(FILE *fput) {
  int c = next_c(fput);
  while (isspace(c)) {
    c = next_c(fput);
  }
  ungetc(c, fput);
}

// next_string() gets the next string from the file handle and emits an error
// if a string can not be obtained.
char *next_string(FILE *fput) {
  char buffer[129];
  int c = next_c(fput);
  if (c != '"') {
    fprintf(stderr, "Error: Expected string on line %d.\n", line);
    exit(1);
  }
  c = next_c(fput);
  int i = 0;
  while (c != '"') {
    if (i >= 128) {
      fprintf(stderr, "Error: Strings longer than 128 characters in length are "
              "not supported.\n");
      exit(1);
    }
    if (c == '\\') {
      fprintf(stderr, "Error: Strings with escape codes are not supported.\n");
      exit(1);
    }
    if (c < 32 || c > 126) {
      fprintf(stderr, "Error: Strings may contain only ascii characters.\n");
      exit(1);
    }
    buffer[i] = c;
    i += 1;
    c = next_c(fput);
  }
  buffer[i] = 0;
  return strdup(buffer);
}

double next_number(FILE *fput) {
  double value;
  fscanf(fput, "%lf", &value);
  // Error check this..
  return value;
}

double *next_vector(FILE *fput) {
  double *v = malloc(3 * sizeof(double));
  expect_c(fput, '[');
  skip_ws(fput);
  v[0] = next_number(fput);
  skip_ws(fput);
  expect_c(fput, ',');
  skip_ws(fput);
  v[1] = next_number(fput);
  skip_ws(fput);
  expect_c(fput, ',');
  skip_ws(fput);
  v[2] = next_number(fput);
  skip_ws(fput);
  expect_c(fput, ']');
  return v;
}

/**
 * Procedure to parse JSON and store them
 * in an object array.
 *
 * Returns: Array of Object types
 */
 Obj** read_scene(char* filename, Obj** light) {
   int lightIndex = 0;
   int index = 0;
   int c;
   FILE* json = fopen(filename, "r");

   if (json == NULL) {
     fprintf(stderr, "Error: Could not open file \"%s\"\n", filename);
     exit(1);
   }
   Obj** objs = malloc(sizeof(Obj*)*128);

   objs[127] = NULL;
   light[127] = NULL;
   Obj* obj;

   skip_ws(json);

   // Find the beginning of the list
   expect_c(json, '[');

   skip_ws(json);

   // Find the objects

   while (1) {
     c = fgetc(json);
     if (c == ']') {
       fprintf(stderr, "Error: This is the worst scene file EVER.\n");
       fclose(json);
       return NULL;
     }
     if (c == '{') {
       skip_ws(json);

       // Parse the obj
       char* key = next_string(json);
       if (strcmp(key, "type") != 0) {
 	fprintf(stderr, "Error: Expected \"type\" key on line number %d.\n", line);
 	exit(1);
       }

       skip_ws(json);

       expect_c(json, ':');

       skip_ws(json);

       char* value = next_string(json);

       obj = malloc(sizeof(Obj));
       obj->refractivity = 0;
       obj->reflectivity = 0;
       obj->refracIndex = 1;

       if (strcmp(value, "camera") == 0) {
         obj->type = 0;
         objs[index++] = obj;
       } else if (strcmp(value, "sphere") == 0) {
         obj->type = 1;
         objs[index++] = obj;
       } else if (strcmp(value, "plane") == 0) {
         obj->type = 2;
         objs[index++] = obj;
       } else if(strcmp(value, "light") == 0) {
         obj->type = 3;
         obj->Light.direct = NULL;
         light[lightIndex++] = obj;
       }
         else {
 	       fprintf(stderr, "Error: Unknown type, \"%s\", on line number %d.\n", value, line);
 	       exit(1);
       }

       skip_ws(json);

       while (1) {
 	// , }
 	c = next_c(json);
 	if (c == '}') {
 	  // stop parsing this obj
 	  break;

 	} else if (c == ',') {
 	  // read another field
 	  skip_ws(json);
 	  char* key = next_string(json);
 	  skip_ws(json);
 	  expect_c(json, ':');
 	  skip_ws(json);

 	  if ((strcmp(key, "width") == 0) ||
 	      (strcmp(key, "height") == 0) ||
 	      (strcmp(key, "radius") == 0) ||
         (strcmp(key, "theta") == 0) ||
         (strcmp(key, "radial-a2") == 0) ||
         (strcmp(key, "radial-a1") == 0) ||
         (strcmp(key, "radial-a0") == 0) ||
         (strcmp(key, "angular_a0") == 0)||
         (strcmp(key, "refractivity") == 0) ||
         (strcmp(key, "reflectivity") == 0) ||
         (strcmp(key, "ior") == 0)) {
 	         double value = next_number(json);

           if(strcmp(key, "width") == 0) {
             obj->Camera.width = value;

         } else if(strcmp(key, "height") == 0) {
             obj->Camera.height = value;

         } else if(strcmp(key, "reflectivity") == 0) {
             obj->reflectivity = value;

         } else if(strcmp(key, "refractivity") == 0) {
             obj->refractivity = value;

         } else if(strcmp(key, "ior") == 0) {
             obj->refracIndex = value;

         } else if(strcmp(key, "radius") == 0) {
             obj->Sphere.radius = value;

         } else if(strcmp(key, "theta") == 0) {
             obj->Light.theta = value;

         }  else if(strcmp(key, "radial-a2") == 0) {
             obj->Light.radial_a2 = value;

         } else if(strcmp(key, "radial-a1") == 0) {
             obj->Light.radial_a1 = value;

         } else if(strcmp(key, "radial-a0") == 0) {
             obj->Light.radial_a0 = value;

         } else if(strcmp(key, "angular_a0") == 0) {
             obj->Light.angular_a0 = value;
         }
 	  } else if ((strcmp(key, "diffuse_color") == 0) ||
 		     (strcmp(key, "position") == 0) ||
 		     (strcmp(key, "normal") == 0) ||
          (strcmp(key, "color") == 0) ||
          (strcmp(key, "specular_color") == 0) ||
          (strcmp(key, "direction") == 0)) {
           double* value = next_vector(json);

           if(strcmp(key, "diffuse_color") == 0) {
             obj->diffuse = value;

         } else if(strcmp(key, "direction") == 0) {
             //printf("Got direct damn\n");
             obj->Light.direct = value;

         } else if(strcmp(key, "specular_color") == 0) {
             obj->specular = value;

         } else if(strcmp(key, "color") == 0) {
             obj->diffuse = value;

         } else if(strcmp(key, "position") == 0 && obj->type == 2) {
             obj->Plane.position = value;

         } else if(strcmp(key, "position") == 0 && obj->type == 1) {
             obj->Sphere.position = value;

         } else if(strcmp(key, "position") == 0 && obj->type == 3) {
             obj->Light.position = value;

         } else if(strcmp(key, "normal") == 0) {
             obj->Plane.normal = value;
         }
 	  } else {
 	    fprintf(stderr, "Error: Unknown property, \"%s\", on line %d.\n",
 		    key, line);
 	    //char* value = next_string(json);
 	  }
 	  skip_ws(json);
 	} else {
 	  fprintf(stderr, "Error: Unexpected value on line %d\n", line);
 	  exit(1);
 	}
       }
       skip_ws(json);
       c = next_c(json);
       if (c == ',') {
 	// noop
 	    skip_ws(json);
       } else if (c == ']') {
 	        fclose(json);
           objs[index] = NULL;
           light[lightIndex] = NULL;
 	        return objs;
       } else {
 	       fprintf(stderr, "Error: Expecting ',' or ']' on line %d.\n", line);
 	       exit(1);
       }
     }
   }
 }
