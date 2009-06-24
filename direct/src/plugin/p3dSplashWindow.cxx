// Filename: p3dSplashWindow.cxx
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

#include "p3dSplashWindow.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DSplashWindow::
P3DSplashWindow(P3DInstance *inst) : 
  _inst(inst),
  _fparams(inst->get_fparams()),
  _wparams(inst->get_wparams())
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DSplashWindow::
~P3DSplashWindow() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::set_fparams
//       Access: Public, Virtual
//  Description: Sets up the file parameters for the window.
////////////////////////////////////////////////////////////////////
void P3DSplashWindow::
set_fparams(const P3DFileParams &fparams) {
  _fparams = fparams;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::set_wparams
//       Access: Public, Virtual
//  Description: Changes the window parameters, e.g. to resize or
//               reposition the window; or sets the parameters for the
//               first time, creating the initial window.
////////////////////////////////////////////////////////////////////
void P3DSplashWindow::
set_wparams(const P3DWindowParams &wparams) {
  _wparams = wparams;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::open_window
//       Access: Public, Virtual
//  Description: Creates the splash window.
////////////////////////////////////////////////////////////////////
void P3DSplashWindow::
open_window() {
  nout << "P3DSplashWindow::open_window()\n" << flush;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::close_window
//       Access: Public, Virtual
//  Description: Closes the window created above.
////////////////////////////////////////////////////////////////////
void P3DSplashWindow::
close_window() {
  nout << "P3DSplashWindow::close_window()\n" << flush;
}
