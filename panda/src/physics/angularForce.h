// Filename: angularForce.h
// Created by:  charles (08Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef ANGULARFORCE_H
#define ANGULARFORCE_H

#include "baseForce.h"

////////////////////////////////////////////////////////////////////
//       Class : AngularForce
// Description : pure virtual parent of all quat-based forces.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS AngularForce : public BaseForce {
private:
  virtual LVector3f get_child_vector(const PhysicsObject *po) = 0;

protected:
  AngularForce(void);
  AngularForce(const AngularForce &copy);

PUBLISHED:
  virtual ~AngularForce(void);

  virtual AngularForce *make_copy(void) const = 0;
  LVector3f get_vector(const PhysicsObject *po);
  virtual bool is_linear(void) const;

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    BaseForce::init_type();
    register_type(_type_handle, "AngularForce",
		  BaseForce::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#endif // ANGULARFORCE_H
