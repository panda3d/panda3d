// Filename: texGenTransition.h
// Created by:  mike (18Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef TEXGENTRANSITION_H
#define TEXGENTRANSITION_H

#include <pandabase.h>

#include "texGenProperty.h"

#include <onTransition.h>

////////////////////////////////////////////////////////////////////
//       Class : TexGenTransition
// Description : This controls the kinds of blending between colors
//               being rendered and the existing frame buffer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TexGenTransition : public OnTransition {
public:
  INLINE TexGenTransition();
  INLINE static TexGenTransition texture_projector();
  INLINE static TexGenTransition sphere_map();

  INLINE void set_none();
  INLINE void set_texture_projector();
  INLINE void set_sphere_map();

  INLINE TexGenProperty::Mode get_mode() const;
  INLINE LMatrix4f get_plane() const;
  
  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnTransition *other) const;
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
    OnTransition::init_type();
    register_type(_type_handle, "TexGenTransition",
                  OnTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class TexGenAttribute;
};

#include "texGenTransition.I"

#endif
