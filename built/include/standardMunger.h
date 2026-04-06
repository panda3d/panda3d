/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file standardMunger.h
 * @author drose
 * @date 2005-03-21
 */

#ifndef STANDARDMUNGER_H
#define STANDARDMUNGER_H

#include "pandabase.h"
#include "stateMunger.h"
#include "graphicsStateGuardian.h"
#include "colorAttrib.h"
#include "colorScaleAttrib.h"
#include "renderModeAttrib.h"
#include "pointerTo.h"
#include "weakPointerTo.h"

/**
 * Performs some generic munging that is appropriate for all GSG types; for
 * instance, applies ColorAttrib and ColorScaleAttrib to the vertices, and
 * checks for hardware-accelerated animation capabilities.
 */
class EXPCL_PANDA_DISPLAY StandardMunger : public StateMunger {
public:
  StandardMunger(GraphicsStateGuardianBase *gsg, const RenderState *state,
                 int num_components, NumericType numeric_type,
                 Contents contents);
  virtual ~StandardMunger();

  INLINE GraphicsStateGuardian *get_gsg() const;

protected:
  virtual CPT(GeomVertexData) munge_data_impl(const GeomVertexData *data);
  virtual int compare_to_impl(const GeomMunger *other) const;
  virtual void munge_geom_impl(CPT(Geom) &geom, CPT(GeomVertexData) &data,
                               Thread *current_thread);
  virtual void premunge_geom_impl(CPT(Geom) &geom, CPT(GeomVertexData) &data);
  virtual int geom_compare_to_impl(const GeomMunger *other) const;
  virtual CPT(RenderState) munge_state_impl(const RenderState *state);

private:
  int _num_components;
  NumericType _numeric_type;
  Contents _contents;

  bool _auto_shader;
  bool _shader_skinning;
  bool _remove_material;

protected:
  bool _munge_color;
  bool _munge_color_scale;
  LColor _color;
  LVecBase4 _color_scale;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    StateMunger::init_type();
    register_type(_type_handle, "StandardMunger",
                  StateMunger::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "standardMunger.I"

#endif
