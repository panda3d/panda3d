/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file particleSystemManager.I
 * @author charles
 * @date 2000-06-28
 */

/**

 */

INLINE void ParticleSystemManager::
set_frame_stepping(int every_nth_frame) {
  _nth_frame = every_nth_frame;
}

/**

 */

INLINE int ParticleSystemManager::
get_frame_stepping() const {
  return _nth_frame;
}

/**

 */

INLINE void ParticleSystemManager::
attach_particlesystem(ParticleSystem *ps) {
  ps->_manager = this;
  plist< PT(ParticleSystem) >::iterator found;
  PT(ParticleSystem) ptps = ps;
  found = find(_ps_list.begin(), _ps_list.end(), ptps);
  if (found == _ps_list.end())
    _ps_list.push_back(ps);
}

/**

 */

INLINE void ParticleSystemManager::
clear() {
  _ps_list.erase(_ps_list.begin(), _ps_list.end());
}
