// Filename: eggObject.h
// Created by:  drose (17Jan99)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGOBJECT_H
#define EGGOBJECT_H

#include <pandabase.h>

#include <typedReferenceCount.h>

#ifndef WIN32_VC
class ostream;
#endif

////////////////////////////////////////////////////////////////////
// 	 Class : EggObject
// Description : The highest-level base class in the egg directory.
//               (Almost) all things egg inherit from this.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEGG EggObject : public TypedReferenceCount {
public:
  INLINE EggObject();
  INLINE EggObject(const EggObject &copy);
  INLINE EggObject &operator = (const EggObject &copy);

  virtual ~EggObject();

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "EggObject",
		  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggObject.I"

#endif
