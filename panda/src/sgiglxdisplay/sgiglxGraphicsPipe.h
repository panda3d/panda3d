// Filename: sgiglxGraphicsPipe.h
// Created by:  cary (01Oct99)
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

#ifndef __SGIGLXGRAPHICSPIPE_H__
#define __SGIGLXGRAPHICSPIPE_H__

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include <sgiGraphicsPipe.h>
#include <glxDisplay.h>

////////////////////////////////////////////////////////////////////
//       Class : SgiGlxGraphicsPipe
// Description : This kind of pipe can create glx windows but
//               supports the functionality of an sgi pipe with
//               hardware channels
////////////////////////////////////////////////////////////////////
class SgiGlxGraphicsPipe : public sgiGraphicsPipe, public glxDisplay {
public:
  SgiGlxGraphicsPipe( const PipeSpecifier& );

  virtual TypeHandle get_window_type() const;
  virtual glxDisplay *get_glx_display();

  static GraphicsPipe* make_sgiglxgraphicspipe(const FactoryParams &params);

  static TypeHandle get_class_type( void );
  static void init_type( void );
  virtual TypeHandle get_type( void ) const;
  virtual TypeHandle force_init_type( void );

private:
  static TypeHandle _type_handle;
};

#endif /* __SGIGLXGRAPHICSPIPE_H__ */
