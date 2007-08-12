// Filename: ffmpegVideo.cxx
// Created by: jyelon (01Aug2007)
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

#include "ffmpegVideo.h"
#include "ffmpegMovie.h"
#include "config_movies.h"

TypeHandle FfmpegVideo::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::Constructor
//       Access: Public
//  Description: xxx
////////////////////////////////////////////////////////////////////
FfmpegVideo::
FfmpegVideo(CPT(FfmpegMovie) source, double offset) :
  MovieVideo((const FfmpegMovie *)source),
  _sourcep(source)
{
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
FfmpegVideo::
~FfmpegVideo() {
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideo::fetch_into_buffer
//       Access: Public, Virtual
//  Description: See MovieVideo::fetch_into_buffer.
////////////////////////////////////////////////////////////////////
void FfmpegVideo::
fetch_into_buffer(double time, unsigned char *data, bool rgba) {
}

