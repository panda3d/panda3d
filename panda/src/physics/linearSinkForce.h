// Filename: LinearSinkForce.h
// Created by:  charles (21Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LINEARSINKFORCE_H
#define LINEARSINKFORCE_H

#include "linearDistanceForce.h"

////////////////////////////////////////////////////////////////////
//       Class : LinearSinkForce
// Description : Attractor force.  Think black hole.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearSinkForce : public LinearDistanceForce {
private:
  virtual LVector3f get_child_vector(const PhysicsObject *po);
  virtual LinearForce *make_copy(void);

PUBLISHED:
  LinearSinkForce(const LPoint3f& p, FalloffType f, float r, float a = 1.0f, 
            bool m = true);
  LinearSinkForce(void);
  LinearSinkForce(const LinearSinkForce &copy);
  virtual ~LinearSinkForce(void);

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LinearDistanceForce::init_type();
    register_type(_type_handle, "LinearSinkForce",
                  LinearDistanceForce::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // LINEARSINKFORCE_H
