// Filename: zSpinParticle.h
// Created by:  charles (16Aug00)
//
////////////////////////////////////////////////////////////////////

#ifndef ZSPINPARTICLE_H
#define ZSPINPARTICLE_H

#include "baseParticle.h"

////////////////////////////////////////////////////////////////////
//       Class : ZSpinParticle
// Description : describes a particle that spins along its z axis.
//               this is kind of an intermediary class- if you're
//               using a SpriteParticleRenderer and you want your
//               sprites to spin without having them be full-blown
//               oriented (i.e. angry quat math), use this.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS ZSpinParticle : public BaseParticle {
private:
  float _initial_angle;
  float _final_angle;
  float _cur_angle;

public:
  ZSpinParticle(void);
  ZSpinParticle(const ZSpinParticle &copy);
  virtual ~ZSpinParticle(void);

  virtual PhysicsObject *make_copy(void) const;

  virtual void init(void);
  virtual void update(void);
  virtual void die(void);

  virtual float get_theta(void) const;

  INLINE void set_initial_angle(float t);
  INLINE float get_initial_angle(void) const;

  INLINE void set_final_angle(float t);
  INLINE float get_final_angle(void) const;
};

#include "zSpinParticle.I"

#endif // ZSPINPARTICLE_H
