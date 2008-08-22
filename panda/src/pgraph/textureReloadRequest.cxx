// Filename: textureReloadRequest.cxx
// Created by:  drose (12Aug08)
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

#include "textureReloadRequest.h"
#include "loader.h"

TypeHandle TextureReloadRequest::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TextureReloadRequest::do_task
//       Access: Protected, Virtual
//  Description: Performs the task: that is, loads the one model.
////////////////////////////////////////////////////////////////////
bool TextureReloadRequest::
do_task() {
  // Don't reload the texture if it doesn't need it.
  if (_texture_context->was_image_modified()) {
    if (_allow_compressed) {
      _texture->get_ram_image();
    } else {
      _texture->get_uncompressed_ram_image();
    }
  }
  _is_ready = true;

  // Don't continue the task; we're done.
  return false;
}
