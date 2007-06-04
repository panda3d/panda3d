// Filename: cullableObject.I
// Created by:  drose (04Mar02)
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
//     Function: CullableObject::Constructor
//       Access: Public
//  Description: Creates an empty CullableObject whose pointers can be
//               filled in later.
////////////////////////////////////////////////////////////////////
INLINE CullableObject::
CullableObject(CullableObject *next) :
  _next(next)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::Constructor
//       Access: Public
//  Description: Creates a CullableObject based the indicated geom,
//               with the indicated render state and transform.
////////////////////////////////////////////////////////////////////
INLINE CullableObject::
CullableObject(const Geom *geom, const RenderState *state,
               const TransformState *net_transform,
               const TransformState *modelview_transform,
               const GraphicsStateGuardianBase *gsg,
               CullableObject *next) :
  _geom(geom),
  _state(state),
  _net_transform(net_transform),
  _modelview_transform(modelview_transform),
  _internal_transform(gsg->get_cs_transform()->compose(modelview_transform)),
  _next(next)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::Constructor
//       Access: Public
//  Description: Creates a CullableObject based the indicated geom,
//               with the indicated render state and transform.
////////////////////////////////////////////////////////////////////
INLINE CullableObject::
CullableObject(const Geom *geom, const RenderState *state,
               const TransformState *net_transform,
               const TransformState *modelview_transform,
               const TransformState *internal_transform,
               CullableObject *next) :
  _geom(geom),
  _state(state),
  _net_transform(net_transform),
  _modelview_transform(modelview_transform),
  _internal_transform(internal_transform),
  _next(next)
{
}
  

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::Copy Constructor
//       Access: Public
//  Description: Copies the CullableObject, but does not copy its
//               children (decals).
////////////////////////////////////////////////////////////////////
INLINE CullableObject::
CullableObject(const CullableObject &copy) :
  _geom(copy._geom),
  _munger(copy._munger),
  _munged_data(copy._munged_data),
  _state(copy._state),
  _net_transform(copy._net_transform),
  _modelview_transform(copy._modelview_transform),
  _internal_transform(copy._internal_transform),
  _next((CullableObject *)NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::Copy Assignment Operator
//       Access: Public
//  Description: Copies the CullableObject, but does not copy its
//               children (decals).
////////////////////////////////////////////////////////////////////
INLINE void CullableObject::
operator = (const CullableObject &copy) {
  _geom = copy._geom;
  _munger = copy._munger;
  _munged_data = copy._munged_data;
  _state = copy._state;
  _net_transform = copy._net_transform;
  _modelview_transform = copy._modelview_transform;
  _internal_transform = copy._internal_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::has_decals
//       Access: Public
//  Description: Returns true if the object has one or more decals
//               placed on it, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool CullableObject::
has_decals() const {
  return (_next != (CullableObject *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::draw
//       Access: Public
//  Description: Draws the cullable object on the GSG immediately, in
//               the GSG's current state.  This should only be called
//               from the draw thread.
////////////////////////////////////////////////////////////////////
INLINE void CullableObject::
draw(GraphicsStateGuardianBase *gsg, Thread *current_thread) {
  _geom->draw(gsg, _munger, _munged_data, current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::flush_level
//       Access: Public, Static
//  Description: Flushes the PStatCollectors used during traversal.
////////////////////////////////////////////////////////////////////
INLINE void CullableObject::
flush_level() {
  _sw_sprites_pcollector.flush_level();
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::SortPoints::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE CullableObject::SortPoints::
SortPoints(const CullableObject::PointData *array) :
  _array(array)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::SortPoints::operator ()
//       Access: Public
//  Description: Orders the points from back-to-front for correct
//               transparency sorting in munge_points_to_quads
////////////////////////////////////////////////////////////////////
INLINE bool CullableObject::SortPoints::
operator () (unsigned short a, unsigned short b) const {
  return _array[a]._dist > _array[b]._dist;
}
