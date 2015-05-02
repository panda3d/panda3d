// Filename: geom.cxx
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

#include "geom.h"
#include "geomPoints.h"
#include "geomVertexReader.h"
#include "geomVertexRewriter.h"
#include "graphicsStateGuardianBase.h"
#include "preparedGraphicsObjects.h"
#include "pStatTimer.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "boundingSphere.h"
#include "boundingBox.h"
#include "lightMutexHolder.h"
#include "config_mathutil.h"

UpdateSeq Geom::_next_modified;
PStatCollector Geom::_draw_primitive_setup_pcollector("Draw:Primitive:Setup");

TypeHandle Geom::_type_handle;
TypeHandle Geom::CDataCache::_type_handle;
TypeHandle Geom::CacheEntry::_type_handle;
TypeHandle Geom::CData::_type_handle;
TypeHandle GeomPipelineReader::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Geom::make_cow_copy
//       Access: Protected, Virtual
//  Description: Required to implement CopyOnWriteObject.
////////////////////////////////////////////////////////////////////
PT(CopyOnWriteObject) Geom::
make_cow_copy() {
  return make_copy();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Geom::
Geom(const GeomVertexData *data) {
  // Let's ensure the vertex data gets set on all stages at once.
  OPEN_ITERATE_ALL_STAGES(_cycler) {
    CDStageWriter cdata(_cycler, pipeline_stage);
    cdata->_data = (GeomVertexData *)data;
  }
  CLOSE_ITERATE_ALL_STAGES(_cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::Copy Constructor
//       Access: Protected
//  Description: Use make_copy() to duplicate a Geom.
////////////////////////////////////////////////////////////////////
Geom::
Geom(const Geom &copy) :
  CopyOnWriteObject(copy),
  _cycler(copy._cycler)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::Copy Assignment Operator
//       Access: Published
//  Description: The copy assignment operator is not pipeline-safe.
//               This will completely obliterate all stages of the
//               pipeline, so don't do it for a Geom that is actively
//               being used for rendering.
////////////////////////////////////////////////////////////////////
void Geom::
operator = (const Geom &copy) {
  CopyOnWriteObject::operator = (copy);

  clear_cache();

  _cycler = copy._cycler;

  OPEN_ITERATE_ALL_STAGES(_cycler) {
    CDStageWriter cdata(_cycler, pipeline_stage);
    mark_internal_bounds_stale(cdata);
  }
  CLOSE_ITERATE_ALL_STAGES(_cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
Geom::
~Geom() {
  clear_cache();
  release_all();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::make_copy
//       Access: Protected, Virtual
//  Description: Returns a newly-allocated Geom that is a shallow copy
//               of this one.  It will be a different Geom pointer,
//               but its internal data may or may not be shared with
//               that of the original Geom.
////////////////////////////////////////////////////////////////////
Geom *Geom::
make_copy() const {
  return new Geom(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::set_usage_hint
//       Access: Published
//  Description: Changes the UsageHint hint for all of the primitives
//               on this Geom to the same value.  See
//               get_usage_hint().
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
void Geom::
set_usage_hint(Geom::UsageHint usage_hint) {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);
  cdata->_usage_hint = usage_hint;

  Primitives::iterator pi;
  for (pi = cdata->_primitives.begin(); pi != cdata->_primitives.end(); ++pi) {
    PT(GeomPrimitive) prim = (*pi).get_write_pointer();
    prim->set_usage_hint(usage_hint);
  }

  clear_cache_stage(current_thread);
  cdata->_modified = Geom::get_next_modified();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::modify_vertex_data
//       Access: Published
//  Description: Returns a modifiable pointer to the GeomVertexData,
//               so that application code may directly maniuplate the
//               geom's underlying data.
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
PT(GeomVertexData) Geom::
modify_vertex_data() {
  Thread *current_thread = Thread::get_current_thread();
  // Perform copy-on-write: if the reference count on the vertex data
  // is greater than 1, assume some other Geom has the same pointer,
  // so make a copy of it first.
  CDWriter cdata(_cycler, true, current_thread);
  clear_cache_stage(current_thread);
  mark_internal_bounds_stale(cdata);
  return cdata->_data.get_write_pointer();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::set_vertex_data
//       Access: Published
//  Description: Replaces the Geom's underlying vertex data table with
//               a completely new table.
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
void Geom::
set_vertex_data(const GeomVertexData *data) {
  Thread *current_thread = Thread::get_current_thread();
  nassertv(check_will_be_valid(data));
  CDWriter cdata(_cycler, true, current_thread);
  cdata->_data = (GeomVertexData *)data;
  clear_cache_stage(current_thread);
  mark_internal_bounds_stale(cdata);
  reset_geom_rendering(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::offset_vertices
//       Access: Published
//  Description: Replaces a Geom's vertex table with a new table, and
//               simultaneously adds the indicated offset to all
//               vertex references within the Geom's primitives.  This
//               is intended to be used to combine multiple
//               GeomVertexDatas from different Geoms into a single
//               big buffer, with each Geom referencing a subset of
//               the vertices in the buffer.
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
void Geom::
offset_vertices(const GeomVertexData *data, int offset) {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);
  cdata->_data = (GeomVertexData *)data;

#ifndef NDEBUG
  bool all_is_valid = true;
#endif
  Primitives::iterator pi;
  for (pi = cdata->_primitives.begin(); pi != cdata->_primitives.end(); ++pi) {
    PT(GeomPrimitive) prim = (*pi).get_write_pointer();
    prim->offset_vertices(offset);

#ifndef NDEBUG
    if (!prim->check_valid(data)) {
      gobj_cat.warning()
        << *prim << " is invalid for " << *data << ":\n";
      prim->write(gobj_cat.warning(false), 4);

      all_is_valid = false;
    }
#endif
  }

  cdata->_modified = Geom::get_next_modified();
  clear_cache_stage(current_thread);
  nassertv(all_is_valid);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::make_nonindexed
//       Access: Published
//  Description: Converts the geom from indexed to nonindexed by
//               duplicating vertices as necessary.  If composite_only
//               is true, then only composite primitives such as
//               trifans and tristrips are converted.  Returns the
//               number of GeomPrimitive objects converted.
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
int Geom::
make_nonindexed(bool composite_only) {
  Thread *current_thread = Thread::get_current_thread();
  int num_changed = 0;

  CDWriter cdata(_cycler, true, current_thread);
  CPT(GeomVertexData) orig_data = cdata->_data.get_read_pointer();
  PT(GeomVertexData) new_data = new GeomVertexData(*orig_data);
  new_data->clear_rows();

#ifndef NDEBUG
  bool all_is_valid = true;
#endif
  Primitives::iterator pi;
  Primitives new_prims;
  new_prims.reserve(cdata->_primitives.size());
  for (pi = cdata->_primitives.begin(); pi != cdata->_primitives.end(); ++pi) {
    PT(GeomPrimitive) primitive = (*pi).get_read_pointer()->make_copy();
    new_prims.push_back(primitive.p());

    // GeomPoints are considered "composite" for the purposes of
    // making nonindexed, since there's no particular advantage to
    // having indexed points (as opposed to, say, indexed triangles or
    // indexed lines).
    if (primitive->is_indexed() &&
        (primitive->is_composite() ||
         primitive->is_exact_type(GeomPoints::get_class_type()) ||
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
    cdata->_modified = Geom::get_next_modified();
    clear_cache_stage(current_thread);
  }

  return num_changed;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::set_primitive
//       Access: Published
//  Description: Replaces the ith GeomPrimitive object stored within
//               the Geom with the new object.
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
void Geom::
set_primitive(int i, const GeomPrimitive *primitive) {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);
  nassertv(i >= 0 && i < (int)cdata->_primitives.size());
  nassertv(primitive->check_valid(cdata->_data.get_read_pointer()));

  // All primitives within a particular Geom must have the same
  // fundamental primitive type (triangles, points, or lines).
  nassertv(cdata->_primitive_type == PT_none ||
           cdata->_primitive_type == primitive->get_primitive_type());

  // They also should have a compatible shade model.
  CPT(GeomPrimitive) compat = primitive->match_shade_model(cdata->_shade_model);
  nassertv_always(compat != (GeomPrimitive *)NULL);

  cdata->_primitives[i] = (GeomPrimitive *)compat.p();
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
  cdata->_modified = Geom::get_next_modified();
  clear_cache_stage(current_thread);
  mark_internal_bounds_stale(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::add_primitive
//       Access: Published
//  Description: Adds a new GeomPrimitive structure to the Geom
//               object.  This specifies a particular subset of
//               vertices that are used to define geometric primitives
//               of the indicated type.
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
void Geom::
add_primitive(const GeomPrimitive *primitive) {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);

  nassertv(primitive->check_valid(cdata->_data.get_read_pointer()));

  // All primitives within a particular Geom must have the same
  // fundamental primitive type (triangles, points, or lines).
  nassertv(cdata->_primitive_type == PT_none ||
           cdata->_primitive_type == primitive->get_primitive_type());

  // They also should have a compatible shade model.
  CPT(GeomPrimitive) compat = primitive->match_shade_model(cdata->_shade_model);
  nassertv_always(compat != (GeomPrimitive *)NULL);

  cdata->_primitives.push_back((GeomPrimitive *)compat.p());
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
  cdata->_modified = Geom::get_next_modified();
  clear_cache_stage(current_thread);
  mark_internal_bounds_stale(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::remove_primitive
//       Access: Published
//  Description: Removes the ith primitive from the list.
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
void Geom::
remove_primitive(int i) {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);
  nassertv(i >= 0 && i < (int)cdata->_primitives.size());
  cdata->_primitives.erase(cdata->_primitives.begin() + i);
  if (cdata->_primitives.empty()) {
    cdata->_primitive_type = PT_none;
    cdata->_shade_model = SM_uniform;
  }
  reset_geom_rendering(cdata);
  cdata->_got_usage_hint = false;
  cdata->_modified = Geom::get_next_modified();
  clear_cache_stage(current_thread);
  mark_internal_bounds_stale(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::clear_primitives
//       Access: Published
//  Description: Removes all the primitives from the Geom object (but
//               keeps the same table of vertices).  You may then
//               re-add primitives one at a time via calls to
//               add_primitive().
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
void Geom::
clear_primitives() {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);
  cdata->_primitives.clear();
  cdata->_primitive_type = PT_none;
  cdata->_shade_model = SM_uniform;
  reset_geom_rendering(cdata);
  clear_cache_stage(current_thread);
  mark_internal_bounds_stale(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::decompose_in_place
//       Access: Published
//  Description: Decomposes all of the primitives within this Geom,
//               leaving the results in place.  See
//               GeomPrimitive::decompose().
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
void Geom::
decompose_in_place() {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);

#ifndef NDEBUG
  bool all_is_valid = true;
#endif
  Primitives::iterator pi;
  for (pi = cdata->_primitives.begin(); pi != cdata->_primitives.end(); ++pi) {
    CPT(GeomPrimitive) new_prim = (*pi).get_read_pointer()->decompose();
    (*pi) = (GeomPrimitive *)new_prim.p();

#ifndef NDEBUG
    if (!new_prim->check_valid(cdata->_data.get_read_pointer())) {
      all_is_valid = false;
    }
#endif
  }

  cdata->_modified = Geom::get_next_modified();
  reset_geom_rendering(cdata);
  clear_cache_stage(current_thread);

  nassertv(all_is_valid);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::doubleside_in_place
//       Access: Published
//  Description: Doublesides all of the primitives within this Geom,
//               leaving the results in place.  See
//               GeomPrimitive::doubleside().
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
void Geom::
doubleside_in_place() {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);

#ifndef NDEBUG
  bool all_is_valid = true;
#endif
  Primitives::iterator pi;
  for (pi = cdata->_primitives.begin(); pi != cdata->_primitives.end(); ++pi) {
    CPT(GeomPrimitive) new_prim = (*pi).get_read_pointer()->doubleside();
    (*pi) = (GeomPrimitive *)new_prim.p();

#ifndef NDEBUG
    if (!new_prim->check_valid(cdata->_data.get_read_pointer())) {
      all_is_valid = false;
    }
#endif
  }

  cdata->_modified = Geom::get_next_modified();
  reset_geom_rendering(cdata);
  clear_cache_stage(current_thread);

  nassertv(all_is_valid);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::reverse_in_place
//       Access: Published
//  Description: Reverses all of the primitives within this Geom,
//               leaving the results in place.  See
//               GeomPrimitive::reverse().
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
void Geom::
reverse_in_place() {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);

#ifndef NDEBUG
  bool all_is_valid = true;
#endif
  Primitives::iterator pi;
  for (pi = cdata->_primitives.begin(); pi != cdata->_primitives.end(); ++pi) {
    CPT(GeomPrimitive) new_prim = (*pi).get_read_pointer()->reverse();
    (*pi) = (GeomPrimitive *)new_prim.p();

#ifndef NDEBUG
    if (!new_prim->check_valid(cdata->_data.get_read_pointer())) {
      all_is_valid = false;
    }
#endif
  }

  cdata->_modified = Geom::get_next_modified();
  reset_geom_rendering(cdata);
  clear_cache_stage(current_thread);

  nassertv(all_is_valid);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::rotate_in_place
//       Access: Published
//  Description: Rotates all of the primitives within this Geom,
//               leaving the results in place.  See
//               GeomPrimitive::rotate().
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
void Geom::
rotate_in_place() {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);

#ifndef NDEBUG
  bool all_is_valid = true;
#endif
  Primitives::iterator pi;
  for (pi = cdata->_primitives.begin(); pi != cdata->_primitives.end(); ++pi) {
    CPT(GeomPrimitive) new_prim = (*pi).get_read_pointer()->rotate();
    (*pi) = (GeomPrimitive *)new_prim.p();

#ifndef NDEBUG
    if (!new_prim->check_valid(cdata->_data.get_read_pointer())) {
      all_is_valid = false;
    }
#endif
  }

  switch (cdata->_shade_model) {
  case SM_flat_first_vertex:
    cdata->_shade_model = SM_flat_last_vertex;
    break;

  case SM_flat_last_vertex:
    cdata->_shade_model = SM_flat_first_vertex;
    break;

  default:
    break;
  }

  cdata->_modified = Geom::get_next_modified();
  clear_cache_stage(current_thread);

  nassertv(all_is_valid);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::unify_in_place
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
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
void Geom::
unify_in_place(int max_indices, bool preserve_order) {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "unify_in_place(" << max_indices << ", " << preserve_order
      << "): " << *this << "\n";
  }

  Thread *current_thread = Thread::get_current_thread();
  if (get_num_primitives() <= 1) {
    // If we don't have more than one primitive to start with, no need
    // to do anything.
    return;
  }

  CDWriter cdata(_cycler, true, current_thread);

  typedef pmap<TypeHandle, PT(GeomPrimitive) > NewPrims;

  NewPrims new_prims;

  bool keep_different_types = preserve_triangle_strips && !preserve_order;

  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin(); pi != cdata->_primitives.end(); ++pi) {
    CPT(GeomPrimitive) primitive = (*pi).get_read_pointer();
    NewPrims::iterator npi = new_prims.find(primitive->get_type());
    if (npi == new_prims.end()) {
      // This is the first primitive of this type.
      if (!keep_different_types && !new_prims.empty()) {
        // Actually, since we aren't trying to keep the different
        // types of primitives, we should try to combine this type and
        // the other type by decomposing them both (into triangles,
        // segments, or whatever).

        // First, decompose the incoming one.
        primitive = primitive->decompose();
        npi = new_prims.find(primitive->get_type());
        if (npi == new_prims.end()) {
          // That didn't help, so decompose the one already in the
          // table.
          nassertv(new_prims.size() == 1);
          npi = new_prims.begin();
          CPT(GeomPrimitive) np = (*npi).second->decompose();
          new_prims.clear();
          new_prims.insert(NewPrims::value_type(np->get_type(), np->make_copy()));
          npi = new_prims.find(primitive->get_type());
        }
      }
    }

    if (npi == new_prims.end()) {
      // This is the first primitive of this type.  Just store it.
      new_prims.insert(NewPrims::value_type(primitive->get_type(), primitive->make_copy()));

    } else {
      // We have already encountered another primitive of this type.
      // Combine them.
      combine_primitives((*npi).second, primitive, current_thread);
    }
  }

  // Now, we have one or more primitives, but only one of each type.
#ifndef NDEBUG
  if (!keep_different_types && new_prims.size() > 1) {
    // This shouldn't be possible, because we decompose as we go, in
    // the loop above.  (We have to decompose as we go to preserve the
    // ordering of the primitives.)
    nassertv(false);
  }
#endif

  // Finally, iterate through the remaining primitives, and copy them
  // to the output list.
  cdata->_primitives.clear();
  NewPrims::iterator npi;
  for (npi = new_prims.begin(); npi != new_prims.end(); ++npi) {
    GeomPrimitive *prim = (*npi).second;

    nassertv(prim->check_valid(cdata->_data.get_read_pointer()));

    // Each new primitive, naturally, inherits the Geom's overall
    // shade model.
    prim->set_shade_model(cdata->_shade_model);

    // Should we split it up again to satisfy max_indices?
    if (prim->get_num_vertices() > max_indices) {
      // Copy prim into smaller prims, no one of which has more than
      // max_indices vertices.
      int i = 0;

      while (i < prim->get_num_primitives()) {
        PT(GeomPrimitive) smaller = prim->make_copy();
        smaller->clear_vertices();
        while (i < prim->get_num_primitives() &&
               smaller->get_num_vertices() + prim->get_primitive_num_vertices(i) < max_indices) {
          int start = prim->get_primitive_start(i);
          int end = prim->get_primitive_end(i);
          for (int n = start; n < end; ++n) {
            smaller->add_vertex(prim->get_vertex(n));
          }
          smaller->close_primitive();

          ++i;
        }

        cdata->_primitives.push_back(smaller.p());
      }

    } else {
      // The prim has few enough vertices; keep it.
      cdata->_primitives.push_back(prim);
    }
  }

  cdata->_modified = Geom::get_next_modified();
  clear_cache_stage(current_thread);
  reset_geom_rendering(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::make_lines_in_place
//       Access: Published
//  Description: Replaces the GeomPrimitives within this Geom with
//               corresponding GeomLines, representing a wireframe
//               of the primitives.  See GeomPrimitive::make_lines().
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
void Geom::
make_lines_in_place() {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);

#ifndef NDEBUG
  bool all_is_valid = true;
#endif
  Primitives::iterator pi;
  for (pi = cdata->_primitives.begin(); pi != cdata->_primitives.end(); ++pi) {
    CPT(GeomPrimitive) new_prim = (*pi).get_read_pointer()->make_lines();
    (*pi) = (GeomPrimitive *)new_prim.p();

#ifndef NDEBUG
    if (!new_prim->check_valid(cdata->_data.get_read_pointer())) {
      all_is_valid = false;
    }
#endif
  }

  cdata->_modified = Geom::get_next_modified();
  reset_geom_rendering(cdata);
  clear_cache_stage(current_thread);

  nassertv(all_is_valid);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::make_points_in_place
//       Access: Published
//  Description: Replaces the GeomPrimitives within this Geom with
//               corresponding GeomPoints.  See
//               GeomPrimitive::make_points().
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
void Geom::
make_points_in_place() {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);

#ifndef NDEBUG
  bool all_is_valid = true;
#endif
  Primitives::iterator pi;
  for (pi = cdata->_primitives.begin(); pi != cdata->_primitives.end(); ++pi) {
    CPT(GeomPrimitive) new_prim = (*pi).get_read_pointer()->make_points();
    (*pi) = (GeomPrimitive *)new_prim.p();

#ifndef NDEBUG
    if (!new_prim->check_valid(cdata->_data.get_read_pointer())) {
      all_is_valid = false;
    }
#endif
  }

  cdata->_modified = Geom::get_next_modified();
  reset_geom_rendering(cdata);
  clear_cache_stage(current_thread);

  nassertv(all_is_valid);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::make_patches_in_place
//       Access: Published
//  Description: Replaces the GeomPrimitives within this Geom with
//               corresponding GeomPatches.  See
//               GeomPrimitive::make_patches().
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
void Geom::
make_patches_in_place() {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true, current_thread);

#ifndef NDEBUG
  bool all_is_valid = true;
#endif
  Primitives::iterator pi;
  for (pi = cdata->_primitives.begin(); pi != cdata->_primitives.end(); ++pi) {
    CPT(GeomPrimitive) new_prim = (*pi).get_read_pointer()->make_patches();
    (*pi) = (GeomPrimitive *)new_prim.p();

#ifndef NDEBUG
    if (!new_prim->check_valid(cdata->_data.get_read_pointer())) {
      all_is_valid = false;
    }
#endif
  }

  cdata->_modified = Geom::get_next_modified();
  reset_geom_rendering(cdata);
  clear_cache_stage(current_thread);

  nassertv(all_is_valid);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::copy_primitives_from
//       Access: Published, Virtual
//  Description: Copies the primitives from the indicated Geom into
//               this one.  This does require that both Geoms contain
//               the same fundamental type primitives, both have a
//               compatible shade model, and both use the same
//               GeomVertexData.  Both Geoms must also be the same
//               specific class type (i.e. if one is a GeomTextGlyph,
//               they both must be.)
//
//               Returns true if the copy is successful, or false
//               otherwise (because the Geoms were mismatched).
////////////////////////////////////////////////////////////////////
bool Geom::
copy_primitives_from(const Geom *other) {
  if (get_primitive_type() != PT_none &&
      other->get_primitive_type() != get_primitive_type()) {
    return false;
  }
  if (get_vertex_data() != other->get_vertex_data()) {
    return false;
  }
  if (get_type() != other->get_type()) {
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
//     Function: Geom::get_num_bytes
//       Access: Published
//  Description: Returns the number of bytes consumed by the geom and
//               its primitives (but not including its vertex table).
////////////////////////////////////////////////////////////////////
int Geom::
get_num_bytes() const {
  CDReader cdata(_cycler);

  int num_bytes = sizeof(Geom);
  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin();
       pi != cdata->_primitives.end();
       ++pi) {
    num_bytes += (*pi).get_read_pointer()->get_num_bytes();
  }

  return num_bytes;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::request_resident
//       Access: Published
//  Description: Returns true if all the primitive arrays are
//               currently resident in memory.  If this returns false,
//               the data will be brought back into memory shortly;
//               try again later.
//
//               This does not also test the Geom's associated
//               GeomVertexData.  That must be tested separately.
////////////////////////////////////////////////////////////////////
bool Geom::
request_resident() const {
  CDReader cdata(_cycler);

  bool resident = true;

  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin();
       pi != cdata->_primitives.end();
       ++pi) {
    if (!(*pi).get_read_pointer()->request_resident()) {
      resident = false;
    }
  }

  return resident;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::transform_vertices
//       Access: Published
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
void Geom::
transform_vertices(const LMatrix4 &mat) {
  PT(GeomVertexData) new_data = modify_vertex_data();
  CPT(GeomVertexFormat) format = new_data->get_format();

  int ci;
  for (ci = 0; ci < format->get_num_points(); ci++) {
    GeomVertexRewriter data(new_data, format->get_point(ci));

    while (!data.is_at_end()) {
      const LPoint3 &point = data.get_data3();
      data.set_data3(point * mat);
    }
  }
  for (ci = 0; ci < format->get_num_vectors(); ci++) {
    GeomVertexRewriter data(new_data, format->get_vector(ci));

    while (!data.is_at_end()) {
      const LVector3 &vector = data.get_data3();
      data.set_data3(normalize(vector * mat));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::check_valid
//       Access: Published
//  Description: Verifies that the all of the primitives within the
//               geom reference vertices that actually exist within
//               the geom's GeomVertexData.  Returns true if the geom
//               appears to be valid, false otherwise.
////////////////////////////////////////////////////////////////////
bool Geom::
check_valid() const {
  Thread *current_thread = Thread::get_current_thread();
  GeomPipelineReader geom_reader(this, current_thread);
  GeomVertexDataPipelineReader data_reader(geom_reader.get_vertex_data(), current_thread);
  data_reader.check_array_readers();
  return geom_reader.check_valid(&data_reader);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::check_valid
//       Access: Published
//  Description: Verifies that the all of the primitives within the
//               geom reference vertices that actually exist within
//               the indicated GeomVertexData.  Returns true if the
//               geom appears to be valid, false otherwise.
////////////////////////////////////////////////////////////////////
bool Geom::
check_valid(const GeomVertexData *vertex_data) const {
  Thread *current_thread = Thread::get_current_thread();
  GeomPipelineReader geom_reader(this, current_thread);
  GeomVertexDataPipelineReader data_reader(vertex_data, current_thread);
  data_reader.check_array_readers();
  return geom_reader.check_valid(&data_reader);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_bounds
//       Access: Published
//  Description: Returns the bounding volume for the Geom.
////////////////////////////////////////////////////////////////////
CPT(BoundingVolume) Geom::
get_bounds(Thread *current_thread) const {
  CDLockedReader cdata(_cycler, current_thread);
  if (cdata->_user_bounds != (BoundingVolume *)NULL) {
    return cdata->_user_bounds;
  }

  if (cdata->_internal_bounds_stale) {
    CDWriter cdataw(((Geom *)this)->_cycler, cdata, false);
    compute_internal_bounds(cdataw, current_thread);
    return cdataw->_internal_bounds;
  }
  return cdata->_internal_bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_nested_vertices
//       Access: Published
//  Description: Returns the number of vertices rendered by all
//               primitives within the Geom.
////////////////////////////////////////////////////////////////////
int Geom::
get_nested_vertices(Thread *current_thread) const {
  CDLockedReader cdata(_cycler, current_thread);
  if (cdata->_internal_bounds_stale) {
    CDWriter cdataw(((Geom *)this)->_cycler, cdata, false);
    compute_internal_bounds(cdataw, current_thread);
    return cdataw->_nested_vertices;
  }
  return cdata->_nested_vertices;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::output
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void Geom::
output(ostream &out) const {
  CDReader cdata(_cycler);

  // Get a list of the primitive types contained within this object.
  int num_faces = 0;
  pset<TypeHandle> types;
  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin();
       pi != cdata->_primitives.end();
       ++pi) {
    CPT(GeomPrimitive) prim = (*pi).get_read_pointer();
    num_faces += prim->get_num_faces();
    types.insert(prim->get_type());
  }

  out << get_type() << " [";
  pset<TypeHandle>::iterator ti;
  for (ti = types.begin(); ti != types.end(); ++ti) {
    out << " " << (*ti);
  }
  out << " ], " << num_faces << " faces";
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::write
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void Geom::
write(ostream &out, int indent_level) const {
  CDReader cdata(_cycler);

  // Get a list of the primitive types contained within this object.
  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin();
       pi != cdata->_primitives.end();
       ++pi) {
    (*pi).get_read_pointer()->write(out, indent_level);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::clear_cache
//       Access: Published
//  Description: Removes all of the previously-cached results of
//               munge_geom().
//
//               This blows away the entire cache, upstream and
//               downstream the pipeline.  Use clear_cache_stage()
//               instead if you only want to blow away the cache at
//               the current stage and upstream.
////////////////////////////////////////////////////////////////////
void Geom::
clear_cache() {
  LightMutexHolder holder(_cache_lock);
  for (Cache::iterator ci = _cache.begin();
       ci != _cache.end();
       ++ci) {
    CacheEntry *entry = (*ci).second;
    entry->erase();
  }
  _cache.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::clear_cache_stage
//       Access: Published
//  Description: Removes all of the previously-cached results of
//               munge_geom(), at the current pipeline stage and
//               upstream.  Does not affect the downstream cache.
//
//               Don't call this in a downstream thread unless you
//               don't mind it blowing away other changes you might
//               have recently made in an upstream thread.
////////////////////////////////////////////////////////////////////
void Geom::
clear_cache_stage(Thread *current_thread) {
  LightMutexHolder holder(_cache_lock);
  for (Cache::iterator ci = _cache.begin();
       ci != _cache.end();
       ++ci) {
    CacheEntry *entry = (*ci).second;
    CDCacheWriter cdata(entry->_cycler, current_thread);
    cdata->set_result(NULL, NULL);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::prepare
//       Access: Published
//  Description: Indicates that the geom should be enqueued to be
//               prepared in the indicated prepared_objects at the
//               beginning of the next frame.  This will ensure the
//               geom is already loaded into geom memory if it
//               is expected to be rendered soon.
//
//               Use this function instead of prepare_now() to preload
//               geoms from a user interface standpoint.
////////////////////////////////////////////////////////////////////
void Geom::
prepare(PreparedGraphicsObjects *prepared_objects) {
  prepared_objects->enqueue_geom(this);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::is_prepared
//       Access: Published
//  Description: Returns true if the geom has already been prepared
//               or enqueued for preparation on the indicated GSG,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool Geom::
is_prepared(PreparedGraphicsObjects *prepared_objects) const {
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    return true;
  }
  return prepared_objects->is_geom_queued(this);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::release
//       Access: Published
//  Description: Frees the geom context only on the indicated object,
//               if it exists there.  Returns true if it was released,
//               false if it had not been prepared.
////////////////////////////////////////////////////////////////////
bool Geom::
release(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    GeomContext *gc = (*ci).second;
    prepared_objects->release_geom(gc);
    return true;
  }

  // Maybe it wasn't prepared yet, but it's about to be.
  return prepared_objects->dequeue_geom(this);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::release_all
//       Access: Published
//  Description: Frees the context allocated on all objects for which
//               the geom has been declared.  Returns the number of
//               contexts which have been freed.
////////////////////////////////////////////////////////////////////
int Geom::
release_all() {
  // We have to traverse a copy of the _contexts list, because the
  // PreparedGraphicsObjects object will call clear_prepared() in response
  // to each release_geom(), and we don't want to be modifying the
  // _contexts list while we're traversing it.
  Contexts temp = _contexts;
  int num_freed = (int)_contexts.size();

  Contexts::const_iterator ci;
  for (ci = temp.begin(); ci != temp.end(); ++ci) {
    PreparedGraphicsObjects *prepared_objects = (*ci).first;
    GeomContext *gc = (*ci).second;
    prepared_objects->release_geom(gc);
  }

  // Now that we've called release_geom() on every known context,
  // the _contexts list should have completely emptied itself.
  nassertr(_contexts.empty(), num_freed);

  return num_freed;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::prepare_now
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
GeomContext *Geom::
prepare_now(PreparedGraphicsObjects *prepared_objects,
            GraphicsStateGuardianBase *gsg) {
  Contexts::const_iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    return (*ci).second;
  }

  GeomContext *gc = prepared_objects->prepare_geom_now(this, gsg);
  if (gc != (GeomContext *)NULL) {
    _contexts[prepared_objects] = gc;
  }
  return gc;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::draw
//       Access: Public
//  Description: Actually draws the Geom with the indicated GSG, using
//               the indicated vertex data (which might have been
//               pre-munged to support the GSG's needs).
//
//               Returns true if all of the primitives were drawn
//               normally, false if there was a problem (for instance,
//               some of the data was nonresident).  If force is
//               passed true, it will wait for the data to become
//               resident if necessary.
////////////////////////////////////////////////////////////////////
bool Geom::
draw(GraphicsStateGuardianBase *gsg, const GeomMunger *munger,
     const GeomVertexData *vertex_data, bool force,
     Thread *current_thread) const {
  GeomPipelineReader geom_reader(this, current_thread);
  geom_reader.check_usage_hint();

  GeomVertexDataPipelineReader data_reader(vertex_data, current_thread);
  data_reader.check_array_readers();

  return geom_reader.draw(gsg, munger, &data_reader, force);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::get_next_modified
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
UpdateSeq Geom::
get_next_modified() {
  ++_next_modified;
  return _next_modified;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::compute_internal_bounds
//       Access: Private
//  Description: Recomputes the dynamic bounding volume for this Geom.
//               This includes all of the vertices.
////////////////////////////////////////////////////////////////////
void Geom::
compute_internal_bounds(Geom::CData *cdata, Thread *current_thread) const {
  int num_vertices = 0;

  // Get the vertex data, after animation.
  CPT(GeomVertexData) vertex_data = cdata->_data.get_read_pointer();
  vertex_data = vertex_data->animate_vertices(true, current_thread);

  // Now actually compute the bounding volume.  We do this by using
  // calc_tight_bounds to determine our box first.
  LPoint3 pmin, pmax;
  PN_stdfloat sq_center_dist;
  bool found_any = false;
  do_calc_tight_bounds(pmin, pmax, sq_center_dist, found_any,
                       vertex_data, false, LMatrix4::ident_mat(),
                       InternalName::get_vertex(),
                       cdata, current_thread);

  BoundingVolume::BoundsType btype = cdata->_bounds_type;
  if (btype == BoundingVolume::BT_default) {
    btype = bounds_type;
  }

  if (found_any) {
    // Then we put the bounding volume around both of those points.
    PN_stdfloat avg_box_area;
    switch (btype) {
    case BoundingVolume::BT_best:
    case BoundingVolume::BT_fastest:
    case BoundingVolume::BT_default:
      {
        // When considering a box, calculate (roughly) the average area
        // of the sides.  We will use this to determine whether a sphere
        // or box is a better fit.
        PN_stdfloat min_extent = min(pmax[0] - pmin[0],
                                 min(pmax[1] - pmin[1],
                                     pmax[2] - pmin[2]));
        PN_stdfloat max_extent = max(pmax[0] - pmin[0],
                                 max(pmax[1] - pmin[1],
                                     pmax[2] - pmin[2]));
        avg_box_area = ((min_extent * min_extent) + (max_extent * max_extent)) / 2;
      }
      //  Fall through
    case BoundingVolume::BT_sphere:
      {
        // Determine the best radius for a bounding sphere.
        LPoint3 aabb_center = (pmin + pmax) * 0.5f;
        PN_stdfloat best_sq_radius = (pmax - aabb_center).length_squared();

        if (btype != BoundingVolume::BT_fastest &&
            aabb_center.length_squared() / best_sq_radius >= (0.2f * 0.2f)) {
          // Hmm, this is an off-center model.  Maybe we can do a better
          // job by calculating the bounding sphere from the AABB center.

          PN_stdfloat better_sq_radius;
          bool found_any = false;
          do_calc_sphere_radius(aabb_center, better_sq_radius, found_any,
                                vertex_data, cdata, current_thread);

          if (found_any && better_sq_radius <= best_sq_radius) {
            // Great.  This is as good a sphere as we're going to get.
            if (btype == BoundingVolume::BT_best &&
                avg_box_area < better_sq_radius * MathNumbers::pi) {
              // But the box is better, anyway.  Use that instead.
              cdata->_internal_bounds = new BoundingBox(pmin, pmax);
              break;
            }
            cdata->_internal_bounds =
              new BoundingSphere(aabb_center, csqrt(better_sq_radius));
            break;
          }
        }

        if (btype != BoundingVolume::BT_sphere &&
            avg_box_area < sq_center_dist * MathNumbers::pi) {
          // A box is probably a tighter fit.
          cdata->_internal_bounds = new BoundingBox(pmin, pmax);
          break;

        } else if (sq_center_dist <= best_sq_radius) {
          // No, but a sphere centered on the origin is apparently
          // still better than a sphere around the bounding box.
          cdata->_internal_bounds =
            new BoundingSphere(LPoint3::origin(), csqrt(sq_center_dist));
          break;

        } else if (btype == BoundingVolume::BT_sphere) {
          // This is the worst sphere we can make, which is why we will only
          // do it when the user specifically requests a sphere.
          cdata->_internal_bounds =
            new BoundingSphere(aabb_center, csqrt(best_sq_radius));
          break;
        }
      }
      // Fall through.

    case BoundingVolume::BT_box:
      cdata->_internal_bounds = new BoundingBox(pmin, pmax);
    }

    Primitives::const_iterator pi;
    for (pi = cdata->_primitives.begin();
         pi != cdata->_primitives.end();
         ++pi) {
      CPT(GeomPrimitive) prim = (*pi).get_read_pointer();
      num_vertices += prim->get_num_vertices();
    }

  } else {
    // No points; empty bounding volume.
    if (btype == BoundingVolume::BT_sphere) {
      cdata->_internal_bounds = new BoundingSphere;
    } else {
      cdata->_internal_bounds = new BoundingBox;
    }
  }

  cdata->_nested_vertices = num_vertices;
  cdata->_internal_bounds_stale = false;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::do_calc_tight_bounds
//       Access: Private
//  Description: The private implementation of calc_tight_bounds().
////////////////////////////////////////////////////////////////////
void Geom::
do_calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                     PN_stdfloat &sq_center_dist, bool &found_any,
                     const GeomVertexData *vertex_data,
                     bool got_mat, const LMatrix4 &mat,
                     const InternalName *column_name,
                     const CData *cdata, Thread *current_thread) const {
  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin();
       pi != cdata->_primitives.end();
       ++pi) {
    CPT(GeomPrimitive) prim = (*pi).get_read_pointer();
    prim->calc_tight_bounds(min_point, max_point, sq_center_dist,
                            found_any, vertex_data, got_mat, mat,
                            column_name, current_thread);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::do_calc_sphere_radius
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void Geom::
do_calc_sphere_radius(const LPoint3 &center, PN_stdfloat &sq_radius,
                      bool &found_any, const GeomVertexData *vertex_data,
                      const CData *cdata, Thread *current_thread) const {
  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin();
       pi != cdata->_primitives.end();
       ++pi) {
    CPT(GeomPrimitive) prim = (*pi).get_read_pointer();
    prim->calc_sphere_radius(center, sq_radius, found_any,
                             vertex_data, current_thread);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::clear_prepared
//       Access: Private
//  Description: Removes the indicated PreparedGraphicsObjects table
//               from the Geom's table, without actually releasing
//               the geom.  This is intended to be called only from
//               PreparedGraphicsObjects::release_geom(); it should
//               never be called by user code.
////////////////////////////////////////////////////////////////////
void Geom::
clear_prepared(PreparedGraphicsObjects *prepared_objects) {
  Contexts::iterator ci;
  ci = _contexts.find(prepared_objects);
  if (ci != _contexts.end()) {
    _contexts.erase(ci);
  } else {
    // If this assertion fails, clear_prepared() was given a
    // prepared_objects that the geom didn't know about.
    nassertv(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::check_will_be_valid
//       Access: Private
//  Description: Verifies that the all of the primitives within the
//               geom reference vertices that actually exist within
//               the indicated GeomVertexData (presumably in
//               preparation for assigning the geom to use this data).
//               Returns true if the data appears to be valid, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool Geom::
check_will_be_valid(const GeomVertexData *vertex_data) const {
  CDReader cdata(_cycler);

  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin();
       pi != cdata->_primitives.end();
       ++pi) {
    if (!(*pi).get_read_pointer()->check_valid(vertex_data)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::reset_usage_hint
//       Access: Private
//  Description: Recomputes the minimum usage_hint.
////////////////////////////////////////////////////////////////////
void Geom::
reset_usage_hint(Geom::CData *cdata) {
  cdata->_usage_hint = UH_unspecified;
  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin();
       pi != cdata->_primitives.end();
       ++pi) {
    cdata->_usage_hint = min(cdata->_usage_hint,
                             (*pi).get_read_pointer()->get_usage_hint());
  }
  cdata->_got_usage_hint = true;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::reset_geom_rendering
//       Access: Private
//  Description: Rederives the _geom_rendering member.
////////////////////////////////////////////////////////////////////
void Geom::
reset_geom_rendering(Geom::CData *cdata) {
  cdata->_geom_rendering = 0;
  Primitives::const_iterator pi;
  for (pi = cdata->_primitives.begin();
       pi != cdata->_primitives.end();
       ++pi) {
    cdata->_geom_rendering |= (*pi).get_read_pointer()->get_geom_rendering();
  }

  if ((cdata->_geom_rendering & GR_point) != 0) {
    CPT(GeomVertexData) data = cdata->_data.get_read_pointer();
    if (data->has_column(InternalName::get_size())) {
      cdata->_geom_rendering |= GR_per_point_size;
    }
    if (data->has_column(InternalName::get_aspect_ratio())) {
      cdata->_geom_rendering |= GR_point_aspect_ratio;
    }
    if (data->has_column(InternalName::get_rotate())) {
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
//     Function: Geom::combine_primitives
//       Access: Private
//  Description: Combines two primitives of the same type into a
//               single primitive.  a_prim is modified to append the
//               vertices from b_prim, which is unmodified.
////////////////////////////////////////////////////////////////////
void Geom::
combine_primitives(GeomPrimitive *a_prim, const GeomPrimitive *b_prim,
                   Thread *current_thread) {
  nassertv(a_prim != b_prim);
  nassertv(a_prim->get_type() == b_prim->get_type());

  CPT(GeomPrimitive) b_prim2 = b_prim;

  if (a_prim->get_index_type() != b_prim2->get_index_type()) {
    GeomPrimitive::NumericType index_type = max(a_prim->get_index_type(), b_prim2->get_index_type());
    a_prim->set_index_type(index_type);
    if (b_prim2->get_index_type() != index_type) {
      PT(GeomPrimitive) b_prim_copy = b_prim2->make_copy();
      b_prim_copy->set_index_type(index_type);
      b_prim2 = b_prim_copy;
    }
  }

  if (!b_prim2->is_indexed()) {
    PT(GeomPrimitive) b_prim_copy = b_prim2->make_copy();
    b_prim_copy->make_indexed();
    b_prim2 = b_prim_copy;
  }

  PT(GeomVertexArrayData) a_vertices = a_prim->modify_vertices();
  CPT(GeomVertexArrayData) b_vertices = b_prim2->get_vertices();

  if (a_prim->requires_unused_vertices()) {
    GeomVertexReader index(b_vertices, 0);
    int b_vertex = index.get_data1i();
    a_prim->append_unused_vertices(a_vertices, b_vertex);
  }

  PT(GeomVertexArrayDataHandle) a_handle = a_vertices->modify_handle(current_thread);
  CPT(GeomVertexArrayDataHandle) b_handle = b_vertices->get_handle(current_thread);

  size_t orig_a_vertices = a_handle->get_num_rows();

  a_handle->copy_subdata_from(a_handle->get_data_size_bytes(), 0,
                              b_handle, 0, b_handle->get_data_size_bytes());
  a_prim->clear_minmax();
  if (a_prim->is_composite()) {
    // Also copy the ends array.
    PTA_int a_ends = a_prim->modify_ends();
    CPTA_int b_ends = b_prim2->get_ends();
    for (size_t i = 0; i < b_ends.size(); ++i) {
      a_ends.push_back(b_ends[i] + orig_a_vertices);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               Geom.
////////////////////////////////////////////////////////////////////
void Geom::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void Geom::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  manager->write_cdata(dg, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type Geom is encountered
//               in the Bam file.  It should create the Geom
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *Geom::
make_from_bam(const FactoryParams &params) {
  Geom *object = new Geom(NULL);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  object->fillin(scan, manager);
  manager->register_finalize(object);

  return object;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::finalize
//       Access: Public, Virtual
//  Description: Called by the BamReader to perform any final actions
//               needed for setting up the object after all objects
//               have been read and all pointers have been completed.
////////////////////////////////////////////////////////////////////
void Geom::
finalize(BamReader *manager) {
  CDWriter cdata(_cycler, true);

  // Make sure our GeomVertexData is finalized first.  This may result
  // in the data getting finalized multiple times, but it doesn't mind
  // that.
  if (!cdata->_data.is_null()) {
    // We shouldn't call get_write_pointer(), which might replicate
    // the GeomVertexData unnecessarily.
    cdata->_data.get_unsafe_pointer()->finalize(manager);
  }

  reset_geom_rendering(cdata);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new Geom.
////////////////////////////////////////////////////////////////////
void Geom::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  manager->read_cdata(scan, _cycler);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CDataCache::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
Geom::CDataCache::
~CDataCache() {
  set_result(NULL, NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CDataCache::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *Geom::CDataCache::
make_copy() const {
  return new CDataCache(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CacheEntry::evict_callback
//       Access: Public, Virtual
//  Description: Called when the entry is evicted from the cache, this
//               should clean up the owning object appropriately.
////////////////////////////////////////////////////////////////////
void Geom::CacheEntry::
evict_callback() {
  LightMutexHolder holder(_source->_cache_lock);
  Cache::iterator ci = _source->_cache.find(&_key);
  nassertv(ci != _source->_cache.end());
  nassertv((*ci).second == this);
  _source->_cache.erase(ci);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CacheEntry::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void Geom::CacheEntry::
output(ostream &out) const {
  out << "geom " << (void *)_source << ", "
      << (const void *)_key._modifier;
}


////////////////////////////////////////////////////////////////////
//     Function: Geom::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *Geom::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void Geom::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  manager->write_pointer(dg, _data.get_read_pointer());

  dg.add_uint16(_primitives.size());
  Primitives::const_iterator pi;
  for (pi = _primitives.begin(); pi != _primitives.end(); ++pi) {
    manager->write_pointer(dg, (*pi).get_read_pointer());
  }

  dg.add_uint8(_primitive_type);
  dg.add_uint8(_shade_model);

  // Actually, we shouldn't bother writing out _geom_rendering; we'll
  // just throw it away anyway.
  dg.add_uint16(_geom_rendering);

  dg.add_uint8(_bounds_type);
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CData::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int Geom::CData::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = CycleData::complete_pointers(p_list, manager);

  _data = DCAST(GeomVertexData, p_list[pi++]);

  Primitives::iterator pri;
  for (pri = _primitives.begin(); pri != _primitives.end(); ++pri) {
    (*pri) = DCAST(GeomPrimitive, p_list[pi++]);
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: Geom::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new Geom.
////////////////////////////////////////////////////////////////////
void Geom::CData::
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

  // To be removed: we no longer read _geom_rendering from the bam
  // file; instead, we rederive it in finalize().
  scan.get_uint16();

  _got_usage_hint = false;
  _modified = Geom::get_next_modified();

  _bounds_type = BoundingVolume::BT_default;
  if (manager->get_file_minor_ver() >= 19) {
    _bounds_type = (BoundingVolume::BoundsType)scan.get_uint8();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::check_usage_hint
//       Access: Public
//  Description: Ensures that the Geom's usage_hint cache has been
//               computed.
////////////////////////////////////////////////////////////////////
void GeomPipelineReader::
check_usage_hint() const {
  if (!_cdata->_got_usage_hint) {
    // We'll need to get a fresh pointer, since another thread might
    // already have modified the pointer on the object since we
    // queried it.
    {
#ifdef DO_PIPELINING
      unref_delete((CycleData *)_cdata);
#endif
      Geom::CDWriter fresh_cdata(((Geom *)_object.p())->_cycler,
                                 false, _current_thread);
      ((GeomPipelineReader *)this)->_cdata = fresh_cdata;
#ifdef DO_PIPELINING
      _cdata->ref();
#endif
      if (!fresh_cdata->_got_usage_hint) {
        // The cache is still stale.  We have to do the work of
        // freshening it.
        ((Geom *)_object.p())->reset_usage_hint(fresh_cdata);
        nassertv(fresh_cdata->_got_usage_hint);
      }

      // When fresh_cdata goes out of scope, its write lock is
      // released, and _cdata reverts to our usual convention of an
      // unlocked copy of the data.
    }
  }

  nassertv(_cdata->_got_usage_hint);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::check_valid
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool GeomPipelineReader::
check_valid(const GeomVertexDataPipelineReader *data_reader) const {
  Geom::Primitives::const_iterator pi;
  for (pi = _cdata->_primitives.begin();
       pi != _cdata->_primitives.end();
       ++pi) {
    CPT(GeomPrimitive) primitive = (*pi).get_read_pointer();
    GeomPrimitivePipelineReader reader(primitive, _current_thread);
    reader.check_minmax();
    if (!reader.check_valid(data_reader)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPipelineReader::draw
//       Access: Public
//  Description: The implementation of Geom::draw().
////////////////////////////////////////////////////////////////////
bool GeomPipelineReader::
draw(GraphicsStateGuardianBase *gsg, const GeomMunger *munger,
     const GeomVertexDataPipelineReader *data_reader, bool force) const {
  PStatTimer timer(Geom::_draw_primitive_setup_pcollector);
  bool all_ok = gsg->begin_draw_primitives(this, munger, data_reader, force);
  if (all_ok) {
    Geom::Primitives::const_iterator pi;
    for (pi = _cdata->_primitives.begin();
         pi != _cdata->_primitives.end();
         ++pi) {
      CPT(GeomPrimitive) primitive = (*pi).get_read_pointer();
      GeomPrimitivePipelineReader reader(primitive, _current_thread);
      if (reader.get_num_vertices() != 0) {
        reader.check_minmax();
        nassertr(reader.check_valid(data_reader), false);
        if (!primitive->draw(gsg, &reader, force)) {
          all_ok = false;
        }
      }
    }
    gsg->end_draw_primitives();
  }

  return all_ok;
}
