// Filename: doublePtrDataAttribute.h
// Created by:  jason (07Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DOUBLEPTRDATAATTRIBUTE_H
#define DOUBLEPTRDATAATTRIBUTE_H

#include <pandabase.h>

#include "pointerDataAttribute.h"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerDataAttribute<double>);

////////////////////////////////////////////////////////////////////
//       Class : DoublePtrDataAttribute
// Description : A PointerDataAttribute templated on double types.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DoublePtrDataAttribute :
  public PointerDataAttribute<double> {
public:
  INLINE DoublePtrDataAttribute();
  INLINE DoublePtrDataAttribute(double* ptr);

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
    PointerDataAttribute<double>::init_type();
    register_type(_type_handle, "DoublePtrDataAttribute", 
                  PointerDataAttribute<double>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "doublePtrDataAttribute.I"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
