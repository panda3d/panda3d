/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderModeAttrib.h
 * @author drose
 * @date 2002-03-14
 */

#ifndef RENDERMODEATTRIB_H
#define RENDERMODEATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "geom.h"

class FactoryParams;

/**
 * Specifies how polygons are to be drawn.
 */
class EXPCL_PANDA_PGRAPH RenderModeAttrib : public RenderAttrib {
PUBLISHED:
  enum Mode {
    M_unchanged,

    // Normal, filled polygons.
    M_filled,

    // Wireframe polygons, possibly with thickness.
    M_wireframe,

    // Points at vertices only, possibly with thickness andor perspective
    // sizing.
    M_point,

    // Filled polygons, without any particular emphasis on perspective
    // correctness (a particularly useful designation for software rendering
    // sprites).
    M_filled_flat,

    // Filled polygons with wireframe rendered in front.  The wireframe is
    // given a solid color.
    M_filled_wireframe
  };

private:
  INLINE RenderModeAttrib(Mode mode, PN_stdfloat thickness, bool perspective,
                          const LColor &wireframe_color = LColor::zero());

PUBLISHED:
  static CPT(RenderAttrib) make(Mode mode, PN_stdfloat thickness = 1.0f,
                                bool perspective = false,
                                const LColor &wireframe_color = LColor::zero());
  static CPT(RenderAttrib) make_default();

  INLINE Mode get_mode() const;
  INLINE PN_stdfloat get_thickness() const;
  INLINE bool get_perspective() const;
  INLINE const LColor &get_wireframe_color() const;
  INLINE int get_geom_rendering(int geom_rendering) const;

PUBLISHED:
  MAKE_PROPERTY(mode, get_mode);
  MAKE_PROPERTY(thickness, get_thickness);
  MAKE_PROPERTY(perspective, get_perspective);
  MAKE_PROPERTY(wireframe_color, get_wireframe_color);

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;

private:
  Mode _mode;
  PN_stdfloat _thickness;
  bool _perspective;
  LColor _wireframe_color;

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
    register_type(_type_handle, "RenderModeAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 100,
                                 new RenderModeAttrib(M_filled, 1, false));
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "renderModeAttrib.I"

#endif
