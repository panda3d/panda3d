// Filename: transformTransition.h
// Created by:  drose (24Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TRANSFORMTRANSITION_H
#define TRANSFORMTRANSITION_H

#include <pandabase.h>

#include <lmatrix4fTransition.h>
#include <lmatrix.h>

////////////////////////////////////////////////////////////////////
// 	 Class : TransformTransition
// Description : This defines a new coordinate system.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TransformTransition : public LMatrix4fTransition {
public:
  INLINE TransformTransition();
  INLINE TransformTransition(const LMatrix4f &matrix);

  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

protected:
  virtual MatrixTransition<LMatrix4f> *
  make_with_matrix(const LMatrix4f &matrix) const;

public:
  static void register_with_read_factory(void);

  static TypedWritable *make_TransformTransition(const FactoryParams &params);

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
    register_type(_type_handle, "TransformTransition",
		  LMatrix4fTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class TransformAttribute;
};

#include "transformTransition.I"

#endif


