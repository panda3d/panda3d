// Filename: colorAttrib.h
// Created by:  drose (22Feb02)
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

#ifndef COLORATTRIB_H
#define COLORATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "luse.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : ColorAttrib
// Description : Indicates what color should be applied to renderable
//               geometry.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH ColorAttrib : public RenderAttrib {
PUBLISHED:
  enum Type {
    T_vertex, T_flat, T_off
  };

private:
  INLINE ColorAttrib(Type type = T_vertex, 
                     const Colorf &color = Colorf(0.0f, 0.0f, 0.0f, 1.0f));

PUBLISHED:
  static CPT(RenderAttrib) make_vertex();
  static CPT(RenderAttrib) make_flat(const Colorf &color);
  static CPT(RenderAttrib) make_off();

  INLINE Type get_color_type() const;
  INLINE const Colorf &get_color() const;

public:
  virtual void output(ostream &out) const;
  virtual void store_into_slot(AttribSlots *slots) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  void quantize_color();

private:
  Type _type;
  Colorf _color;
  static CPT(RenderAttrib) _off;
  static CPT(RenderAttrib) _vertex;

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
    register_type(_type_handle, "ColorAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "colorAttrib.I"

#endif

