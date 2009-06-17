// Filename: p3dProgressWindow.h
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


#ifndef P3DPROGRESSWINDOW_H
#define P3DPROGRESSWINDOW_H

#include "p3d_plugin_common.h"
#include "p3dPackage.h"

class P3DInstance;

////////////////////////////////////////////////////////////////////
//       Class : P3DProgressWindow
// Description : This window is displayed temporarily, in place of an
//               instance's actual window, during the initial download
//               of the Panda3D code.  When the download is finished,
//               it notifies the session.
//
//               This is the base implementation; it contains no
//               specific code to open a window.
////////////////////////////////////////////////////////////////////
class P3DProgressWindow : public P3DPackage::Callback {
public:
  P3DProgressWindow(P3DPackage *package, P3DSession *session, P3DInstance *inst);

  virtual void package_ready(P3DPackage *package, bool success);
  
protected:
  P3DPackage *_package;
  P3DSession *_session;
  P3DInstance *_inst;

  P3D_window_type _window_type;
  int _win_x, _win_y;
  int _win_width, _win_height;
  P3D_window_handle _parent_window;
};

#include "p3dProgressWindow.I"

#endif
