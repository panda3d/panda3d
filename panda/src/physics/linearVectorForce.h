// Filename: linearVectorForce.h
// Created by:  charles (13Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LINEARVECTORFORCE_H
#define LINEARVECTORFORCE_H

#include "linearForce.h"

////////////////////////////////////////////////////////////////
//       Class : LinearVectorForce
// Description : Simple directed vector force.  Suitable for
//               gravity, non-turbulent wind, etc...
////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearVectorForce : public LinearForce {
private:
  LVector3f _fvec;

  virtual LinearForce *make_copy(void);
  virtual LVector3f get_child_vector(const PhysicsObject *po);

public:
  LinearVectorForce(const LVector3f& vec, float a = 1.0f, bool mass = false);
  LinearVectorForce(const LinearVectorForce &copy);
  LinearVectorForce(float x = 0.0f, float y = 0.0f, float z = 0.0f, 
 	      float a = 1.0f, bool mass = false);
  virtual ~LinearVectorForce(void);

  INLINE void set_vector(const LVector3f& v);
  INLINE void set_vector(float x, float y, float z);

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LinearForce::init_type();
    register_type(_type_handle, "LinearVectorForce",
		  LinearForce::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "linearVectorForce.I"

#endif // LINEARVECTORFORCE_H
