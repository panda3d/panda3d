// Filename: geom.I
// Created by:  drose (06Mar05)
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
//     Function: Geom::get_primitive_type
//       Access: Published
//  Description: Returns the fundamental primitive type that is common
//               to all GeomPrimitives added within the Geom.  All
//               nested primitives within a particular Geom must be
//               the same type (that is, you can mix triangles and
//               tristrips, because they are both the same fundamental
//               type PT_polygons, but you cannot mix triangles and
//               points withn the same Geom).
////////////////////////////////////////////////////////////////////
INLINE Geom::PrimitiveType Geom::
get_primitive_type() const {
  CDReader cdata(_cycler);
  return cdata->_primitive_type;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_shade_model
//       Access: Published
//  Description: Returns the shade model common to all of the
//               individual GeomPrimitives that have been added to the
//               geom.
////////////////////////////////////////////////////////////////////
INLINE Geom::ShadeModel Geom::
get_shade_model() const {
  CDReader cdata(_cycler);
  return cdata->_shade_model;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_geom_rendering
//       Access: Published
//  Description: Returns the set of GeomRendering bits that represent
//               the rendering properties required to properly render
//               this Geom.
////////////////////////////////////////////////////////////////////
INLINE int Geom::
get_geom_rendering() const {
  CDReader cdata(_cycler);
  return cdata->_geom_rendering;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_usage_hint
//       Access: Published
//  Description: Returns the minimum (i.e. most dynamic) usage_hint
//               among all of the individual GeomPrimitives that have
//               been added to the geom.
////////////////////////////////////////////////////////////////////
INLINE Geom::UsageHint Geom::
get_usage_hint() const {
  CDLockedReader cdata(_cycler);
  if (!cdata->_got_usage_hint) {
    CDWriter cdataw(((Geom *)this)->_cycler, cdata, false);
    ((Geom *)this)->reset_usage_hint(cdataw);
    return cdataw->_usage_hint;
  }
  return cdata->_usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_vertex_data
//       Access: Published
//  Description: Returns a const pointer to the GeomVertexData,
//               for application code to directly examine (but not
//               modify) the geom's underlying data.
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomVertexData) Geom::
get_vertex_data(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_data.get_read_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::is_empty
//       Access: Published
//  Description: Returns true if there appear to be no vertices to be
//               rendered by this Geom, false if has some actual data.
////////////////////////////////////////////////////////////////////
INLINE bool Geom::
is_empty() const {
  CDReader cdata(_cycler);
  return cdata->_primitives.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_num_primitives
//       Access: Published
//  Description: Returns the number of GeomPrimitive objects stored
//               within the Geom, each of which represents a number of
//               primitives of a particular type.
////////////////////////////////////////////////////////////////////
INLINE int Geom::
get_num_primitives() const {
  CDReader cdata(_cycler);
  return cdata->_primitives.size();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_primitive
//       Access: Published
//  Description: Returns a const pointer to the ith GeomPrimitive
//               object stored within the Geom.  Use this call only to
//               inspect the ith object; use modify_primitive() or
//               set_primitive() if you want to modify it.
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomPrimitive) Geom::
get_primitive(int i) const {
  CDReader cdata(_cycler);
  nassertr(i >= 0 && i < (int)cdata->_primitives.size(), NULL);
  return cdata->_primitives[i].get_read_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::modify_primitive
//       Access: Published
//  Description: Returns a modifiable pointer to the ith GeomPrimitive
//               object stored within the Geom, so application code
//               can directly manipulate the properties of this
//               primitive.
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
INLINE PT(GeomPrimitive) Geom::
modify_primitive(int i) {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);
  nassertr(i >= 0 && i < (int)cdata->_primitives.size(), NULL);
  cdata->_got_usage_hint = false;
  cdata->_modified = Geom::get_next_modified();
  clear_cache_stage(current_thread);
  return cdata->_primitives[i].get_write_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::decompose
//       Access: Published
//  Description: Decomposes all of the primitives within this Geom,
//               returning the result.  See
//               GeomPrimitive::decompose().
////////////////////////////////////////////////////////////////////
INLINE PT(Geom) Geom::
decompose() const {
  PT(Geom) new_geom = make_copy();
  new_geom->decompose_in_place();
  return new_geom;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::doubleside
//       Access: Published
//  Description: Doublesides all of the primitives within this Geom,
//               returning the result.  See
//               GeomPrimitive::doubleside().
////////////////////////////////////////////////////////////////////
INLINE PT(Geom) Geom::
doubleside() const {
  PT(Geom) new_geom = make_copy();
  new_geom->doubleside_in_place();
  return new_geom;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::reverse
//       Access: Published
//  Description: Reverses all of the primitives within this Geom,
//               returning the result.  See
//               GeomPrimitive::reverse().
////////////////////////////////////////////////////////////////////
INLINE PT(Geom) Geom::
reverse() const {
  PT(Geom) new_geom = make_copy();
  new_geom->reverse_in_place();
  return new_geom;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::rotate
//       Access: Published
//  Description: Rotates all of the primitives within this Geom,
//               returning the result.  See
//               GeomPrimitive::rotate().
////////////////////////////////////////////////////////////////////
INLINE PT(Geom) Geom::
rotate() const {
  PT(Geom) new_geom = make_copy();
  new_geom->rotate_in_place();
  return new_geom;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::unify
//       Access: Published
//  Description: Unifies all of the primitives contained within this
//               Geom into a single (or as few as possible, within the
//               constraints of max_indices) primitive objects.  This
//               may require decomposing the primitives if, for
//               instance, the Geom contains both triangle strips and
//               triangle fans.
//
//               max_indices represents the maximum number of indices
//               that will be put in any one GeomPrimitive.  If
//               preserve_order is true, then the primitives will not
//               be reordered during the operation, even if this
//               results in a suboptimal result.
////////////////////////////////////////////////////////////////////
INLINE PT(Geom) Geom::
unify(int max_indices, bool preserve_order) const {
  PT(Geom) new_geom = make_copy();
  new_geom->unify_in_place(max_indices, preserve_order);
  return new_geom;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::make_points
//       Access: Published
//  Description: Returns a new Geom with points at all the vertices.
//               See GeomPrimitive::make_points().
////////////////////////////////////////////////////////////////////
INLINE PT(Geom) Geom::
make_points() const {
  PT(Geom) new_geom = make_copy();
  new_geom->make_points_in_place();
  return new_geom;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::make_lines
//       Access: Published
//  Description: Returns a new Geom with lines at all the edges.
//               See GeomPrimitive::make_lines().
////////////////////////////////////////////////////////////////////
INLINE PT(Geom) Geom::
make_lines() const {
  PT(Geom) new_geom = make_copy();
  new_geom->make_lines_in_place();
  return new_geom;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::make_patches
//       Access: Published
//  Description: Returns a new Geom with each primitive converted
//               into a patch.  Calls decompose() first.
////////////////////////////////////////////////////////////////////
INLINE PT(Geom) Geom::
make_patches() const {
  PT(Geom) new_geom = make_copy();
  new_geom->make_patches_in_place();
  return new_geom;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_modified
//       Access: Published
//  Description: Returns a sequence number which is guaranteed to
//               change at least every time any of the primitives in
//               the Geom is modified, or the set of primitives is
//               modified.  However, this does not include
//               modifications to the vertex data, which should be
//               tested separately.
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq Geom::
get_modified(Thread *current_thread) const {
  CDReader cdata(_cycler, current_thread);
  return cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::mark_bounds_stale
//       Access: Published
//  Description: Marks the bounding volume of the Geom as stale so
//               that it should be recomputed.  Usually it is not
//               necessary to call this explicitly.
////////////////////////////////////////////////////////////////////
INLINE void Geom::
mark_bounds_stale() const {
  CDWriter cdata(((Geom *)this)->_cycler, false);
  ((Geom *)this)->mark_internal_bounds_stale(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::set_bounds_type
//       Access: Published
//  Description: Specifies the desired type of bounding volume that
//               will be created for this Geom.  This is normally
//               BoundingVolume::BT_default, which means to set the
//               type according to the config variable "bounds-type".
//
//               If this is BT_sphere or BT_box, a BoundingSphere or
//               BoundingBox is explicitly created.  If it is BT_best,
//               a BoundingBox is created.
//
//               This affects the implicit bounding volume only.  If
//               an explicit bounding volume is set on the Geom with
//               set_bounds(), that bounding volume type is used.
//               (This is different behavior from the similar method
//               on PandaNode.)
////////////////////////////////////////////////////////////////////
INLINE void Geom::
set_bounds_type(BoundingVolume::BoundsType bounds_type) {
  CDWriter cdata(_cycler, true);
  cdata->_bounds_type = bounds_type;
  mark_internal_bounds_stale(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_bounds_type
//       Access: Published
//  Description: Returns the bounding volume type set with
//               set_bounds_type().
////////////////////////////////////////////////////////////////////
INLINE BoundingVolume::BoundsType Geom::
get_bounds_type() const {
  CDReader cdata(_cycler);
  return cdata->_bounds_type;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::set_bounds
//       Access: Published
//  Description: Resets the bounding volume so that it is the
//               indicated volume.  When it is explicitly set, the
//               bounding volume will no longer be automatically
//               computed; call clear_bounds() if you would like to
//               return the bounding volume to its default behavior.
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
INLINE void Geom::
set_bounds(const BoundingVolume *volume) {
  CDWriter cdata(_cycler, true);
  if (volume == NULL) {
    cdata->_user_bounds = NULL;
  } else {
    cdata->_user_bounds = volume->make_copy();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::clear_bounds
//       Access: Published
//  Description: Reverses the effect of a previous call to
//               set_bounds(), and allows the bounding volume to be
//               automatically computed once more based on the
//               vertices.
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
INLINE void Geom::
clear_bounds() {
  CDWriter cdata(_cycler, true);
  cdata->_user_bounds = NULL;
  mark_internal_bounds_stale(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::calc_tight_bounds
//       Access: Public
//  Description: Expands min_point and max_point to include all of the
//               vertices in the Geom, if any.  found_any is set true
//               if any points are found.  It is the caller's
//               responsibility to initialize min_point, max_point,
//               and found_any before calling this function.
//
//               This version of the method allows the Geom to specify
//               an alternate vertex data table (for instance, if the
//               vertex data has already been munged), and also allows
//               the result to be computed in any coordinate space by
//               specifying a transform matrix.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: Geom::calc_tight_bounds
//       Access: Public, Virtual
//  Description: Expands min_point and max_point to include all of the
//               vertices in the Geom, if any.  found_any is set true
//               if any points are found.  It is the caller's
//               responsibility to initialize min_point, max_point,
//               and found_any before calling this function.
//
//               This version of the method assumes the Geom will use
//               its own vertex data, and the results are computed in
//               the Geom's own coordinate space.
////////////////////////////////////////////////////////////////////
INLINE void Geom::
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                  bool &found_any, Thread *current_thread) const {

  calc_tight_bounds(min_point, max_point, found_any,
                    get_vertex_data(current_thread), false,
                    LMatrix4::ident_mat(),
                    current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::calc_tight_bounds
//       Access: Public
//  Description: Similar to calc_tight_bounds(), for UV coordinates or
//               other named columns.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: Geom::mark_internal_bounds_stale
//       Access: Private
//  Description: Should be called to mark the internal bounding
//               volume stale, so that recompute_internal_bounds()
//               will be called when the bounding volume is next
//               requested.
////////////////////////////////////////////////////////////////////
INLINE void Geom::
mark_internal_bounds_stale(CData *cdata) {
  cdata->_internal_bounds_stale = true;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CDataCache::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Geom::CDataCache::
CDataCache() :
  _source(NULL),
  _geom_result(NULL),
  _data_result(NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CDataCache::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Geom::CDataCache::
CDataCache(const Geom::CDataCache &copy) :
  _source(copy._source),
  _geom_result(copy._geom_result),
  _data_result(copy._data_result)
{
  if (_geom_result != _source && _geom_result != (Geom *)NULL) {
    _geom_result->ref();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CDataCache::set_result
//       Access: Public
//  Description: Stores the geom_result and data_result on the cache,
//               upping and/or dropping the reference count
//               appropriately.
////////////////////////////////////////////////////////////////////
INLINE void Geom::CDataCache::
set_result(const Geom *geom_result, const GeomVertexData *data_result) {
  if (geom_result != _geom_result) {
    if (_geom_result != _source && _geom_result != (Geom *)NULL) {
      unref_delete(_geom_result);
    }
    _geom_result = geom_result;
    if (_geom_result != _source && _geom_result != (Geom *)NULL) {
      _geom_result->ref();
    }
  }
  _data_result = data_result;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CacheKey::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Geom::CacheKey::
CacheKey(const GeomVertexData *source_data, const GeomMunger *modifier) :
  _source_data(source_data),
  _modifier(modifier)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CacheKey::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Geom::CacheKey::
CacheKey(const CacheKey &copy) :
  _source_data(copy._source_data),
  _modifier(copy._modifier)
{
}

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: Geom::CacheKey::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Geom::CacheKey::
CacheKey(CacheKey &&from) NOEXCEPT :
  _source_data(move(from._source_data)),
  _modifier(move(from._modifier))
{
}
#endif  // USE_MOVE_SEMANTICS

////////////////////////////////////////////////////////////////////
//     Function: Geom::CacheKey::operator <
//       Access: Public
//  Description: Provides a unique ordering within the map.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: Geom::CacheEntry::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Geom::CacheEntry::
CacheEntry(Geom *source, const GeomVertexData *source_data,
           const GeomMunger *modifier) :
  _source(source),
  _key(source_data, modifier)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CacheEntry::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Geom::CacheEntry::
CacheEntry(Geom *source, const Geom::CacheKey &key) :
  _source(source),
  _key(key)
{
}

#ifdef USE_MOVE_SEMANTICS
////////////////////////////////////////////////////////////////////
//     Function: Geom::CacheEntry::Move Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Geom::CacheEntry::
CacheEntry(Geom *source, Geom::CacheKey &&key) NOEXCEPT :
  _source(source),
  _key(move(key))
{
}
#endif  // USE_MOVE_SEMANTICS

////////////////////////////////////////////////////////////////////
//     Function: Geom::CData::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Geom::CData::
CData() :
  _primitive_type(PT_none),
  _shade_model(SM_uniform),
  _geom_rendering(0),
  _usage_hint(UH_unspecified),
  _got_usage_hint(false),
  _nested_vertices(0),
  _internal_bounds_stale(true),
  _bounds_type(BoundingVolume::BT_default)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CData::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Geom::CData::
CData(const Geom::CData &copy) :
  _data(copy._data),
  _primitives(copy._primitives),
  _primitive_type(copy._primitive_type),
  _shade_model(copy._shade_model),
  _geom_rendering(copy._geom_rendering),
  _usage_hint(copy._usage_hint),
  _got_usage_hint(copy._got_usage_hint),
  _modified(copy._modified),
  _internal_bounds(copy._internal_bounds),
  _nested_vertices(copy._nested_vertices),
  _internal_bounds_stale(copy._internal_bounds_stale),
  _bounds_type(copy._bounds_type),
  _user_bounds(copy._user_bounds)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::Copy Constructor
//       Access: Private
//  Description: Don't attempt to copy these objects.
////////////////////////////////////////////////////////////////////
INLINE GeomPipelineReader::
GeomPipelineReader(const GeomPipelineReader &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::Copy Assignment Operator
//       Access: Private
//  Description: Don't attempt to copy these objects.
////////////////////////////////////////////////////////////////////
INLINE void GeomPipelineReader::
operator = (const GeomPipelineReader &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomPipelineReader::
~GeomPipelineReader() {
#ifdef _DEBUG
  nassertv(_object->test_ref_count_nonzero());
#endif // _DEBUG
  //  _object->_cycler.release_read(_cdata);

#ifdef DO_PIPELINING
  unref_delete((CycleData *)_cdata);
#endif  // DO_PIPELINING

#ifdef _DEBUG
  _object = NULL;
  _cdata = NULL;
#endif  // _DEBUG
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::get_object
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const Geom *GeomPipelineReader::
get_object() const {
  return _object;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::get_current_thread
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Thread *GeomPipelineReader::
get_current_thread() const {
  return _current_thread;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::get_primitive_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomPipelineReader::PrimitiveType GeomPipelineReader::
get_primitive_type() const {
  return _cdata->_primitive_type;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::get_shade_model
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomPipelineReader::ShadeModel GeomPipelineReader::
get_shade_model() const {
  return _cdata->_shade_model;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::get_geom_rendering
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int GeomPipelineReader::
get_geom_rendering() const {
  return _cdata->_geom_rendering;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::get_usage_hint
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE GeomPipelineReader::UsageHint GeomPipelineReader::
get_usage_hint() const {
  nassertr(_cdata->_got_usage_hint, UH_static);
  return _cdata->_usage_hint;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::get_vertex_data
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomVertexData) GeomPipelineReader::
get_vertex_data() const {
  return _cdata->_data.get_read_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::get_num_primitives
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE int GeomPipelineReader::
get_num_primitives() const {
  return _cdata->_primitives.size();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::get_primitive
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CPT(GeomPrimitive) GeomPipelineReader::
get_primitive(int i) const {
  nassertr(i >= 0 && i < (int)_cdata->_primitives.size(), NULL);
  return _cdata->_primitives[i].get_read_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::get_modified
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE UpdateSeq GeomPipelineReader::
get_modified() const {
  return _cdata->_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::prepare_now
//       Access: Public
//  Description: Creates a context for the geom on the particular
//               GSG, if it does not already exist.  Returns the new
//               (or old) GeomContext.  This assumes that the
//               GraphicsStateGuardian is the currently active
//               rendering context and that it is ready to accept new
//               geoms.  If this is not necessarily the case, you
//               should use prepare() instead.
//
//               Normally, this is not called directly except by the
//               GraphicsStateGuardian; a geom does not need to be
//               explicitly prepared by the user before it may be
//               rendered.
////////////////////////////////////////////////////////////////////
INLINE GeomContext *GeomPipelineReader::
prepare_now(PreparedGraphicsObjects *prepared_objects,
            GraphicsStateGuardianBase *gsg) const {
  return ((Geom *)_object.p())->prepare_now(prepared_objects, gsg);
}

INLINE ostream &
operator << (ostream &out, const Geom &obj) {
  obj.output(out);
  return out;
}
