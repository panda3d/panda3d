// Filename: transformAttribute.h
// Created by:  drose (24Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TRANSFORMATTRIBUTE_H
#define TRANSFORMATTRIBUTE_H

#include <pandabase.h>

#include <lmatrix4fTransition.h>
#include <lmatrix.h>

////////////////////////////////////////////////////////////////////
//       Class : TransformAttribute
// Description : See TransformTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TransformAttribute : public LMatrix4fAttribute {
public:
  INLINE TransformAttribute();

  virtual TypeHandle get_handle() const;
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LMatrix4fAttribute::init_type();
    register_type(_type_handle, "TransformAttribute",
                  LMatrix4fAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "transformAttribute.I"

#endif
