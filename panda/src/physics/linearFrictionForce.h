// Filename: LinearFrictionForce.h
// Created by:  charles (23Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LINEARFRICTIONFORCE_H
#define LINEARFRICTIONFORCE_H

#include "linearForce.h"

////////////////////////////////////////////////////////////////////
//       Class : LinearFrictionForce
// Description : Friction-based drag force
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearFrictionForce : public LinearForce {
private:
  float _coef;

  virtual LinearForce *make_copy(void);
  virtual LVector3f get_child_vector(const PhysicsObject *);

PUBLISHED:
  LinearFrictionForce(float coef = 1.0f, float a = 1.0f, bool m = false);
  LinearFrictionForce(const LinearFrictionForce &copy);
  virtual ~LinearFrictionForce(void);

  INLINE void set_coef(float coef);
  INLINE float get_coef(void) const;

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LinearForce::init_type();
    register_type(_type_handle, "LinearFrictionForce",
                  LinearForce::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "linearFrictionForce.I"

#endif // LINEARFRICTIONFORCE_H
