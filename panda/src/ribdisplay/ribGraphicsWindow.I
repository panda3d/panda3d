// Filename: ribGraphicsWindow.I
// Created by:  drose (17Feb99)
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

#include "ribGraphicsStateGuardian.h"

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::set_rib_filename_template
//       Access: Public
//  Description: Specifies the string that defines the filename that
//               will be generated for each RIB file output by the
//               GraphicsWindow.  The template may contain any of the
//               % format characters described in ribGraphicsWindow.h.
//               In particular, if it contains %f (or some variant),
//               there will be a separate RIB file generated for each
//               frame; otherwise, all the frames will be written to
//               the same RIB file.
////////////////////////////////////////////////////////////////////
INLINE void RIBGraphicsWindow::
set_rib_filename_template(const string &str) {
  _rib_filename_template = str;
  _rib_per_frame = check_per_frame(str);
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::get_rib_filename_template
//       Access: Public
//  Description: Returns the string that defines the RIB filename.
//               See set_rib_filename_template().
////////////////////////////////////////////////////////////////////
INLINE string RIBGraphicsWindow::
get_rib_filename_template() const {
  return _rib_filename_template;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::get_rib_filename
//       Access: Public
//  Description: Returns the name of the RIB file that will be created
//               at the next (or possibly only the first) call to
//               begin_frame().
////////////////////////////////////////////////////////////////////
INLINE string RIBGraphicsWindow::
get_rib_filename() const {
  return format_name(_rib_filename_template);
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::rib_per_frame
//       Access: Public
//  Description: Returns true if the RIB filename template contains
//               the format specification %f, which means there will
//               be a separate RIB file generated for each frame.
////////////////////////////////////////////////////////////////////
INLINE bool RIBGraphicsWindow::
rib_per_frame() const {
  return _rib_per_frame;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::set_image_filename_template
//       Access: Public
//  Description: Specifies the string that defines the filename that
//               will be written to the RIB file as an output filename
//               request.  The template may contain any of the
//               % format characters descimageed in RIBGraphicsWindow.h.
//               In particular, if it contains %f (or some variant),
//               there will be a separate image file generated for each
//               frame; otherwise, all the frames will be written to
//               the same image file, whatever that might mean.
//
//               This string may also be empty.  If it is, no
//               "Display" command will be written to the RIB file,
//               and the renderer will choose whatever output filename
//               is appropriate.
////////////////////////////////////////////////////////////////////
INLINE void RIBGraphicsWindow::
set_image_filename_template(const string &str) {
  _image_filename_template = str;
  _image_per_frame = check_per_frame(str);
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::get_image_filename_template
//       Access: Public
//  Description: Returns the string that defines the image filename.
//               See set_image_filename_template().
////////////////////////////////////////////////////////////////////
INLINE string RIBGraphicsWindow::
get_image_filename_template() const {
  return _image_filename_template;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::get_image_filename
//       Access: Public
//  Description: Returns the name of the image file that will be
//               created at the next call to begin_frame().
////////////////////////////////////////////////////////////////////
INLINE string RIBGraphicsWindow::
get_image_filename() const {
  return format_name(_image_filename_template);
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::image_per_frame
//       Access: Public
//  Description: Returns true if the image filename template contains
//               the format specification %f, which means there will
//               be a separate image file generated for each frame.
////////////////////////////////////////////////////////////////////
INLINE bool RIBGraphicsWindow::
image_per_frame() const {
  return _image_per_frame;
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsStateWindow::set_texture_directory
//       Access: Public
//  Description: Sets the name of the directory into which texture
//               maps are copied to be available to the RIB file.
////////////////////////////////////////////////////////////////////
INLINE void RIBGraphicsWindow::
set_texture_directory(const string &directory) {
  DCAST(RIBGraphicsStateGuardian, _gsg)->set_texture_directory(directory);
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::get_texture_directory
//       Access: Public
//  Description: Returns the name of the directory into which texture
//               maps are copied to be available to the RIB file.
////////////////////////////////////////////////////////////////////
INLINE string RIBGraphicsWindow::
get_texture_directory() const {
  return DCAST(RIBGraphicsStateGuardian, _gsg)->get_texture_directory();
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::set_texture_extension
//       Access: Public
//  Description: Specifies the filename extension that texture map
//               files are given when they are copied into the
//               directory for RIB files.  This might also imply an
//               image type.  The default is "tiff", which implies
//               TIFF files.
////////////////////////////////////////////////////////////////////
INLINE void RIBGraphicsWindow::
set_texture_extension(const string &extension) {
  DCAST(RIBGraphicsStateGuardian, _gsg)->set_texture_extension(extension);
}

////////////////////////////////////////////////////////////////////
//     Function: RIBGraphicsWindow::get_texture_extension
//       Access: Public
//  Description: Returns the filename extension that texture map
//               files are given when they are copied into the
//               directory for RIB files.
////////////////////////////////////////////////////////////////////
INLINE string RIBGraphicsWindow::
get_texture_extension() const {
  return DCAST(RIBGraphicsStateGuardian, _gsg)->get_texture_extension();
}
