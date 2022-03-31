/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleSystemManager.h
 * @author charles
 * @date 2000-06-28
 */

#ifndef PARTICLESYSTEMMANAGER_H
#define PARTICLESYSTEMMANAGER_H

#include "pandabase.h"
#include "plist.h"
#include "particleSystem.h"
#include "pStatCollector.h"

/**
 * Manages a set of individual ParticleSystem objects, so that each individual
 * one doesn't have to be updated and rendered every frame See Also :
 * particleSystemManager.cxx
 */
class EXPCL_PANDA_PARTICLESYSTEM ParticleSystemManager {
PUBLISHED:
  explicit ParticleSystemManager(int every_nth_frame = 1);
  virtual ~ParticleSystemManager();

  INLINE void set_frame_stepping(int every_nth_frame);
  INLINE int get_frame_stepping() const;

  INLINE void attach_particlesystem(ParticleSystem *ps);
  void remove_particlesystem(ParticleSystem *ps);
  INLINE void clear();

  void do_particles(PN_stdfloat dt);
  void do_particles(PN_stdfloat dt, ParticleSystem * ps, bool do_render = true);

  virtual void output(std::ostream &out) const;
  virtual void write_ps_list(std::ostream &out, int indent=0) const;
  virtual void write(std::ostream &out, int indent=0) const;

private:
  plist< PT(ParticleSystem) > _ps_list;

  int _nth_frame;
  int _cur_frame;

  static PStatCollector _do_particles_collector;
};

#include "particleSystemManager.I"

#endif // PARTICLESYSTEMMANAGER_H
