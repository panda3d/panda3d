// Filename: light.h
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

#ifndef LIGHT_H
#define LIGHT_H

#include "pandabase.h"

#include "referenceCount.h"
#include "luse.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataLockedReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"
#include "updateSeq.h"
#include "geomNode.h"

class NodePath;
class PandaNode;
class GraphicsStateGuardianBase;

////////////////////////////////////////////////////////////////////
//       Class : Light
// Description : The abstract interface to all kinds of lights.  The
//               actual light objects also inherit from PandaNode, and
//               can therefore be added to the scene graph at some
//               arbitrary point to define the coordinate system of
//               effect.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH Light {
PUBLISHED:
  INLINE Light();
  INLINE Light(const Light &copy);
  virtual ~Light();

  virtual PandaNode *as_node()=0;
  virtual bool is_ambient_light() const;

  INLINE const LColor &get_color() const;
  INLINE void set_color(const LColor &color);

  virtual PN_stdfloat get_exponent() const;
  virtual const LColor &get_specular_color() const;
  virtual const LVecBase3 &get_attenuation() const;

  INLINE void set_priority(int priority);
  INLINE int get_priority() const;
  virtual int get_class_priority() const=0;

public:
  virtual void output(ostream &out) const=0;
  virtual void write(ostream &out, int indent_level) const=0;
  virtual void bind(GraphicsStateGuardianBase *gsg, const NodePath &light,
                    int light_id)=0;

  virtual bool get_vector_to_light(LVector3 &result,
                                   const LPoint3 &from_object_point,
                                   const LMatrix4 &to_object_space);

  GeomNode *get_viz();

  INLINE static UpdateSeq get_sort_seq();

protected:
  virtual void fill_viz_geom(GeomNode *viz_geom);
  INLINE void mark_viz_stale();

  // This enumerated class defines the relative class priority of
  // different kinds of lights.  This hierarchy is only used to
  // resolve multiple lights of the same priority specified by
  // set_priority().  In general, the first items in this list have a
  // lesser priority than later items.
  enum ClassPriority {
    CP_ambient_priority,
    CP_point_priority,
    CP_directional_priority,
    CP_spot_priority,
  };

private:
  // The priority is not cycled, because there's no real reason to do
  // so, and cycling it makes it difficult to synchronize with the
  // LightAttribs.
  int _priority;
  static UpdateSeq _sort_seq;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA_PGRAPH CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return Light::get_class_type();
    }

    LColor _color;

    PT(GeomNode) _viz_geom;
    bool _viz_geom_stale;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataLockedReader<CData> CDLockedReader;
  typedef CycleDataWriter<CData> CDWriter;

protected:
  void write_datagram(BamWriter *manager, Datagram &dg);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    ReferenceCount::init_type();
    register_type(_type_handle, "Light",
                  ReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const Light &light) {
  light.output(out);
  return out;
}

#include "light.I"

#endif
