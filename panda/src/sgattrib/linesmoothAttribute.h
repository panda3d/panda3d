// Filename: linesmoothAttribute.h
// Created by:  drose (24Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LINESMOOTHATTRIBUTE_H
#define LINESMOOTHATTRIBUTE_H

#include <pandabase.h>

#include <onOffAttribute.h>

////////////////////////////////////////////////////////////////////
// 	 Class : LinesmoothAttribute
// Description : See LinesmoothTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LinesmoothAttribute : public OnOffAttribute {
public:
  INLINE LinesmoothAttribute();

  virtual TypeHandle get_handle() const;
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

  virtual void issue(GraphicsStateGuardianBase *gsgbase);

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
    register_type(_type_handle, "LinesmoothAttribute",
		  OnOffAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "linesmoothAttribute.I"

#endif
