/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fog.h
 * @author drose
 * @date 2002-03-14
 */

#ifndef FOG_H
#define FOG_H

#include "pandabase.h"

#include "pandaNode.h"
#include "luse.h"
#include "cmath.h"
#include "deg_2_rad.h"

class TransformState;

/**
 * Specifies how atmospheric fog effects are applied to geometry.  The Fog
 * object is now a PandaNode, which means it can be used similarly to a Light
 * to define effects relative to a particular coordinate system within the
 * scene graph.
 *
 * In exponential mode, the fog effects are always camera-relative, and it
 * does not matter where the Fog node is parented.  However, in linear mode,
 * the onset and opaque distances are defined as offsets along the local
 * forward axis (e.g.  the Y axis).  This allows the fog effect to be
 * localized to a particular region in space, rather than always camera-
 * relative.  If the fog object is not parented to any node, it is used to
 * generate traditonal camera-relative fog, as if it were parented to the
 * camera.
 */
class EXPCL_PANDA_PGRAPH Fog : public PandaNode {
PUBLISHED:
  explicit Fog(const std::string &name);

protected:
  Fog(const Fog &copy);

public:
  virtual ~Fog();

  virtual PandaNode *make_copy() const;
  virtual void xform(const LMatrix4 &mat);

PUBLISHED:
  enum Mode {
    M_linear,                   // f = (end - z) / (end - start)
    M_exponential,              // f = e^(-density * z)
    M_exponential_squared       // f = e^((-density * z)^2)
  };

  INLINE Mode get_mode() const;
  INLINE void set_mode(Mode mode);
  MAKE_PROPERTY(mode, get_mode, set_mode);

  INLINE const LColor &get_color() const;
  INLINE void set_color(PN_stdfloat r, PN_stdfloat g, PN_stdfloat b);
  INLINE void set_color(const LColor &color);
  MAKE_PROPERTY(color, get_color, set_color);

  INLINE void set_linear_range(PN_stdfloat onset, PN_stdfloat opaque);

  INLINE const LPoint3 &get_linear_onset_point() const;
  INLINE void set_linear_onset_point(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  INLINE void set_linear_onset_point(const LPoint3 &linear_onset_point);
  MAKE_PROPERTY(linear_onset_point, get_linear_onset_point, set_linear_onset_point);

  INLINE const LPoint3 &get_linear_opaque_point() const;
  INLINE void set_linear_opaque_point(const LPoint3 &linear_opaque_point);
  INLINE void set_linear_opaque_point(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);
  MAKE_PROPERTY(linear_opaque_point, get_linear_opaque_point, set_linear_opaque_point);

  INLINE void set_linear_fallback(PN_stdfloat angle, PN_stdfloat onset, PN_stdfloat opaque);

  INLINE PN_stdfloat get_exp_density() const;
  INLINE void set_exp_density(PN_stdfloat exp_density);
  MAKE_PROPERTY(exp_density, get_exp_density, set_exp_density);

  void output(std::ostream &out) const;

public:
  void adjust_to_camera(const TransformState *camera_transform);
  void get_linear_range(PN_stdfloat &onset, PN_stdfloat &opaque);

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
  LColor _color;
  LPoint3 _linear_onset_point;
  LPoint3 _linear_opaque_point;
  PN_stdfloat _exp_density;

  PN_stdfloat _linear_fallback_cosa;
  PN_stdfloat _linear_fallback_onset, _linear_fallback_opaque;

  PN_stdfloat _transformed_onset, _transformed_opaque;

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

EXPCL_PANDA_PGRAPH std::ostream &operator << (std::ostream &out, Fog::Mode mode);

INLINE std::ostream &operator << (std::ostream &out, const Fog &fog) {
  fog.output(out);
  return out;
}

#include "fog.I"

#endif
