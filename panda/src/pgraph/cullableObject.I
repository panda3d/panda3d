// Filename: cullableObject.I
// Created by:  drose (04Mar02)
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
//     Function: CullableObject::Constructor
//       Access: Public
//  Description: Creates an empty CullableObject whose pointers can be
//               filled in later.
////////////////////////////////////////////////////////////////////
INLINE CullableObject::
CullableObject() {
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, get_class_type());
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::Constructor
//       Access: Public
//  Description: Creates a CullableObject based the indicated geom,
//               with the indicated render state and transform.
////////////////////////////////////////////////////////////////////
INLINE CullableObject::
CullableObject(const Geom *geom, const RenderState *state,
               const TransformState *internal_transform) :
  _geom(geom),
  _state(state),
  _internal_transform(internal_transform)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, get_class_type());
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::Copy Constructor
//       Access: Public
//  Description: Copies the CullableObject.
////////////////////////////////////////////////////////////////////
INLINE CullableObject::
CullableObject(const CullableObject &copy) :
  _geom(copy._geom),
  _munger(copy._munger),
  _munged_data(copy._munged_data),
  _state(copy._state),
  _internal_transform(copy._internal_transform)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::update_type(this, get_class_type());
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::Copy Assignment Operator
//       Access: Public
//  Description: Copies the CullableObject.
////////////////////////////////////////////////////////////////////
INLINE void CullableObject::
operator = (const CullableObject &copy) {
  _geom = copy._geom;
  _munger = copy._munger;
  _munged_data = copy._munged_data;
  _state = copy._state;
  _internal_transform = copy._internal_transform;
  _draw_callback = copy._draw_callback;
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::draw
//       Access: Public
//  Description: Draws the cullable object on the GSG immediately, in
//               the GSG's current state.  This should only be called
//               from the draw thread.
////////////////////////////////////////////////////////////////////
INLINE void CullableObject::
draw(GraphicsStateGuardianBase *gsg, bool force, Thread *current_thread) {
  if (_draw_callback != (CallbackObject *)NULL) {
    // It has a callback associated.
    gsg->clear_before_callback();
    gsg->set_state_and_transform(_state, _internal_transform);
    GeomDrawCallbackData cbdata(this, gsg, force);
    _draw_callback->do_callback(&cbdata);
    if (cbdata.get_lost_state()) {
      // Tell the GSG to forget its state.
      gsg->clear_state_and_transform();
    }
    // Now the callback has taken care of drawing.
  } else {
    nassertv(_geom != (Geom *)NULL);
    gsg->set_state_and_transform(_state, _internal_transform);
    draw_inline(gsg, force, current_thread);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::request_resident
//       Access: Public
//  Description: Returns true if all the data necessary to render this
//               object is currently resident in memory.  If this
//               returns false, the data will be brought back into
//               memory shortly; try again later.
////////////////////////////////////////////////////////////////////
INLINE bool CullableObject::
request_resident() const {
  bool resident = true;
  if (!_geom->request_resident()) {
    resident = false;
  }
  if (!_munged_data->request_resident()) {
    resident = false;
  }
  return resident;
}


////////////////////////////////////////////////////////////////////
//     Function: CullableObject::set_draw_callback
//       Access: Public
//  Description: Specifies a CallbackObject that will be responsible
//               for drawing this object.
////////////////////////////////////////////////////////////////////
INLINE void CullableObject::
set_draw_callback(CallbackObject *draw_callback) {
  _draw_callback = draw_callback;
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
//     Function: CullableObject::draw_inline
//       Access: Private
//  Description: Draws the cullable object on the GSG immediately, in
//               the GSG's current state.  This should only be called
//               from the draw thread.  Assumes the GSG has already
//               been set to the appropriate state.
////////////////////////////////////////////////////////////////////
INLINE void CullableObject::
draw_inline(GraphicsStateGuardianBase *gsg, bool force, Thread *current_thread) {
  _geom->draw(gsg, _munger, _munged_data, force, current_thread);
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

////////////////////////////////////////////////////////////////////
//     Function: CullableObject::SourceFormat::operator <
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool CullableObject::SourceFormat::
operator < (const CullableObject::SourceFormat &other) const {
  if (_format != other._format) {
    return _format < other._format;
  }
  if (_sprite_texcoord != other._sprite_texcoord) {
    return (int)_sprite_texcoord < (int)other._sprite_texcoord;
  }
  if (_retransform_sprites != other._retransform_sprites) {
    return (int)_retransform_sprites < (int)other._retransform_sprites;
  }
  return false;
}
