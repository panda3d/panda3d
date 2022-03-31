/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullFaceAttrib.h
 * @author drose
 * @date 2002-02-27
 */

#ifndef CULLFACEATTRIB_H
#define CULLFACEATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"

class FactoryParams;

/**
 * Indicates which faces should be culled based on their vertex ordering.
 */
class EXPCL_PANDA_PGRAPH CullFaceAttrib : public RenderAttrib {
PUBLISHED:
  enum Mode {
    M_cull_none,                // Cull no polygons
    M_cull_clockwise,           // Cull clockwise-oriented polygons
    M_cull_counter_clockwise,   // Cull counter-clockwise-oriented polygons
    M_cull_unchanged,           // Do not change existing cull behavior
  };

private:
  INLINE CullFaceAttrib(Mode mode, bool reverse);

PUBLISHED:
  static CPT(RenderAttrib) make(Mode mode = M_cull_clockwise);
  static CPT(RenderAttrib) make_reverse();
  static CPT(RenderAttrib) make_default();

  INLINE Mode get_actual_mode() const;
  INLINE bool get_reverse() const;
  Mode get_effective_mode() const;

PUBLISHED:
  MAKE_PROPERTY(mode, get_actual_mode);
  MAKE_PROPERTY(reverse, get_reverse);
  MAKE_PROPERTY(effective_mode, get_effective_mode);

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;

private:
  Mode _mode;
  bool _reverse;

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
    register_type(_type_handle, "CullFaceAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 100,
                                 new CullFaceAttrib(M_cull_clockwise, false));
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "cullFaceAttrib.I"

#endif
