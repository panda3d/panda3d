// Filename: doublePtrDataTransition.h
// Created by:  jason (07Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DOUBLEPTRDATATRANSITION_H
#define DOUBLEPTRDATATRANSITION_H

#include <pandabase.h>

#include "pointerDataTransition.h"

EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, PointerDataTransition<double>);

////////////////////////////////////////////////////////////////////
// 	 Class : DoublePtrDataTransition
// Description : A PointerDataTransition templated on double types.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DoublePtrDataTransition :
  public PointerDataTransition<double> {
public:
  INLINE DoublePtrDataTransition();
  INLINE DoublePtrDataTransition(double* ptr);

  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PointerDataTransition<double>::init_type();
    register_type(_type_handle, "DoublePtrDataTransition", 
		  PointerDataTransition<double>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "doublePtrDataTransition.I"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
