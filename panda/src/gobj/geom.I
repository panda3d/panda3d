/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geom.I
 * @author drose
 * @date 2005-03-06
 */

/**
 * Returns the fundamental primitive type that is common to all GeomPrimitives
 * added within the Geom.  All nested primitives within a particular Geom must
 * be the same type (that is, you can mix triangles and tristrips, because
 * they are both the same fundamental type PT_polygons, but you cannot mix
 * triangles and points withn the same Geom).
 */
INLINE Geom::PrimitiveType Geom::
get_primitive_type() const {
  CDReader cdata(_cycler);
  return cdata->_primitive_type;
}

/**
 * Returns the shade model common to all of the individual GeomPrimitives that
 * have been added to the geom.
 */
INLINE Geom::ShadeModel Geom::
get_shade_model() const {
  CDReader cdata(_cycler);
  return cdata->_shade_model;
}

/**
 * Returns the set of GeomRendering bits that represent the rendering
 * properties required to properly render this Geom.
 */
INLINE int Geom::
get_geom_rendering() const {
  CDReader cdata(_cycler);
  return cdata->_geom_rendering;
}

/**
 * Returns a const pointer to the GeomVertexData, for application code to
 * directly examine (but not modify) the geom's underlying data.
 */
INLINE CPT(GeomVertexData) Geom::
get_vertex_data(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_data.get_read_pointer(current_thread);
}

/**
 * Returns true if there appear to be no vertices to be rendered by this Geom,
 * false if has some actual data.
 */
INLINE bool Geom::
is_empty() const {
  CDReader cdata(_cycler);
  return cdata->_primitives.empty();
}

/**
 * Returns the number of GeomPrimitive objects stored within the Geom, each of
 * which represents a number of primitives of a particular type.
 */
INLINE size_t Geom::
get_num_primitives() const {
  CDReader cdata(_cycler);
  return cdata->_primitives.size();
}

/**
 * Returns a const pointer to the ith GeomPrimitive object stored within the
 * Geom.  Use this call only to inspect the ith object; use modify_primitive()
 * or set_primitive() if you want to modify it.
 */
INLINE CPT(GeomPrimitive) Geom::
get_primitive(size_t i) const {
  CDReader cdata(_cycler);
  nassertr(i < cdata->_primitives.size(), nullptr);
  return cdata->_primitives[i].get_read_pointer();
}

/**
 * Returns a modifiable pointer to the ith GeomPrimitive object stored within
 * the Geom, so application code can directly manipulate the properties of
 * this primitive.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
INLINE PT(GeomPrimitive) Geom::
modify_primitive(size_t i) {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);
  nassertr(i < cdata->_primitives.size(), nullptr);
  cdata->_modified = Geom::get_next_modified();
  clear_cache_stage(current_thread);
  return cdata->_primitives[i].get_write_pointer();
}

/**
 * Inserts a new GeomPrimitive structure to the Geom object.  This specifies a
 * particular subset of vertices that are used to define geometric primitives
 * of the indicated type.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
INLINE void Geom::
add_primitive(const GeomPrimitive *primitive) {
  insert_primitive((size_t)-1, primitive);
}

/**
 * Decomposes all of the primitives within this Geom, returning the result.
 * See GeomPrimitive::decompose().
 */
INLINE PT(Geom) Geom::
decompose() const {
  PT(Geom) new_geom = make_copy();
  new_geom->decompose_in_place();
  return new_geom;
}

/**
 * Doublesides all of the primitives within this Geom, returning the result.
 * See GeomPrimitive::doubleside().
 */
INLINE PT(Geom) Geom::
doubleside() const {
  PT(Geom) new_geom = make_copy();
  new_geom->doubleside_in_place();
  return new_geom;
}

/**
 * Reverses all of the primitives within this Geom, returning the result.  See
 * GeomPrimitive::reverse().
 */
INLINE PT(Geom) Geom::
reverse() const {
  PT(Geom) new_geom = make_copy();
  new_geom->reverse_in_place();
  return new_geom;
}

/**
 * Rotates all of the primitives within this Geom, returning the result.  See
 * GeomPrimitive::rotate().
 */
INLINE PT(Geom) Geom::
rotate() const {
  PT(Geom) new_geom = make_copy();
  new_geom->rotate_in_place();
  return new_geom;
}

/**
 * Unifies all of the primitives contained within this Geom into a single (or
 * as few as possible, within the constraints of max_indices) primitive
 * objects.  This may require decomposing the primitives if, for instance, the
 * Geom contains both triangle strips and triangle fans.
 *
 * max_indices represents the maximum number of indices that will be put in
 * any one GeomPrimitive.  If preserve_order is true, then the primitives will
 * not be reordered during the operation, even if this results in a suboptimal
 * result.
 */
INLINE PT(Geom) Geom::
unify(int max_indices, bool preserve_order) const {
  PT(Geom) new_geom = make_copy();
  new_geom->unify_in_place(max_indices, preserve_order);
  return new_geom;
}

/**
 * Returns a new Geom with points at all the vertices.  See
 * GeomPrimitive::make_points().
 */
INLINE PT(Geom) Geom::
make_points() const {
  PT(Geom) new_geom = make_copy();
  new_geom->make_points_in_place();
  return new_geom;
}

/**
 * Returns a new Geom with lines at all the edges.  See
 * GeomPrimitive::make_lines().
 */
INLINE PT(Geom) Geom::
make_lines() const {
  PT(Geom) new_geom = make_copy();
  new_geom->make_lines_in_place();
  return new_geom;
}

/**
 * Returns a new Geom with each primitive converted into a patch.  Calls
 * decompose() first.
 */
INLINE PT(Geom) Geom::
make_patches() const {
  PT(Geom) new_geom = make_copy();
  new_geom->make_patches_in_place();
  return new_geom;
}

/**
 * Returns a new Geom with each primitive converted into a corresponding
 * version with adjacency information.
 *
 * @since 1.10.0
 */
INLINE PT(Geom) Geom::
make_adjacency() const {
  PT(Geom) new_geom = make_copy();
  new_geom->make_adjacency_in_place();
  return new_geom;
}

/**
 * Returns a sequence number which is guaranteed to change at least every time
 * any of the primitives in the Geom is modified, or the set of primitives is
 * modified.  However, this does not include modifications to the vertex data,
 * which should be tested separately.
 */
INLINE UpdateSeq Geom::
get_modified(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_modified;
}

/**
 * Marks the bounding volume of the Geom as stale so that it should be
 * recomputed.  Usually it is not necessary to call this explicitly.
 */
INLINE void Geom::
mark_bounds_stale() const {
  CDWriter cdata(((Geom *)this)->_cycler, false);
  ((Geom *)this)->mark_internal_bounds_stale(cdata);
}

/**
 * Specifies the desired type of bounding volume that will be created for this
 * Geom.  This is normally BoundingVolume::BT_default, which means to set the
 * type according to the config variable "bounds-type".
 *
 * If this is BT_sphere or BT_box, a BoundingSphere or BoundingBox is
 * explicitly created.  If it is BT_best, a BoundingBox is created.
 *
 * This affects the implicit bounding volume only.  If an explicit bounding
 * volume is set on the Geom with set_bounds(), that bounding volume type is
 * used.  (This is different behavior from the similar method on PandaNode.)
 */
INLINE void Geom::
set_bounds_type(BoundingVolume::BoundsType bounds_type) {
  CDWriter cdata(_cycler, true);
  cdata->_bounds_type = bounds_type;
  mark_internal_bounds_stale(cdata);
}

/**
 * Returns the bounding volume type set with set_bounds_type().
 */
INLINE BoundingVolume::BoundsType Geom::
get_bounds_type() const {
  CDReader cdata(_cycler);
  return cdata->_bounds_type;
}

/**
 * Resets the bounding volume so that it is the indicated volume.  When it is
 * explicitly set, the bounding volume will no longer be automatically
 * computed; call clear_bounds() if you would like to return the bounding
 * volume to its default behavior.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
INLINE void Geom::
set_bounds(const BoundingVolume *volume) {
  CDWriter cdata(_cycler, true);
  if (volume == nullptr) {
    cdata->_user_bounds = nullptr;
  } else {
    cdata->_user_bounds = volume->make_copy();
  }
}

/**
 * Reverses the effect of a previous call to set_bounds(), and allows the
 * bounding volume to be automatically computed once more based on the
 * vertices.
 *
 * Don't call this in a downstream thread unless you don't mind it blowing
 * away other changes you might have recently made in an upstream thread.
 */
INLINE void Geom::
clear_bounds() {
  CDWriter cdata(_cycler, true);
  cdata->_user_bounds = nullptr;
  mark_internal_bounds_stale(cdata);
}

/**
 * Expands min_point and max_point to include all of the vertices in the Geom,
 * if any.  found_any is set true if any points are found.  It is the caller's
 * responsibility to initialize min_point, max_point, and found_any before
 * calling this function.
 *
 * This version of the method allows the Geom to specify an alternate vertex
 * data table (for instance, if the vertex data has already been munged), and
 * also allows the result to be computed in any coordinate space by specifying
 * a transform matrix.
 */
INLINE void Geom::
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                  bool &found_any,
                  const GeomVertexData *vertex_data,
                  bool got_mat, const LMatrix4 &mat,
                  Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);

  PN_stdfloat sq_radius;
  do_calc_tight_bounds(min_point, max_point, sq_radius, found_any,
                       vertex_data, got_mat, mat,
                       InternalName::get_vertex(),
                       cdata, current_thread);
}

/**
 * Expands min_point and max_point to include all of the vertices in the Geom,
 * if any.  found_any is set true if any points are found.  It is the caller's
 * responsibility to initialize min_point, max_point, and found_any before
 * calling this function.
 *
 * This version of the method assumes the Geom will use its own vertex data,
 * and the results are computed in the Geom's own coordinate space.
 */
INLINE void Geom::
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                  bool &found_any, Thread *current_thread) const {

  calc_tight_bounds(min_point, max_point, found_any,
                    get_vertex_data(current_thread), false,
                    LMatrix4::ident_mat(),
                    current_thread);
}

/**
 * Similar to calc_tight_bounds(), for UV coordinates or other named columns.
 */
INLINE void Geom::
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                  bool &found_any,
                  const GeomVertexData *vertex_data,
                  bool got_mat, const LMatrix4 &mat,
                  const InternalName *column_name,
                  Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);

  PN_stdfloat sq_radius;
  do_calc_tight_bounds(min_point, max_point, sq_radius, found_any,
                       vertex_data, got_mat, mat,
                       column_name, cdata, current_thread);
}

/**
 * Should be called to mark the internal bounding volume stale, so that
 * recompute_internal_bounds() will be called when the bounding volume is next
 * requested.
 */
INLINE void Geom::
mark_internal_bounds_stale(CData *cdata) {
  cdata->_internal_bounds_stale = true;
}

/**
 *
 */
INLINE Geom::CDataCache::
CDataCache() :
  _source(nullptr),
  _geom_result(nullptr),
  _data_result(nullptr)
{
}

/**
 *
 */
INLINE Geom::CDataCache::
CDataCache(const Geom::CDataCache &copy) :
  _source(copy._source),
  _geom_result(copy._geom_result),
  _data_result(copy._data_result)
{
  if (_geom_result != _source && _geom_result != nullptr) {
    _geom_result->ref();
  }
}

/**
 * Stores the geom_result and data_result on the cache, upping and/or dropping
 * the reference count appropriately.
 */
INLINE void Geom::CDataCache::
set_result(const Geom *geom_result, const GeomVertexData *data_result) {
  if (geom_result != _geom_result) {
    if (_geom_result != _source && _geom_result != nullptr) {
      unref_delete(_geom_result);
    }
    _geom_result = geom_result;
    if (_geom_result != _source && _geom_result != nullptr) {
      _geom_result->ref();
    }
  }
  _data_result = data_result;
}

/**
 *
 */
INLINE Geom::CacheKey::
CacheKey(const GeomVertexData *source_data, const GeomMunger *modifier) :
  _source_data(source_data),
  _modifier(modifier)
{
}

/**
 *
 */
INLINE Geom::CacheKey::
CacheKey(const CacheKey &copy) :
  _source_data(copy._source_data),
  _modifier(copy._modifier)
{
}

/**
 *
 */
INLINE Geom::CacheKey::
CacheKey(CacheKey &&from) noexcept :
  _source_data(std::move(from._source_data)),
  _modifier(std::move(from._modifier))
{
}

/**
 * Provides a unique ordering within the map.
 */
INLINE bool Geom::CacheKey::
operator < (const CacheKey &other) const {
  if (_modifier != other._modifier) {
    int compare = _modifier->geom_compare_to(*other._modifier);
    if (compare != 0) {
      return (compare < 0);
    }
  }
  if (_source_data != other._source_data) {
    return (_source_data < other._source_data);
  }
  return 0;
}

/**
 *
 */
INLINE Geom::CacheEntry::
CacheEntry(Geom *source, const GeomVertexData *source_data,
           const GeomMunger *modifier) :
  _source(source),
  _key(source_data, modifier)
{
}

/**
 *
 */
INLINE Geom::CacheEntry::
CacheEntry(Geom *source, const Geom::CacheKey &key) :
  _source(source),
  _key(key)
{
}

/**
 *
 */
INLINE Geom::CacheEntry::
CacheEntry(Geom *source, Geom::CacheKey &&key) noexcept :
  _source(source),
  _key(std::move(key))
{
}

/**
 *
 */
INLINE Geom::CData::
CData() :
  _primitive_type(PT_none),
  _shade_model(SM_uniform),
  _geom_rendering(0),
  _nested_vertices(0),
  _internal_bounds_stale(true),
  _bounds_type(BoundingVolume::BT_default)
{
}

/**
 *
 */
INLINE Geom::CData::
CData(GeomVertexData *data) :
  _data(data),
  _primitive_type(PT_none),
  _shade_model(SM_uniform),
  _geom_rendering(0),
  _nested_vertices(0),
  _internal_bounds_stale(true),
  _bounds_type(BoundingVolume::BT_default)
{
}

/**
 *
 */
INLINE GeomPipelineReader::
GeomPipelineReader(Thread *current_thread) :
  _object(nullptr),
  _current_thread(current_thread),
  _cdata(nullptr)
{
}

/**
 *
 */
INLINE GeomPipelineReader::
GeomPipelineReader(const Geom *object, Thread *current_thread) :
  _object(object),
  _current_thread(current_thread),
  _cdata(object->_cycler.read_unlocked(current_thread))
{
#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#endif // _DEBUG
#ifdef DO_PIPELINING
  _cdata->ref();
#endif  // DO_PIPELINING
}

/**
 *
 */
INLINE GeomPipelineReader::
~GeomPipelineReader() {
#ifdef _DEBUG
  if (_object != nullptr) {
    nassertv(_object->test_ref_count_nonzero());
  }
#endif // _DEBUG
  // _object->_cycler.release_read(_cdata);

#ifdef DO_PIPELINING
  if (_cdata != nullptr) {
    unref_delete((CycleData *)_cdata);
  }
#endif  // DO_PIPELINING

#ifdef _DEBUG
  _object = nullptr;
  _cdata = nullptr;
#endif  // _DEBUG
}

/**
 *
 */
INLINE void GeomPipelineReader::
set_object(const Geom *object) {
  if (object != _object) {
    // _object->_cycler.release_read(_cdata);

#ifdef DO_PIPELINING
    if (_cdata != nullptr) {
      unref_delete((CycleData *)_cdata);
    }
#endif  // DO_PIPELINING

    _cdata = object->_cycler.read_unlocked(_current_thread);

#ifdef DO_PIPELINING
    _cdata->ref();
#endif  // DO_PIPELINING

    _object = object;
  }
}

/**
 *
 */
INLINE const Geom *GeomPipelineReader::
get_object() const {
  return _object;
}

/**
 *
 */
INLINE Thread *GeomPipelineReader::
get_current_thread() const {
  return _current_thread;
}

/**
 *
 */
INLINE GeomPipelineReader::PrimitiveType GeomPipelineReader::
get_primitive_type() const {
  return _cdata->_primitive_type;
}

/**
 *
 */
INLINE GeomPipelineReader::ShadeModel GeomPipelineReader::
get_shade_model() const {
  return _cdata->_shade_model;
}

/**
 *
 */
INLINE int GeomPipelineReader::
get_geom_rendering() const {
  return _cdata->_geom_rendering;
}

/**
 *
 */
INLINE CPT(GeomVertexData) GeomPipelineReader::
get_vertex_data() const {
  return _cdata->_data.get_read_pointer();
}

/**
 *
 */
INLINE int GeomPipelineReader::
get_num_primitives() const {
  return _cdata->_primitives.size();
}

/**
 *
 */
INLINE CPT(GeomPrimitive) GeomPipelineReader::
get_primitive(int i) const {
  nassertr(i >= 0 && i < (int)_cdata->_primitives.size(), nullptr);
  return _cdata->_primitives[i].get_read_pointer();
}

/**
 *
 */
INLINE UpdateSeq GeomPipelineReader::
get_modified() const {
  return _cdata->_modified;
}

/**
 * Creates a context for the geom on the particular GSG, if it does not
 * already exist.  Returns the new (or old) GeomContext.  This assumes that
 * the GraphicsStateGuardian is the currently active rendering context and
 * that it is ready to accept new geoms.  If this is not necessarily the case,
 * you should use prepare() instead.
 *
 * Normally, this is not called directly except by the GraphicsStateGuardian;
 * a geom does not need to be explicitly prepared by the user before it may be
 * rendered.
 */
INLINE GeomContext *GeomPipelineReader::
prepare_now(PreparedGraphicsObjects *prepared_objects,
            GraphicsStateGuardianBase *gsg) const {
  return ((Geom *)_object)->prepare_now(prepared_objects, gsg);
}

INLINE std::ostream &
operator << (std::ostream &out, const Geom &obj) {
  obj.output(out);
  return out;
}
