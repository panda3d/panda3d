// Filename: fltLightSourceDefinition.h
// Created by:  drose (26Aug00)
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

#ifndef FLTLIGHTSOURCEDEFINITION_H
#define FLTLIGHTSOURCEDEFINITION_H

#include "pandatoolbase.h"

#include "fltRecord.h"

#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : FltLightSourceDefinition
// Description : Represents a single entry in the light source
//               palette.  This completely defines the color, etc. of
//               a single light source, which may be referenced later
//               by a FltLightSource bead in the hierarchy.
////////////////////////////////////////////////////////////////////
class FltLightSourceDefinition : public FltRecord {
public:
  FltLightSourceDefinition(FltHeader *header);

  enum LightType {
    LT_infinite = 0,
    LT_local    = 1,
    LT_spot     = 2
  };

  int _light_index;
  string _light_name;
  Colorf _ambient;
  Colorf _diffuse;
  Colorf _specular;
  LightType _light_type;
  float _exponential_dropoff;
  float _cutoff_angle;  // in degrees

  // yaw and pitch only for modeling lights, which are positioned at
  // the eyepoint.
  float _yaw;
  float _pitch;

  float _constant_coefficient;
  float _linear_coefficient;
  float _quadratic_coefficient;
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


