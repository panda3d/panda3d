// Filename: stitchImageConverter.h
// Created by:  drose (06Nov99)
// 
////////////////////////////////////////////////////////////////////

#ifndef STITCHIMAGECONVERTER_H
#define STITCHIMAGECONVERTER_H

#include "stitchImageVisualizer.h"

class StitchImage;

class StitchImageConverter : public StitchImageVisualizer {
public:
  StitchImageConverter();

  virtual void add_output_image(StitchImage *image);

protected:
  virtual void override_chan_cfg(ChanCfgOverrides &override);
  virtual void setup_camera(const RenderRelation &camera_arc);
  virtual bool is_interactive() const;
  virtual void create_image_geometry(Image &im);

  StitchImage *_output_image;
};

#endif
