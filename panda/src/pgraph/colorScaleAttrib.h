/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file colorScaleAttrib.h
 * @author drose
 * @date 2002-03-14
 */

#ifndef COLORSCALEATTRIB_H
#define COLORSCALEATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "luse.h"

class FactoryParams;

/**
 * Applies a scale to colors in the scene graph and on vertices.
 */
class EXPCL_PANDA_PGRAPH ColorScaleAttrib : public RenderAttrib {
protected:
  ColorScaleAttrib(bool off, const LVecBase4 &scale);
  INLINE ColorScaleAttrib(const ColorScaleAttrib &copy);

PUBLISHED:
  static CPT(RenderAttrib) make_identity();
  static CPT(RenderAttrib) make(const LVecBase4 &scale);
  static CPT(RenderAttrib) make_off();
  static CPT(RenderAttrib) make_default();

  INLINE bool is_off() const;
  INLINE bool is_identity() const;
  INLINE bool has_scale() const;
  INLINE bool has_rgb_scale() const;
  INLINE bool has_alpha_scale() const;
  INLINE const LVecBase4 &get_scale() const;
  CPT(RenderAttrib) set_scale(const LVecBase4 &scale) const;

PUBLISHED:
  MAKE_PROPERTY2(scale, has_scale, get_scale);

public:
  virtual bool lower_attrib_can_override() const;
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;

private:
  void quantize_scale();

private:
  bool _off;
  bool _has_scale;
  bool _has_rgb_scale;
  bool _has_alpha_scale;
  LVecBase4 _scale;
  static CPT(RenderAttrib) _identity_attrib;

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
    register_type(_type_handle, "ColorScaleAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 100,
      new ColorScaleAttrib(false, LVecBase4(1, 1, 1, 1)));
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "colorScaleAttrib.I"

#endif
