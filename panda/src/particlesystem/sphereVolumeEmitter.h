// Filename: sphereVolumeEmitter.h
// Created by:  charles (22Jun00)
//
////////////////////////////////////////////////////////////////////

#ifndef SPHEREVOLUMEEMITTER_H
#define SPHEREVOLUMEEMITTER_H

#include "baseParticleEmitter.h"

////////////////////////////////////////////////////////////////////
//       Class : SphereVolumeEmitter
// Description : Describes a voluminous spherical region in which
//               particles are generated.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS SphereVolumeEmitter : public BaseParticleEmitter {
private:
  float _radius;

  // CUSTOM EMISSION PARAMETERS
  // none

  ///////////////////////////////
  // scratch variables that carry over from position calc to velocity calc
  LPoint3f _particle_pos;
  ///////////////////////////////

  virtual void assign_initial_position(LPoint3f& pos);
  virtual void assign_initial_velocity(LVector3f& vel);

PUBLISHED:
  SphereVolumeEmitter(void);
  SphereVolumeEmitter(const SphereVolumeEmitter &copy);
  virtual ~SphereVolumeEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);

  INLINE void set_radius(float r);
  INLINE float get_radius(void) const;
};

#include "sphereVolumeEmitter.I"

#endif // SPHEREVOLUMEEMITTER_H
