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

  virtual void assign_initial_values(LPoint3f& pos, LVector3f& vel);
  
public:
  SphereVolumeEmitter(void);
  SphereVolumeEmitter(const SphereVolumeEmitter &copy);
  virtual ~SphereVolumeEmitter(void);

  virtual BaseParticleEmitter *make_copy(void);
  INLINE void set_radius(float r);
};

#include "sphereVolumeEmitter.I"

#endif // SPHEREVOLUMEEMITTER_H
