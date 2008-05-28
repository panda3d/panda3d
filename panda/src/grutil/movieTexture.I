// Filename: movieTexture.I
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
//     Function: MovieTexture::get_video_width
//       Access: Published
//  Description: Returns the width in texels of the source video
//               stream.  This is not necessarily the width of the
//               actual texture, since the texture may have been
//               expanded to raise it to a power of 2.
////////////////////////////////////////////////////////////////////
INLINE int MovieTexture::
get_video_width() const {
  CDReader cdata(_cycler);
  return cdata->_video_width;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::get_video_height
//       Access: Published
//  Description: Returns the height in texels of the source video
//               stream.  This is not necessarily the height of the
//               actual texture, since the texture may have been
//               expanded to raise it to a power of 2.
////////////////////////////////////////////////////////////////////
INLINE int MovieTexture::
get_video_height() const {
  CDReader cdata(_cycler);
  return cdata->_video_height;
}

////////////////////////////////////////////////////////////////////
//     Function: MovieTexture::get_tex_scale
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
INLINE LVecBase2f MovieTexture::
get_tex_scale() const {
  CDReader cdata(_cycler);
  if (cdata->_video_width == 0 ||
      cdata->_video_height == 0 ||
      _x_size == 0 ||
      _y_size == 0) {
    return LVecBase2f(1.0f, 1.0f);
  }
  return LVecBase2f((float)cdata->_video_width / _x_size,
                    (float)cdata->_video_height / _y_size);
}

