// Filename: intDataAttribute.h
// Created by:  drose (27Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef INTDATAATTRIBUTE_H
#define INTDATAATTRIBUTE_H

#include <pandabase.h>

#include "numericDataAttribute.h"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, NumericDataAttribute<int>);

////////////////////////////////////////////////////////////////////
// 	 Class : IntDataAttribute
// Description : A NumericDataAttribute templated on integer types.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA IntDataAttribute :
  public NumericDataAttribute<int> {
public:
  INLINE IntDataAttribute();
  INLINE IntDataAttribute(int value);

  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NumericDataAttribute<int>::init_type();
    register_type(_type_handle, "IntDataAttribute", 
		  NumericDataAttribute<int>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "intDataAttribute.I"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
