// Filename: colorBlendTransition.h
// Created by:  mike (18Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef COLORBLENDTRANSITION_H
#define COLORBLENDTRANSITION_H

#include <pandabase.h>

#include "colorBlendProperty.h"

#include <onTransition.h>

////////////////////////////////////////////////////////////////////
//       Class : ColorBlendTransition
// Description : This controls the kinds of blending between colors
//               being rendered and the existing frame buffer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorBlendTransition : public OnTransition {
public:
  INLINE ColorBlendTransition(ColorBlendProperty::Mode mode);

  INLINE void set_mode(ColorBlendProperty::Mode mode);
  INLINE ColorBlendProperty::Mode get_mode() const;
  
  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  ColorBlendProperty _value;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OnTransition::init_type();
    register_type(_type_handle, "ColorBlendTransition",
                  OnTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class ColorBlendAttribute;
};

#include "colorBlendTransition.I"

#endif
