// Filename: imageBase.h
// Created by:  drose (19Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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


