// Filename: eggBackPointer.h
// Created by:  drose (26Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGBACKPOINTER_H
#define EGGBACKPOINTER_H

#include <pandatoolbase.h>

#include <typedObject.h>

////////////////////////////////////////////////////////////////////
//       Class : EggBackPointer
// Description : This stores a pointer from an EggJointData or
//               EggSlider object back to the referencing data in an
//               egg file.  One of these objects corresponds to each
//               model appearing in an egg file, and may reference
//               either a single node, or a table, or a slew of
//               vertices and primitives, depending on the type of
//               data stored.
//
//               This is just an abstract base class.  The actual
//               details are stored in the various subclasses.
////////////////////////////////////////////////////////////////////
class EggBackPointer : public TypedObject {
public:
  EggBackPointer();


public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "EggBackPointer",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#endif


