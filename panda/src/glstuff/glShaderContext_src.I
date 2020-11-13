/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glShaderContext_src.I
 * @author jyelon
 * @date 2005-09-01
 */

/**
 * Returns the uniform location for the given seqno id, or -1 if the uniform is
 * not used.
 */
INLINE GLint CLP(ShaderContext)::
get_uniform_location(int seqno) const {
  nassertr(seqno >= 0, -1);

  if (_remap_uniform_locations) {
    if ((size_t)seqno < _uniform_location_map.size()) {
      return _uniform_location_map[(size_t)seqno];
    } else {
      return -1;
    }
  } else {
    return (GLint)seqno;
  }
}

/**
 * Sets the uniform location for the given seqno id.
 */
INLINE void CLP(ShaderContext)::
set_uniform_location(int seqno, GLint location) {
  while (seqno >= _uniform_location_map.size()) {
    _uniform_location_map.push_back(-1);
  }

  _uniform_location_map[seqno] = location;
}
