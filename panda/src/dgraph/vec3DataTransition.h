// Filename: vec3DataTransition.h
// Created by:  drose (27Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VEC3DATATRANSITION_H
#define VEC3DATATRANSITION_H

#include <pandabase.h>

#include "vectorDataTransition.h"

#include <luse.h>

// We need to define this temporary macro so we can pass a parameter
// containing a comma through the macro.
#define VECTORDATATRANSITION_LPOINT3F VectorDataTransition<LPoint3f, LMatrix4f>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, VECTORDATATRANSITION_LPOINT3F);

////////////////////////////////////////////////////////////////////
// 	 Class : Vec3DataTransition
// Description : A VectorDataTransition templated on LPoint3f.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Vec3DataTransition :
  public VectorDataTransition<LPoint3f, LMatrix4f> {
public:
  INLINE Vec3DataTransition();
  INLINE Vec3DataTransition(const LMatrix4f &matrix);

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
    VectorDataTransition<LPoint3f, LMatrix4f>::init_type();
    register_type(_type_handle, "Vec3DataTransition", 
		  VectorDataTransition<LPoint3f, LMatrix4f>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "vec3DataTransition.I"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
