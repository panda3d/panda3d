/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderContext.I
 * @author jyelon
 * @date 2005-09-01
 */

/**
 *
 */
INLINE ShaderContext::
ShaderContext(Shader *se) :
  _shader(se)
{
}

/**
 *
 */
INLINE Shader *ShaderContext::
get_shader() const {
  return _shader;
}
