// Filename: particleSystemManager.h
// Created by:  charles (28Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PARTICLESYSTEMMANAGER_H
#define PARTICLESYSTEMMANAGER_H

#include <pandabase.h>
#include <list>

#include "particleSystem.h"

////////////////////////////////////////////////////////////////////
//       Class : ParticleSystemManager
// Description : Manages a set of individual ParticleSystem objects,
//               so that each individual one doesn't have to be
//               updated and rendered every frame
////////////////////////////////////////////////////////////////////

class EXPCL_PANDAPHYSICS ParticleSystemManager {
private:

  list< PT(ParticleSystem) > _ps_list;

  int _nth_frame;
  int _cur_frame;

public:

  ParticleSystemManager(int every_nth_frame = 1);

  INLINE void set_frame_stepping(int every_nth_frame);
  INLINE int get_frame_stepping(void) const;

  INLINE void attach_particlesystem(ParticleSystem *ps);
  void remove_particlesystem(ParticleSystem *ps);
  INLINE void clear(void);

  void do_particles(float dt);
};

#include "particleSystemManager.I"

#endif // PARTICLESYSTEMMANAGER_H
