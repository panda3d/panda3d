// Filename: onOffAttribute.h
// Created by:  drose (20Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef ONOFFATTRIBUTE_H
#define ONOFFATTRIBUTE_H

#include <pandabase.h>

#include "nodeAttribute.h"

class OnOffTransition;

////////////////////////////////////////////////////////////////////
// 	 Class : OnOffAttribute
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA OnOffAttribute : public NodeAttribute {
protected:
  INLINE_GRAPH OnOffAttribute(bool is_on = false);
  INLINE_GRAPH OnOffAttribute(const OnOffAttribute &copy);
  INLINE_GRAPH void operator = (const OnOffAttribute &copy);

PUBLISHED:
  INLINE_GRAPH void set_on();
  INLINE_GRAPH void set_off();

  INLINE_GRAPH bool is_on() const;
  INLINE_GRAPH bool is_off() const;

public:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int internal_compare_to(const NodeAttribute *other) const;

  virtual void set_value_from(const OnOffTransition *other);
  virtual int compare_values(const OnOffAttribute *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

private:
  bool _is_on;

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
    register_type(_type_handle, "OnOffAttribute",
		  NodeAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
friend class OnOffTransition;
};

#ifdef BUILDING_PANDA
#include "onOffAttribute.I"
#endif

#endif
