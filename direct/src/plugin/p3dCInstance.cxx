// Filename: p3dCInstance.cxx
// Created by:  drose (08Jun09)
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

#include "p3dCInstance.h"


////////////////////////////////////////////////////////////////////
//     Function: P3DCInstance::Constructor
//       Access: Public
//  Description: Constructs a new Instance from an XML description.
////////////////////////////////////////////////////////////////////
P3DCInstance::
P3DCInstance(TiXmlElement *xinstance) :
  _func(NULL),
  _window_type(P3D_WT_toplevel),
  _win_x(0), _win_y(0),
  _win_width(0), _win_height(0)
{
  xinstance->Attribute("id", &_instance_id);

  const char *p3d_filename = xinstance->Attribute("p3d_filename");
  if (p3d_filename != NULL) {
    _p3d_filename = p3d_filename;
  }

  const char *window_type = xinstance->Attribute("window_type");
  if (window_type != NULL) {
    if (strcmp(window_type, "embedded") == 0) {
      _window_type = P3D_WT_embedded;
    } else if (strcmp(window_type, "toplevel") == 0) {
      _window_type = P3D_WT_toplevel;
    } else if (strcmp(window_type, "fullscreen") == 0) {
      _window_type = P3D_WT_fullscreen;
    } else if (strcmp(window_type, "hidden") == 0) {
      _window_type = P3D_WT_hidden;
    }
  }

  xinstance->Attribute("win_x", &_win_x);
  xinstance->Attribute("win_y", &_win_y);
  xinstance->Attribute("win_width", &_win_width);
  xinstance->Attribute("win_height", &_win_height);

#ifdef _WIN32
  int hwnd;
  if (xinstance->Attribute("parent_hwnd", &hwnd)) {
    _parent_window._hwnd = (HWND)hwnd;
  }
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: P3DCInstance::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DCInstance::
~P3DCInstance() {
}
