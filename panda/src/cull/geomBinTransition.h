// Filename: geomBinTransition.h
// Created by:  drose (07Apr00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GEOMBINTRANSITION_H
#define GEOMBINTRANSITION_H

#include <pandabase.h>

#include <onOffTransition.h>
#include "geomBin.h"

////////////////////////////////////////////////////////////////////
// 	 Class : GeomBinTransition
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA GeomBinTransition : public OnOffTransition {
public:
  INLINE GeomBinTransition();
  INLINE GeomBinTransition(GeomBin *bin, int draw_order = 0);
  INLINE static GeomBinTransition off();

  INLINE void set_on(GeomBin *bin, int draw_order = 0);
  INLINE PT(GeomBin) get_bin() const;
  INLINE int get_draw_order() const;
  
  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

protected:
  virtual void set_value_from(const OnOffTransition *other);
  virtual int compare_values(const OnOffTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  PT(GeomBin) _value;
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
    OnOffTransition::init_type();
    register_type(_type_handle, "GeomBinTransition",
		  OnOffTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class GeomBinAttribute;
};

#include "geomBinTransition.I"

#endif
