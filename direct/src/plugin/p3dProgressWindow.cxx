// Filename: p3dProgressWindow.cxx
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

#include "p3dProgressWindow.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DProgressWindow::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DProgressWindow::
P3DProgressWindow(P3DPackage *package, P3DSession *session, 
                  P3DInstance *inst) : 
  _package(package),
  _session(session),
  _inst(inst)
{
  _window_type = inst->get_window_type();
  _win_x = inst->get_win_x();
  _win_y = inst->get_win_y();
  _win_width = inst->get_win_width();
  _win_height = inst->get_win_height();
  _parent_window = inst->get_parent_window();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DProgressWindow::package_ready
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DProgressWindow::
package_ready(P3DPackage *package, bool success) {
  if (this == _session->_panda3d_callback) {
    _session->_panda3d_callback = NULL;
    if (package == _session->_panda3d) {
      if (success) {
        _session->start_p3dpython();
      } else {
        cerr << "Failed to install " << package->get_package_name()
             << "_" << package->get_package_version() << "\n";
      }
    } else {
      cerr << "Unexpected panda3d package: " << package << "\n";
    }
  } else {
    cerr << "Unexpected callback for P3DSession\n";
  }
}

