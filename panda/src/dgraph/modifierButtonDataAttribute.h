// Filename: modifierButtonDataAttribute.h
// Created by:  drose (27Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef MODIFIERBUTTONDATAATTRIBUTE_H
#define MODIFIERBUTTONDATAATTRIBUTE_H

#include <pandabase.h>

#include <nodeAttribute.h>
#include <modifierButtons.h>

class ModifierButtonDataTransition;

////////////////////////////////////////////////////////////////////
// 	 Class : ModifierButtonDataAttribute
// Description : This data graph attribute stores the current state of
//               some various modifier buttons (e.g. shift, control,
//               alt, and the mouse buttons), as generated for
//               instance by a GraphicsWindow.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ModifierButtonDataAttribute : public NodeAttribute {
public:
  INLINE ModifierButtonDataAttribute();
  INLINE ModifierButtonDataAttribute(const ModifierButtonDataAttribute &copy);
  INLINE void operator = (const ModifierButtonDataAttribute &copy);

public:
  INLINE void set_mods(const ModifierButtons &mods);
  INLINE const ModifierButtons &get_mods() const;

  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;
  
  virtual TypeHandle get_handle() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int internal_compare_to(const NodeAttribute *other) const;

private:
  ModifierButtons _mods;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NodeAttribute::init_type();
    register_type(_type_handle, "ModifierButtonDataAttribute",
		  NodeAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
friend class ModifierButtonDataTransition;
};

#include "modifierButtonDataAttribute.I"

#endif
