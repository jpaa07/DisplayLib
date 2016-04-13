// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how write text.
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"
#include "graphics.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace rgb_matrix;

static int usage(const char *progname) {
  return 1;
}


int main(int argc, char *argv[]) {
  Color color(100, 0, 0);
  const char *bdf_font_file = "fonts/10x20.bdf";
  int rows = 32;
  int chain = 2;
  int parallel = 1;
  int x_orig = 0;
  int y_orig = 6;
  
  /*
   * Load font. This needs to be a filename with a bdf bitmap font.
   */
  rgb_matrix::Font font;
  if (!font.LoadFont(bdf_font_file)) {
    fprintf(stderr, "Couldn't load font '%s'\n", bdf_font_file);
    return usage(argv[0]);
  }

  /*
   * Set up GPIO pins. This fails when not running as root.
   */
  GPIO io;
  if (!io.Init())
    return 1;

  
  /*
   * Set up the RGBMatrix. It implements a 'Canvas' interface.
   */
  RGBMatrix *canvas = new RGBMatrix(&io, rows, chain, parallel);

  bool all_extreme_colors = true;
  all_extreme_colors &= color.r == 0 || color.r == 255;
  all_extreme_colors &= color.g == 0 || color.g == 255;
  all_extreme_colors &= color.b == 0 || color.b == 255;
  if (all_extreme_colors)
    canvas->SetPWMBits(1);

  const int x = x_orig;
  int y = y_orig;

  
  rgb_matrix::DrawText(canvas, font, x, y + font.baseline(), color,"netbeam");
  y += font.height();
  



  return 0;
}
