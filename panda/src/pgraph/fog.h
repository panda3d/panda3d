// Filename: fog.h
// Created by:  drose (14Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef FOG_H
#define FOG_H

#include "pandabase.h"

#include "pandaNode.h"
#include "luse.h"
#include "cmath.h"
#include "deg_2_rad.h"

class TransformState;

////////////////////////////////////////////////////////////////////
//       Class : Fog
// Description : Specifies how atmospheric fog effects are applied to
//               geometry.  The Fog object is now a PandaNode, which
//               means it can be used similarly to a Light to define
//               effects relative to a particular coordinate system
//               within the scene graph.
//
//               In exponential mode, the fog effects are always
//               camera-relative, and it does not matter where the Fog
//               node is parented.  However, in linear mode, the onset
//               and opaque distances are defined as offsets along the
//               local forward axis (e.g. the Y axis).  This allows
//               the fog effect to be localized to a particular region
//               in space, rather than always camera-relative.  If the
//               fog object is not parented to any node, it is used to
//               generate traditonal camera-relative fog, as if it
//               were parented to the camera.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Fog : public PandaNode {
PUBLISHED:
  Fog(const string &name);

protected:
  Fog(const Fog &copy);

public:
  virtual ~Fog();

  virtual PandaNode *make_copy() const;
  virtual void xform(const LMatrix4f &mat);

PUBLISHED:
  enum Mode {
    M_linear,                   // f = (end - z) / (end - start)
    M_exponential,              // f = e^(-density * z)
    M_exponential_squared       // f = e^((-density * z)^2)
  };

  INLINE Mode get_mode() const;
  INLINE void set_mode(Mode mode);

  INLINE const Colorf &get_color() const;
  INLINE void set_color(float r, float g, float b);
  INLINE void set_color(const Colorf &color);

  INLINE void set_linear_range(float onset, float opaque);

  INLINE const LPoint3f &get_linear_onset_point() const;
  INLINE void set_linear_onset_point(float x, float y, float z);
  INLINE void set_linear_onset_point(const LPoint3f &linear_onset_point);

  INLINE const LPoint3f &get_linear_opaque_point() const;
  INLINE void set_linear_opaque_point(const LPoint3f &linear_opaque_point);
  INLINE void set_linear_opaque_point(float x, float y, float z);

  INLINE void set_linear_fallback(float angle, float onset, float opaque);

  INLINE float get_exp_density() const;
  INLINE void set_exp_density(float exp_density);

  void output(ostream &out) const;

public:
  void adjust_to_camera(const TransformState *camera_transform);
  void get_linear_range(float &onset, float &opaque);

protected:
  void compute_density();

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

protected:
  Mode _mode;
  Colorf _color;
  LPoint3f _linear_onset_point;
  LPoint3f _linear_opaque_point;
  float _exp_density;

  float _linear_fallback_cosa;
  float _linear_fallback_onset, _linear_fallback_opaque;
  
  float _transformed_onset, _transformed_opaque;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "Fog",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

EXPCL_PANDA ostream &operator << (ostream &out, Fog::Mode mode);

INLINE ostream &operator << (ostream &out, const Fog &fog) {
  fog.output(out);
  return out;
}

#include "fog.I"

#endif
