// Filename: p3dSplashWindow.I
// Created by:  drose (17Jun09)
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
//     Function: P3DSplashWindow::get_fparams
//       Access: Public
//  Description: Returns the current file parameters.
////////////////////////////////////////////////////////////////////
inline const P3DFileParams &P3DSplashWindow::
get_fparams() const {
  return _fparams;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::get_wparams
//       Access: Public
//  Description: Returns the current window parameters.
////////////////////////////////////////////////////////////////////
inline const P3DWindowParams &P3DSplashWindow::
get_wparams() const {
  return _wparams;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::get_visible
//       Access: Public
//  Description: Returns the current setting of the "visible" flag.
//               If false, the splash window is hidden.
////////////////////////////////////////////////////////////////////
inline bool P3DSplashWindow::
get_visible() const {
  return _visible;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::ImageData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
inline P3DSplashWindow::ImageData::
ImageData() {
  _width = 0;
  _height = 0;
  _num_channels = 0;
}
