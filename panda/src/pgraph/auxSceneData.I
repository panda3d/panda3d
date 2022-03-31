/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file auxSceneData.I
 * @author drose
 * @date 2004-09-27
 */

/**
 * This is protected, since you normally don't want to create a plain
 * AuxSceneData object; instead, create an instance of a derived class that
 * actually has some useful data in it.
 */
INLINE AuxSceneData::
AuxSceneData(double duration) :
  _duration(duration),
  _last_render_time(0.0)
{
}

/**
 * Specifies the minimum length in time, in seconds, to keep this AuxSceneData
 * object around in the scene graph after the last time it was rendered.
 */
INLINE void AuxSceneData::
set_duration(double duration) {
  _duration = duration;
}

/**
 * Returns the minimum length in time, in seconds, to keep this AuxSceneData
 * object around in the scene graph after the last time it was rendered.
 */
INLINE double AuxSceneData::
get_duration() const {
  return _duration;
}

/**
 * Should be called with the current frame_time each time the AuxSceneData is
 * used during traversal.
 */
INLINE void AuxSceneData::
set_last_render_time(double last_render_time) {
  _last_render_time = last_render_time;
}

/**
 * Returns the last time this object was used during traversal (according to
 * set_last_render_time()).
 */
INLINE double AuxSceneData::
get_last_render_time() const {
  return _last_render_time;
}

/**
 * Returns the frame_time at which this AuxSceneData object is currently
 * scheduled to be removed from the scene graph.
 */
INLINE double AuxSceneData::
get_expiration_time() const {
  return _last_render_time + _duration;
}

INLINE std::ostream &
operator << (std::ostream &out, const AuxSceneData &data) {
  data.output(out);
  return out;
}
