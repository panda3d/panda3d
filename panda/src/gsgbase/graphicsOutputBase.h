// Filename: graphicsOutputBase.h
// Created by:  drose (27May09)
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

#ifndef GRAPHICSOUTPUTBASE_H
#define GRAPHICSOUTPUTBASE_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"

class Texture;

////////////////////////////////////////////////////////////////////
//       Class : GraphicsOutputBase
// Description : An abstract base class for GraphicsOutput, for all
//               the usual reasons.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GSGBASE GraphicsOutputBase : public TypedWritableReferenceCount {
PUBLISHED:
  virtual void set_sort(int sort)=0;
  virtual Texture *get_texture(int i=0) const=0;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "GraphicsOutputBase",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GraphicsPipe;
  friend class GraphicsEngine;
  friend class DisplayRegion;
};

#include "graphicsOutputBase.I"

#endif
