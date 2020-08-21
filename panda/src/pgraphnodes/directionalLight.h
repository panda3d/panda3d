/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file directionalLight.h
 * @author mike
 * @date 1997-01-09
 */

#ifndef DIRECTIONALLIGHT_H
#define DIRECTIONALLIGHT_H

#include "pandabase.h"

#include "lightLensNode.h"

/**
 * A light shining from infinitely far away in a particular direction, like
 * sunlight.
 */
class EXPCL_PANDA_PGRAPHNODES DirectionalLight : public LightLensNode {
PUBLISHED:
  explicit DirectionalLight(const std::string &name);

protected:
  DirectionalLight(const DirectionalLight &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual void xform(const LMatrix4 &mat);
  virtual void write(std::ostream &out, int indent_level) const;

  virtual bool get_vector_to_light(LVector3 &result,
                                   const LPoint3 &from_object_point,
                                   const LMatrix4 &to_object_space);

PUBLISHED:
  INLINE const LColor &get_specular_color() const final;
  INLINE void set_specular_color(const LColor &color);
  INLINE void clear_specular_color();
  MAKE_PROPERTY(specular_color, get_specular_color, set_specular_color);

  INLINE const LPoint3 &get_point() const;
  INLINE void set_point(const LPoint3 &point);
  MAKE_PROPERTY(point, get_point, set_point);

  INLINE const LVector3 &get_direction() const;
  INLINE void set_direction(const LVector3 &direction);
  MAKE_PROPERTY(direction, get_direction, set_direction);

  virtual int get_class_priority() const;

public:
  virtual void bind(GraphicsStateGuardianBase *gsg, const NodePath &light,
                    int light_id);

private:
  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA_PGRAPHNODES CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return DirectionalLight::get_class_type();
    }

    LColor _specular_color;
    LPoint3 _point;
    LVector3 _direction;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

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
    LightLensNode::init_type();
    register_type(_type_handle, "DirectionalLight",
                  LightLensNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const DirectionalLight &light) {
  light.output(out);
  return out;
}

#include "directionalLight.I"

#endif
