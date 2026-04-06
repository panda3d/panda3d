/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltLightSourceDefinition.h
 * @author drose
 * @date 2000-08-26
 */

#ifndef FLTLIGHTSOURCEDEFINITION_H
#define FLTLIGHTSOURCEDEFINITION_H

#include "pandatoolbase.h"

#include "fltRecord.h"

#include "luse.h"

/**
 * Represents a single entry in the light source palette.  This completely
 * defines the color, etc.  of a single light source, which may be referenced
 * later by a FltLightSource bead in the hierarchy.
 */
class FltLightSourceDefinition : public FltRecord {
public:
  FltLightSourceDefinition(FltHeader *header);

  enum LightType {
    LT_infinite = 0,
    LT_local    = 1,
    LT_spot     = 2
  };

  int _light_index;
  std::string _light_name;
  LColor _ambient;
  LColor _diffuse;
  LColor _specular;
  LightType _light_type;
  PN_stdfloat _exponential_dropoff;
  PN_stdfloat _cutoff_angle;  // in degrees

  // yaw and pitch only for modeling lights, which are positioned at the
  // eyepoint.
  PN_stdfloat _yaw;
  PN_stdfloat _pitch;

  PN_stdfloat _constant_coefficient;
  PN_stdfloat _linear_coefficient;
  PN_stdfloat _quadratic_coefficient;
  bool _modeling_light;

protected:
  virtual bool extract_record(FltRecordReader &reader);
  virtual bool build_record(FltRecordWriter &writer) const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    FltRecord::init_type();
    register_type(_type_handle, "FltLightSourceDefinition",
                  FltRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class FltHeader;
};

#endif
