// Filename: renderModeTransition.h
// Created by:  drose (08Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef RENDERMODETRANSITION_H
#define RENDERMODETRANSITION_H

#include <pandabase.h>

#include "renderModeProperty.h"

#include <onTransition.h>

////////////////////////////////////////////////////////////////////
// 	 Class : RenderModeTransition
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA RenderModeTransition : public OnTransition {
public:
  INLINE RenderModeTransition(RenderModeProperty::Mode mode, 
			      double line_width = 1.0);

  INLINE void set_mode(RenderModeProperty::Mode mode);
  INLINE RenderModeProperty::Mode get_mode() const;

  INLINE void set_line_width(double line_width);
  INLINE double get_line_width() const;
  
  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  RenderModeProperty _value;

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
    register_type(_type_handle, "RenderModeTransition",
		  OnTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class RenderModeAttribute;
};

#include "renderModeTransition.I"

#endif
