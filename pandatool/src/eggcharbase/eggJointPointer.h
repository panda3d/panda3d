// Filename: eggJointPointer.h
// Created by:  drose (26Feb01)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGJOINTPOINTER_H
#define EGGJOINTPOINTER_H

#include <pandatoolbase.h>

#include "eggBackPointer.h"

#include <eggGroup.h>
#include <pointerTo.h>

////////////////////////////////////////////////////////////////////
// 	 Class : EggJointPointer
// Description : This stores a pointer back to a <Joint> node.
////////////////////////////////////////////////////////////////////
class EggJointPointer : public EggBackPointer {
public:
  EggJointPointer(EggObject *object);

private:
  PT(EggGroup) _joint;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggBackPointer::init_type();
    register_type(_type_handle, "EggJointPointer",
		  EggBackPointer::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
 
private:
  static TypeHandle _type_handle;
};

#endif


