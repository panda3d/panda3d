// Filename: LinearSourceForce.h
// Created by:  charles (21Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LINEARSOURCEFORCE_H
#define LINEARSOURCEFORCE_H

#include "linearDistanceForce.h"

////////////////////////////////////////////////////////////////////
//       Class : LinearSourceForce
// Description : Repellant force.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearSourceForce : public LinearDistanceForce {
private:
  virtual LVector3f get_child_vector(const PhysicsObject *po);
  virtual LinearForce *make_copy(void);

PUBLISHED:
  LinearSourceForce(const LPoint3f& p, FalloffType f, float r, float a = 1.0f, 
	      bool mass = true);
  LinearSourceForce(void);
  LinearSourceForce(const LinearSourceForce &copy);
  virtual ~LinearSourceForce(void);

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LinearDistanceForce::init_type();
    register_type(_type_handle, "LinearSourceForce",
		  LinearDistanceForce::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // LINEARSOURCEFORCE_H
