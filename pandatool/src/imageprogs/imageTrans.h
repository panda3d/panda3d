// Filename: imageTrans.h
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

#ifndef IMAGETRANS_H
#define IMAGETRANS_H

#include <pandatoolbase.h>

#include <imageFilter.h>

////////////////////////////////////////////////////////////////////
//       Class : ImageTrans
// Description : A program to read an image file and write an
//               equivalent image file, possibly performing some minor
//               operations along the way.
////////////////////////////////////////////////////////////////////
class ImageTrans : public ImageFilter {
public:
  ImageTrans();

  void run();
};

#endif

