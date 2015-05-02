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
#include "get_tinyxml.h"
#include "windowHandle.h"

#include <Python.h>

class P3DSession;

////////////////////////////////////////////////////////////////////
//       Class : P3DCInstance
// Description : This is an instance of a Panda3D window, as seen in
//               the child-level process.
////////////////////////////////////////////////////////////////////
class P3DCInstance : public P3D_instance {
public:
  P3DCInstance(TiXmlElement *xinstance);
  ~P3DCInstance();

  inline int get_instance_id() const;

public:
  PT(WindowHandle) _parent_window_handle;

private:
  P3D_request_ready_func *_func;

  int _instance_id;

  friend class P3DPythonRun;
};

#include "p3dCInstance.I"

#endif
