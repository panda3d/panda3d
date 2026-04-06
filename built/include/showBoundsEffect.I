/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file showBoundsEffect.I
 * @author drose
 * @date 2002-03-25
 */

/**
 * Use ShowBoundsEffect::make() to construct a new ShowBoundsEffect object.
 */
INLINE ShowBoundsEffect::
ShowBoundsEffect() :
  _tight(false)
{
}

/**
 * Returns true if the "tight" flag was set, meaning the effect should compute
 * and draw the tight bounding-box of the node's vertices every frame.
 */
INLINE bool ShowBoundsEffect::
get_tight() const {
  return _tight;
}
