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
Obj **read_scene(char *filename) {
  int c;
  int objNum = 64;
  FILE *fput = fopen(filename, "r");
  if (fput == NULL) {
    fprintf(stderr, "Error: Could not open file \"%s\"\n", filename);
    exit(1);
  }

  Obj **objs;
  objs = malloc(sizeof(Obj *) * objNum);

  skip_ws(fput);

  // Getting the beginning of the list to parse
  expect_c(fput, '[');
  skip_ws(fput);

  size_t currentObj = 0;
  while (1) {
    objs[currentObj] = malloc(sizeof(Obj));

    c = fgetc(fput);
    if (c == ']') {
      fprintf(stderr, "Error: This is the worst scene file EVER.\n");
      fclose(fput);
      return NULL;
    }
    if (c == '{') {
      skip_ws(fput);

      // Parse the object
      char *key = next_string(fput);
      if (strcmp(key, "type") != 0) {
        fprintf(stderr, "Error: Expected \"type\" key on line number %d.\n",
                line);
        exit(1);
      }

      skip_ws(fput);
      expect_c(fput, ':');
      skip_ws(fput);

      char *value = next_string(fput);

      if (strcmp(value, "camera") == 0) {
        objs[currentObj]->type = 0;
      } else if (strcmp(value, "sphere") == 0) {
        objs[currentObj]->type = 1;
      } else if (strcmp(value, "plane") == 0) {
        objs[currentObj]->type = 2;
      } else if (strcmp(value, "light") == 0) {
        objs[currentObj]->type = 3;
      } else {
        fprintf(stderr, "Error: Unknown type, \"%s\", on line number %d.\n",
                value, line);
        exit(1);
      }

      skip_ws(fput);

      while (1) {
        c = next_c(fput);
        if (c == '}') {
          // stop parsing this object
          break;
        } else if (c == ',') {
          // read another field
          skip_ws(fput);
          char *key = next_string(fput);
          skip_ws(fput);
          expect_c(fput, ':');
          skip_ws(fput);

          // Start storing values

          // Scaler types
          if (strcmp(key, "width") == 0) {
            double value = next_number(fput);
            if (value < 0) {
              fprintf(stderr, "Error: Width cannot be less than 0. Found %lf "
                      "on line number %d.\n",
                      value, line);
              exit(1);
            }

            objs[currentObj]->Camera.width = value;

          } else if (strcmp(key, "height") == 0) {
            double value = next_number(fput);
            if (value < 0) {
              fprintf(stderr, "Error: Height cannot be less than 0. Found %lf "
                      "on line number %d.\n",
                      value, line);
              exit(1);
            }

            objs[currentObj]->Camera.height = value;

          } else if (strcmp(key, "radius") == 0) {
            double value = next_number(fput);
            if (value < 0) {
              fprintf(stderr, "Error: Radius cannot be less than 0. Found %lf "
                      "on line number %d.\n",
                      value, line);
              exit(1);
            }

            objs[currentObj]->Sphere.radius = value;

          } else if (strcmp(key, "theta") == 0) {
            double value = next_number(fput);
            value = fmod(value, (2.0 * M_PI));
            objs[currentObj]->Light.theta = value;

          } else if (strcmp(key, "angular-a0") == 0) {
            double value = next_number(fput);
            if (value < 0) {
              fprintf(stderr, "Error: Angular-a0 cannot be less than 0. Found %lf "
                      "on line number %d.\n",
                      value, line);
              exit(1);
            }


            objs[currentObj]->Light.angular_a0 = value;

          } else if (strcmp(key, "radial-a0") == 0) {
            double value = next_number(fput);
            if (value < 0) {
              fprintf(stderr, "Error: Radial-a0 cannot be less than 0. Found %lf "
                      "on line number %d.\n",
                      value, line);
              exit(1);
            }


            objs[currentObj]->Light.angular_a0 = value;

          } else if (strcmp(key, "radial-a1") == 0) {
            double value = next_number(fput);
            if (value < 0) {
              fprintf(stderr, "Error: Radial-a1 cannot be less than 0. Found %lf "
                      "on line number %d.\n",
                      value, line);
              exit(1);
            }

            objs[currentObj]->Light.radial_a1 = value;

          } else if (strcmp(key, "radial-a2") == 0) {
            double value = next_number(fput);
            if (value < 0) {
              fprintf(stderr, "Error: Radial-a2 cannot be less than 0. Found %lf "
                      "on line number %d.\n",
                      value, line);
              exit(1);
            }

            objs[currentObj]->Light.radial_a2 = value;

          // Reflectivity, Refraction, and ior
          } else if (strcmp(key, "reflectivity") == 0) {
            double value = next_number(fput);
            if (value > 1 || value < 0) {
              fprintf(stderr, "Error: Reflectivity must be between 0 - 1."
                " Currently it is %.2f on line %d\n.", value, line);
                exit(1);
            }
            objs[currentObj]->Sphere.reflect = value;
          } else if (strcmp(key, "refractivity") == 0) {
            double value = next_number(fput);
            if (value > 1 || value < 0) {
              fprintf(stderr, "Error: Refactivity must be between 0 - 1."
                " Currently it is %.2f on line %d\n.", value, line);
                exit(1);
            }
            objs[currentObj]->Sphere.refract = value;
          } else if (strcmp(key, "ior") == 0) {
            double value = next_number(fput);
            objs[currentObj]->Sphere.ior = value;
          }

          // Vector types
          else if (strcmp(key, "color") == 0) {
            double *value = next_vector(fput);
            if (value[0] < 0 || value[1] < 0 || value[2] < 0) {
              fprintf(stderr, "Error: color values cannot be less than 0. "
                      "On line number %d.\n",
                      line);
              exit(1);
            }

            if (objs[currentObj]->type != 3) {
              fprintf(stderr, "Error: Object does not support a plain color attribute. "
                      "On line number %d.\n",
                      line);
              exit(1);
            }


            objs[currentObj]->Light.color.r = value[0];
            objs[currentObj]->Light.color.g = value[1];
            objs[currentObj]->Light.color.b = value[2];

          } else if (strcmp(key, "specular_color") == 0) {
            double *value = next_vector(fput);

            if (value[0] < 0 || value[1] < 0 || value[2] < 0) {
              fprintf(stderr, "Error: color values cannot be less than 0. "
                      "On line number %d.\n",
                      line);
              exit(1);
            }

            if (objs[currentObj]->type == 1) {
              objs[currentObj]->Sphere.specular.r = value[0];
              objs[currentObj]->Sphere.specular.g = value[1];
              objs[currentObj]->Sphere.specular.b = value[2];
            } else if (objs[currentObj]->type == 2) {
              objs[currentObj]->Plane.specular.r = value[0];
              objs[currentObj]->Plane.specular.g = value[1];
              objs[currentObj]->Plane.specular.b = value[2];
            } else {
              fprintf(stderr, "Error: Object does not support specular color. "
                      "On line number %d.\n",
                      line);
              exit(1);
            }

          } else if (strcmp(key, "diffuse_color") == 0) {
            double *value = next_vector(fput);

            if (value[0] < 0 || value[1] < 0 || value[2] < 0) {
              fprintf(stderr, "Error: color values cannot be less than 0. "
                      "On line number %d.\n",
                      line);
              exit(1);
            }

            if (objs[currentObj]->type == 1) {
              objs[currentObj]->Sphere.diffuse.r = value[0];
              objs[currentObj]->Sphere.diffuse.g = value[1];
              objs[currentObj]->Sphere.diffuse.b = value[2];
            } else if (objs[currentObj]->type == 2) {
              objs[currentObj]->Plane.diffuse.r = value[0];
              objs[currentObj]->Plane.diffuse.g = value[1];
              objs[currentObj]->Plane.diffuse.b = value[2];
            } else {
              fprintf(stderr, "Error: Object does not support diffuse color. "
                      "On line number %d.\n",
                      line);
              exit(1);
            }

          } else if (strcmp(key, "position") == 0) {
            double *value = next_vector(fput);

            if (objs[currentObj]->type == 2) {
              v3_cpy(objs[currentObj]->Plane.position, value);
            } else if (objs[currentObj]->type == 1) {
              v3_cpy(objs[currentObj]->Sphere.position, value);
            } else if (objs[currentObj]->type == 3) {
              v3_cpy(objs[currentObj]->Light.position, value);
            }
          } else if (strcmp(key, "normal") == 0) {
            double *value = next_vector(fput);
            v3_cpy(objs[currentObj]->Plane.normal, value);
          } else if (strcmp(key, "direction") == 0) {
            double *value = next_vector(fput);
            v3_cpy(objs[currentObj]->Light.dirction, value);
          } else {
            fprintf(stderr, "Error: Unknown property, \"%s\", on line %d.\n",
                    key, line);
            exit(1);
          }

          skip_ws(fput);
        } else {
          fprintf(stderr, "Error: Unexpected value on line %d\n", line);
          exit(1);
        }
      }

      skip_ws(fput);
      c = next_c(fput);
      if (c == ',') {
        skip_ws(fput);
      } else if (c == ']') {
        fclose(fput);
        objs[currentObj + 1] = 0;
        return objs;
      } else {
        fprintf(stderr, "Error: Expecting ',' or ']' on line %d.\n", line);
        exit(1);
      }
    }
    currentObj++;
  }
}
