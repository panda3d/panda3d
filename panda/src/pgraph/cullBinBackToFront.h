// Filename: cullBinBackToFront.h
// Created by:  drose (28Feb02)
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

#ifndef CULLBINBACKTOFRONT_H
#define CULLBINBACKTOFRONT_H

#include "pandabase.h"

#include "cullBin.h"
#include "geom.h"
#include "transformState.h"
#include "renderState.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : CullBinBackToFront
// Description : A specific kind of CullBin that sorts geometry in
//               order from furthest to nearest based on the center of
//               its bounding volume.  This is primarily intended for
//               rendering transparent and semi-transparent geometry
//               that must be sorted from back to front.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullBinBackToFront : public CullBin {
public:
  INLINE CullBinBackToFront(const string &name, GraphicsStateGuardianBase *gsg);
  virtual ~CullBinBackToFront();

  virtual void add_object(CullableObject *object);
  virtual void finish_cull();
  virtual void draw();

private:
  class ObjectData {
  public:
    INLINE ObjectData(CullableObject *object, float dist);
    INLINE bool operator < (const ObjectData &other) const;
    
    CullableObject *_object;
    float _dist;
  };

  typedef pvector<ObjectData> Objects;
  Objects _objects;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CullBin::init_type();
    register_type(_type_handle, "CullBinBackToFront",
                  CullBin::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cullBinBackToFront.I"

#endif


  
