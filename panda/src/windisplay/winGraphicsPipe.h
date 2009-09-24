// Filename: winGraphicsPipe.h
// Created by:  drose (20Dec02)
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

  virtual void lookup_cpu_data();

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

extern EXPCL_PANDAWIN bool MyLoadLib(HINSTANCE &hDLL, const char *DLLname);
extern EXPCL_PANDAWIN bool MyGetProcAddr(HINSTANCE hDLL, FARPROC *pFn, const char *szExportedFnName);

#include "winGraphicsPipe.I"

#endif
