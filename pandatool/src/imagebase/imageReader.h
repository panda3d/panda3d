// Filename: imageReader.h
// Created by:  drose (19Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef IMAGEREADER_H
#define IMAGEREADER_H

#include "pandatoolbase.h"

#include "imageBase.h"

////////////////////////////////////////////////////////////////////
//       Class : ImageReader
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


