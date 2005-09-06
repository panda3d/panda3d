// Filename: particleSystemManager.I
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

////////////////////////////////////////////////////////////////////
// Function : set_frame_stepping
//   Access : public
////////////////////////////////////////////////////////////////////

INLINE void ParticleSystemManager::
set_frame_stepping(int every_nth_frame) {
  _nth_frame = every_nth_frame;
}

////////////////////////////////////////////////////////////////////
// Function : get_frame_stepping
//   Access : public
////////////////////////////////////////////////////////////////////

INLINE int ParticleSystemManager::
get_frame_stepping() const {
  return _nth_frame;
}

////////////////////////////////////////////////////////////////////
// Function : attach_particlesystem
//   Access : public
////////////////////////////////////////////////////////////////////

INLINE void ParticleSystemManager::
attach_particlesystem(ParticleSystem *ps) {
  ps->_manager = this;
  plist< PT(ParticleSystem) >::iterator found;
  PT(ParticleSystem) ptps = ps;
  found = find(_ps_list.begin(), _ps_list.end(), ptps);
  if (found == _ps_list.end())
    _ps_list.push_back(ps);
}

////////////////////////////////////////////////////////////////////
// Function : clear
//   Access : public
////////////////////////////////////////////////////////////////////

INLINE void ParticleSystemManager::
clear() {
  _ps_list.erase(_ps_list.begin(), _ps_list.end());
}
