// Filename: stitchImageRasterizer.h
// Created by:  drose (06Nov99)
// 
////////////////////////////////////////////////////////////////////

#ifndef STITCHIMAGERASTERIZER_H
#define STITCHIMAGERASTERIZER_H

#include "stitchImageOutputter.h"

#include <luse.h>

class Stitcher;
class StitchImage;

class StitchImageRasterizer : public StitchImageOutputter {
public:
  StitchImageRasterizer();

  virtual void add_input_image(StitchImage *image);
  virtual void add_output_image(StitchImage *image);
  virtual void add_stitcher(Stitcher *stitcher);

  virtual void execute();

  bool _filter_output;
  
protected:
  void draw_points(StitchImage *output, StitchImage *input,
		   const Colord &color, double radius);
  void draw_points(StitchImage *output, Stitcher *input,
		   const Colord &color, double radius);
  void draw_image(StitchImage *output, StitchImage *input);

  typedef vector<StitchImage *> Images;
  Images _input_images;
  Images _output_images;

  typedef vector<Stitcher *> Stitchers;
  Stitchers _stitchers;

protected:
  void draw_spot(StitchImage *output,
		 const LPoint2d pixel_center, const Colord &color,
		 double radius);
};

#endif

