// Filename: imageBuffer.I
// Created by:  drose (21Nov00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::has_filename
//       Access: Published
//  Description: Returns true if the filename has been set and
//               is available.  See set_filename().
////////////////////////////////////////////////////////////////////
INLINE bool ImageBuffer::
has_filename() const {
  return !_filename.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::get_filename
//       Access: Published
//  Description: Returns the filename that has been set.  Use
//               this in conjunction with get_name() to get the names
//               of the file(s) that were loaded into the buffer.  See
//               set_filename().
////////////////////////////////////////////////////////////////////
INLINE const Filename &ImageBuffer::
get_filename() const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::has_alpha_filename
//       Access: Published
//  Description: Returns true if the alpha_filename has been set and
//               is available.  See set_alpha_filename().
////////////////////////////////////////////////////////////////////
INLINE bool ImageBuffer::
has_alpha_filename() const {
  return !_alpha_filename.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::get_alpha_filename
//       Access: Published
//  Description: Returns the alpha_filename that has been set.  Use
//               this in conjunction with get_name() to get the names
//               of the file(s) that were loaded into the buffer.  See
//               set_alpha_filename().
////////////////////////////////////////////////////////////////////
INLINE const Filename &ImageBuffer::
get_alpha_filename() const {
  return _alpha_filename;
}


////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::set_filename
//       Access: Public
//  Description: Sets the name of the file that contains the image's
//               contents.  Normally, this is set automatically when
//               the image is loaded, for instance via
//               Texture::read().
//
//               The ImageBuffer's get_name() function used to return
//               the filename, but now returns just the basename
//               (without the extension), which is a more useful name
//               for identifying an image in show code.
////////////////////////////////////////////////////////////////////
INLINE void ImageBuffer::
set_filename(const Filename &filename) {
  _filename = filename;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::clear_filename
//       Access: Public
//  Description: Removes the alpha filename, if it was previously set.
//               See set_filename().
////////////////////////////////////////////////////////////////////
INLINE void ImageBuffer::
clear_filename() {
  _filename = Filename();
}


////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::set_alpha_filename
//       Access: Public
//  Description: Sets the name of the file that contains the image's
//               alpha channel contents.  Normally, this is set
//               automatically when the image is loaded, for instance
//               via Texture::read().
//
//               The ImageBuffer's get_filename() function returns the
//               name of the image file that was loaded into the
//               buffer.  In the case where a texture specified two
//               separate files to load, a 1- or 3-channel color image
//               and a 1-channel alpha image, this Filename is update
//               to contain the name of the image file that was loaded
//               into the buffer's alpha channel.
////////////////////////////////////////////////////////////////////
INLINE void ImageBuffer::
set_alpha_filename(const Filename &alpha_filename) {
  _alpha_filename = alpha_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::clear_alpha_filename
//       Access: Public
//  Description: Removes the alpha filename, if it was previously set.
//               See set_alpha_filename().
////////////////////////////////////////////////////////////////////
INLINE void ImageBuffer::
clear_alpha_filename() {
  _alpha_filename = Filename();
}
