/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltLocalVertexPool.h
 * @author drose
 * @date 2001-02-28
 */

#ifndef FLTLOCALVERTEXPOOL_H
#define FLTLOCALVERTEXPOOL_H

#include "pandatoolbase.h"

#include "fltRecord.h"
#include "fltHeader.h"
#include "fltVertex.h"

#include "pointerTo.h"

/**
 * A local vertex pool, as might appear in the middle of the hierarchy, for
 * instance for a mesh.
 */
class FltLocalVertexPool : public FltRecord {
public:
  FltLocalVertexPool(FltHeader *header);

  // These bits are not stored in the vertex pool, but are read from the .flt
  // file and used immediately.
  enum AttributeMask {
    AM_has_position      = 0x80000000,
    AM_has_color_index   = 0x40000000,
    AM_has_packed_color  = 0x20000000,
    AM_has_normal        = 0x10000000,
    AM_has_base_uv       = 0x08000000,
    AM_has_uv_1          = 0x04000000,
    AM_has_uv_2          = 0x02000000,
    AM_has_uv_3          = 0x01000000,
    AM_has_uv_4          = 0x00800000,
    AM_has_uv_5          = 0x00400000,
    AM_has_uv_6          = 0x00200000,
    AM_has_uv_7          = 0x00100000
  };

  typedef pvector<PT(FltVertex)> Vertices;
  Vertices _vertices;

public:
  virtual bool extract_record(FltRecordReader &reader);
  virtual bool build_record(FltRecordWriter &writer) const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FltRecord::init_type();
    register_type(_type_handle, "FltLocalVertexPool",
                  FltRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "fltLocalVertexPool.I"

#endif
