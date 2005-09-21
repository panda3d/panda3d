// Filename: colorScaleAttrib.h
// Created by:  drose (14Mar02)
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

#ifndef COLORSCALEATTRIB_H
#define COLORSCALEATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "luse.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : ColorScaleAttrib
// Description : Applies a scale to colors in the scene graph and on
//               vertices.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorScaleAttrib : public RenderAttrib {
protected:
  INLINE ColorScaleAttrib(bool off, const LVecBase4f &scale);
  INLINE ColorScaleAttrib(const ColorScaleAttrib &copy);

PUBLISHED:
  static CPT(RenderAttrib) make_identity();
  static CPT(RenderAttrib) make(const LVecBase4f &scale);
  static CPT(RenderAttrib) make_off();

  INLINE bool is_off() const;
  INLINE bool is_identity() const;
  INLINE bool has_scale() const;
  INLINE const LVecBase4f &get_scale() const;
  CPT(RenderAttrib) set_scale(const LVecBase4f &scale) const;

public:
  virtual bool lower_attrib_can_override() const;
  virtual void output(ostream &out) const;
  virtual void store_into_slot(AttribSlots *slots) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  void quantize_scale();

private:
  bool _off;
  bool _has_scale;
  LVecBase4f _scale;
  static CPT(RenderAttrib) _identity_attrib;

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
    register_type(_type_handle, "ColorScaleAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "colorScaleAttrib.I"

#endif

