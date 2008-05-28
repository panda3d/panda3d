// Filename: imageFilter.h
// Created by:  drose (19Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef IMAGEFILTER_H
#define IMAGEFILTER_H

#include "pandatoolbase.h"

#include "imageReader.h"
#include "imageWriter.h"

////////////////////////////////////////////////////////////////////
//       Class : ImageFilter
// Description : This is the base class for a program that reads an
//               image file, operates on it, and writes another image
//               file out.
////////////////////////////////////////////////////////////////////
class ImageFilter : public ImageReader, public ImageWriter {
public:
  ImageFilter(bool allow_last_param);

protected:
  virtual bool handle_args(Args &args);
};

#endif


