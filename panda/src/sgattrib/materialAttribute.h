// Filename: materialAttribute.h
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef MATERIALATTRIBUTE_H
#define MATERIALATTRIBUTE_H

#include <pandabase.h>

#include <onOffAttribute.h>
#include <material.h>

////////////////////////////////////////////////////////////////////
// 	 Class : MaterialAttribute
// Description : See MaterialTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA MaterialAttribute : public OnOffAttribute {
public:
  INLINE MaterialAttribute();

  INLINE void set_on(Material *material);
  INLINE PT(Material) get_material() const;

  virtual TypeHandle get_handle() const;
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnOffTransition *other);
  virtual int compare_values(const OnOffAttribute *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  PT(Material) _value;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OnOffAttribute::init_type();
    register_type(_type_handle, "MaterialAttribute",
		  OnOffAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "materialAttribute.I"

#endif
