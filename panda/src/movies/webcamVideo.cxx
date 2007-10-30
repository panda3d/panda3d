// Filename: webcamVideo.cxx
// Created by: jyelon (01Nov2007)
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

#include "webcamVideo.h"
#include "pandabase.h"
#include "movieVideoCursor.h"

TypeHandle WebcamVideo::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideo::Constructor
//       Access: Public
//  Description: The parameters x,y, and fps are suggestions.  The
//               webcam will match these as closely as it can, but
//               of course, there are no guarantees.
////////////////////////////////////////////////////////////////////
WebcamVideo::
WebcamVideo(const string &dev, int x, int y, int fps) :
  MovieVideo("webcam"),
  _specified_device(dev),
  _specified_x(x),
  _specified_y(y),
  _specified_fps(y)
{
}

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideo::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
WebcamVideo::
~WebcamVideo() {
}

////////////////////////////////////////////////////////////////////
// The rest of this file is OS-dependent.
// We include the appropriate version depending 
// the user's compile-configuration.
////////////////////////////////////////////////////////////////////

#if defined(HAVE_DX9)

#include "webcamVideoDX.cxx"

#elif defined(HAVE_VIDEO4LINUX)

#include "webcamVideoV4L.cxx"

#else

#include "webcamVideoNull.cxx"

#endif

