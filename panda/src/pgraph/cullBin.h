// Filename: cullBin.h
// Created by:  drose (27Feb02)
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

#ifndef CULLBIN_H
#define CULLBIN_H

#include "pandabase.h"

#include "typedReferenceCount.h"
#include "pStatCollector.h"
#include "pointerTo.h"

class CullableObject;
class GraphicsStateGuardianBase;

////////////////////////////////////////////////////////////////////
//       Class : CullBin
// Description : A collection of Geoms and their associated state, for
//               a particular scene.  The cull traversal (and the
//               BinCullHandler) assigns Geoms to bins as it comes
//               across them.
//
//               This is an abstract base class; derived classes like
//               CullBinStateSorted and CullBinBackToFront provide the
//               actual implementation.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullBin : public TypedReferenceCount {
public:
  INLINE CullBin(const string &name, GraphicsStateGuardianBase *gsg);
  virtual ~CullBin();

  virtual PT(CullBin) make_next() const;

  virtual void add_object(CullableObject *object)=0;
  virtual void finish_cull();

  virtual void draw()=0;

protected:
  GraphicsStateGuardianBase *_gsg;

  static PStatCollector _cull_bin_pcollector;
  static PStatCollector _draw_bin_pcollector;
  PStatCollector _cull_this_pcollector;
  PStatCollector _draw_this_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "CullBin",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cullBin.I"

#endif


  
