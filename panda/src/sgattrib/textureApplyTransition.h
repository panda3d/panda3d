// Filename: textureApplyTransition.h
// Created by:  drose (06Oct99)
//
////////////////////////////////////////////////////////////////////

#ifndef TEXTUREAPPLYTRANSITION_H
#define TEXTUREAPPLYTRANSITION_H

#include <pandabase.h>

#include "textureApplyProperty.h"

#include <onTransition.h>

////////////////////////////////////////////////////////////////////
// 	 Class : TextureApplyTransition
// Description : This controls the way textures modify the colors
//               assigned to geometry.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TextureApplyTransition : public OnTransition {
public:
  INLINE TextureApplyTransition(TextureApplyProperty::Mode mode);

  INLINE void set_mode(TextureApplyProperty::Mode mode);
  INLINE TextureApplyProperty::Mode get_mode() const;
  
  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

protected:
  INLINE TextureApplyTransition(void);
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  TextureApplyProperty _value;

public:
  static void register_with_read_factory(void);
  virtual void write_datagram(BamWriter* manager, Datagram &me);  

  static TypedWritable *make_TextureApplyTransition(const FactoryParams &params);

protected:
  void fillin(DatagramIterator& scan, BamReader* manager);

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
    register_type(_type_handle, "TextureApplyTransition",
		  OnTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class TextureApplyAttribute;
};

#include "textureApplyTransition.I"

#endif



