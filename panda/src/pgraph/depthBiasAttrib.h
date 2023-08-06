/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file depthBiasAttrib.h
 * @author rdb
 * @date 2021-08-24
 */

#ifndef DEPTHBIASATTRIB_H
#define DEPTHBIASATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "luse.h"

class FactoryParams;

/**
 * This is a special kind of attribute that instructs the graphics driver to
 * apply an offset or bias to the generated depth values for rendered
 * polygons, before they are written to the depth buffer.
 *
 * This class replaces the old DepthOffsetAttrib, which had a more limited
 * parameterization.  The differences are:
 * - The sign of the factor parameter was inverted.
 * - The slope and constant factors are specified separately.
 * - The factors are specified as floating-point instead of integer.
 * - There is a new clamp parameter.
 *
 * Nested DepthBiasAttrib values accumulate; that is, a DepthBiasAttrib
 * with a value of 1 beneath another DepthBiasAttrib with a value of 2
 * presents a net offset of 3.  (A DepthBiasAttrib will not, however,
 * combine with any other DepthBiasAttribs with a lower override parameter.)
 *
 * @since 1.11.0
 */
class EXPCL_PANDA_PGRAPH DepthBiasAttrib final : public RenderAttrib {
private:
  INLINE DepthBiasAttrib(PN_stdfloat slope_factor, PN_stdfloat constant_factor,
                         PN_stdfloat clamp = 0);

PUBLISHED:
  static CPT(RenderAttrib) make(PN_stdfloat slope_factor, PN_stdfloat constant_factor,
                                PN_stdfloat clamp = 0);
  static CPT(RenderAttrib) make_default();

public:
  INLINE PN_stdfloat get_slope_factor() const;
  INLINE PN_stdfloat get_constant_factor() const;
  INLINE PN_stdfloat get_clamp() const;

PUBLISHED:
  MAKE_PROPERTY(slope_factor, get_slope_factor);
  MAKE_PROPERTY(constant_factor, get_constant_factor);
  MAKE_PROPERTY(clamp, get_clamp);

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;

private:
  PN_stdfloat _slope_factor;
  PN_stdfloat _constant_factor;
  PN_stdfloat _clamp;

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
    register_type(_type_handle, "DepthBiasAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 100,
                                 new DepthBiasAttrib(0, 0, 0));
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "depthBiasAttrib.I"

#endif
