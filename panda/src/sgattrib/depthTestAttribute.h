// Filename: depthTestAttribute.h
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DEPTHTESTATTRIBUTE_H
#define DEPTHTESTATTRIBUTE_H

#include <pandabase.h>

#include "depthTestProperty.h"

#include <onAttribute.h>

////////////////////////////////////////////////////////////////////
// 	 Class : DepthTestAttribute
// Description : See DepthTestTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DepthTestAttribute : public OnAttribute {
public:
  INLINE DepthTestAttribute();

  INLINE void set_mode(DepthTestProperty::Mode);
  INLINE DepthTestProperty::Mode get_mode() const;

  virtual TypeHandle get_handle() const;
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnAttribute *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  DepthTestProperty _value;

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
    register_type(_type_handle, "DepthTestAttribute",
		  OnAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "depthTestAttribute.I"

#endif
