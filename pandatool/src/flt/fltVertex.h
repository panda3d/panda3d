// Filename: fltVertex.h
// Created by:  drose (25Aug00)
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

#ifndef FLTVERTEX_H
#define FLTVERTEX_H

#include "pandatoolbase.h"

#include "fltRecord.h"
#include "fltPackedColor.h"

#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : FltVertex
// Description : Represents a single vertex in the vertex palette.
//               Flt files index vertices by their byte offset in the
//               vertex palette; within this library, we map those
//               byte offsets to pointers automatically.
//
//               This may represent a vertex with or without a normal
//               or texture coordinates.
////////////////////////////////////////////////////////////////////
class FltVertex : public FltRecord {
public:
  FltVertex(FltHeader *header);

  FltOpcode get_opcode() const;
  int get_record_length() const;

  enum Flags {
    F_hard_edge         = 0x8000,
    F_normal_frozen     = 0x4000,
    F_no_color          = 0x2000,
    F_packed_color      = 0x1000
  };

  int _color_name_index;
  unsigned int _flags;
  LPoint3d _pos;
  LPoint3f _normal;
  LPoint2f _uv;
  FltPackedColor _packed_color;
  int _color_index;

  bool _has_normal;
  bool _has_uv;

public:
  INLINE bool has_color() const;
  Colorf get_color() const;
  INLINE void set_color(const Colorf &color);
  RGBColorf get_rgb() const;
  void set_rgb(const RGBColorf &rgb);


protected:
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
    register_type(_type_handle, "FltVertex",
                  FltRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class FltHeader;
};

#include "fltVertex.I"

#endif


