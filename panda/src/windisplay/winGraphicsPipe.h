// Filename: winGraphicsPipe.h
// Created by:  drose (20Dec02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef WINGRAPHICSPIPE_H
#define WINGRAPHICSPIPE_H

#include "pandabase.h"
#include "graphicsPipe.h"
#include "winGraphicsWindow.h"

////////////////////////////////////////////////////////////////////
//       Class : WinGraphicsPipe
// Description : This is an abstract base class for wglGraphicsPipe
//               and wdxGraphicsPipe; that is, those graphics pipes
//               that are specialized for working with Microsoft
//               Windows.
//
//               There isn't much code here, since most of the fancy
//               stuff is handled in WinGraphicsWindow.  You could
//               make a case that we don't even need a WinGraphicsPipe
//               class at all, but it is provided mainly for
//               completeness.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAWIN WinGraphicsPipe : public GraphicsPipe {
public:
  WinGraphicsPipe();
  virtual ~WinGraphicsPipe();

private:
  HINSTANCE _hUser32;
  typedef BOOL (WINAPI *PFN_TRACKMOUSEEVENT)(LPTRACKMOUSEEVENT);
  PFN_TRACKMOUSEEVENT _pfnTrackMouseEvent;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsPipe::init_type();
    register_type(_type_handle, "WinGraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class WinGraphicsWindow;
};

#include "winGraphicsPipe.I"

#endif
