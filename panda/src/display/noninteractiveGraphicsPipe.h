// Filename: noninteractiveGraphicsPipe.h
// Created by:  cary (10Mar99)
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

#ifndef __NONINTERACTIVEGRAPHICSPIPE_H__
#define __NONINTERACTIVEGRAPHICSPIPE_H__

#include <pandabase.h>

#include "graphicsPipe.h"

class EXPCL_PANDA NoninteractiveGraphicsPipe : public GraphicsPipe {
public:

  NoninteractiveGraphicsPipe( const PipeSpecifier& );
  virtual ~NoninteractiveGraphicsPipe(void) = 0;

public:

  static TypeHandle get_class_type(void);
  static void init_type(void);
  virtual TypeHandle get_type(void) const;
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:

  static TypeHandle _type_handle;

protected:

  NoninteractiveGraphicsPipe(void);
  NoninteractiveGraphicsPipe(const NoninteractiveGraphicsPipe&);
  NoninteractiveGraphicsPipe& operator=(const NoninteractiveGraphicsPipe&);
};

#include "noninteractiveGraphicsPipe.I"

#endif /* __NONINTERACTIVEGRAPHICSPIPE_H__ */
