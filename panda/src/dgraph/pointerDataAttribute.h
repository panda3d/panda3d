// Filename: pointerDataAttribute.h
// Created by:  jason (07Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef POINTERDATAATTRIBUTE_H
#define POINTERDATAATTRIBUTE_H

#include <pandabase.h>

#include <nodeAttribute.h>
#include <indent.h>

template<class PtrType>
class PointerDataTransition;

////////////////////////////////////////////////////////////////////
//       Class : PointerDataTransition
// Description : 
////////////////////////////////////////////////////////////////////
template<class PtrType>
class PointerDataAttribute : public NodeAttribute {
public:
  INLINE PointerDataAttribute();
  INLINE PointerDataAttribute(PtrType* ptr);
  INLINE PointerDataAttribute(const PointerDataAttribute<PtrType> &copy);
  INLINE void operator = (const PointerDataAttribute<PtrType> &copy);

public:
  INLINE void set_value(PtrType* ptr);
  INLINE PtrType* get_value() const;

  virtual TypeHandle get_handle() const;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

protected:
  virtual int internal_compare_to(const NodeAttribute *other) const;

private:
  PtrType* _ptr;

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
    register_type(_type_handle, 
                  string("PointerDataAttribute<") + 
                  get_type_handle(PtrType).get_name() + ">",
                  NodeAttribute::get_class_type());
  }

private:
  static TypeHandle _type_handle;
  friend class PointerDataTransition<PtrType>;

  //Variable is just to make the compiler happy for the default
  //constructor
  static PtrType* _null_ptr;
};

#include "pointerDataAttribute.I"

#endif
