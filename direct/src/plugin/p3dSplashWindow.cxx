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
//  Description: By the time the SplashWindow is created, the instance
//               has received both its fparams and its wparams.  Copy
//               them both into this class for reference.
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
//     Function: P3DSplashWindow::set_install_label
//       Access: Public, Virtual
//  Description: Specifies the text that is displayed above the
//               install progress bar.
////////////////////////////////////////////////////////////////////
void P3DSplashWindow::
set_install_label(const string &install_label) {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DSplashWindow::set_install_progress
//       Access: Public, Virtual
//  Description: Moves the install progress bar from 0.0 to 1.0.
////////////////////////////////////////////////////////////////////
void P3DSplashWindow::
set_install_progress(double install_progress) {
}

