// Filename: stitchImageOutputter.h
// Created by:  drose (09Nov99)
// 
////////////////////////////////////////////////////////////////////

#ifndef STITCHIMAGEOUTPUTTER_H
#define STITCHIMAGEOUTPUTTER_H

class StitchImage;
class Stitcher;

#include <luse.h>

class StitchImageOutputter {
public:
  StitchImageOutputter();
  virtual ~StitchImageOutputter();

  virtual void add_input_image(StitchImage *image)=0;
  virtual void add_output_image(StitchImage *image)=0;
  virtual void add_stitcher(Stitcher *stitcher)=0;

  virtual void execute()=0;
};

#endif
