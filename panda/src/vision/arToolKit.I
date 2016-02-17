/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file arToolKit.I
 * @author jyelon
 * @date 2007-11-01
 */

/**
 * As part of its analysis, the ARToolKit occasionally converts images to
 * black and white by thresholding them.  The threshold is set to 0.5 by
 * default, but you can tweak it here.
 */
INLINE void ARToolKit::
set_threshold(double thresh) {
  _threshold = thresh;
}
