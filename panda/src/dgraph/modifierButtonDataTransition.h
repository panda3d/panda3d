// Filename: modifierButtonDataTransition.h
// Created by:  drose (01Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef MODIFIERBUTTONDATATRANSITION_H
#define MODIFIERBUTTONDATATRANSITION_H

#include <pandabase.h>

#include <nodeTransition.h>

////////////////////////////////////////////////////////////////////
// 	 Class : ModifierButtonDataTransition
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ModifierButtonDataTransition : public NodeTransition {
public:
  INLINE ModifierButtonDataTransition();
  INLINE ModifierButtonDataTransition(const ModifierButtonDataTransition &copy);
  INLINE void operator = (const ModifierButtonDataTransition &copy);

public:
  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

  virtual NodeTransition *compose(const NodeTransition *other) const;
  virtual NodeTransition *invert() const;
  virtual NodeAttribute *apply(const NodeAttribute *attrib) const;

protected:
  virtual int internal_compare_to(const NodeTransition *other) const;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NodeTransition::init_type();
    register_type(_type_handle, "ModifierButtonDataTransition",
		  NodeTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "modifierButtonDataTransition.I"

#endif
