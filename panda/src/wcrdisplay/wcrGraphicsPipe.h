// Filename: wcrGraphicsPipe.h
// Created by:  skyler, based on wgl* file.
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
#ifndef WCRGRAPHICSPIPE_H
#define WCRGRAPHICSPIPE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "pandabase.h"

#include <string>
#include <interactiveGraphicsPipe.h>
//#include "wcrGraphicsWindow.h"
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef WINDOWS_LEAN_AND_MEAN

////////////////////////////////////////////////////////////////////
//       Class : wcrGraphicsPipe
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDACR wcrGraphicsPipe : public InteractiveGraphicsPipe {
PUBLISHED:
  wcrGraphicsPipe(const PipeSpecifier&);

  virtual TypeHandle get_window_type() const;

public:

  static GraphicsPipe* make_wcrGraphicsPipe(const FactoryParams &params);

  static TypeHandle get_class_type();
  static void init_type();
  virtual TypeHandle get_type() const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;

protected:

  wcrGraphicsPipe();
  wcrGraphicsPipe(const wcrGraphicsPipe&);
  wcrGraphicsPipe& operator=(const wcrGraphicsPipe&);
};

#endif
