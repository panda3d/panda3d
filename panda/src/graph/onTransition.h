// Filename: onTransition.h
// Created by:  drose (22Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef ONTRANSITION_H
#define ONTRANSITION_H

#include <pandabase.h>

#include "nodeTransition.h"

class NodeAttribute;
class NodeRelation;

////////////////////////////////////////////////////////////////////
// 	 Class : OnTransition
// Description : This is an abstract class that encapsulates a family
//               of transitions for states that are always on, in one
//               state or another.  It's similar to OnOffTransition,
//               except the state can never be 'off'; it is always on
//               in some sense.
//
//               Most scene graph attributes that have an enumerated
//               set of states fall into this category.
//
//               There is no explicit identity transition.  Each
//               transition always changes the state.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA OnTransition : public NodeTransition {
protected:
  INLINE OnTransition();
  INLINE OnTransition(const OnTransition &copy);
  INLINE void operator = (const OnTransition &copy);

public:
  virtual NodeTransition *compose(const NodeTransition *other) const;
  virtual NodeTransition *invert() const;
  virtual NodeAttribute *apply(const NodeAttribute *attrib) const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int internal_compare_to(const NodeTransition *other) const;

  virtual void set_value_from(const OnTransition *other)=0;
  virtual int compare_values(const OnTransition *other) const=0;
  virtual void output_value(ostream &out) const=0;
  virtual void write_value(ostream &out, int indent_level) const=0;

  //No new data is defined in OnTransition, and OnTransition is only
  //an Abstract class, so no Read/Write methods are defined

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
    register_type(_type_handle, "OnTransition",
		  NodeTransition::get_class_type());
  }

private:
  static TypeHandle _type_handle;

friend class OnAttribute;
};

#include "onTransition.I"

#endif
