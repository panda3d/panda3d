// Filename: p3dOsxSplashWindow.I
// Created by:  drose (16Jul09)
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
//     Function: P3DOsxSplashWindow::OsxImageData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
inline P3DOsxSplashWindow::OsxImageData::
OsxImageData() {
  _raw_data = NULL;
  _image = NULL;
  _color_space = NULL;
  _provider = NULL;
  _data = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DOsxSplashWindow::OsxImageData::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
inline P3DOsxSplashWindow::OsxImageData::
~OsxImageData() {
  dump_image();
}

