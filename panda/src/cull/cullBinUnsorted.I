/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBinUnsorted.I
 * @author drose
 * @date 2002-02-28
 */

/**
 *
 */
INLINE CullBinUnsorted::
CullBinUnsorted(const std::string &name, GraphicsStateGuardianBase *gsg,
                const PStatCollector &draw_region_pcollector) :
  CullBin(name, BT_unsorted, gsg, draw_region_pcollector)
{
}
