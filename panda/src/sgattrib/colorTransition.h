// Filename: colorTransition.h
// Created by:  drose (18Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef COLORTRANSITION_H
#define COLORTRANSITION_H

#include <pandabase.h>

#include "colorProperty.h"

#include <onOffTransition.h>

////////////////////////////////////////////////////////////////////
// 	 Class : ColorTransition
// Description : This is scene graph color (which is not to be
//               confused with geometry color).  By setting a
//               ColorTransition on the scene graph, you can override
//               the color at the object level, but not at the
//               individual primitive or vertex level.  If a scene
//               graph color is set, it overrides the primitive color;
//               otherwise, the primitive color shows through.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorTransition : public OnOffTransition {
public:
  INLINE ColorTransition();
  INLINE ColorTransition(const Colorf &color);
  INLINE ColorTransition(float r, float g, float b, float a);
  INLINE static ColorTransition uncolor();
  INLINE static ColorTransition off();

  INLINE void set_on(const Colorf &color);
  INLINE void set_on(float r, float g, float b, float a);
  INLINE void set_uncolor();

  INLINE bool is_real() const;
  INLINE Colorf get_color() const;
  
  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

protected:
  virtual void set_value_from(const OnOffTransition *other);
  virtual int compare_values(const OnOffTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  ColorProperty _value;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    OnOffTransition::init_type();
    register_type(_type_handle, "ColorTransition",
		  OnOffTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class ColorAttribute;
};

#include "colorTransition.I"

#endif
