// Filename: decalAttrib.h
// Created by:  drose (04Mar02)
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

#ifndef DECALATTRIB_H
#define DECALATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"

////////////////////////////////////////////////////////////////////
//       Class : DecalAttrib
// Description : Applied to a GeomNode to indicate that the children
//               of this GeomNode are coplanar and should be drawn as
//               decals (eliminating Z-fighting).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DecalAttrib : public RenderAttrib {
private:
  INLINE DecalAttrib();

PUBLISHED:
  static CPT(RenderAttrib) make();

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "DecalAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "decalAttrib.I"

#endif

