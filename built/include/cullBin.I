/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullBin.I
 * @author drose
 * @date 2002-02-28
 */

/**
 *
 */
INLINE CullBin::
CullBin(const CullBin &copy) :
  _name(copy._name),
  _bin_type(copy._bin_type),
  _gsg(copy._gsg),
  _cull_this_pcollector(copy._cull_this_pcollector),
  _draw_this_pcollector(copy._draw_this_pcollector)
{
}

/**
 *
 */
INLINE CullBin::
CullBin(const std::string &name, CullBin::BinType bin_type,
        GraphicsStateGuardianBase *gsg,
        const PStatCollector &draw_region_pcollector) :
  _name(name),
  _bin_type(bin_type),
  _gsg(gsg),
  _cull_this_pcollector(_cull_bin_pcollector, name),
  _draw_this_pcollector(draw_region_pcollector, name)
{
}

/**
 *
 */
INLINE const std::string &CullBin::
get_name() const {
  return _name;
}

/**
 *
 */
INLINE CullBin::BinType CullBin::
get_bin_type() const {
  return _bin_type;
}
