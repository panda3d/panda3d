// Filename: textureAttribute.h
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TEXTUREATTRIBUTE_H
#define TEXTUREATTRIBUTE_H

#include <pandabase.h>

#include <onOffAttribute.h>
#include <texture.h>

////////////////////////////////////////////////////////////////////
// 	 Class : TextureAttribute
// Description : See TextureTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TextureAttribute : public OnOffAttribute {
public:
  INLINE TextureAttribute();

  INLINE void set_on(Texture *texture);
  INLINE PT(Texture) get_texture() const;

  virtual TypeHandle get_handle() const;
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

protected:
  virtual void set_value_from(const OnOffTransition *other);
  virtual int compare_values(const OnOffAttribute *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  PT(Texture) _value;

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
    register_type(_type_handle, "TextureAttribute",
		  OnOffAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "textureAttribute.I"

#endif
