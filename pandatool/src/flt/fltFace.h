// Filename: fltFace.h
// Created by:  drose (25Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef FLTFACE_H
#define FLTFACE_H

#include <pandatoolbase.h>

#include "fltGeometry.h"

////////////////////////////////////////////////////////////////////
//       Class : FltFace
// Description : A single face bead, e.g. a polygon.
////////////////////////////////////////////////////////////////////
class FltFace : public FltGeometry {
public:
  FltFace(FltHeader *header);


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
    FltGeometry::init_type();
    register_type(_type_handle, "FltFace",
                  FltGeometry::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "fltFace.I"

#endif


