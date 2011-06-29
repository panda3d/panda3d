// Filename: inkblotVideo.cxx
// Created by: jyelon (02Jul07)
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

#include "inkblotVideo.h"
#include "inkblotVideoCursor.h"

TypeHandle InkblotVideo::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: InkblotVideo::Constructor
//       Access: Public
//  Description: xxx
////////////////////////////////////////////////////////////////////
InkblotVideo::
InkblotVideo(int x, int y, int fps) :
  _specified_x(x),
  _specified_y(y),
  _specified_fps(fps)
{
}

////////////////////////////////////////////////////////////////////
//     Function: InkblotVideo::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
InkblotVideo::
~InkblotVideo() {
}

////////////////////////////////////////////////////////////////////
//     Function: InkblotVideo::open
//       Access: Published, Virtual
//  Description: Open this video, returning a MovieVideoCursor.
////////////////////////////////////////////////////////////////////
PT(MovieVideoCursor) InkblotVideo::
open() {
  return new InkblotVideoCursor(this);
}
