// Filename: finiteBoundingVolume.h
// Created by:  drose (02Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef FINITEBOUNDINGVOLUME_H
#define FINITEBOUNDINGVOLUME_H

#include <pandabase.h>

#include "geometricBoundingVolume.h"


///////////////////////////////////////////////////////////////////
// 	 Class : FiniteBoundingVolume
// Description : A special kind of GeometricBoundingVolume that is
//               known to be finite.  It is possible to query this
//               kind of volume for its minimum and maximum extents.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FiniteBoundingVolume : public GeometricBoundingVolume {
public:
  virtual LPoint3f get_min() const=0;
  virtual LPoint3f get_max() const=0;

 
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeometricBoundingVolume::init_type();
    register_type(_type_handle, "FiniteBoundingVolume",
		  GeometricBoundingVolume::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif



















