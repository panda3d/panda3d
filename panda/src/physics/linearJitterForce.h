// Filename: LinearJitterForce.h
// Created by:  charles (13Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LINEARJITTERFORCE_H
#define LINEARJITTERFORCE_H

#include "linearRandomForce.h"

////////////////////////////////////////////////////////////////////
//       Class : LinearJitterForce
// Description : Completely random noise force vector.  Not
//               repeatable, reliable, or predictable.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearJitterForce : public LinearRandomForce {
private:
  virtual LVector3f get_child_vector(const PhysicsObject *po);
  virtual LinearForce *make_copy(void);

PUBLISHED:
  LinearJitterForce(float a = 1.0f, bool m = false);
  LinearJitterForce(const LinearJitterForce &copy);
  virtual ~LinearJitterForce(void);

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LinearRandomForce::init_type();
    register_type(_type_handle, "LinearJitterForce",
		  LinearRandomForce::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // LINEARJITTERFORCE_H
