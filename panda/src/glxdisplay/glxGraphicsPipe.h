// Filename: glxGraphicsPipe.h
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

#ifndef GLXGRAPHICSPIPE_H
#define GLXGRAPHICSPIPE_H

#include "pandabase.h"
#include "glxDisplay.h"
#include "interactiveGraphicsPipe.h"

////////////////////////////////////////////////////////////////////
//       Class : glxGraphicsPipe
// Description :
////////////////////////////////////////////////////////////////////
class glxGraphicsPipe : public InteractiveGraphicsPipe, public glxDisplay {
PUBLISHED:
  glxGraphicsPipe( const PipeSpecifier& );

  virtual TypeHandle get_window_type() const;

public:  
  virtual glxDisplay *get_glx_display();

public:
  static GraphicsPipe *make_glxGraphicsPipe(const FactoryParams &params);
  
  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  
private:
  static TypeHandle _type_handle;
};

#endif
