// Filename: lightLensNode.h
// Created by:  drose (26Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef LIGHTLENSNODE_H
#define LIGHTLENSNODE_H

#include "pandabase.h"

#include "light.h"
#include "camera.h"
#include "graphicsStateGuardianBase.h"
#include "graphicsOutputBase.h"

class ShaderGenerator;
class GraphicsStateGuardian;

////////////////////////////////////////////////////////////////////
//       Class : LightLensNode
// Description : A derivative of Light and of Camera. The name might
//               be misleading: it does not directly derive from
//               LensNode, but through the Camera class. The Camera
//               serves no purpose unless shadows are enabled.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPHNODES LightLensNode : public Light, public Camera {
PUBLISHED:
  LightLensNode(const string &name, Lens *lens = new PerspectiveLens());
  virtual ~LightLensNode();

  INLINE bool is_shadow_caster();
  INLINE void set_shadow_caster(bool caster);
  INLINE void set_shadow_caster(bool caster, int buffer_xsize, int buffer_ysize, int sort = -10);

  INLINE GraphicsOutputBase *get_shadow_buffer(GraphicsStateGuardianBase *gsg);

protected:
  LightLensNode(const LightLensNode &copy);
  void clear_shadow_buffers();

  bool _shadow_caster;
  int _sb_xsize, _sb_ysize, _sb_sort;

  // This is really a map of GSG -> GraphicsOutput.
  typedef pmap<PT(GraphicsStateGuardianBase), PT(GraphicsOutputBase) > ShadowBuffers;
  ShadowBuffers _sbuffers;

public:
  virtual PandaNode *as_node();
  virtual Light *as_light();

PUBLISHED:
  // We have to explicitly publish these because they resolve the
  // multiple inheritance.
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
