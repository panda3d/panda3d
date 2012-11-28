// Filename: imageTrans.h
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
  LColor _color_scale;
  bool _has_color_scale;
  bool _flip, _mirror, _cw, _ccw;
};

#endif

