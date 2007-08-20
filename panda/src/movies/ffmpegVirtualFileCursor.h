// Filename: ffmpegVirtualFile.h
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

#ifndef FFMPEGVIRTUALFILE_H
#define FFMPEGVIRTUALFILE_H
#ifdef HAVE_FFMPEG

#include "config_movies.h"

////////////////////////////////////////////////////////////////////
//       Class : FfmpegVirtualFile
// Description : Enables ffmpeg to access panda's VFS.
//
//               This class only has one public method,
//               register_hooks.  Once the hooks are registered,
//               ffmpeg will be able to open "URLs" that look
//               like this:
//               
//               pandavfs:/c/mygame/foo.avi
//
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MOVIES FfmpegVirtualFile {
 public:
  static void register_protocol();
};

#include "ffmpegVirtualFile.I"

#endif // HAVE_FFMPEG
#endif // FFMPEGVIRTUALFILE_H

