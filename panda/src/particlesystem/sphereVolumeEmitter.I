// Filename: sphereVolumeEmitter.I
// Created by:  charles (26Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "config_particlesystem.h"

////////////////////////////////////////////////////////////////////
//    Function : set_radius
//      Access : public
// Description : radius set
////////////////////////////////////////////////////////////////////

INLINE void SphereVolumeEmitter::
set_radius(float r) { 
  if (r == 0.0f)
    particlesystem_cat.error() << "SphereVolumeEmitter can not have "
			       << "radius of 0." << endl;
  else
    _radius = r; 
}
