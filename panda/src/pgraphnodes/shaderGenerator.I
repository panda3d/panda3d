/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file shaderGenerator.I
 * @author jyelon
 * @date 2007-12-15
 */

/**
 * Returns the alpha test mode subsumed by this shader.
 */
INLINE RenderAttrib::PandaCompareFunc ShaderGenerator::ShaderKey::
get_alpha_test_mode() const {
  return (RenderAttrib::PandaCompareFunc)
    ((_flags & ShaderKey::F_ALPHA_TEST_MASK) >> ShaderKey::F_ALPHA_TEST_SHIFT);
}
