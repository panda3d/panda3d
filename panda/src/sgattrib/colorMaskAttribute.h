// Filename: colorMaskAttribute.h
// Created by:  drose (23Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef COLORMASKATTRIBUTE_H
#define COLORMASKATTRIBUTE_H

#include <pandabase.h>

#include "colorMaskProperty.h"

#include <onAttribute.h>

////////////////////////////////////////////////////////////////////
// 	 Class : ColorMaskAttribute
// Description : See ColorMaskTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorMaskAttribute : public OnAttribute {
public:
  INLINE ColorMaskAttribute();

  INLINE void set_mask(int mask);
  INLINE int get_mask() const;

  INLINE bool is_write_r() const;
  INLINE bool is_write_g() const;
  INLINE bool is_write_b() const;
  INLINE bool is_write_a() const;

  virtual TypeHandle get_handle() const;
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnAttribute *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  ColorMaskProperty _value;

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
    register_type(_type_handle, "ColorMaskAttribute",
		  OnAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "colorMaskAttribute.I"

#endif
