// Filename: colorMatrixTransition.h
// Created by:  jason (01Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef COLOR_MATRIX_TRANSITION_H
#define COLOR_MATRIX_TRANSITION_H

#include <pandabase.h>

#include <lmatrix4fTransition.h>
#include <lmatrix.h>

////////////////////////////////////////////////////////////////////
// 	 Class : ColorMatrixTransition
// Description : This defines a transformation for color. I.E. to
//               make something blue, or rotate the colors, or whatever
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorMatrixTransition : public LMatrix4fTransition {
public:
  INLINE ColorMatrixTransition();
  INLINE ColorMatrixTransition(const LMatrix4f &matrix);

  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

protected:
  virtual MatrixTransition<LMatrix4f> *
  make_with_matrix(const LMatrix4f &matrix) const;

public:
  static void register_with_read_factory(void);

  static TypedWriteable *make_ColorMatrixTransition(const FactoryParams &params);

protected:

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    LMatrix4fTransition::init_type();
    register_type(_type_handle, "ColorMatrixTransition",
		  LMatrix4fTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class TransformAttribute;
};

#include "colorMatrixTransition.I"

#endif
