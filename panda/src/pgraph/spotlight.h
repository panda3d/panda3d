// Filename: spotlight.h
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

#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "pandabase.h"

#include "lightLensNode.h"

class Texture;

////////////////////////////////////////////////////////////////////
//       Class : Spotlight
// Description : A light originating from a single point in space, and
//               shining in a particular direction, with a cone-shaped
//               falloff.
//
//               The Spotlight frustum is defined using a Lens, so it
//               can have any of the properties that a camera lens can
//               have.
//
//               Note that the class is named Spotlight instead of
//               SpotLight, because "spotlight" is a single English
//               word, instead of two words.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Spotlight : public LightLensNode {
PUBLISHED:
  Spotlight(const string &name);

protected:
  Spotlight(const Spotlight &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual void xform(const LMatrix4f &mat);
  virtual void write(ostream &out, int indent_level) const;

PUBLISHED:
  INLINE float get_exponent() const;
  INLINE void set_exponent(float exponent);
  
  INLINE const Colorf &get_specular_color() const;
  INLINE void set_specular_color(const Colorf &color);
  
  INLINE const LVecBase3f &get_attenuation() const;
  INLINE void set_attenuation(const LVecBase3f &attenuation);
  
public:
  virtual void bind(GraphicsStateGuardianBase *gsg, int light_id);

  bool make_image(Texture *texture, float radius);

protected:
  virtual void fill_viz_geom(GeomNode *viz_geom);

private:
  CPT(RenderState) get_viz_state();

private:
  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);

    float _exponent;
    Colorf _specular_color;
    LVecBase3f _attenuation;
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
    register_type(_type_handle, "Spotlight",
                  LightLensNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const Spotlight &light) {
  light.output(out);
  return out;
}

#include "spotlight.I"

#endif
