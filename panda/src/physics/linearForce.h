// Filename: LinearForce.h
// Created by:  charles (13Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LINEARFORCE_H
#define LINEARFORCE_H

#include "baseForce.h"

///////////////////////////////////////////////////////////////////
//       Class : LinearForce
// Description : A force that acts on a PhysicsObject by way of an 
//               Integrator.  This is a pure virtual base class.
///////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearForce : public BaseForce {
private:
  float _amplitude;
  bool _mass_dependent;

  bool _x_mask;
  bool _y_mask;
  bool _z_mask;

  virtual LVector3f get_child_vector(const PhysicsObject *po) = 0;

protected:
  LinearForce(float a, bool mass);
  LinearForce(const LinearForce& copy);

PUBLISHED:
  ~LinearForce(void);

  INLINE void set_amplitude(const float a);
  INLINE void set_mass_dependent(bool m);

  INLINE float get_amplitude(void) const;
  INLINE bool get_mass_dependent(void) const;

  INLINE void set_vector_masks(bool x, bool y, bool z);

  virtual LVector3f get_vector(const PhysicsObject *po);

  virtual LinearForce *make_copy(void) = 0;

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    BaseForce::init_type();
    register_type(_type_handle, "LinearForce",
		  BaseForce::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "linearForce.I"

#endif // BASELINEARFORCE_H
