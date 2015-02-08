// Filename: pointLight.h
// Created by:  mike (09Jan97)
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

#ifndef POINTLIGHT_H
#define POINTLIGHT_H

#include "pandabase.h"

#include "lightLensNode.h"

////////////////////////////////////////////////////////////////////
//       Class : PointLight
// Description : A light originating from a single point in space, and
//               shining in all directions.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPHNODES PointLight : public LightLensNode {
PUBLISHED:
  PointLight(const string &name);

protected:
  PointLight(const PointLight &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual void xform(const LMatrix4 &mat);
  virtual void write(ostream &out, int indent_level) const;

  virtual bool get_vector_to_light(LVector3 &result,
                                   const LPoint3 &from_object_point,
                                   const LMatrix4 &to_object_space);

PUBLISHED:
  INLINE const LColor &get_specular_color() const FINAL;
  INLINE void set_specular_color(const LColor &color);

  INLINE const LVecBase3 &get_attenuation() const FINAL;
  INLINE void set_attenuation(const LVecBase3 &attenuation);

  INLINE const LPoint3 &get_point() const;
  INLINE void set_point(const LPoint3 &point);

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
      return PointLight::get_class_type();
    }

    LColor _specular_color;
    LVecBase3 _attenuation;
    LPoint3 _point;
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
    register_type(_type_handle, "PointLight",
                  LightLensNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const PointLight &light) {
  light.output(out);
  return out;
}

#include "pointLight.I"

#endif
