// Filename: pointParticle.h
// Created by:  charles (19Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef POINTPARTICLE_H
#define POINTPARTICLE_H

#include "baseParticle.h"

////////////////////////////////////////////////////////////////////
//       Class : PointParticle
// Description : Describes a particle that requires representation
//               by a point (pixel, sparkle, billboard)
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS PointParticle : public BaseParticle {
public:
  PointParticle(int lifespan = 0, bool alive = false);
  PointParticle(const PointParticle &copy);
  virtual ~PointParticle(void);

  virtual void init(void);
  virtual void die(void);
  virtual void update(void);

  virtual PhysicsObject *make_copy(void) const;
};

#endif // POINTPARTICLE_H
