// Filename: orientedParticle.h
// Created by:  charles (19Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef ORIENTEDPARTICLE_H
#define ORIENTEDPARTICLE_H

#include "baseParticle.h"

#include <renderRelation.h>

///////////////////////////////////////////////////////////////////
//        Class : OrientedParticle
//  Description : Describes a particle that has angular
//                characteristics (velocity, orientation).
///////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS OrientedParticle : public BaseParticle {
public:
  OrientedParticle(int lifespan = 0, bool alive = false);
  OrientedParticle(const OrientedParticle &copy);
  virtual ~OrientedParticle(void);

  virtual PhysicsObject *make_copy(void) const;

  INLINE void set_velocity(void);
  INLINE void set_orientation(void);

  virtual void init(void);
  virtual void update(void);
  virtual void die(void);
};

#include "orientedParticle.I"

#endif // ORIENTEDPARTICLE_H
