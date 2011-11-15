// Filename: ffmpegVirtualFile.I
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
//     Function: FfmpegVirtualFile::is_open
//       Access: Public
//  Description: Returns true if the stream is successfully opened,
//               false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool FfmpegVirtualFile::
is_open() const {
  return (_format_context != NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: FfmpegVirtualFile::get_format_context
//       Access: Public
//  Description: Returns a pointer to the opened ffmpeg context, or
//               NULL if the file was not successfully opened.
////////////////////////////////////////////////////////////////////
INLINE AVFormatContext *FfmpegVirtualFile::
get_format_context() const {
  return _format_context;
}
