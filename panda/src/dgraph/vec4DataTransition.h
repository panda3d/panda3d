// Filename: vec4DataTransition.h
// Created by:  jason (03Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef VEC4DATATRANSITION_H
#define VEC4DATATRANSITION_H

#include <pandabase.h>

#include "vectorDataTransition.h"

#include <luse.h>

// We need to define this temporary macro so we can pass a parameter
// containing a comma through the macro.
#define VECTORDATATRANSITION_LPOINT4F VectorDataTransition<LPoint4f, LMatrix4f>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, VECTORDATATRANSITION_LPOINT4F);

////////////////////////////////////////////////////////////////////
//       Class : Vec4DataTransition
// Description : A VectorDataTransition templated on LPoint4f.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA Vec4DataTransition :
  public VectorDataTransition<LPoint4f, LMatrix4f> {
public:
  INLINE Vec4DataTransition();
  INLINE Vec4DataTransition(const LMatrix4f &matrix);

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
    VectorDataTransition<LPoint4f, LMatrix4f>::init_type();
    register_type(_type_handle, "Vec4DataTransition", 
                  VectorDataTransition<LPoint4f, LMatrix4f>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "vec4DataTransition.I"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
