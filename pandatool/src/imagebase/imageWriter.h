// Filename: imageWriter.h
// Created by:  drose (19Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef IMAGEWRITER_H
#define IMAGEWRITER_H

#include <pandatoolbase.h>

#include "imageBase.h"

#include <filename.h>

////////////////////////////////////////////////////////////////////
//       Class : ImageWriter
// Description : This is the base class for a program that generates
//               an image file output, but doesn't read any for input.
////////////////////////////////////////////////////////////////////
class ImageWriter : virtual public ImageBase {
public:
  ImageWriter();

  INLINE void write_image();
  void write_image(const PNMImage &image);

protected:
  virtual bool handle_args(Args &args);

protected:
  bool _got_output_filename;
  Filename _output_filename;
};

#include "imageWriter.I"

#endif


