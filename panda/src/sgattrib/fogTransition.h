// Filename: fogTransition.h
// Created by:  mike (19Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef FOGTRANSITION_H
#define FOGTRANSITION_H

#include <pandabase.h>

#include <onOffTransition.h>
#include <fog.h>

////////////////////////////////////////////////////////////////////
// 	 Class : FogTransition
// Description : This controls the enabling of fog.  This typically
//               attenuates the colors based on the distance from the
//               observer (e.g. the z depth).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA FogTransition : public OnOffTransition {
public:
  INLINE FogTransition();
  INLINE FogTransition(Fog *fog);
  INLINE static FogTransition off();

  INLINE void set_on(Fog *fog);
  INLINE Fog *get_fog() const;

public:  
  virtual NodeTransition *make_copy() const;
  virtual NodeAttribute *make_attrib() const;

protected:
  virtual void set_value_from(const OnOffTransition *other);
  virtual int compare_values(const OnOffTransition *other) const;
  virtual void output_value(ostream &out) const;
  virtual void write_value(ostream &out, int indent_level) const;

  PT(Fog) _value;

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
    register_type(_type_handle, "FogTransition",
		  OnOffTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class FogAttribute;
};

#include "fogTransition.I"

#endif
