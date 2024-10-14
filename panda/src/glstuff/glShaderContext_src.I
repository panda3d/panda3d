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
 *
 */
INLINE void CLP(ShaderContext)::
set_display_region(const DisplayRegion *display_region) {
  if (_display_region != display_region) {
    _display_region = display_region;
    issue_parameters(Shader::D_scene);
  }
}
