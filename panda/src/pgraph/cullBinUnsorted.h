// Filename: cullBinUnsorted.h
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

#ifndef CULLBINUNSORTED_H
#define CULLBINUNSORTED_H

#include "pandabase.h"

#include "cullBin.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : CullBinUnsorted
// Description : A specific kind of CullBin that does not reorder the
//               geometry; it simply passes it through to the GSG in
//               the same order it was encountered, which will be in
//               scene-graph order.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullBinUnsorted : public CullBin {
public:
  INLINE CullBinUnsorted(const string &name, GraphicsStateGuardianBase *gsg);
  ~CullBinUnsorted();

  virtual void add_object(CullableObject *object);
  virtual void draw();

private:
  typedef pvector<CullableObject *> Objects;
  Objects _objects;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CullBin::init_type();
    register_type(_type_handle, "CullBinUnsorted",
                  CullBin::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cullBinUnsorted.I"

#endif


  
