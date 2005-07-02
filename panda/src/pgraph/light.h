// Filename: light.h
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

#ifndef LIGHT_H
#define LIGHT_H

#include "pandabase.h"

#include "referenceCount.h"
#include "luse.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"
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
class EXPCL_PANDA Light : virtual public ReferenceCount {
  // We inherit from ReferenceCount instead of TypedReferenceCount so
  // that LightNode does not inherit from TypedObject twice.  Note
  // that we also inherit virtually from ReferenceCount for the same
  // reason.
PUBLISHED:
  INLINE Light();
  INLINE Light(const Light &copy);
  virtual ~Light();

  virtual PandaNode *as_node()=0;

  INLINE const Colorf &get_color() const;
  INLINE void set_color(const Colorf &color);

public:
  virtual void output(ostream &out) const=0;
  virtual void write(ostream &out, int indent_level) const=0;
  virtual void bind(GraphicsStateGuardianBase *gsg, const NodePath &light,
                    int light_id)=0;

  virtual bool get_vector_to_light(LVector3f &result,
                                   const LPoint3f &from_object_point, 
                                   const LMatrix4f &to_object_space);

  GeomNode *get_viz();

protected:
  virtual void fill_viz_geom(GeomNode *viz_geom);
  INLINE void mark_viz_stale();

private:
  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);

    Colorf _color;

    PT(GeomNode) _viz_geom;
    bool _viz_geom_stale;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
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
