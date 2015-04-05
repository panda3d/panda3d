// Filename: p3dWinSplashWindow.I
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
//     Function: P3DWinSplashWindow::WinImageData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
inline P3DWinSplashWindow::WinImageData::
WinImageData() {
  _filename_changed = false;
  _bitmap = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DWinSplashWindow::WinImageData::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
inline P3DWinSplashWindow::WinImageData::
~WinImageData() {
  dump_image();
}

