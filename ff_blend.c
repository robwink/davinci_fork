#include "parser.h"
#include "func.h"

// Notation for the blend algorithms.
// M = Mask (lower layer)
// I = Image (upper layer)


double blend_multiply(double M, double I);
double blend_divide(double M, double I);
double blend_overlay(double M, double I);
double blend_screen(double M, double I);
double blend_dissolve(double M, double I);
double blend_dodge(double M, double I);
double blend_burn(double M, double I);
double blend_hardlight(double M, double I);
double blend_softlight(double M, double I);
double blend_grainextract(double M, double I);
double blend_grainmerge(double M, double I);
double blend_difference(double M, double I);
double blend_addition(double M, double I);
double blend_subtract(double M, double I);
double blend_darkenonly(double M, double I);
double blend_lightenonly(double M, double I);
double blend_hue(double M, double I);
double blend_saturation(double M, double I);
double blend_color(double M, double I);
double blend_value(double M, double I);

struct _blend_modes {
  char *name;
  double (*fptr)(double, double);
} modes[] = {
  { "multiply",     blend_multiply },
  { "divide",       blend_divide },
  { "overlay",      blend_overlay },
  { "screen",       blend_screen },
  { "dissolve",     blend_dissolve },
  { "dodge",        blend_dodge },
  { "burn",         blend_burn },
  { "hardlight",    blend_hardlight },
  { "softlight",    blend_softlight },
  { "grainextract", blend_grainextract },
  { "grainmerge",   blend_grainmerge },
  { "difference",   blend_difference },
  { "addition",     blend_addition },
  { "subtract",     blend_subtract },
  { "darkenonly",   blend_darkenonly },
  { "lightenonly",  blend_lightenonly },
  /*
  { "hue",          blend_hue },
  { "saturation",   blend_saturation },
  { "color",        blend_color },
  { "value",        blend_value },
  */
  { NULL, NULL }
};

Var *
ff_blend(vfuncptr func, Var * arg)
{
  Var *obj1 = NULL, *obj2 = NULL;
  int x,y,z, i, j, dsize;
  int x2, y2, z2;

  int npixels;
  double v1, v2, v3;
  char *data;
  double (*fptr)(double, double) = NULL;

  float start = MAXFLOAT, size= MAXFLOAT;
  int steps = MAXINT;
  char *mode_str = NULL;

  Alist alist[9];
  alist[0] = make_alist( "obj1", ID_VAL,    NULL,    &obj1);
  alist[1] = make_alist( "obj2", ID_VAL,    NULL,    &obj2);
  alist[2] = make_alist( "mode", ID_STRING, NULL,    &mode_str);
  alist[3].name = NULL;

  if (parse_args(func, arg, alist) == 0) return(NULL);

  for (i = 0 ; modes[i].name != NULL ; i++) {
    if (!strcmp(modes[i].name, mode_str)) {
      fptr = modes[i].fptr;
    }
  }
  if (fptr == NULL) {
    parse_error("%s: Unknown mode.", func->name);
  }

  if (obj1 == NULL) {
    parse_error("%s: No object specified\n", func->name);
    return(NULL);
  }
  if (obj2 == NULL) {
    parse_error("%s: No second object specified\n", func->name);
    return(NULL);
  }

  x = GetSamples(V_SIZE(obj1), V_ORG(obj1));
  y = GetLines(V_SIZE(obj1), V_ORG(obj1));
  z = GetBands(V_SIZE(obj1), V_ORG(obj1));

  x2 = GetSamples(V_SIZE(obj2), V_ORG(obj2));
  y2 = GetLines(V_SIZE(obj2), V_ORG(obj2));
  z2 = GetBands(V_SIZE(obj2), V_ORG(obj2));

  if (x != x2 || y != y2 || z != z2) {
    parse_error("%s: Objects are not the same size", func->name);
    return(NULL);
  }

  if (V_ORG(obj1) != V_ORG(obj2)) {
    parse_error("%s: Objects must have the same org.", func->name);
    return(NULL);
  }

  npixels = V_DSIZE(obj1);
  data = calloc(NBYTES(BYTE), npixels);

  for (i = 0 ; i < npixels ; i++) {
    v1 = extract_double(obj1, i);
    v2 = extract_double(obj2, i);
    v3 = fptr(v1, v2);
    data[i] = (int)v3;
  }

  return(newVal(V_ORG(obj1),
                V_SIZE(obj1)[0], V_SIZE(obj1)[1], V_SIZE(obj1)[2],
                INT, data));
}

// Notation for the blend algorithms.
// M = Mask (lower layer)
// I = Image (upper layer)

double blend_multiply(double M, double I) {
  // Multiply  mode multiplies the pixel values of the upper layer with those
  // of the layer below it and then divides the result by 255. The result is
  // usually a darker image. If either layer is white, the resulting image is
  // the same as the other layer (1 * I = I). If either layer is black, the
  // resulting image is completely black (0 * I = 0).

  double E = 1/255.0 * (M * I);
  return (E);
}

double blend_divide(double M, double I) {
  //  Divide  mode multiplies each pixel value in the lower layer by 256 and
  //  then divides that by the corresponding pixel value of the upper layer
  //  plus one. (Adding one to the denominator avoids dividing by zero.) The
  //  resulting image is often lighter, and sometimes looks “burned out”.
  double E = I * 256.0 / (M+1);
  return(E);
}

double blend_screen(double M, double I) {
  // Screen  mode inverts the values of each of the visible pixels in the two
  // layers of the image. (That is, it subtracts each of them from 255.) Then
  // it multiplies them together, divides by 255 and inverts this value again.
  // The resulting image is usually brighter, and sometimes “washed out” in
  // appearance. The exceptions to this are a black layer, which does not
  // change the other layer, and a white layer, which results in a white image.
  // Darker colors in the image appear to be more transparent.
  double E = 255.0 - ((255-M) * (255-I)) / 255.0;
  return(E);
}

double blend_overlay(double M, double I) {
  // Overlay  mode inverts the pixel value of the lower layer, multiplies it by
  // two times the pixel value of the upper layer, adds that to the original
  // pixel value of the lower layer, divides by 255, and then multiplies by the
  // pixel value of the original lower layer and divides by 255 again. It
  // darkens the image, but not as much as with “Multiply” mode.
  double E = (I / 255.0) *  (I + (2*M/255.0) * (255.0 - I));
  return(E);
}

double blend_dissolve(double M, double I) {
  // Dissolve  mode dissolves the upper layer into the layer beneath it by
  // drawing a random pattern of pixels in areas of partial transparency. It is
  // useful as a layer mode, but it is also often useful as a painting mode.
  if (random() % 2) return(M);
  return(I);
};

double blend_dodge(double M, double I) {
  // Dodge  mode multiplies the pixel value of the lower layer by 256, then
  // divides that by the inverse of the pixel value of the top layer. The
  // resulting image is usually lighter, but some colors may be inverted.
  double E = I*256.0 / ((255.0 - M) + 1);
  return (E);

}
double blend_burn(double M, double I) {
  // Burn  mode inverts the pixel value of the lower layer, multiplies it by
  // 256, divides that by one plus the pixel value of the upper layer, then
  // inverts the result. It tends to make the image darker, somewhat similar to
  // “Multiply” mode.
  double E = 255.0 - (((255.0 - I) * 256.0) / (M+1));
  return (E);
}

double blend_hardlight(double M, double I) {
  // 
  // Hard light mode is rather complicated because the equation consists of two
  // parts, one for darker colors and one for brighter colors. If the pixel
  // color of the upper layer is greater than 128, the layers are combined
  // according to the first formula shown below. Otherwise, the pixel values of
  // the upper and lower layers are multiplied together and multiplied by two,
  // then divided by 256. You might use this mode to combine two photographs
  // and obtain bright colors and sharp edges. 
  double E;
  if (M > 128) {
    E = 255.0 - ((255.0-I) * ( 255.0-(2.0*(M-128.0)))) / 256.0;
  } else {
    E = I*M / 128.0;
  }
  return(E);
}
double blend_softlight(double M, double I) {
  // Soft light is not related to “Hard light” in anything but the name, but it
  // does tend to make the edges softer and the colors not so bright. It is
  // similar to “Overlay” mode. In some versions of GIMP, “Overlay” mode and
  // “Soft light” mode are identical.
  double Rs = blend_screen(M, I);
  double E = ((255.0 - I) * M * I) + (I * Rs) / 255.0;
  return(E);
}
double blend_grainextract(double M, double I) {
  // Grain extract mode is supposed to extract the “film grain” from a layer to
  // produce a new layer that is pure grain, but it can also be useful for
  // giving images an embossed appearance. It subtracts the pixel value of the
  // upper layer from that of the lower layer and adds 128.
  double E = I - M + 128.0;
  return(E);
}
double blend_grainmerge(double M, double I) {
  //  Grain merge mode merges a grain layer (possibly one created from the
  //  “Grain extract” mode) into the current layer, leaving a grainy version of
  //  the original layer. It does just the opposite of “Grain extract”. It adds
  //  the pixel values of the upper and lower layers together and subtracts
  //  128.
  double E = I + M + 128.0;
  return(E);
}
double blend_difference(double M, double I) {
  //  Grain merge mode merges a grain layer (possibly one created from the
  //  “Grain extract” mode) into the current layer, leaving a grainy version of
  //  the original layer. It does just the opposite of “Grain extract”. It adds
  //  the pixel values of the upper and lower layers together and subtracts
  //  128.
  double E = fabs(I-M);
  return(E);
}
double blend_addition(double M, double I) {
  // Addition  mode is very simple. The pixel values of the upper and lower
  // layers are added to each other. The resulting image is usually lighter.
  // The equation can result in color values greater than 255, so some of the
  // light colors may be set to the maximum value of 255.
  double E = M + I;
  return(E);
}
double blend_subtract(double M, double I) {
  // Subtract  mode subtracts the pixel values of the upper layer from the
  // pixel values of the lower layer. The resulting image is normally darker.
  // You might get a lot of black or near-black in the resulting image. The
  // equation can result in negative color values, so some of the dark colors
  // may be set to the minimum value of 0.
  double E = I - M;
  return(E);
}
double blend_darkenonly(double M, double I) {
  // Darken only  mode compares each component of each pixel in the upper layer
  // with the corresponding one in the lower layer and uses the smaller value
  // in the resulting image. Completely white layers have no effect on the
  // final image and completely black layers result in a black image.
  double E = min(M, I);
  return(E);
}
double blend_lightenonly(double M, double I) {
  // Lighten only  mode compares each component of each pixel in the upper
  // layer with the corresponding one in the lower layer and uses the larger
  // value in the resulting image. Completely black layers have no effect on
  // the final image and completely white layers result in a white image.
  double E = max(M, I);
  return(E);
}

/*
 * Can't do these on a single color element basis.
typedef struct { float r, g, b; } RGB;
typedef struct { float h, s, v; } HSV;
typedef struct { float c, m, y; } CMY;
typedef struct { float c, m, y, k; } KCMY;
RGB HSVToRGB(HSV hsv);
HSV RGBToHSV(RGB rgb);

double blend_hue(double M, double I) {
  RGB rgb = { 
  return(E);
}
double blend_saturation(double M, double I) {
return(E);
}
double blend_color(double M, double I) {
return(E);
}
double blend_value(double M, double I) {
return(E);
}
*/
