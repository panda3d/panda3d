// Filename: texGenAttribute.h
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TEXGENATTRIBUTE_H
#define TEXGENATTRIBUTE_H

#include <pandabase.h>

#include "texGenProperty.h"

#include <onAttribute.h>

////////////////////////////////////////////////////////////////////
// 	 Class : TexGenAttribute
// Description : See TexGenTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TexGenAttribute : public OnAttribute {
public:
  INLINE TexGenAttribute();

  INLINE void set_none();
  INLINE void set_texture_projector();
  INLINE void set_sphere_map();

  INLINE TexGenProperty::Mode get_mode() const;
  INLINE LMatrix4f get_plane() const;

  virtual TypeHandle get_handle() const;
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnAttribute *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  TexGenProperty _value;

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
    register_type(_type_handle, "TexGenAttribute",
		  OnAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "texGenAttribute.I"

#endif
