/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sphereLight.h
 * @author rdb
 * @date 2016-04-15
 */

#ifndef SPHERELIGHT_H
#define SPHERELIGHT_H

#include "pandabase.h"

#include "lightLensNode.h"
#include "pointLight.h"

/**
 * A sphere light is like a point light, except that it represents a sphere
 * with a radius, rather than being an infinitely thin point in space.
 *
 * @since 1.10.0
 */
class EXPCL_PANDA_PGRAPHNODES SphereLight : public PointLight {
PUBLISHED:
  explicit SphereLight(const std::string &name);

protected:
  SphereLight(const SphereLight &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual void xform(const LMatrix4 &mat);
  virtual void write(std::ostream &out, int indent_level) const;

PUBLISHED:
  INLINE PN_stdfloat get_radius() const;
  INLINE void set_radius(PN_stdfloat radius);
  MAKE_PROPERTY(radius, get_radius, set_radius);

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
      return SphereLight::get_class_type();
    }

    PN_stdfloat _radius;
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
    PointLight::init_type();
    register_type(_type_handle, "SphereLight",
                  PointLight::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "sphereLight.I"

#endif
