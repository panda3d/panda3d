/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullableObject.I
 * @author drose
 * @date 2002-03-04
 */

/**
 * Creates an empty CullableObject whose pointers can be filled in later.
 */
INLINE CullableObject::
CullableObject() {
#ifdef DO_MEMORY_USAGE
  MemoryUsage::record_pointer(this, get_class_type());
#endif
}

/**
 * Creates a CullableObject based the indicated geom, with the indicated
 * render state and transform.
 */
INLINE CullableObject::
CullableObject(CPT(Geom) geom, CPT(RenderState) state,
               CPT(TransformState) internal_transform) :
  _geom(std::move(geom)),
  _state(std::move(state)),
  _internal_transform(std::move(internal_transform))
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::record_pointer(this, get_class_type());
#endif
}

/**
 * Copies the CullableObject.
 */
INLINE CullableObject::
CullableObject(const CullableObject &copy) :
  _geom(copy._geom),
  _munged_data(copy._munged_data),
  _state(copy._state),
  _internal_transform(copy._internal_transform)
{
#ifdef DO_MEMORY_USAGE
  MemoryUsage::record_pointer(this, get_class_type());
#endif
}

/**
 * Copies the CullableObject.
 */
INLINE void CullableObject::
operator = (const CullableObject &copy) {
  _geom = copy._geom;
  _munged_data = copy._munged_data;
  _state = copy._state;
  _internal_transform = copy._internal_transform;
  _draw_callback = copy._draw_callback;
}

/**
 * Draws the cullable object on the GSG immediately, in the GSG's current
 * state.  This should only be called from the draw thread.
 */
INLINE void CullableObject::
draw(GraphicsStateGuardianBase *gsg, bool force, Thread *current_thread) {
  if (_draw_callback != nullptr) {
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
    nassertv(_geom != nullptr);
    gsg->set_state_and_transform(_state, _internal_transform);
    draw_inline(gsg, force, current_thread);
  }
}

/**
 * Returns true if all the data necessary to render this object is currently
 * resident in memory.  If this returns false, the data will be brought back
 * into memory shortly; try again later.
 */
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


/**
 * Specifies a CallbackObject that will be responsible for drawing this
 * object.
 */
INLINE void CullableObject::
set_draw_callback(CallbackObject *draw_callback) {
  _draw_callback = draw_callback;
}

/**
 * Flushes the PStatCollectors used during traversal.
 */
INLINE void CullableObject::
flush_level() {
  _sw_sprites_pcollector.flush_level();
}

/**
 * Draws the cullable object on the GSG immediately, in the GSG's current
 * state.  This should only be called from the draw thread.  Assumes the GSG
 * has already been set to the appropriate state.
 */
INLINE void CullableObject::
draw_inline(GraphicsStateGuardianBase *gsg, bool force, Thread *current_thread) {
  _geom->draw(gsg, _munged_data, force, current_thread);
}

/**
 * Invokes the draw callback, assuming one is set.  Crashes if not.
 */
INLINE void CullableObject::
draw_callback(GraphicsStateGuardianBase *gsg, bool force, Thread *current_thread) {
  gsg->clear_before_callback();
  gsg->set_state_and_transform(_state, _internal_transform);
  GeomDrawCallbackData cbdata(this, gsg, force);
  _draw_callback->do_callback(&cbdata);
  if (cbdata.get_lost_state()) {
    // Tell the GSG to forget its state.
    gsg->clear_state_and_transform();
  }
}

/**
 *
 */
INLINE CullableObject::SortPoints::
SortPoints(const CullableObject::PointData *array) :
  _array(array)
{
}

/**
 * Orders the points from back-to-front for correct transparency sorting in
 * munge_points_to_quads
 */
INLINE bool CullableObject::SortPoints::
operator () (unsigned short a, unsigned short b) const {
  return _array[a]._dist > _array[b]._dist;
}

/**
 *
 */
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
