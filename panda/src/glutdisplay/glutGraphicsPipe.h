// Filename: glutGraphicsPipe.h
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
#ifndef GLUTGRAPHICSPIPE_H
#define GLUTGRAPHICSPIPE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <string>
#include <interactiveGraphicsPipe.h>
#include "glutGraphicsWindow.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : glutGraphicsPipe
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAGLUT glutGraphicsPipe : public InteractiveGraphicsPipe
{
public:
  glutGraphicsPipe( const PipeSpecifier& );

  virtual TypeHandle get_window_type() const;

public:
  static GraphicsPipe* make_glutGraphicsPipe(const FactoryParams &params);

  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

protected:
  glutGraphicsPipe( void );
  glutGraphicsPipe( const glutGraphicsPipe& );
  glutGraphicsPipe& operator=(const glutGraphicsPipe&);
};

#endif
