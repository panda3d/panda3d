// Filename: wdxGraphicsPipe.h
// Created by:  mike (09Jan97)
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
#ifndef WDXGRAPHICSPIPE_H
#define WDXGRAPHICSPIPE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////

#include <string>
#include "wdxGraphicsWindow.h"
#include <interactiveGraphicsPipe.h>

////////////////////////////////////////////////////////////////////
//       Class : wdxGraphicsPipe
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDADX wdxGraphicsPipe : public InteractiveGraphicsPipe {
public:
  wdxGraphicsPipe(const PipeSpecifier&);

  wdxGraphicsWindow* find_window(HWND win);
//  ButtonHandle lookup_key(WPARAM wparam) const;

  virtual TypeHandle get_window_type() const;

public:

  static GraphicsPipe* make_wdxGraphicsPipe(const FactoryParams &params);

  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;

//  int               _width;
//  int               _height;
//  bool              _shift;

protected:

  wdxGraphicsPipe(void);
  wdxGraphicsPipe(const wdxGraphicsPipe&);
  wdxGraphicsPipe& operator=(const wdxGraphicsPipe&);

};

#endif
