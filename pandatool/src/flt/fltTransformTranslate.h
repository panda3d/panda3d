// Filename: fltTransformTranslate.h
// Created by:  drose (30Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef FLTTRANSFORMTRANSLATE_H
#define FLTTRANSFORMTRANSLATE_H

#include <pandatoolbase.h>

#include "fltTransformRecord.h"

////////////////////////////////////////////////////////////////////
// 	 Class : FltTransformTranslate
// Description : A transformation that applies a (possibly nonuniform)
//               scale.
////////////////////////////////////////////////////////////////////
class FltTransformTranslate : public FltTransformRecord {
public:
  FltTransformTranslate(FltHeader *header);

  void set(const LPoint3d &from, const LVector3d &delta);

  const LPoint3d &get_from() const;
  const LVector3d &get_delta() const;

private:
  void recompute_matrix();

  LPoint3d _from;
  LVector3d _delta;

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
    FltTransformRecord::init_type();
    register_type(_type_handle, "FltTransformTranslate",
		  FltTransformRecord::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
