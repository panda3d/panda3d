// Filename: immediateAttribute.h
// Created by:  drose (24Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef IMMEDIATEATTRIBUTE_H
#define IMMEDIATEATTRIBUTE_H

#include <pandabase.h>

#include "nodeAttribute.h"

class ImmediateTransition;

////////////////////////////////////////////////////////////////////
//       Class : ImmediateAttribute
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ImmediateAttribute : public NodeAttribute {
public:

  INLINE_GRAPH ImmediateAttribute() {};
  INLINE_GRAPH ImmediateAttribute(const ImmediateAttribute &copy) :
               NodeAttribute(copy){};
  INLINE_GRAPH void operator = (const ImmediateAttribute &copy)
               {NodeAttribute::operator = (copy);}
public:
  virtual NodeAttribute *make_copy() const;
  virtual NodeAttribute *make_initial() const;
  virtual TypeHandle get_handle() const;

protected:
  virtual int internal_compare_to(const NodeAttribute *other) const;

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
    register_type(_type_handle, "ImmediateAttribute",
                  NodeAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#endif
