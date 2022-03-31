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
  explicit LightLensNode(const std::string &name, Lens *lens = new PerspectiveLens());
  virtual ~LightLensNode();

  INLINE bool has_specular_color() const;

  INLINE bool is_shadow_caster() const;
  void set_shadow_caster(bool caster);
  void set_shadow_caster(bool caster, int buffer_xsize, int buffer_ysize, int sort = -10);

  INLINE int get_shadow_buffer_sort() const;

  INLINE LVecBase2i get_shadow_buffer_size() const;
  INLINE void set_shadow_buffer_size(const LVecBase2i &size);

  INLINE GraphicsOutputBase *get_shadow_buffer(GraphicsStateGuardianBase *gsg);

PUBLISHED:
  MAKE_PROPERTY(shadow_caster, is_shadow_caster);
  MAKE_PROPERTY(shadow_buffer_size, get_shadow_buffer_size, set_shadow_buffer_size);

public:
  INLINE void mark_used_by_auto_shader() const;

protected:
  LightLensNode(const LightLensNode &copy);
  void clear_shadow_buffers();
  virtual void setup_shadow_map();

  LVecBase2i _sb_size;
  bool _shadow_caster;
  bool _has_specular_color;
  int _sb_sort;
  mutable bool _used_by_auto_shader = false;

  PT(Texture) _shadow_map;

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
  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

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
};

INLINE std::ostream &operator << (std::ostream &out, const LightLensNode &light) {
  light.output(out);
  return out;
}

#include "lightLensNode.I"

#endif
