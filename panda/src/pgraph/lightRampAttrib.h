/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightRampAttrib.h
 * @author drose
 * @date 2002-03-04
 */

#ifndef LIGHTRAMPATTRIB_H
#define LIGHTRAMPATTRIB_H

#include "pandabase.h"
#include "renderAttrib.h"

class FactoryParams;

/**
 * A Light Ramp is any unary operator that takes a rendered pixel as input,
 * and adjusts the brightness of that pixel.  For example, gamma correction is
 * a kind of light ramp.  So is HDR tone mapping.  So is cartoon shading.  See
 * the constructors for an explanation of each kind of ramp.
 */
class EXPCL_PANDA_PGRAPH LightRampAttrib : public RenderAttrib {
private:
  INLINE LightRampAttrib();

PUBLISHED:
  enum LightRampMode {
    LRT_default,
    LRT_identity,
    LRT_single_threshold,
    LRT_double_threshold,
    LRT_hdr0,
    LRT_hdr1,
    LRT_hdr2,
  };
  static CPT(RenderAttrib) make_default();
  static CPT(RenderAttrib) make_identity();
  static CPT(RenderAttrib) make_single_threshold(PN_stdfloat thresh0, PN_stdfloat lev0);
  static CPT(RenderAttrib) make_double_threshold(PN_stdfloat thresh0, PN_stdfloat lev0, PN_stdfloat thresh1, PN_stdfloat lev1);
  static CPT(RenderAttrib) make_hdr0();
  static CPT(RenderAttrib) make_hdr1();
  static CPT(RenderAttrib) make_hdr2();


  INLINE LightRampMode get_mode() const;
  INLINE PN_stdfloat get_level(int n) const;
  INLINE PN_stdfloat get_threshold(int n) const;

PUBLISHED:
  MAKE_PROPERTY(mode, get_mode);

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;

private:
  LightRampMode _mode;
  PN_stdfloat _level[2];
  PN_stdfloat _threshold[2];

  static CPT(RenderAttrib) _default;

PUBLISHED:
  static int get_class_slot() {
    return _attrib_slot;
  }
  virtual int get_slot() const {
    return get_class_slot();
  }
  MAKE_PROPERTY(class_slot, get_class_slot);

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
    register_type(_type_handle, "LightRampAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 100, new LightRampAttrib);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "lightRampAttrib.I"

#endif
