/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rectangleLight.h
 * @author rdb
 * @date 2016-12-19
 */

#ifndef RECTANGLELIGHT_H
#define RECTANGLELIGHT_H

#include "pandabase.h"

#include "lightLensNode.h"
#include "pointLight.h"

/**
 * This is a type of area light that is an axis aligned rectangle, pointing
 * along the Y axis in the positive direction.
 *
 * @since 1.10.0
 */
class EXPCL_PANDA_PGRAPHNODES RectangleLight : public LightLensNode {
PUBLISHED:
  explicit RectangleLight(const std::string &name);

protected:
  RectangleLight(const RectangleLight &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual void write(std::ostream &out, int indent_level) const;

PUBLISHED:
  INLINE const LColor &get_specular_color() const final;

  INLINE PN_stdfloat get_max_distance() const;
  INLINE void set_max_distance(PN_stdfloat max_distance);
  MAKE_PROPERTY(max_distance, get_max_distance, set_max_distance);

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
      return RectangleLight::get_class_type();
    }

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
    register_type(_type_handle, "RectangleLight",
                  LightLensNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "rectangleLight.I"

#endif
