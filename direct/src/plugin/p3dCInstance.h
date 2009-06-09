// Filename: p3dCInstance.h
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

#ifndef P3DCINSTANCE_H
#define P3DCINSTANCE_H

#include "pandabase.h"

#include "p3d_plugin.h"
#include "pvector.h"

#include <tinyxml.h>

class P3DSession;

////////////////////////////////////////////////////////////////////
//       Class : P3DCInstance
// Description : This is an instance of a Panda3D window, as seen in
//               the parent-level process.
////////////////////////////////////////////////////////////////////
class P3DCInstance : public P3D_instance {
public:
  P3DCInstance(TiXmlElement *xinstance);
  ~P3DCInstance();

  inline const string &get_p3d_filename() const;
  inline int get_instance_id() const;

private:
  class Token {
  public:
    string _keyword;
    string _value;
  };
  typedef pvector<Token> Tokens;

  P3D_request_ready_func *_func;
  string _p3d_filename;
  P3D_window_type _window_type;
  int _win_x, _win_y;
  int _win_width, _win_height;
  P3D_window_handle _parent_window;

  Tokens _tokens;

  int _instance_id;

  friend class P3DPythonRun;
};

#include "p3dCInstance.I"

#endif
