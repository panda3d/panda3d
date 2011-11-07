// Filename: alphaTestAttrib.h
// Created by:  drose (04Mar02)
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

#ifndef ALPHATESTATTRIB_H
#define ALPHATESTATTRIB_H

#include "pandabase.h"
#include "renderAttrib.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : AlphaTestAttrib
// Description : Enables or disables writing of pixel to framebuffer
//               based on its alpha value relative to a reference alpha value
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH AlphaTestAttrib : public RenderAttrib {
private:
  INLINE AlphaTestAttrib(PandaCompareFunc mode = M_always,
                         PN_stdfloat reference_alpha = 1.0f);

PUBLISHED:
  static CPT(RenderAttrib) make(PandaCompareFunc mode,
                                PN_stdfloat reference_alpha);
  static CPT(RenderAttrib) make_default();

  INLINE PN_stdfloat get_reference_alpha() const;
  INLINE PandaCompareFunc get_mode() const;

public:
  virtual void output(ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) get_auto_shader_attrib_impl(const RenderState *state) const;

private:
  PandaCompareFunc _mode;
  PN_stdfloat _reference_alpha;  // should be in range [0.0-1.0]

PUBLISHED:
  static int get_class_slot() {
    return _attrib_slot;
  }
  virtual int get_slot() const {
    return get_class_slot();
  }

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
    register_type(_type_handle, "AlphaTestAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 100, make_default);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "alphaTestAttrib.I"

#endif

