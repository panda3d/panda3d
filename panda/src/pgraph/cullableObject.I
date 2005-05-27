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
//  Description: Creates a CullableObject based on the ith Geom from
//               the indicated GeomNode, with the render state from
//               the indicated CullTraverserData.
////////////////////////////////////////////////////////////////////
INLINE CullableObject::
CullableObject(const CullTraverserData &data,
               GeomNode *geom_node, int i,
               CullableObject *next) :
  _geom(geom_node->get_geom(i)),
  _state(data._state->compose(geom_node->get_geom_state(i))),
  _modelview_transform(data._modelview_transform),
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
               const TransformState *modelview_transform,
               CullableObject *next) :
  _geom(geom),
  _state(state),
  _modelview_transform(modelview_transform),
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
  _state(copy._state),
  _modelview_transform(copy._modelview_transform),
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
  _state = copy._state;
  _modelview_transform = copy._modelview_transform;
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
draw(GraphicsStateGuardianBase *gsg) {
  _geom->draw(gsg, _munger, _munged_data);
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::operator new
//       Access: Public
//  Description: Allocates the memory for a new CullableObject.  This
//               is specialized here to provide for fast allocation of
//               these things (since we may create and destroy
//               thousands of these each frame).
////////////////////////////////////////////////////////////////////
INLINE void *CullableObject::
operator new(size_t size) {
  if (_deleted_chain != (CullableObject *)NULL) {
    CullableObject *obj = _deleted_chain;
    _deleted_chain = _deleted_chain->_next;
    return obj;
  }
#ifndef NDEBUG
  _num_ever_allocated++;
#endif  // NDEBUG
  return ::operator new(size);
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::operator delete
//       Access: Public
//  Description: Frees the memory for a deleted CullableObject.  This
//               is specialized here to provide for fast allocation of
//               these things (since we may create and destroy
//               thousands of these each frame).
////////////////////////////////////////////////////////////////////
INLINE void CullableObject::
operator delete(void *ptr) {
  CullableObject *obj = (CullableObject *)ptr;
  obj->_next = _deleted_chain;
  _deleted_chain = obj;
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::get_num_ever_allocated
//       Access: Published, Static
//  Description: Returns the number of CullableObject pointers ever
//               simultaneously allocated; these are now either in
//               active use or have been recycled into the deleted
//               CullableObject pool to be used again.
////////////////////////////////////////////////////////////////////
INLINE int CullableObject::
get_num_ever_allocated() {
  return _num_ever_allocated;
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
