// Filename: LinearUserDefinedForce.h
// Created by:  charles (31Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LINEARUSERDEFINEDFORCE_H
#define LINEARUSERDEFINEDFORCE_H

#include "linearForce.h"

////////////////////////////////////////////////////////////////////
//       Class : LinearUserDefinedForce
// Description : a programmable force that takes an evaluator fn.
//
//        NOTE : AS OF Interrogate => Squeak, this class does NOT
//               get FFI'd due to the function pointer bug, and is
//               currently NOT getting interrogated.  Change this
//               in the makefile when the time is right or this class
//               becomes needed...
////////////////////////////////////////////////////////////////////
class LinearUserDefinedForce : public LinearForce {
private:
  LVector3f (*_proc)(const PhysicsObject *po);

  virtual LVector3f get_child_vector(const PhysicsObject *po);
  virtual LinearForce *make_copy(void);

PUBLISHED:
  LinearUserDefinedForce(LVector3f (*proc)(const PhysicsObject *) = NULL,
		   float a = 1.0f,
		   bool md = false);
  LinearUserDefinedForce(const LinearUserDefinedForce &copy);
  virtual ~LinearUserDefinedForce(void);

  INLINE void set_proc(LVector3f (*proc)(const PhysicsObject *));

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LinearForce::init_type();
    register_type(_type_handle, "LinearUserDefinedForce",
		  LinearForce::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "linearUserDefinedForce.I"

#endif // LINEARUSERDEFINEDFORCE_H
