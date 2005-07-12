// Filename: directionalLight.h
// Created by:  mike (09Jan97)
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

#ifndef DIRECTIONALLIGHT_H
#define DIRECTIONALLIGHT_H

#include "pandabase.h"

#include "lightNode.h"

////////////////////////////////////////////////////////////////////
//       Class : DirectionalLight
// Description : A light shining from infinitely far away in a
//               particular direction, like sunlight.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DirectionalLight : public LightNode {
PUBLISHED:
  DirectionalLight(const string &name);

protected:
  DirectionalLight(const DirectionalLight &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual void xform(const LMatrix4f &mat);
  virtual void write(ostream &out, int indent_level) const;

  virtual bool get_vector_to_light(LVector3f &result,
                                   const LPoint3f &from_object_point, 
                                   const LMatrix4f &to_object_space);

PUBLISHED:
  INLINE const Colorf &get_specular_color() const;
  INLINE void set_specular_color(const Colorf &color);
  
  INLINE const LPoint3f &get_point() const;
  INLINE void set_point(const LPoint3f &point);
  
  INLINE const LVector3f &get_direction() const;
  INLINE void set_direction(const LVector3f &direction);

  virtual int get_class_priority() const;
  
public:
  virtual void bind(GraphicsStateGuardianBase *gsg, const NodePath &light,
                    int light_id);

private:
  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);

    Colorf _specular_color;
    LPoint3f _point;
    LVector3f _direction;
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
    LightNode::init_type();
    register_type(_type_handle, "DirectionalLight",
                  LightNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const DirectionalLight &light) {
  light.output(out);
  return out;
}

#include "directionalLight.I"

#endif
