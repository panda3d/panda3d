// Filename: geomBinAttribute.h
// Created by:  drose (07Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GEOMBINATTRIBUTE_H
#define GEOMBINATTRIBUTE_H

#include <pandabase.h>

#include "geomBin.h"

#include <onOffAttribute.h>

////////////////////////////////////////////////////////////////////
// 	 Class : GeomBinAttribute
// Description : See GeomBinTransition.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomBinAttribute : public OnOffAttribute {
public:
  INLINE GeomBinAttribute();
  INLINE GeomBinAttribute(const string &bin, int draw_order = 0);

  INLINE void set_on(const string &bin, int draw_order = 0);
  INLINE const string &get_bin() const;
  INLINE int get_draw_order() const;

  virtual TypeHandle get_handle() const;
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;

protected:
  virtual void set_value_from(const OnOffTransition *other);
  virtual int compare_values(const OnOffAttribute *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  string _value;
  int _draw_order;

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
    register_type(_type_handle, "GeomBinAttribute",
		  OnOffAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "geomBinAttribute.I"

#endif
