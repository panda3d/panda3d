// Filename: cullBinStateSorted.h
// Created by:  drose (22Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CULLBINSTATESORTED_H
#define CULLBINSTATESORTED_H

#include "pandabase.h"

#include "cullBin.h"
#include "geom.h"
#include "transformState.h"
#include "renderState.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : CullBinStateSorted
// Description : A specific kind of CullBin that sorts geometry to
//               collect items of the same state together, so that
//               minimal state changes are required on the GSG to
//               render them.
//
//               This also sorts objects front-to-back within a
//               particular state, to take advantage of hierarchical
//               Z-buffer algorithms which can early-out when an
//               object appears behind another one.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullBinStateSorted : public CullBin {
public:
  INLINE CullBinStateSorted(const string &name, GraphicsStateGuardianBase *gsg);
  virtual ~CullBinStateSorted();

  virtual void add_object(CullableObject *object);
  virtual void finish_cull();
  virtual void draw();

private:
  class ObjectData {
  public:
    INLINE ObjectData(CullableObject *object);
    INLINE bool operator < (const ObjectData &other) const;
    
    CullableObject *_object;
  };

  typedef pvector<ObjectData> Objects;
  Objects _objects;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CullBin::init_type();
    register_type(_type_handle, "CullBinStateSorted",
                  CullBin::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cullBinStateSorted.I"

#endif


  
