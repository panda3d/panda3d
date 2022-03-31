/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file spotlight.h
 * @author mike
 * @date 1997-01-09
 */

#ifndef SPOTLIGHT_H
#define SPOTLIGHT_H

#include "pandabase.h"

#include "lightLensNode.h"
#include "texture.h"

/**
 * A light originating from a single point in space, and shining in a
 * particular direction, with a cone-shaped falloff.
 *
 * The Spotlight frustum is defined using a Lens, so it can have any of the
 * properties that a camera lens can have.
 *
 * Note that the class is named Spotlight instead of SpotLight, because
 * "spotlight" is a single English word, instead of two words.
 */
class EXPCL_PANDA_PGRAPHNODES Spotlight : public LightLensNode {
PUBLISHED:
  Spotlight(const std::string &name);

protected:
  Spotlight(const Spotlight &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual void xform(const LMatrix4 &mat);
  virtual void write(std::ostream &out, int indent_level) const;

  virtual bool get_vector_to_light(LVector3 &result,
                                   const LPoint3 &from_object_point,
                                   const LMatrix4 &to_object_space);

PUBLISHED:
  INLINE PN_stdfloat get_exponent() const final;
  INLINE void set_exponent(PN_stdfloat exponent);
  MAKE_PROPERTY(exponent, get_exponent, set_exponent);

  INLINE const LColor &get_specular_color() const final;
  INLINE void set_specular_color(const LColor &color);
  INLINE void clear_specular_color();
  MAKE_PROPERTY(specular_color, get_specular_color, set_specular_color);

  INLINE const LVecBase3 &get_attenuation() const final;
  INLINE void set_attenuation(const LVecBase3 &attenuation);
  MAKE_PROPERTY(attenuation, get_attenuation, set_attenuation);

  INLINE PN_stdfloat get_max_distance() const;
  INLINE void set_max_distance(PN_stdfloat max_distance);
  MAKE_PROPERTY(max_distance, get_max_distance, set_max_distance);

  virtual int get_class_priority() const;

  static PT(Texture) make_spot(int pixel_width, PN_stdfloat full_radius,
                               LColor &fg, LColor &bg);

public:
  virtual void bind(GraphicsStateGuardianBase *gsg, const NodePath &light,
                    int light_id);

protected:
  virtual void fill_viz_geom(GeomNode *viz_geom);

private:
  CPT(RenderState) get_viz_state();

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
      return Spotlight::get_class_type();
    }

    PN_stdfloat _exponent;
    LColor _specular_color;
    LVecBase3 _attenuation;
    PN_stdfloat _max_distance;
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

INLINE std::ostream &operator << (std::ostream &out, const Spotlight &light) {
  light.output(out);
  return out;
}

#include "spotlight.I"

#endif
