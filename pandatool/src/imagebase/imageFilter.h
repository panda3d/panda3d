// Filename: imageFilter.h
// Created by:  drose (19Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef IMAGEFILTER_H
#define IMAGEFILTER_H

#include <pandatoolbase.h>

#include "imageReader.h"
#include "imageWriter.h"

////////////////////////////////////////////////////////////////////
// 	 Class : ImageFilter
// Description : This is the base class for a program that reads an
//               image file, operates on it, and writes another image
//               file out.
////////////////////////////////////////////////////////////////////
class ImageFilter : public ImageReader, public ImageWriter {
public:
  ImageFilter();

protected:
  virtual bool handle_args(Args &args);
};

#endif


