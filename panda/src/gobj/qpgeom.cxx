// Filename: qpgeom.cxx
// Created by:  drose (06Mar05)
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

#include "qpgeom.h"
#include "qpgeomPoints.h"
#include "qpgeomVertexReader.h"
#include "qpgeomVertexRewriter.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"

UpdateSeq qpGeom::_next_modified;
TypeHandle qpGeom::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeom::
qpGeom() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::Copy Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeom::
qpGeom(const qpGeom &copy) :
  /*
  TypedWritableReferenceCount(copy),
  BoundedObject(copy),
  */
  Geom(copy),
  _cycler(copy._cycler)  
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void qpGeom::
operator = (const qpGeom &copy) {
  /*
  TypedWritableReferenceCount::operator = (copy);
  BoundedObject::operator = (copy);
  */
  clear_cache();
  Geom::operator = (copy);
  _cycler = copy._cycler;
  mark_bound_stale();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeom::
~qpGeom() {
  // When we destruct, we should ensure that all of our cached
  // entries, across all pipeline stages, are properly removed from
  // the cache manager.
  int num_stages = _cycler.get_num_stages();
  for (int i = 0; i < num_stages; i++) {
    if (_cycler.is_stage_unique(i)) {
      CData *cdata = _cycler.write_stage(i);
      for (Cache::iterator ci = cdata->_cache.begin();
           ci != cdata->_cache.end();
           ++ci) {
        CacheEntry *entry = (*ci);
        entry->erase();
      }
      cdata->_cache.clear();
      _cycler.release_write_stage(i, cdata);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::make_copy
//       Access: Published, Virtual
//  Description: Temporarily redefined from Geom base class.
////////////////////////////////////////////////////////////////////
Geom *qpGeom::
make_copy() const {
  return new qpGeom(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::set_usage_hint
//       Access: Published
//  Description: Changes the UsageHint hint for all of the primitives
//               on this Geom to the same value.  See
//               get_usage_hint().
////////////////////////////////////////////////////////////////////
void qpGeom::
set_usage_hint(qpGeom::UsageHint usage_hint) {
  clear_cache();
  CDWriter cdata(_cycler);
  cdata->_usage_hint = usage_hint;

  Primitives::iterator pi;
  for (pi = cdata->_primitives.begin(); pi != cdata->_primitives.end(); ++pi) {
    if ((*pi)->get_ref_count() > 1) {
      (*pi) = (*pi)->make_copy();
    }
    (*pi)->set_usage_hint(usage_hint);
  }

  cdata->_modified = qpGeom::get_next_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::modify_vertex_data
//       Access: Published
//  Description: Returns a modifiable pointer to the GeomVertexData,
//               so that application code may directly maniuplate the
//               geom's underlying data.
////////////////////////////////////////////////////////////////////
PT(qpGeomVertexData) qpGeom::
modify_vertex_data() {
  // Perform copy-on-write: if the reference count on the vertex data
  // is greater than 1, assume some other Geom has the same pointer,
  // so make a copy of it first.
  clear_cache();
  CDWriter cdata(_cycler);
  if (cdata->_data->get_ref_count() > 1) {
    cdata->_data = new qpGeomVertexData(*cdata->_data);
  }
  mark_bound_stale();
  return cdata->_data;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::set_vertex_data
//       Access: Published
//  Description: Replaces the Geom's underlying vertex data table with
//               a completely new table.
////////////////////////////////////////////////////////////////////
void qpGeom::
set_vertex_data(const qpGeomVertexData *data) {
  nassertv(check_will_be_valid(data));
  clear_cache();
  CDWriter cdata(_cycler);
  cdata->_data = (qpGeomVertexData *)data;
  mark_bound_stale();
  reset_geom_rendering(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::offset_vertices
//       Access: Published
//  Description: Replaces a Geom's vertex table with a new table, and
//               simultaneously adds the indicated offset to all
//               vertex references within the Geom's primitives.  This
//               is intended to be used to combine multiple
//               GeomVertexDatas from different Geoms into a single
//               big buffer, with each Geom referencing a subset of
//               the vertices in the buffer.
////////////////////////////////////////////////////////////////////
void qpGeom::
offset_vertices(const qpGeomVertexData *data, int offset) {
  clear_cache();
  CDWriter cdata(_cycler);
  cdata->_data = (qpGeomVertexData *)data;

#ifndef NDEBUG
  bool all_is_valid = true;
#endif
  Primitives::iterator pi;
  for (pi = cdata->_primitives.begin(); pi != cdata->_primitives.end(); ++pi) {
    if ((*pi)->get_ref_count() > 1) {
      (*pi) = (*pi)->make_copy();
    }
    (*pi)->offset_vertices(offset);

#ifndef NDEBUG
    if (!(*pi)->check_valid(data)) {
      all_is_valid = false;
    }
#endif
  }

  cdata->_modified = qpGeom::get_next_modified();
  nassertv(all_is_valid);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::make_nonindexed
//       Access: Published
//  Description: Converts the geom from indexed to nonindexed by
//               duplicating vertices as necessary.  If composite_only
//               is true, then only composite primitives such as
//               trifans and tristrips are converted.  Returns the
//               number of GeomPrimitive objects converted.
////////////////////////////////////////////////////////////////////
int qpGeom::
make_nonindexed(bool composite_only) {
  int num_changed = 0;

  clear_cache();
  CDWriter cdata(_cycler);
  CPT(qpGeomVertexData) orig_data = cdata->_data;
  PT(qpGeomVertexData) new_data = new qpGeomVertexData(*cdata->_data);
  new_data->clear_rows();

#ifndef NDEBUG
  bool all_is_valid = true;
#endif
  Primitives::iterator pi;
  Primitives new_prims;
  new_prims.reserve(cdata->_primitives.size());
  for (pi = cdata->_primitives.begin(); pi != cdata->_primitives.end(); ++pi) {
    PT(qpGeomPrimitive) primitive = (*pi)->make_copy();
    new_prims.push_back(primitive);

    // GeomPoints are considered "composite" for the purposes of
    // making nonindexed, since there's no particular advantage to
    // having indexed points (as opposed to, say, indexed triangles or
    // indexed lines).
    if (primitive->is_indexed() && 
        (primitive->is_composite() || 
         primitive->is_exact_type(qpGeomPoints::get_class_type()) || 
         !composite_only)) {
      primitive->make_nonindexed(new_data, orig_data);
      ++num_changed;
    } else {
      // If it's a simple primitive, pack it anyway, so it can share
      // the same GeomVertexData.
      primitive->pack_vertices(new_data, orig_data);
    }

#ifndef NDEBUG
    if (!primitive->check_valid(new_data)) {
      all_is_valid = false;
    }
#endif
  }

  nassertr(all_is_valid, 0);

  if (num_changed != 0) {
    // If any at all were changed, then keep the result (otherwise,
    // discard it, since we might have de-optimized the indexed
    // geometry a bit).
    cdata->_data = new_data;
    cdata->_primitives.swap(new_prims);
    cdata->_modified = qpGeom::get_next_modified();
  }

  return num_changed;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::set_primitive
//       Access: Published
//  Description: Replaces the ith GeomPrimitive object stored within
//               the Geom with the new object.
////////////////////////////////////////////////////////////////////
void qpGeom::
set_primitive(int i, const qpGeomPrimitive *primitive) {
  clear_cache();
  CDWriter cdata(_cycler);
  nassertv(i >= 0 && i < (int)cdata->_primitives.size());
  nassertv(primitive->check_valid(cdata->_data));

  // All primitives within a particular Geom must have the same
  // fundamental primitive type (triangles, points, or lines).
  nassertv(cdata->_primitive_type == PT_none ||
           cdata->_primitive_type == primitive->get_primitive_type());

  // They also should have the a compatible shade model.
  CPT(qpGeomPrimitive) compat = primitive->match_shade_model(cdata->_shade_model);
  nassertv_always(compat != (qpGeomPrimitive *)NULL);

  cdata->_primitives[i] = (qpGeomPrimitive *)compat.p();
  PrimitiveType new_primitive_type = compat->get_primitive_type();
  if (new_primitive_type != cdata->_primitive_type) {
    cdata->_primitive_type = new_primitive_type;
  }
  ShadeModel new_shade_model = compat->get_shade_model();
  if (new_shade_model != cdata->_shade_model &&
      new_shade_model != SM_uniform) {
    cdata->_shade_model = new_shade_model;
  }

  reset_geom_rendering(cdata);
  cdata->_got_usage_hint = false;
  cdata->_modified = qpGeom::get_next_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::add_primitive
//       Access: Published
//  Description: Adds a new GeomPrimitive structure to the Geom
//               object.  This specifies a particular subset of
//               vertices that are used to define geometric primitives
//               of the indicated type.
////////////////////////////////////////////////////////////////////
void qpGeom::
add_primitive(const qpGeomPrimitive *primitive) {
  clear_cache();
  CDWriter cdata(_cycler);

  nassertv(primitive->check_valid(cdata->_data));

  // All primitives within a particular Geom must have the same
  // fundamental primitive type (triangles, points, or lines).
  nassertv(cdata->_primitive_type == PT_none ||
           cdata->_primitive_type == primitive->get_primitive_type());

  // They also should have the a compatible shade model.
  CPT(qpGeomPrimitive) compat = primitive->match_shade_model(cdata->_shade_model);
  nassertv_always(compat != (qpGeomPrimitive *)NULL);

  cdata->_primitives.push_back((qpGeomPrimitive *)compat.p());
  PrimitiveType new_primitive_type = compat->get_primitive_type();
  if (new_primitive_type != cdata->_primitive_type) {
    cdata->_primitive_type = new_primitive_type;
  }
  ShadeModel new_shade_model = compat->get_shade_model();
  if (new_shade_model != cdata->_shade_model &&
      new_shade_model != SM_uniform) {
    cdata->_shade_model = new_shade_model;
  }

  reset_geom_rendering(cdata);
  cdata->_got_usage_hint = false;
  cdata->_modified = qpGeom::get_next_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::remove_primitive
//       Access: Published
//  Description: Removes the ith primitive from the list.
////////////////////////////////////////////////////////////////////
void qpGeom::
remove_primitive(int i) {
  clear_cache();
  CDWriter cdata(_cycler);
  nassertv(i >= 0 && i < (int)cdata->_primitives.size());
  cdata->_primitives.erase(cdata->_primitives.begin() + i);
  if (cdata->_primitives.empty()) {
    cdata->_primitive_type = PT_none;
    cdata->_shade_model = SM_uniform;
  }
  reset_geom_rendering(cdata);
  cdata->_got_usage_hint = false;
  cdata->_modified = qpGeom::get_next_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::clear_primitives
//       Access: Published
//  Description: Removes all the primitives from the Geom object (but
//               keeps the same table of vertices).  You may then
//               re-add primitives one at a time via calls to
//               add_primitive().
////////////////////////////////////////////////////////////////////
void qpGeom::
clear_primitives() {
  clear_cache();
  CDWriter cdata(_cycler);
  cdata->_primitives.clear();
  cdata->_primitive_type = PT_none;
  cdata->_shade_model = SM_uniform;
  reset_geom_rendering(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::decompose_in_place
//       Access: Published
//  Description: Decomposes all of the primitives within this Geom,
//               leaving the results in place.  See
//               GeomPrimitive::decompose().
////////////////////////////////////////////////////////////////////
void qpGeom::
decompose_in_place() {
  clear_cache();
  CDWriter cdata(_cycler);

#ifndef NDEBUG
  bool all_is_valid = true;
#endif
  Primitives::iterator pi;
  for (pi = cdata->_primitives.begin(); pi != cdata->_primitives.end(); ++pi) {
    CPT(qpGeomPrimitive) new_prim = (*pi)->decompose();
    (*pi) = (qpGeomPrimitive *)new_prim.p();

#ifndef NDEBUG
    if (!(*pi)->check_valid(cdata->_data)) {
      all_is_valid = false;
    }
#endif
  }

  cdata->_modified = qpGeom::get_next_modified();
  reset_geom_rendering(cdata);

  nassertv(all_is_valid);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::rotate_in_place
//       Access: Published
//  Description: Rotates all of the primitives within this Geom,
//               leaving the results in place.  See
//               GeomPrimitive::rotate().
////////////////////////////////////////////////////////////////////
void qpGeom::
rotate_in_place() {
  clear_cache();
  CDWriter cdata(_cycler);

#ifndef NDEBUG
  bool all_is_valid = true;
#endif
  Primitives::iterator pi;
  for (pi = cdata->_primitives.begin(); pi != cdata->_primitives.end(); ++pi) {
    CPT(qpGeomPrimitive) new_prim = (*pi)->rotate();
    (*pi) = (qpGeomPrimitive *)new_prim.p();

#ifndef NDEBUG
    if (!(*pi)->check_valid(cdata->_data)) {
      all_is_valid = false;
    }
#endif
  }

  cdata->_modified = qpGeom::get_next_modified();

  nassertv(all_is_valid);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::unify_in_place
//       Access: Published
//  Description: Unifies all of the primitives contained within this
//               Geom into a single primitive object.  This may
//               require decomposing the primitives if, for instance,
//               the Geom contains both triangle strips and triangle
//               fans.
////////////////////////////////////////////////////////////////////
void qpGeom::
unify_in_place() {
  if (get_num_primitives() <= 1) {
    // If we don't have more than one primitive to start with, no need
    // to do anything.
    return;
  }

  clear_cache();
  CDWriter cdata(_cycler);

  PT(qpGeomPrimitive) new_prim;

  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin(); pi != cdata->_primitives.end(); ++pi) {
    CPT(qpGeomPrimitive) primitive = (*pi);
    if (new_prim == (qpGeomPrimitive *)NULL) {
      // The first primitive type we come across is copied directly.
      new_prim = primitive->make_copy();
    } else {
      // Thereafter, we must try to merge primitives.  If they are not
      // the same type, we have to decompose both of them.
      if (new_prim->get_type() != primitive->get_type()) {
        CPT(qpGeomPrimitive) decomposed = new_prim->decompose();
        new_prim = (qpGeomPrimitive *)decomposed.p();
        primitive = primitive->decompose();

        nassertv(new_prim->get_type() == primitive->get_type());
      }

      // Now simply copy in the vertices.
      int num_primitives = primitive->get_num_primitives();
      for (int pi = 0; pi < num_primitives; ++pi) {
        int start = primitive->get_primitive_start(pi);
        int end = primitive->get_primitive_end(pi);
        for (int vi = start; vi < end; ++vi) {
          new_prim->add_vertex(primitive->get_vertex(vi));
        }
        new_prim->close_primitive();
      }
    }
  }

  // At the end of the day, we have just one primitive, which becomes
  // the one primitive in our list of primitives.
  nassertv(new_prim->check_valid(cdata->_data));

  // The new primitive, naturally, inherits the Geom's overall shade
  // model.
  new_prim->set_shade_model(cdata->_shade_model);

  cdata->_primitives.clear();
  cdata->_primitives.push_back(new_prim);

  cdata->_modified = qpGeom::get_next_modified();
  reset_geom_rendering(cdata);
}


////////////////////////////////////////////////////////////////////
//     Function: qpGeom::copy_primitives_from
//       Access: Published
//  Description: Copies the primitives from the indicated Geom into
//               this one.  This does require that both Geoms contain
//               the same fundamental type primitives, both have a
//               compatible shade model, and both use the same
//               GeomVertexData.
//
//               Returns true if the copy is successful, or false
//               otherwise (because the Geoms were mismatched).
////////////////////////////////////////////////////////////////////
bool qpGeom::
copy_primitives_from(const qpGeom *other) {
  if (get_primitive_type() != PT_none &&
      other->get_primitive_type() != get_primitive_type()) {
    return false;
  }
  if (get_vertex_data() != other->get_vertex_data()) {
    return false;
  }

  ShadeModel this_shade_model = get_shade_model();
  ShadeModel other_shade_model = other->get_shade_model();
  if (this_shade_model != SM_uniform && other_shade_model != SM_uniform &&
      this_shade_model != other_shade_model) {
    if ((this_shade_model == SM_flat_first_vertex && other_shade_model == SM_flat_last_vertex) ||
        (this_shade_model == SM_flat_last_vertex && other_shade_model == SM_flat_first_vertex)) {
      // This is acceptable; we can rotate the primitives to match.

    } else {
      // Otherwise, we have incompatible shade models.
      return false;
    }
  }

  int num_primitives = other->get_num_primitives();
  for (int i = 0; i < num_primitives; i++) {
    add_primitive(other->get_primitive(i));
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::get_num_bytes
//       Access: Published
//  Description: Returns the number of bytes consumed by the geom and
//               its primitives (but not including its vertex table).
////////////////////////////////////////////////////////////////////
int qpGeom::
get_num_bytes() const {
  CDReader cdata(_cycler);

  int num_bytes = sizeof(qpGeom);
  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin(); 
       pi != cdata->_primitives.end();
       ++pi) {
    num_bytes += (*pi)->get_num_bytes();
  }

  return num_bytes;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::transform_vertices
//       Access: Published, Virtual
//  Description: Applies the indicated transform to all of the
//               vertices in the Geom.  If the Geom happens to share a
//               vertex table with another Geom, this operation will
//               duplicate the vertex table instead of breaking the
//               other Geom; however, if multiple Geoms with shared
//               tables are transformed by the same matrix, they will
//               no longer share tables after the operation.  Consider
//               using the GeomTransformer if you will be applying the
//               same transform to multiple Geoms.
////////////////////////////////////////////////////////////////////
void qpGeom::
transform_vertices(const LMatrix4f &mat) {
  PT(qpGeomVertexData) new_data = modify_vertex_data();
  CPT(qpGeomVertexFormat) format = new_data->get_format();
  
  int ci;
  for (ci = 0; ci < format->get_num_points(); ci++) {
    qpGeomVertexRewriter data(new_data, format->get_point(ci));
    
    while (!data.is_at_end()) {
      const LPoint3f &point = data.get_data3f();
      data.set_data3f(point * mat);
    }
  }
  for (ci = 0; ci < format->get_num_vectors(); ci++) {
    qpGeomVertexRewriter data(new_data, format->get_vector(ci));
    
    while (!data.is_at_end()) {
      const LVector3f &vector = data.get_data3f();
      data.set_data3f(normalize(vector * mat));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::munge_geom
//       Access: Published
//  Description: Applies the indicated munger to the geom and its
//               data, and returns a (possibly different) geom and
//               data, according to the munger's whim.  
//
//               The assumption is that for a particular geom and a
//               particular munger, the result will always be the
//               same; so this result may be cached.
////////////////////////////////////////////////////////////////////
void qpGeom::
munge_geom(const qpGeomMunger *munger,
           CPT(qpGeom) &result, CPT(qpGeomVertexData) &data) const {
  CPT(qpGeomVertexData) source_data = data;

  // Look up the munger in our cache--maybe we've recently applied it.
  {
    CDReader cdata(_cycler);
    CacheEntry temp_entry(source_data, munger);
    temp_entry.local_object();
    Cache::const_iterator ci = cdata->_cache.find(&temp_entry);
    if (ci != cdata->_cache.end()) {
      CacheEntry *entry = (*ci);

      if (get_modified() <= entry->_geom_result->get_modified() &&
          data->get_modified() <= entry->_data_result->get_modified()) {
        // The cache entry is still good; use it.

        // Record a cache hit, so this element will stay in the cache a
        // while longer.
        entry->refresh();
        result = entry->_geom_result;
        data = entry->_data_result;
        return;
      }

      // The cache entry is stale, remove it.
      if (gobj_cat.is_debug()) {
        gobj_cat.debug()
          << "Cache entry " << *entry << " is stale, removing.\n";
      }
      entry->erase();
      CDWriter cdataw(((qpGeom *)this)->_cycler, cdata);
      cdataw->_cache.erase(entry);
    }
  }

  // Ok, invoke the munger.
  PStatTimer timer(qpGeomMunger::_munge_pcollector);

  result = this;
  if (munger != (qpGeomMunger *)NULL) {
    data = munger->munge_data(data);
    ((qpGeomMunger *)munger)->munge_geom_impl(result, data);
  }

  {
    // Record the new result in the cache.
    CacheEntry *entry;
    {
      CDWriter cdata(((qpGeom *)this)->_cycler);
      entry = new CacheEntry((qpGeom *)this, source_data, munger,
                             result, data);
      bool inserted = cdata->_cache.insert(entry).second;
      nassertv(inserted);
    }
    
    // And tell the cache manager about the new entry.  (It might
    // immediately request a delete from the cache of the thing we
    // just added.)
    entry->record();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::check_valid
//       Access: Published
//  Description: Verifies that the all of the primitives within the
//               geom reference vertices that actually exist within
//               the geom's GeomVertexData.  Returns true if the geom
//               appears to be valid, false otherwise.
////////////////////////////////////////////////////////////////////
bool qpGeom::
check_valid() const {
  CDReader cdata(_cycler);

  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin(); 
       pi != cdata->_primitives.end();
       ++pi) {
    if (!(*pi)->check_valid(cdata->_data)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeom::
output(ostream &out) const {
  CDReader cdata(_cycler);

  // Get a list of the primitive types contained within this object.
  int num_faces = 0;
  pset<TypeHandle> types;
  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin(); 
       pi != cdata->_primitives.end();
       ++pi) {
    num_faces += (*pi)->get_num_faces();
    types.insert((*pi)->get_type());
  }

  out << "Geom [";
  pset<TypeHandle>::iterator ti;
  for (ti = types.begin(); ti != types.end(); ++ti) {
    out << " " << (*ti);
  }
  out << " ], " << num_faces << " faces";
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeom::
write(ostream &out, int indent_level) const {
  CDReader cdata(_cycler);

  // Get a list of the primitive types contained within this object.
  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin(); 
       pi != cdata->_primitives.end();
       ++pi) {
    (*pi)->write(out, indent_level);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::clear_cache
//       Access: Published
//  Description: Removes all of the previously-cached results of
//               munge_geom().
////////////////////////////////////////////////////////////////////
void qpGeom::
clear_cache() {
  // Probably we shouldn't do anything at all here unless we are
  // running in pipeline stage 0.
  CData *cdata = CDWriter(_cycler);
  for (Cache::iterator ci = cdata->_cache.begin();
       ci != cdata->_cache.end();
       ++ci) {
    CacheEntry *entry = (*ci);
    entry->erase();
  }
  cdata->_cache.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::draw
//       Access: Public
//  Description: Actually draws the Geom with the indicated GSG, using
//               the indicated vertex data (which might have been
//               pre-munged to support the GSG's needs).
////////////////////////////////////////////////////////////////////
void qpGeom::
draw(GraphicsStateGuardianBase *gsg, const qpGeomMunger *munger,
     const qpGeomVertexData *vertex_data) const {
#ifdef DO_PIPELINING
  // Make sure the usage_hint is already updated before we start to
  // draw, so we don't end up with a circular lock if the GSG asks us
  // to update this while we're holding the read lock.
  {
    CDReader cdata(_cycler);
    if (!cdata->_got_usage_hint) {
      CDWriter cdataw(((qpGeom *)this)->_cycler, cdata);
      ((qpGeom *)this)->reset_usage_hint(cdataw);
    }
  }
  // TODO: fix up the race condition between this line and the next.
  // Maybe CDWriter's elevate-to-write should return the read lock to
  // its original locked state when it's done.
#endif  // DO_PIPELINING

  CDReader cdata(_cycler);
  
  if (gsg->begin_draw_primitives(this, munger, vertex_data)) {
    Primitives::const_iterator pi;
    for (pi = cdata->_primitives.begin(); 
         pi != cdata->_primitives.end();
         ++pi) {
      const qpGeomPrimitive *primitive = (*pi);
      if (primitive->get_num_vertices() != 0) {
        (*pi)->draw(gsg);
      }
    }
    gsg->end_draw_primitives();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::calc_tight_bounds
//       Access: Public, Virtual
//  Description: Expands min_point and max_point to include all of the
//               vertices in the Geom, if any.  found_any is set true
//               if any points are found.  It is the caller's
//               responsibility to initialize min_point, max_point,
//               and found_any before calling this function.
////////////////////////////////////////////////////////////////////
void qpGeom::
calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point,
                  bool &found_any, 
                  const qpGeomVertexData *vertex_data,
                  bool got_mat, const LMatrix4f &mat) const {

  CDReader cdata(_cycler);
  
  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin(); 
       pi != cdata->_primitives.end();
       ++pi) {
    (*pi)->calc_tight_bounds(min_point, max_point, found_any, vertex_data,
                             got_mat, mat);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::get_next_modified
//       Access: Public, Static
//  Description: Returns a monotonically increasing sequence.  Each
//               time this is called, a new sequence number is
//               returned, higher than the previous value.
//
//               This is used to ensure that
//               GeomVertexArrayData::get_modified() and
//               GeomPrimitive::get_modified() update from the same
//               space, so that Geom::get_modified() returns a
//               meaningful value.
////////////////////////////////////////////////////////////////////
UpdateSeq qpGeom::
get_next_modified() {
  ++_next_modified;
  return _next_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::recompute_bound
//       Access: Protected, Virtual
//  Description: Recomputes the dynamic bounding volume for this Geom.
//               This includes all of the vertices.
////////////////////////////////////////////////////////////////////
BoundingVolume *qpGeom::
recompute_bound() {
  // First, get ourselves a fresh, empty bounding volume.
  BoundingVolume *bound = BoundedObject::recompute_bound();
  nassertr(bound != (BoundingVolume*)0L, bound);

  GeometricBoundingVolume *gbv = DCAST(GeometricBoundingVolume, bound);

  // Now actually compute the bounding volume.  We do this by using
  // calc_tight_bounds to determine our minmax first.
  LPoint3f points[2];
  bool found_any = false;
  calc_tight_bounds(points[0], points[1], found_any, get_vertex_data(),
                    false, LMatrix4f::ident_mat());
  if (found_any) {
    // Then we put the bounding volume around both of those points.
    // Technically, we should put it around the eight points at the
    // corners of the rectangular solid, but we happen to know that
    // the two diagonally opposite points is good enough to define any
    // of our bound volume types.

    const LPoint3f *points_begin = &points[0];
    const LPoint3f *points_end = points_begin + 2;
    gbv->around(points_begin, points_end);
  }

  return bound;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::check_will_be_valid
//       Access: Private
//  Description: Verifies that the all of the primitives within the
//               geom reference vertices that actually exist within
//               the indicated GeomVertexData (presumably in
//               preparation for assigning the geom to use this data).
//               Returns true if the data appears to be valid, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool qpGeom::
check_will_be_valid(const qpGeomVertexData *vertex_data) const {
  CDReader cdata(_cycler);

  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin(); 
       pi != cdata->_primitives.end();
       ++pi) {
    if (!(*pi)->check_valid(vertex_data)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::reset_usage_hint
//       Access: Private
//  Description: Recomputes the minimum usage_hint.
////////////////////////////////////////////////////////////////////
void qpGeom::
reset_usage_hint(qpGeom::CDWriter &cdata) {
  cdata->_usage_hint = UH_unspecified;
  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin(); 
       pi != cdata->_primitives.end();
       ++pi) {
    cdata->_usage_hint = min(cdata->_usage_hint, (*pi)->get_usage_hint());
  }
  cdata->_got_usage_hint = true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::reset_geom_rendering
//       Access: Private
//  Description: Rederives the _geom_rendering member.
////////////////////////////////////////////////////////////////////
void qpGeom::
reset_geom_rendering(qpGeom::CDWriter &cdata) {
  cdata->_geom_rendering = 0;
  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin(); 
       pi != cdata->_primitives.end();
       ++pi) {
    cdata->_geom_rendering |= (*pi)->get_geom_rendering();
  }

  if ((cdata->_geom_rendering & GR_point) != 0) {
    if (cdata->_data->has_column(InternalName::get_size())) {
      cdata->_geom_rendering |= GR_per_point_size;
    }
    if (cdata->_data->has_column(InternalName::get_aspect_ratio())) {
      cdata->_geom_rendering |= GR_point_aspect_ratio;
    }
    if (cdata->_data->has_column(InternalName::get_rotate())) {
      cdata->_geom_rendering |= GR_point_rotate;
    }
  }

  switch (get_shade_model()) {
  case SM_flat_first_vertex:
    cdata->_geom_rendering |= GR_flat_first_vertex;
    break;

  case SM_flat_last_vertex:
    cdata->_geom_rendering |= GR_flat_last_vertex;
    break;

  default:
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               qpGeom.
////////////////////////////////////////////////////////////////////
void qpGeom::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpGeom::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type qpGeom is encountered
//               in the Bam file.  It should create the qpGeom
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *qpGeom::
make_from_bam(const FactoryParams &params) {
  qpGeom *object = new qpGeom;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpGeom.
////////////////////////////////////////////////////////////////////
void qpGeom::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  manager->read_cdata(scan, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::CacheEntry::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeom::CacheEntry::
~CacheEntry() {
  if (_geom_result != _source) {
    unref_delete(_geom_result);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::CacheEntry::evict_callback
//       Access: Public, Virtual
//  Description: Called when the entry is evicted from the cache, this
//               should clean up the owning object appropriately.
////////////////////////////////////////////////////////////////////
void qpGeom::CacheEntry::
evict_callback() {
  // We have to operate on stage 0 of the pipeline, since that's where
  // the cache really counts.  Because of the multistage pipeline, we
  // might not actually have a cache entry there (it might have been
  // added to stage 1 instead).  No big deal if we don't.
  CData *cdata = _source->_cycler.write_stage(0);
  Cache::iterator ci = cdata->_cache.find(this);
  if (ci != cdata->_cache.end()) {
    cdata->_cache.erase(ci);
  }
  _source->_cycler.release_write_stage(0, cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::CacheEntry::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeom::CacheEntry::
output(ostream &out) const {
  out << "geom " << (void *)_source << ", " 
      << (const void *)_modifier;
}


////////////////////////////////////////////////////////////////////
//     Function: qpGeom::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *qpGeom::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void qpGeom::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  manager->write_pointer(dg, _data);

  dg.add_uint16(_primitives.size());
  Primitives::const_iterator pi;
  for (pi = _primitives.begin(); pi != _primitives.end(); ++pi) {
    manager->write_pointer(dg, *pi);
  }

  dg.add_uint8(_primitive_type);
  dg.add_uint8(_shade_model);
  dg.add_uint16(_geom_rendering);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::CData::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int qpGeom::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  _data = DCAST(qpGeomVertexData, p_list[pi++]);

  Primitives::iterator pri;
  for (pri = _primitives.begin(); pri != _primitives.end(); ++pri) {
    (*pri) = DCAST(qpGeomPrimitive, p_list[pi++]);
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeom::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new qpGeom.
////////////////////////////////////////////////////////////////////
void qpGeom::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  manager->read_pointer(scan);

  int num_primitives = scan.get_uint16();
  _primitives.reserve(num_primitives);
  for (int i = 0; i < num_primitives; ++i) {
    manager->read_pointer(scan);
    _primitives.push_back(NULL);
  }

  _primitive_type = (PrimitiveType)scan.get_uint8();
  _shade_model = (ShadeModel)scan.get_uint8();
  _geom_rendering = scan.get_uint16();
  _got_usage_hint = false;
  _modified = qpGeom::get_next_modified();
}
