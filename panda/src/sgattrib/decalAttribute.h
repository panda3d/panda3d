// Filename: decalAttribute.h
// Created by:  drose (17Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DECALATTRIBUTE_H
#define DECALATTRIBUTE_H

#include <pandabase.h>

#include <onOffAttribute.h>

////////////////////////////////////////////////////////////////////
// 	 Class : DecalAttribute
// Description : See DecalTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA DecalAttribute : public OnOffAttribute {
public:
  INLINE DecalAttribute();

  virtual TypeHandle get_handle() const;
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

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
    register_type(_type_handle, "DecalAttribute",
		  OnOffAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "decalAttribute.I"

#endif
