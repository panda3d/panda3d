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
//  Description: Returns the filename that has been set.  This is the
//               name of the file as it was requested.  Also see
//               get_fullpath().
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
//  Description: Returns the alpha_filename that has been set.  If
//               this is set, it represents the name of the alpha
//               component, which is stored in a separate file.  See
//               also get_filename(), and get_alpha_fullpath().
////////////////////////////////////////////////////////////////////
INLINE const Filename &ImageBuffer::
get_alpha_filename() const {
  return _alpha_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::has_fullpath
//       Access: Published
//  Description: Returns true if the fullpath has been set and
//               is available.  See set_fullpath().
////////////////////////////////////////////////////////////////////
INLINE bool ImageBuffer::
has_fullpath() const {
  return !_fullpath.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::get_fullpath
//       Access: Published
//  Description: Returns the fullpath that has been set.  This is the
//               full path to the file as it was found along the
//               texture search path.
////////////////////////////////////////////////////////////////////
INLINE const Filename &ImageBuffer::
get_fullpath() const {
  return _fullpath;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::has_alpha_fullpath
//       Access: Published
//  Description: Returns true if the alpha_fullpath has been set and
//               is available.  See set_alpha_fullpath().
////////////////////////////////////////////////////////////////////
INLINE bool ImageBuffer::
has_alpha_fullpath() const {
  return !_alpha_fullpath.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::get_alpha_fullpath
//       Access: Published
//  Description: 
//               Returns the alpha_fullpath that has been set.  This
//               is the full path to the alpha part of the image file
//               as it was found along the texture search path.
////////////////////////////////////////////////////////////////////
INLINE const Filename &ImageBuffer::
get_alpha_fullpath() const {
  return _alpha_fullpath;
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

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::set_fullpath
//       Access: Public
//  Description: Sets the full pathname to the file that contains the
//               image's contents, as found along the search path.
//               Normally, this is set automatically when the image is
//               loaded, for instance via Texture::read().
////////////////////////////////////////////////////////////////////
INLINE void ImageBuffer::
set_fullpath(const Filename &fullpath) {
  _fullpath = fullpath;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::clear_fullpath
//       Access: Public
//  Description: Removes the alpha fullpath, if it was previously set.
//               See set_fullpath().
////////////////////////////////////////////////////////////////////
INLINE void ImageBuffer::
clear_fullpath() {
  _fullpath = Filename();
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::set_alpha_fullpath
//       Access: Public
//  Description: Sets the full pathname to the file that contains the
//               image's alpha channel contents, as found along the
//               search path.  Normally, this is set automatically
//               when the image is loaded, for instance via
//               Texture::read().
////////////////////////////////////////////////////////////////////
INLINE void ImageBuffer::
set_alpha_fullpath(const Filename &alpha_fullpath) {
  _alpha_fullpath = alpha_fullpath;
}

////////////////////////////////////////////////////////////////////
//     Function: ImageBuffer::clear_alpha_fullpath
//       Access: Public
//  Description: Removes the alpha fullpath, if it was previously set.
//               See set_alpha_fullpath().
////////////////////////////////////////////////////////////////////
INLINE void ImageBuffer::
clear_alpha_fullpath() {
  _alpha_fullpath = Filename();
}
