/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file depthOffsetAttrib.h
 * @author drose
 * @date 2002-03-14
 */

#ifndef DEPTHOFFSETATTRIB_H
#define DEPTHOFFSETATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "luse.h"

class FactoryParams;

/**
 * This is a special kind of attribute that instructs the graphics driver to
 * apply an offset or bias to the generated depth values for rendered
 * polygons, before they are written to the depth buffer.
 *
 * This can be used to shift polygons forward slightly, to resolve depth
 * conflicts.  The cull traverser may optionally use this, for instance, to
 * implement decals.  However, driver support for this feature seems to be
 * spotty, so use with caution.
 *
 * The bias is always an integer number, and each integer increment represents
 * the smallest possible increment in Z that is sufficient to completely
 * resolve two coplanar polygons.  Positive numbers are closer towards the
 * camera.
 *
 * Nested DepthOffsetAttrib values accumulate; that is, a DepthOffsetAttrib
 * with a value of 1 beneath another DepthOffsetAttrib with a value of 2
 * presents a net offset of 3.  (A DepthOffsetAttrib will not, however,
 * combine with any other DepthOffsetAttribs with a lower override parameter.)
 * The net value should probably not exceed 16 or drop below 0 for maximum
 * portability.
 *
 * Also, and only tangentially related, the DepthOffsetAttrib can be used to
 * constrain the Z output value to a subset of the usual [0, 1] range (or
 * reversing its direction) by specifying a new min_value and max_value.
 */
class EXPCL_PANDA_PGRAPH DepthOffsetAttrib : public RenderAttrib {
private:
  INLINE DepthOffsetAttrib(int offset, PN_stdfloat min_value, PN_stdfloat max_value);

PUBLISHED:
  static CPT(RenderAttrib) make(int offset = 1);
  static CPT(RenderAttrib) make(int offset, PN_stdfloat min_value, PN_stdfloat max_value);
  static CPT(RenderAttrib) make_default();

  INLINE int get_offset() const;
  INLINE PN_stdfloat get_min_value() const;
  INLINE PN_stdfloat get_max_value() const;

PUBLISHED:
  MAKE_PROPERTY(offset, get_offset);
  MAKE_PROPERTY(min_value, get_min_value);
  MAKE_PROPERTY(max_value, get_max_value);

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;

private:
  int _offset;
  PN_stdfloat _min_value;
  PN_stdfloat _max_value;

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
    register_type(_type_handle, "DepthOffsetAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 100,
                                 new DepthOffsetAttrib(0, 0, 1));
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "depthOffsetAttrib.I"

#endif
