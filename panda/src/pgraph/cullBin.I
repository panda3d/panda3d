// Filename: cullBin.I
// Created by:  drose (28Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: CullBin::Copy Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBin::
CullBin(const CullBin &copy) : 
  _name(copy._name),
  _bin_type(copy._bin_type),
  _gsg(copy._gsg),
  _cull_this_pcollector(copy._cull_this_pcollector),
  _draw_this_pcollector(copy._draw_this_pcollector)
{
  check_flash_color();
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBin::
CullBin(const string &name, CullBin::BinType bin_type,
        GraphicsStateGuardianBase *gsg,
        const PStatCollector &draw_region_pcollector) :
  _name(name),
  _bin_type(bin_type),
  _gsg(gsg),
  _cull_this_pcollector(_cull_bin_pcollector, name),
  _draw_this_pcollector(draw_region_pcollector, name)
{
  check_flash_color();
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::get_name
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE const string &CullBin::
get_name() const {
  return _name;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::get_bin_tyep
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullBin::BinType CullBin::
get_bin_type() const {
  return _bin_type;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::has_flash_color
//       Access: Public
//  Description: Returns true if this bin has a flash color configured
//               via the flash-bin-binname config directive, or false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool CullBin::
has_flash_color() const {
  return _has_flash_color;
}

////////////////////////////////////////////////////////////////////
//     Function: CullBin::get_flash_color
//       Access: Public
//  Description: If has_flash_color returns true, this returns the
//               color specified.
////////////////////////////////////////////////////////////////////
INLINE const LColor &CullBin::
get_flash_color() const {
  return _flash_color;
}
