// Filename: stitchImageCommandOutput.h
// Created by:  drose (29Nov99)
// 
////////////////////////////////////////////////////////////////////

#ifndef STITCHIMAGECOMMANDOUTPUT_H
#define STITCHIMAGECOMMANDOUTPUT_H

#include "stitchImageOutputter.h"

#include <luse.h>

class Stitcher;
class StitchImage;
class StitchCommand;

class StitchImageCommandOutput : public StitchImageOutputter {
public:
  StitchImageCommandOutput();

  virtual void add_input_image(StitchImage *image);
  virtual void add_output_image(StitchImage *image);
  virtual void add_stitcher(Stitcher *stitcher);

  virtual void execute();
  
protected:
  void fill_image_cmd(StitchCommand *image_cmd, StitchImage *image);

  typedef vector<StitchImage *> Images;
  Images _input_images;
  Images _output_images;

  typedef vector<Stitcher *> Stitchers;
  Stitchers _stitchers;
};

#endif

