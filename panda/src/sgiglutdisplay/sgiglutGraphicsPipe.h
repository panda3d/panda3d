// Filename: sgiglutGraphicsPipe.h
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
#ifndef SGIGLUTGRAPHICSPIPE_H
#define SGIGLUTGRAPHICSPIPE_H
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <string>
#include <sgiGraphicsPipe.h>
#include <glutGraphicsWindow.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : sgiglutGraphicsPipe
// Description : This kind of pipe can create glut windows but
//               supports the functionality of an sgi pipe with
//               hardware channels
////////////////////////////////////////////////////////////////////
class sgiglutGraphicsPipe : public sgiGraphicsPipe
{
public:
  sgiglutGraphicsPipe( const PipeSpecifier& );

  virtual TypeHandle get_window_type() const;

public:
  static GraphicsPipe* make_sgiglutGraphicsPipe(const FactoryParams &params);

  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif
