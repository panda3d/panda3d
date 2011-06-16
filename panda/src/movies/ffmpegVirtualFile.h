// Filename: ffmpegVirtualFile.h
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

#ifndef FFMPEGVIRTUALFILE_H
#define FFMPEGVIRTUALFILE_H
#ifdef HAVE_FFMPEG

#include "config_movies.h"
#include <stdarg.h>

////////////////////////////////////////////////////////////////////
//       Class : FfmpegVirtualFile
// Description : Enables ffmpeg to access panda's VFS.
//
//               Once register_protocol() is called, ffmpeg will be
//               able to open "URLs" that look like this:
//               
//               pandavfs:/c/mygame/foo.avi
//
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES FfmpegVirtualFile {
 public:
  static void register_protocol();

 private:
  static void log_callback(void *ptr, int level, const char *fmt, va_list v1);
};

#include "ffmpegVirtualFile.I"

#endif // HAVE_FFMPEG
#endif // FFMPEGVIRTUALFILE_H

