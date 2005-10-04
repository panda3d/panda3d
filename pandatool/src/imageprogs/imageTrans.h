// Filename: imageTrans.h
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

#ifndef IMAGETRANS_H
#define IMAGETRANS_H

#include "pandatoolbase.h"

#include "imageFilter.h"

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

private:
  static bool dispatch_channels(const string &opt, const string &arg, void *var);
  void extract_alpha();

  enum Channels {
    C_default,
    C_l = 1,
    C_la = 2,
    C_rgb = 3,
    C_rgba = 4,
    C_r,
    C_g,
    C_b,
    C_a
  };
    
  Channels _channels;
  Colorf _color_scale;
  bool _has_color_scale;
};

#endif

