// Filename: matrixDataTransition.h
// Created by:  drose (27Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef MATRIXDATATRANSITION_H
#define MATRIXDATATRANSITION_H

#include <pandabase.h>

#include "vectorDataTransition.h"

#include <luse.h>

// We need to define this temporary macro so we can pass a parameter
// containing a comma through the macro.
#define VECTORDATATRANSITION_LMATRIX4F VectorDataTransition<LMatrix4f, LMatrix4f>
EXPORT_TEMPLATE_CLASS(EXPCL_PANDA, EXPTP_PANDA, VECTORDATATRANSITION_LMATRIX4F);

////////////////////////////////////////////////////////////////////
//       Class : MatrixDataTransition
// Description : A VectorDataTransition templated on LMatrix4f.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MatrixDataTransition :
  public VectorDataTransition<LMatrix4f, LMatrix4f> {
public:
  INLINE MatrixDataTransition();
  INLINE MatrixDataTransition(const LMatrix4f &matrix);

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
    VectorDataTransition<LMatrix4f, LMatrix4f>::init_type();
    register_type(_type_handle, "MatrixDataTransition", 
                  VectorDataTransition<LMatrix4f, LMatrix4f>::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "matrixDataTransition.I"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma interface
#endif

#endif
