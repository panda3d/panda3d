// Filename: imageWriter.h
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

#ifndef IMAGEWRITER_H
#define IMAGEWRITER_H

#include "pandatoolbase.h"
#include "imageBase.h"
#include "withOutputFile.h"

#include "filename.h"

////////////////////////////////////////////////////////////////////
//       Class : ImageWriter
// Description : This is the base class for a program that generates
//               an image file output, but doesn't read any for input.
////////////////////////////////////////////////////////////////////
class ImageWriter : virtual public ImageBase, public WithOutputFile {
public:
  ImageWriter(bool allow_last_param);

  INLINE void write_image();
  void write_image(const PNMImage &image);

protected:
  virtual bool handle_args(Args &args);
};

#include "imageWriter.I"

#endif


