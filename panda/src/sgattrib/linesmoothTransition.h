// Filename: linesmoothTransition.h
// Created by:  mike (08Feb99)
//
////////////////////////////////////////////////////////////////////

#ifndef LINESMOOTHTRANSITION_H
#define LINESMOOTHTRANSITION_H

#include <pandabase.h>

#include <onOffTransition.h>

////////////////////////////////////////////////////////////////////
// 	 Class : LinesmoothTransition
// Description : This enables or disables the antialiasing of lines.
//               It has no additional properties; it's simply an
//               on-or-off thing.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LinesmoothTransition : public OnOffTransition {
public:
  INLINE LinesmoothTransition();
  INLINE static LinesmoothTransition off();
  
  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

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
    register_type(_type_handle, "LinesmoothTransition",
		  OnOffTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class LinesmoothAttribute;
};

#include "linesmoothTransition.I"

#endif
