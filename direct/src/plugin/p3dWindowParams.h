// Filename: p3dWindowParams.h
// Created by:  drose (22Jun09)
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

#ifndef P3DWINDOWPARAMS_H
#define P3DWINDOWPARAMS_H

#include "p3d_plugin_common.h"
#include "get_tinyxml.h"

class P3DInstance *inst;

////////////////////////////////////////////////////////////////////
//       Class : P3DWindowParams
// Description : Encapsulates the window parameters.
////////////////////////////////////////////////////////////////////
class P3DWindowParams {
public:
  P3DWindowParams();
  P3DWindowParams(P3D_window_type window_type,
                  int win_x, int win_y,
                  int win_width, int win_height,
                  P3D_window_handle parent_window);

  void operator = (const P3DWindowParams &other);

  inline P3D_window_type get_window_type() const;
  inline void set_window_type(P3D_window_type window_type);

  inline int get_win_x() const;
  inline int get_win_y() const;
  inline int get_win_width() const;
  inline int get_win_height() const;
  inline const P3D_window_handle &get_parent_window() const;

  TiXmlElement *make_xml(P3DInstance *inst);

private:
  P3D_window_type _window_type;
  int _win_x, _win_y;
  int _win_width, _win_height;
  P3D_window_handle _parent_window;
};

#include "p3dWindowParams.I"

#endif
