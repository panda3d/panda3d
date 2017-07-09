/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightLensNode.h
 * @author drose
 * @date 2002-03-26
 */

#ifndef LIGHTLENSNODE_H
#define LIGHTLENSNODE_H

#include "pandabase.h"

#include "light.h"
#include "camera.h"
#include "graphicsStateGuardianBase.h"
#include "graphicsOutputBase.h"
#include "atomicAdjust.h"

class ShaderGenerator;
class GraphicsStateGuardian;

/**
 * A derivative of Light and of Camera.  The name might be misleading: it does
 * not directly derive from LensNode, but through the Camera class.  The
 * Camera serves no purpose unless shadows are enabled.
 */
class EXPCL_PANDA_PGRAPHNODES LightLensNode : public Light, public Camera {
PUBLISHED:
  LightLensNode(const string &name, Lens *lens = new PerspectiveLens());
  virtual ~LightLensNode();

  INLINE bool is_shadow_caster() const;
  INLINE void set_shadow_caster(bool caster);
  INLINE void set_shadow_caster(bool caster, int buffer_xsize, int buffer_ysize, int sort = -10);

  INLINE LVecBase2i get_shadow_buffer_size() const;
  INLINE void set_shadow_buffer_size(const LVecBase2i &size);

  INLINE GraphicsOutputBase *get_shadow_buffer(GraphicsStateGuardianBase *gsg);

PUBLISHED:
  MAKE_PROPERTY(shadow_caster, is_shadow_caster);
  MAKE_PROPERTY(shadow_buffer_size, get_shadow_buffer_size, set_shadow_buffer_size);

protected:
  LightLensNode(const LightLensNode &copy);
  void clear_shadow_buffers();

  LVecBase2i _sb_size;
  bool _shadow_caster;
  int _sb_sort;

  // This is really a map of GSG -> GraphicsOutput.
  typedef pmap<PT(GraphicsStateGuardianBase), PT(GraphicsOutputBase) > ShadowBuffers;
  ShadowBuffers _sbuffers;

  // This counts how many LightAttribs in the world are referencing this
  // LightLensNode object.
  AtomicAdjust::Integer _attrib_count;

public:
  virtual void attrib_ref();
  virtual void attrib_unref();

  virtual PandaNode *as_node();
  virtual Light *as_light();

PUBLISHED:
  // We have to explicitly publish these because they resolve the multiple
  // inheritance.
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Light::init_type();
    Camera::init_type();
    register_type(_type_handle, "LightLensNode",
                  Light::get_class_type(),
                  Camera::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GraphicsStateGuardian;
  friend class ShaderGenerator;
};

INLINE ostream &operator << (ostream &out, const LightLensNode &light) {
  light.output(out);
  return out;
}

#include "lightLensNode.I"

#endif
