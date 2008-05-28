// Filename: imageBase.h
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

#ifndef IMAGEBASE_H
#define IMAGEBASE_H

#include "pandatoolbase.h"

#include "programBase.h"
#include "coordinateSystem.h"
#include "pnmImage.h"

////////////////////////////////////////////////////////////////////
//       Class : ImageBase
// Description : This specialization of ProgramBase is intended for
//               programs that read and/or write a single image file.
//               (See ImageMultiBase for programs that operate on
//               multiple image files at once.)
//
//               This is just a base class; see ImageReader, ImageWriter,
//               or ImageFilter according to your particular I/O needs.
////////////////////////////////////////////////////////////////////
class ImageBase : public ProgramBase {
public:
  ImageBase();

protected:
  virtual bool post_command_line();

protected:
  PNMImage _image;
};

#endif


