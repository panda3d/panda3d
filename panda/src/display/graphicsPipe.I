// Filename: graphicsPipe.I
// Created by:  frang (07Mar99)
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
//     Function: GraphicsPipe::is_valid
//       Access: Published
//  Description: Returns false if this pipe is known to be invalid,
//               meaning that an attempt to create a GraphicsWindow
//               with the pipe will certainly fail.  Returns true if
//               the pipe is probably valid (is this case, an attempt
//               to create a GraphicsWindow should succeed, but might
//               still fail).
//
//               Use the GraphicsEngine class to create a
//               GraphicsWindow on a particular pipe.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsPipe::
is_valid() const {
  return _is_valid;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::supports_fullscreen
//       Access: Published
//  Description: Returns false if this pipe is known to not support
//               any creation of fullscreen windows.  If this returns
//               false, any attempt to create a window with the
//               fullscreen property set will certainly fail.
//
//               Returns true when the pipe will probably support
//               fullscreen windows.  This is not, however, a
//               guarantee that an attempt to create a fullscreen
//               window will not fail.
////////////////////////////////////////////////////////////////////
INLINE bool GraphicsPipe::
supports_fullscreen() const {
  return _supports_fullscreen;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_display_width
//       Access: Published
//  Description: Returns the width of the entire display, if it is
//               known.  This may return 0.  This is not a guarantee
//               that windows (particularly fullscreen windows) may
//               not be created larger than this width, but it is
//               intended to provide a hint to the application.
////////////////////////////////////////////////////////////////////
INLINE int GraphicsPipe::
get_display_width() const {
  return _display_width;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_display_height
//       Access: Published
//  Description: Returns the height of the entire display, if it is
//               known.  This may return 0.  See the caveats for
//               get_display_width().
////////////////////////////////////////////////////////////////////
INLINE int GraphicsPipe::
get_display_height() const {
  return _display_height;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphicsPipe::get_device
//       Access: Public
//  Description: Returns a pointer to device object
////////////////////////////////////////////////////////////////////
INLINE GraphicsDevice *GraphicsPipe::
get_device() const {
  return _device;
}

