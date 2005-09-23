// Filename: videoTexture.I
// Created by:  drose (21Sep05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
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
//     Function: VideoTexture::get_tex_scale
//       Access: Published
//  Description: Returns a scale pair that is suitable for applying to
//               geometry via NodePath::set_tex_scale(), which will
//               convert texture coordinates on the geometry from the
//               range 0..1 into the appropriate range to render the
//               video part of the texture.
//
//               This is necessary in the event the video source is
//               not a power of two and set_power_2() is true.  In
//               this case, the video image will be mapped to the
//               lower-left corner of the texture, and the rest of the
//               texture space will be unused; so we will need to
//               remap any texture coordinates to fill the space
//               correctly.
////////////////////////////////////////////////////////////////////
INLINE LVecBase2f VideoTexture::
get_tex_scale() const {
  if (_video_width == 0 || _video_height == 0 ||
      _x_size == 0 || _y_size == 0) {
    LVecBase2f(1.0f, 1.0f);
  }
  return LVecBase2f((float)_video_width / _x_size,
		    (float)_video_height / _y_size);
}

////////////////////////////////////////////////////////////////////
//     Function: VideoTexture::set_video_size
//       Access: Protected
//  Description: Should be called by a derived class to set the size
//               of the video when it is loaded.
////////////////////////////////////////////////////////////////////
INLINE void VideoTexture::
set_video_size(int video_width, int video_height) {
  _video_width = video_width;
  _video_height = video_height;
}

////////////////////////////////////////////////////////////////////
//     Function: VideoTexture::consider_update
//       Access: Protected
//  Description: Calls update_frame() if the current frame has
//               changed.
////////////////////////////////////////////////////////////////////
INLINE void VideoTexture::
consider_update() {
  int this_frame = ClockObject::get_global_clock()->get_frame_count();
  if (this_frame != _last_frame_update) {
    int frame = get_frame();
    if (_current_frame != frame) {
      update_frame(frame);
      _current_frame = frame;
    }
    _last_frame_update = this_frame;
  }
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
