// Filename: angularVectorForce.h
// Created by:  charles (09Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef ANGULARVECTORFORCE_H
#define ANGULARVECTORFORCE_H

#include "angularForce.h"

////////////////////////////////////////////////////////////////////
//       Class : AngularVectorForce
// Description : a simple directed torque force, the angular
//               equivalent of simple vector force.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS AngularVectorForce : public AngularForce {
private:
  LVector3f _fvec;

  virtual AngularForce *make_copy(void) const;
  virtual LVector3f get_child_vector(const PhysicsObject *po);

PUBLISHED:
  AngularVectorForce(const LVector3f& vec);
  AngularVectorForce(float x = 0.0f, float y = 0.0f, float z = 0.0f);
  AngularVectorForce(const AngularVectorForce &copy);
  virtual ~AngularVectorForce(void);

  INLINE void set_vector(const LVector3f& v);
  INLINE void set_vector(float x, float y, float z);
  INLINE LVector3f get_local_vector(void) const;

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    AngularForce::init_type();
    register_type(_type_handle, "AngularVectorForce",
		  AngularForce::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "angularVectorForce.I"

#endif // ANGULARVECTORFORCE_H
