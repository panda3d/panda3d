// Filename: particleSystemManager.h
// Created by:  charles (28Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef PARTICLESYSTEMMANAGER_H
#define PARTICLESYSTEMMANAGER_H

#include "pandabase.h"
#include "plist.h"

#include "particleSystem.h"

////////////////////////////////////////////////////////////////////
//       Class : ParticleSystemManager
// Description : Manages a set of individual ParticleSystem objects,
//               so that each individual one doesn't have to be
//               updated and rendered every frame
////////////////////////////////////////////////////////////////////

class EXPCL_PANDAPHYSICS ParticleSystemManager {
PUBLISHED:
  ParticleSystemManager(int every_nth_frame = 1);
  virtual ~ParticleSystemManager();

  INLINE void set_frame_stepping(int every_nth_frame);
  INLINE int get_frame_stepping() const;

  INLINE void attach_particlesystem(ParticleSystem *ps);
  void remove_particlesystem(ParticleSystem *ps);
  INLINE void clear();

  void do_particles(float dt);

  virtual void output(ostream &out) const;
  virtual void write_ps_list(ostream &out, int indent=0) const;
  virtual void write(ostream &out, int indent=0) const;

private:
  plist< PT(ParticleSystem) > _ps_list;

  int _nth_frame;
  int _cur_frame;
};

#include "particleSystemManager.I"

#endif // PARTICLESYSTEMMANAGER_H
