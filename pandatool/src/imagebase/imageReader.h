// Filename: imageReader.h
// Created by:  drose (19Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef IMAGEREADER_H
#define IMAGEREADER_H

#include <pandatoolbase.h>

#include "imageBase.h"

////////////////////////////////////////////////////////////////////
// 	 Class : ImageReader
// Description : This is the base class for a program that reads an
//               image file, but doesn't write an image file.
////////////////////////////////////////////////////////////////////
class ImageReader : virtual public ImageBase {
public:
  ImageReader();

protected:
  virtual bool handle_args(Args &args);

};

#endif


