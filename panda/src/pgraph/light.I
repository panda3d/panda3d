// Filename: light.I
// Created by:  drose (26Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: Light::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Light::CData::
CData() :
  _color(1.0f, 1.0f, 1.0f, 1.0f),
  _viz_geom_stale(true)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Light::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Light::CData::
CData(const Light::CData &copy) :
  _color(copy._color),
  _viz_geom(copy._viz_geom),
  _viz_geom_stale(copy._viz_geom_stale)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Light::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Light::
Light() {
}

////////////////////////////////////////////////////////////////////
//     Function: Light::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Light::
Light(const Light &copy) :
  _cycler(copy._cycler)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Light::get_color
//       Access: Published
//  Description: Returns the basic color of the light.
////////////////////////////////////////////////////////////////////
INLINE const Colorf &Light::
get_color() const {
  CDReader cdata(_cycler);
  return cdata->_color;
}

////////////////////////////////////////////////////////////////////
//     Function: Light::set_color
//       Access: Published
//  Description: Sets the basic color of the light.
////////////////////////////////////////////////////////////////////
INLINE void Light::
set_color(const Colorf &color) {
  CDWriter cdata(_cycler);
  cdata->_color = color;
  mark_viz_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: Light::mark_viz_stale
//       Access: Public
//  Description: Indicates that the internal visualization object will
//               need to be updated.
////////////////////////////////////////////////////////////////////
INLINE void Light::
mark_viz_stale() {
  CDWriter cdata(_cycler);
  cdata->_viz_geom_stale = true;
}
