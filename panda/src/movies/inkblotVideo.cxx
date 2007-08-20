// Filename: inkblotVideo.cxx
// Created by: jyelon (02Jul07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2007, Disney Enterprises, Inc.  All rights reserved
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
  MovieVideo("inkblot"),
  _specified_x(x),
  _specified_y(y),
  _specified_fps(y)
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
