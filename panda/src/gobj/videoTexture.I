// Filename: videoTexture.I
// Created by:  drose (21Sep05)
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
//     Function: VideoTexture::get_video_width
//       Access: Published
//  Description: Returns the width in texels of the source video
//               stream.  This is not necessarily the width of the
//               actual texture, since the texture may have been
//               expanded to raise it to a power of 2.
////////////////////////////////////////////////////////////////////
INLINE int VideoTexture::
get_video_width() const {
  return _video_width;
}

////////////////////////////////////////////////////////////////////
//     Function: VideoTexture::get_video_height
//       Access: Published
//  Description: Returns the height in texels of the source video
//               stream.  This is not necessarily the height of the
//               actual texture, since the texture may have been
//               expanded to raise it to a power of 2.
////////////////////////////////////////////////////////////////////
INLINE int VideoTexture::
get_video_height() const {
  return _video_height;
}

////////////////////////////////////////////////////////////////////
//     Function: VideoTexture::clear_current_frame
//       Access: Protected
//  Description: Resets the record of the current frame so that it
//               will be forced to reload the next time it is
//               requested.
////////////////////////////////////////////////////////////////////
INLINE void VideoTexture::
clear_current_frame() {
  _last_frame_update = 0;
  _current_frame = -1;
}
