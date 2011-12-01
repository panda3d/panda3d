// Filename: ffmpegVideoCursor.I
// Created by: jyelon (01Aug2007)
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


////////////////////////////////////////////////////////////////////
//     Function: FfmpegVideoCursor::FfmpegBuffer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE FfmpegVideoCursor::FfmpegBuffer::
FfmpegBuffer(size_t block_size, double video_timebase) : 
  Buffer(block_size),
  _begin_frame(-1),
  _end_frame(0),
  _video_timebase(video_timebase)
{
}
