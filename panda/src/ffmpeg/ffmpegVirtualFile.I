/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ffmpegVirtualFile.I
 * @author jyelon
 * @date 2007-08-01
 */

/**
 * Returns true if the stream is successfully opened, false otherwise.
 */
INLINE bool FfmpegVirtualFile::
is_open() const {
  return (_format_context != nullptr);
}

/**
 * Returns a pointer to the opened ffmpeg context, or NULL if the file was not
 * successfully opened.
 */
INLINE AVFormatContext *FfmpegVirtualFile::
get_format_context() const {
  return _format_context;
}
