// Filename: alphaTestAttrib.h
// Created by:  drose (04Mar02)
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
class EXPCL_PANDA AlphaTestAttrib : public RenderAttrib {
private:
  INLINE AlphaTestAttrib(PandaCompareFunc mode = M_always,
                         float reference_alpha = 1.0f);

PUBLISHED:
  static CPT(RenderAttrib) make(PandaCompareFunc mode,
                                float reference_alpha);
  INLINE float get_reference_alpha() const;
  INLINE PandaCompareFunc get_mode() const;

public:
  virtual void output(ostream &out) const;
  virtual void store_into_slot(AttribSlots *slots) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  PandaCompareFunc _mode;
  float _reference_alpha;  // should be in range [0.0-1.0]

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
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "alphaTestAttrib.I"

#endif

