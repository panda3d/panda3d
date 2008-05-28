// Filename: baseParticle.h
// Created by:  charles (14Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef BASEPARTICLE_H
#define BASEPARTICLE_H

#include "pandabase.h"
#include "physicsObject.h"

////////////////////////////////////////////////////////////////////
//       Class : BaseParticle
// Description : An individual, physically-modelable particle
//               abstract base class.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAPHYSICS BaseParticle : public PhysicsObject {
public:
  // local methods
  INLINE void set_age(float age);
  INLINE void set_lifespan(float lifespan);
  INLINE void set_alive(bool alive);
  INLINE void set_index(int index);


  INLINE float get_age() const;
  INLINE float get_lifespan() const;
  INLINE bool get_alive() const;
  INLINE int get_index() const; 

  INLINE float get_parameterized_age() const;
  INLINE float get_parameterized_vel() const;

  // child methods
  virtual void init() = 0;
  virtual void die() = 0;
  virtual void update() = 0;

  // for spriteParticleRenderer
  virtual float get_theta() const;

  // from PhysicsObject
  virtual PhysicsObject *make_copy() const = 0;

  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent=0) const;

protected:
  BaseParticle(float lifespan = 1.0f, bool alive = false);
  BaseParticle(const BaseParticle &copy);
  virtual ~BaseParticle();

private:
  // NOTE: age and lifespan are in seconds.
  float _age;
  float _lifespan;
  bool _alive;
  int _index;

  LPoint3f _last_position;
};

#include "baseParticle.I"

#endif // BASEPARTICLE_H
