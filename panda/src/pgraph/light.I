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
Light() :
  _priority(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Light::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE Light::
Light(const Light &copy) :
  _priority(copy._priority),
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
//     Function: Light::set_priority
//       Access: Published
//  Description: Changes the relative importance of this light
//               relative to the other lights that are applied
//               simultaneously.
//
//               The priority number is used to decide which of the
//               requested lights are to be selected for rendering
//               when more lights are requested than the hardware will
//               support.  The highest-priority n lights are selected
//               for rendering.
//
//               This is similar to TextureStage::set_priority().
////////////////////////////////////////////////////////////////////
INLINE void Light::
set_priority(int priority) {
  _priority = priority;

  // Update the global flag to indicate that all LightAttribs in the
  // world must now re-sort their lists.
  _sort_seq++;
}

////////////////////////////////////////////////////////////////////
//     Function: Light::get_priority
//       Access: Published
//  Description: Returns the priority associated with this light.  See
//               set_priority().
////////////////////////////////////////////////////////////////////
INLINE int Light::
get_priority() const {
  return _priority;
}

////////////////////////////////////////////////////////////////////
//     Function: Light::get_sort_seq
//       Access: Public, Static
//  Description: Returns a global sequence number that is incremented
//               any time any Light in the world changes sort
//               or priority.  This is used by LightAttrib to
//               determine when it is necessary to re-sort its
//               internal array of stages.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq Light::
get_sort_seq() {
  return _sort_seq;
}

////////////////////////////////////////////////////////////////////
//     Function: Light::mark_viz_stale
//       Access: Protected
//  Description: Indicates that the internal visualization object will
//               need to be updated.
////////////////////////////////////////////////////////////////////
INLINE void Light::
mark_viz_stale() {
  CDWriter cdata(_cycler);
  cdata->_viz_geom_stale = true;
}
