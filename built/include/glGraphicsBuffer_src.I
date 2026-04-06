/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glGraphicsBuffer_src.I
 * @author jyelon
 * @date 2006-01-15
 */

/**
 *
 */
INLINE int CLP(GraphicsBuffer)::
get_multisample_count() {
  return _requested_multisamples;
}

/**
 *
 */
INLINE int CLP(GraphicsBuffer)::
get_coverage_sample_count() {
  return _requested_coverage_samples;
}
