/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file scissorAttrib.h
 * @author drose
 * @date 2008-07-29
 */

#ifndef SCISSORATTRIB_H
#define SCISSORATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "luse.h"

class FactoryParams;

/**
 * This restricts rendering to within a rectangular region of the scene,
 * without otherwise affecting the viewport or lens properties.  Geometry that
 * falls outside the scissor region is not rendered.  It is akin to the OpenGL
 * glScissor() function.
 *
 * The ScissorAttrib always specifies its region relative to its enclosing
 * DisplayRegion, in screen space, and performs no culling.
 *
 * See ScissorEffect if you wish to define a region relative to 2-D or 3-D
 * coordinates in the scene graph, with culling.
 */
class EXPCL_PANDA_PGRAPH ScissorAttrib : public RenderAttrib {
private:
  ScissorAttrib(const LVecBase4 &frame);

PUBLISHED:
  static CPT(RenderAttrib) make_off();
  INLINE static CPT(RenderAttrib) make(PN_stdfloat left, PN_stdfloat right, PN_stdfloat bottom, PN_stdfloat top);
  static CPT(RenderAttrib) make(const LVecBase4 &frame);
  static CPT(RenderAttrib) make_default();

  INLINE bool is_off() const;

  INLINE const LVecBase4 &get_frame() const;

PUBLISHED:
  MAKE_PROPERTY(frame, get_frame);

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;

private:
  LVecBase4 _frame;
  bool _off;
  static CPT(RenderAttrib) _off_attrib;

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
    register_type(_type_handle, "ScissorAttrib",
                  RenderAttrib::get_class_type());

    ScissorAttrib *attrib = new ScissorAttrib(LVecBase4(0, 1, 0, 1));
    attrib->_off = true;
    _attrib_slot = register_slot(_type_handle, 100, attrib);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "scissorAttrib.I"

#endif
