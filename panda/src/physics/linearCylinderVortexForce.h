// Filename: LinearCylinderVortexForce.h
// Created by:  charles (24Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LINEARCYLINDERVORTEXFORCE_H
#define LINEARCYLINDERVORTEXFORCE_H

#include "linearForce.h"

////////////////////////////////////////////////////////////////////
//       Class : LinearCylinderVortexForce
// Description : Defines a cylinder inside of which all forces are
//               tangential to the theta of the particle wrt the 
//               z-axis in local coord. space.  This happens by
//               assigning the force a node by which the cylinder is 
//               transformed.  Be warned- this will suck anything
//               that it can reach directly into orbit and will NOT
//               let go.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS LinearCylinderVortexForce : public LinearForce {
private:
  float _radius;
  float _length;
  float _coef;

  virtual LinearForce *make_copy(void);
  virtual LVector3f get_child_vector(const PhysicsObject *po);

PUBLISHED:
  LinearCylinderVortexForce(float radius = 1.0f,
		      float length = 0.0f,
		      float coef = 1.0f,
		      float a = 1.0f,
		      bool md = false);
  LinearCylinderVortexForce(const LinearCylinderVortexForce &copy);
  virtual ~LinearCylinderVortexForce(void);

  INLINE void set_coef(float coef);
  INLINE float get_coef(void) const;

  INLINE void set_radius(float radius);
  INLINE float get_radius(void) const;

  INLINE void set_length(float length);
  INLINE float get_length(void) const;

public:
  static TypeHandle get_class_type(void) {
    return _type_handle;
  }
  static void init_type(void) {
    LinearForce::init_type();
    register_type(_type_handle, "LinearCylinderVortexForce",
		  LinearForce::get_class_type());
  }
  virtual TypeHandle get_type(void) const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "linearCylinderVortexForce.I"

#endif // LINEARCYLINDERVORTEXFORCE_H
