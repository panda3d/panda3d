// Filename: polygonOffsetTransition.h
// Created by:  jason (12Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef POLYGONOFFSETTRANSITION_H
#define POLYGONOFFSETTRANSITION_H

#include <pandabase.h>

#include "polygonOffsetProperty.h"

#include <onTransition.h>

////////////////////////////////////////////////////////////////////
// 	 Class : PolygonOffsetTransition
// Description : This controls the amount of offseting done for depth
//               tests.  The first value is the units, this is the 
//               fixed offseting to be used, and the second value 
//               allows for variable offsetting per polygon based on 
//               the ratio of depth relative to screen area of the polygon
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PolygonOffsetTransition : public OnTransition {
public:
  INLINE PolygonOffsetTransition(int amount = 0, int factor = 0);

  INLINE void set_units(int units);
  INLINE int get_units() const;
  INLINE void set_factor(int factor);
  INLINE int get_factor() const;

  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

protected:
  virtual void set_value_from(const OnTransition *other);
  virtual int compare_values(const OnTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  PolygonOffsetProperty _state;

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
    register_type(_type_handle, "PolygonOffsetTransition",
		  OnTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class PolygonOffsetAttribute;
};


#include "polygonOffsetTransition.I"

#endif
