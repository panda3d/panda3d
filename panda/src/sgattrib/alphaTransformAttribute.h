// Filename: alphaTransformAttribute.h
// Created by:  jason (01Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef ALPHA_TRANSFORM_TRANSITION_H
#define ALPHA_TRANSFORM_TRANSITION_H

#include <pandabase.h>

#include "alphaTransformProperty.h"

#include <onAttribute.h>

////////////////////////////////////////////////////////////////////
// 	 Class : AlphaTransformAttribute
// Description : See AlphaTransformTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AlphaTransformAttribute : public OnAttribute {
public:
  INLINE AlphaTransformAttribute();

  INLINE void set_offset(float offset);
  INLINE float get_offset() const;
  INLINE void set_scale(float scale);
  INLINE float get_scale() const;

  virtual TypeHandle get_handle() const;
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnAttribute *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  AlphaTransformProperty _state;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OnAttribute::init_type();
    register_type(_type_handle, "AlphaTransformAttribute",
		  OnAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "alphaTransformAttribute.I"

#endif
