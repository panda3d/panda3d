// Filename: fltMesh.h
// Created by:  drose (28Feb01)
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

#ifndef FLTMESH_H
#define FLTMESH_H

#include "pandatoolbase.h"

#include "fltGeometry.h"
#include "fltLocalVertexPool.h"

#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : FltMesh
// Description : A mesh of connected polygons and tristrips, etc.,
//               with a local vertex pool.
////////////////////////////////////////////////////////////////////
class FltMesh : public FltGeometry {
public:
  FltMesh(FltHeader *header);

  PT(FltLocalVertexPool) _vpool;

protected:
  virtual bool extract_record(FltRecordReader &reader);
  virtual bool extract_ancillary(FltRecordReader &reader);

  virtual bool build_record(FltRecordWriter &writer) const;
  virtual FltError write_ancillary(FltRecordWriter &writer) const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FltGeometry::init_type();
    register_type(_type_handle, "FltMesh",
                  FltGeometry::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "fltMesh.I"

#endif


