// Filename: eggPrimitive.cxx
// Created by:  drose (16Jan99)
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

#include "eggPrimitive.h"
#include "eggVertexPool.h"
#include "eggMiscFuncs.h"
#include "eggTextureCollection.h"
#include "lexerDefs.h"

#include "indent.h"
#include "vector_int.h"

TypeHandle EggPrimitive::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::determine_alpha_mode
//       Access: Published, Virtual
//  Description: Walks back up the hierarchy, looking for an EggPrimitive
//               or EggPrimitive or some such object at this level or
//               above this primitive that has an alpha_mode other than
//               AM_unspecified.  Returns a valid EggRenderMode pointer
//               if one is found, or NULL otherwise.
////////////////////////////////////////////////////////////////////
EggRenderMode *EggPrimitive::
determine_alpha_mode() {
  if (get_alpha_mode() != AM_unspecified) {
    return this;
  }

  EggRenderMode *result = EggNode::determine_alpha_mode();
  if (result == (EggRenderMode *)NULL) {
    int num_textures = get_num_textures();
    for (int i = 0; i < num_textures && result == (EggRenderMode *)NULL; i++) {
      if (get_texture(i)->get_alpha_mode() != AM_unspecified) {
        result = get_texture(i);
      }
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::determine_depth_write_mode
//       Access: Published, Virtual
//  Description: Walks back up the hierarchy, looking for an EggGroup
//               or EggPrimitive or some such object at this level or
//               above this node that has a depth_write_mode other than
//               DWM_unspecified.  Returns a valid EggRenderMode pointer
//               if one is found, or NULL otherwise.
////////////////////////////////////////////////////////////////////
EggRenderMode *EggPrimitive::
determine_depth_write_mode() {
  if (get_depth_write_mode() != DWM_unspecified) {
    return this;
  }

  EggRenderMode *result = EggNode::determine_depth_write_mode();
  if (result == (EggRenderMode *)NULL) {
    int num_textures = get_num_textures();
    for (int i = 0; i < num_textures && result == (EggRenderMode *)NULL; i++) {
      if (get_texture(i)->get_depth_write_mode() != DWM_unspecified) {
        result = get_texture(i);
      }
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::determine_depth_test_mode
//       Access: Published, Virtual
//  Description: Walks back up the hierarchy, looking for an EggGroup
//               or EggPrimitive or some such object at this level or
//               above this node that has a depth_test_mode other than
//               DTM_unspecified.  Returns a valid EggRenderMode pointer
//               if one is found, or NULL otherwise.
////////////////////////////////////////////////////////////////////
EggRenderMode *EggPrimitive::
determine_depth_test_mode() {
  if (get_depth_test_mode() != DTM_unspecified) {
    return this;
  }

  EggRenderMode *result = EggNode::determine_depth_test_mode();
  if (result == (EggRenderMode *)NULL) {
    int num_textures = get_num_textures();
    for (int i = 0; i < num_textures && result == (EggRenderMode *)NULL; i++) {
      if (get_texture(i)->get_depth_test_mode() != DTM_unspecified) {
        result = get_texture(i);
      }
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::determine_visibility_mode
//       Access: Published, Virtual
//  Description: Walks back up the hierarchy, looking for an EggGroup
//               or EggPrimitive or some such object at this level or
//               above this node that has a visibility_mode other than
//               VM_unspecified.  Returns a valid EggRenderMode pointer
//               if one is found, or NULL otherwise.
////////////////////////////////////////////////////////////////////
EggRenderMode *EggPrimitive::
determine_visibility_mode() {
  if (get_visibility_mode() != VM_unspecified) {
    return this;
  }

  EggRenderMode *result = EggNode::determine_visibility_mode();
  if (result == (EggRenderMode *)NULL) {
    int num_textures = get_num_textures();
    for (int i = 0; i < num_textures && result == (EggRenderMode *)NULL; i++) {
      if (get_texture(i)->get_visibility_mode() != VM_unspecified) {
        result = get_texture(i);
      }
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::determine_draw_order
//       Access: Published, Virtual
//  Description: Walks back up the hierarchy, looking for an EggPrimitive
//               or EggPrimitive or some such object at this level or
//               above this primitive that has a draw_order specified.
//               Returns a valid EggRenderMode pointer if one is found,
//               or NULL otherwise.
////////////////////////////////////////////////////////////////////
EggRenderMode *EggPrimitive::
determine_draw_order() {
  if (has_draw_order()) {
    return this;
  }

  EggRenderMode *result = EggNode::determine_draw_order();
  if (result == (EggRenderMode *)NULL) {
    int num_textures = get_num_textures();
    for (int i = 0; i < num_textures && result == (EggRenderMode *)NULL; i++) {
      if (get_texture(i)->has_draw_order()) {
        result = get_texture(i);
      }
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::determine_bin
//       Access: Published, Virtual
//  Description: Walks back up the hierarchy, looking for an EggPrimitive
//               or EggPrimitive or some such object at this level or
//               above this primitive that has a bin specified.  Returns a
//               valid EggRenderMode pointer if one is found, or NULL
//               otherwise.
////////////////////////////////////////////////////////////////////
EggRenderMode *EggPrimitive::
determine_bin() {
  if (has_bin()) {
    return this;
  }

  EggRenderMode *result = EggNode::determine_bin();
  if (result == (EggRenderMode *)NULL) {
    int num_textures = get_num_textures();
    for (int i = 0; i < num_textures && result == (EggRenderMode *)NULL; i++) {
      if (get_texture(i)->has_bin()) {
        result = get_texture(i);
      }
    }
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::copy_attributes
//       Access: Published
//  Description: Copies the rendering attributes from the indicated
//               primitive.
////////////////////////////////////////////////////////////////////
void EggPrimitive::
copy_attributes(const EggAttributes &other) {
  EggAttributes::operator = (other);
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::copy_attributes
//       Access: Published
//  Description: Copies the rendering attributes from the indicated
//               primitive.
////////////////////////////////////////////////////////////////////
void EggPrimitive::
copy_attributes(const EggPrimitive &other) {
  EggAttributes::operator = (other);
  _textures = other._textures;
  set_material(other.get_material());
  set_bface_flag(other.get_bface_flag());
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::has_vertex_normal
//       Access: Published
//  Description: Returns true if any vertex on the primitive has a
//               specific normal set, false otherwise.
//
//               For the most accurate measure of this, call
//               unify_attributes() first.
////////////////////////////////////////////////////////////////////
bool EggPrimitive::
has_vertex_normal() const {
  Vertices::const_iterator vi;
  for (vi = _vertices.begin(); vi != _vertices.end(); ++vi) {
    if ((*vi)->has_normal()) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::has_vertex_color
//       Access: Published
//  Description: Returns true if any vertex on the primitive has a
//               specific color set, false otherwise.
//
//               For the most accurate measure of this, call
//               unify_attributes() first.
////////////////////////////////////////////////////////////////////
bool EggPrimitive::
has_vertex_color() const {
  Vertices::const_iterator vi;
  for (vi = _vertices.begin(); vi != _vertices.end(); ++vi) {
    if ((*vi)->has_color()) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::is_flat_shaded
//       Access: Published
//  Description: Returns true if the primitive is flat shaded, meaning
//               it has no per-vertex normal and no per-vertex color
//               (but in the case of a composite primitive, it may
//               still have per-component normal and color).
//
//               For the most accurate measure of this, call
//               unify_attributes() first.
////////////////////////////////////////////////////////////////////
bool EggPrimitive::
is_flat_shaded() const {
  return !has_vertex_normal() && !has_vertex_color();
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::unify_attributes
//       Access: Published, Virtual
//  Description: Applies per-vertex normal and color to all vertices,
//               if they are in fact per-vertex (and actually
//               different for each vertex), or moves them to the
//               primitive if they are all the same.
//
//               After this call, either the primitive will have
//               normals or its vertices will, but not both.  Ditto
//               for colors.
//
//               This may create redundant vertices in the vertex
//               pool.
////////////////////////////////////////////////////////////////////
void EggPrimitive::
unify_attributes() {
  typedef pvector< PT(EggVertex) > Vertices;
  Vertices vertices;

  // First, go through the vertices and apply the primitive overall
  // attributes to any vertex that omits them.
  bool all_have_normal = true;
  bool all_have_color = true;
  iterator pi;
  for (pi = begin(); pi != end(); ++pi) {
    EggVertex *orig_vertex = (*pi);
    PT(EggVertex) vertex = new EggVertex(*orig_vertex);

    if (!vertex->has_normal()) {
      if (has_normal()) {
        vertex->set_normal(get_normal());
        vertex->_dnormals = _dnormals;
      } else {
        all_have_normal = false;
      }
    }
    if (!vertex->has_color()) {
      if (has_color()) {
        vertex->set_color(get_color());
        vertex->_drgbas = _drgbas;
      } else {
        all_have_color = false;
      }
    }

    vertices.push_back(vertex);
  }

  clear_normal();
  clear_color();

  // Now see if any ended up different.
  EggAttributes overall;
  bool normal_different = false;
  bool color_different = false;

  Vertices::iterator vi;
  for (vi = vertices.begin(); vi != vertices.end(); ++vi) {
    EggVertex *vertex = (*vi);
    if (!all_have_normal) {
      vertex->clear_normal();
    } else if (!normal_different) {
      if (!overall.has_normal()) {
        overall.set_normal(vertex->get_normal());
        overall._dnormals = vertex->_dnormals;
      } else if (overall.get_normal() != vertex->get_normal() ||
                 overall._dnormals.compare_to(vertex->_dnormals) != 0) {
        normal_different = true;
      }
    }
    if (!all_have_color) {
      vertex->clear_color();
    } else if (!color_different) {
      if (!overall.has_color()) {
        overall.set_color(vertex->get_color());
        overall._drgbas = vertex->_drgbas;
      } else if (overall.get_color() != vertex->get_color() ||
                 overall._drgbas.compare_to(vertex->_drgbas) != 0) {
        color_different = true;
      }
    }
  }

  if (!overall.has_normal()) {
    normal_different = true;
  }
  if (!overall.has_color()) {
    color_different = true;
  }

  if (!color_different || !normal_different) {
    // Ok, it's flat-shaded after all.  Go back through and remove
    // this stuff from the vertices.
    if (!normal_different) {
      set_normal(overall.get_normal());
      _dnormals = overall._dnormals;
    }
    if (!color_different) {
      set_color(overall.get_color());
      _drgbas = overall._drgbas;
    }
    for (vi = vertices.begin(); vi != vertices.end(); ++vi) {
      EggVertex *vertex = (*vi);
      if (!normal_different) {
        vertex->clear_normal();
      }
      if (!color_different) {
        vertex->clear_color();
      }
    }
  }

  // Finally, move the new vertices to the vertex pool.
  EggVertexPool *vertex_pool = get_pool();
  for (size_t i = 0; i < vertices.size(); i++) {
    EggVertex *vertex = vertices[i];
    vertex = vertex_pool->create_unique_vertex(*vertex);
    set_vertex(i, vertex);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::apply_last_attribute
//       Access: Published, Virtual
//  Description: Sets the last vertex of the triangle (or each
//               component) to the primitive normal and/or color, if
//               the primitive is flat-shaded.  This reflects the
//               Panda convention of storing flat-shaded properties on
//               the last vertex, although it is not usually a
//               convention in Egg.
//
//               This may introduce redundant vertices to the vertex
//               pool.
////////////////////////////////////////////////////////////////////
void EggPrimitive::
apply_last_attribute() {
  if (!empty()) {
    // The significant_change flag is set if we have changed the
    // vertex in some important way, that will invalidate it for other
    // primitives that might share it.  We don't consider *adding* a
    // normal where there wasn't one before to be significant, but we
    // do consider it significant to change a vertex's normal to
    // something different.  Similarly for color.
    bool significant_change = false;

    EggVertex *orig_vertex = get_vertex(size() - 1);
    PT(EggVertex) new_vertex = new EggVertex(*orig_vertex);

    if (has_normal()) {
      new_vertex->set_normal(get_normal());
      new_vertex->_dnormals = _dnormals;

      if (orig_vertex->has_normal() &&
          (orig_vertex->get_normal() != new_vertex->get_normal() ||
           orig_vertex->_dnormals.compare_to(new_vertex->_dnormals) != 0)) {
        significant_change = true;
      }
    }
    if (has_color()) {
      new_vertex->set_color(get_color());
      new_vertex->_drgbas = _drgbas;

      if (orig_vertex->has_color() &&
          (orig_vertex->get_color() != new_vertex->get_color() ||
           orig_vertex->_drgbas.compare_to(new_vertex->_drgbas) != 0)) {
        significant_change = true;
      }
    }

    if (significant_change) {
      new_vertex = get_pool()->create_unique_vertex(*new_vertex);
      set_vertex(size() - 1, new_vertex);
    } else {
      // Just copy the new attributes back into the pool.
      ((EggAttributes *)orig_vertex)->operator = (*new_vertex);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::post_apply_last_attribute
//       Access: Published, Virtual
//  Description: Intended as a followup to apply_last_attribute(),
//               this also sets an attribute on the first vertices of
//               the primitive, if they don't already have an
//               attribute set, just so they end up with *something*.
////////////////////////////////////////////////////////////////////
void EggPrimitive::
post_apply_last_attribute() {
  if (!empty()) {
    for (int i = 0; i < (int)size(); i++) {
      EggVertex *vertex = get_vertex(i);

      if (has_normal() && !vertex->has_normal()) {
        vertex->set_normal(get_normal());
      }
      if (has_color() && !vertex->has_color()) {
        vertex->set_color(get_color());
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::reverse_vertex_ordering
//       Access: Published, Virtual
//  Description: Reverses the ordering of the vertices in this
//               primitive, if appropriate, in order to change the
//               direction the polygon appears to be facing.  Does not
//               adjust the surface normal, if any.
////////////////////////////////////////////////////////////////////
void EggPrimitive::
reverse_vertex_ordering() {
  // This really only makes sense for polygons.  Lights don't care
  // about vertex ordering, and NURBS surfaces have to do a bit more
  // work in addition to this.
  reverse(_vertices.begin(), _vertices.end());
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::cleanup
//       Access: Published, Virtual
//  Description: Cleans up modeling errors in whatever context this
//               makes sense.  For instance, for a polygon, this calls
//               remove_doubled_verts(true).  For a point, it calls
//               remove_nonunique_verts().  Returns true if the
//               primitive is valid, or false if it is degenerate.
////////////////////////////////////////////////////////////////////
bool EggPrimitive::
cleanup() {
  return !empty();
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::remove_doubled_verts
//       Access: Published
//  Description: Certain kinds of primitives, particularly polygons,
//               don't like to have the same vertex repeated
//               consecutively.  Unfortunately, some modeling programs
//               (like MultiGen) make this an easy mistake to make.
//
//               It's handy to have a function to remove these
//               redundant vertices.  If closed is true, it also
//               checks that the first and last vertices are not the
//               same.
//
//               This function identifies repeated vertices by pointer
//               only; it does not remove consecutive equivalent but
//               different vertices.
////////////////////////////////////////////////////////////////////
void EggPrimitive::
remove_doubled_verts(bool closed) {
  if (!_vertices.empty()) {
    Vertices new_vertices;
    Vertices::iterator vi, vlast;
    vi = _vertices.begin();
    new_vertices.push_back(*vi);
    int num_removed = 0;

    vlast = vi;
    ++vi;
    while (vi != _vertices.end()) {
      if ((*vi) != (*vlast)) {
        new_vertices.push_back(*vi);
      } else {
        prepare_remove_vertex(*vi, vi - _vertices.begin() - num_removed, 
                              _vertices.size() - num_removed);
        num_removed++;
      }
      vlast = vi;
      ++vi;
    }
    _vertices.swap(new_vertices);
  }

  if (closed) {
    // Then, if this is a polygon (which will be closed anyway),
    // remove the vertex from the end if it's a repeat of the
    // beginning.
    while (_vertices.size() > 1 && _vertices.back() == _vertices.front()) {
      prepare_remove_vertex(_vertices.back(), _vertices.size() - 1, 
                            _vertices.size());
      _vertices.pop_back();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::remove_nonunique_verts
//       Access: Published
//  Description: Removes any multiple appearances of the same vertex
//               from the primitive.  This primarily makes sense for a
//               point primitive, which is really a collection of
//               points and which doesn't make sense to include the
//               same point twice, in any order.
////////////////////////////////////////////////////////////////////
void EggPrimitive::
remove_nonunique_verts() {
  Vertices::iterator vi, vj;
  Vertices new_vertices;
  int num_removed = 0;

  for (vi = _vertices.begin(); vi != _vertices.end(); ++vi) {
    bool okflag = true;
    for (vj = _vertices.begin(); vj != vi && okflag; ++vj) {
      okflag = ((*vi) != (*vj));
    }
    if (okflag) {
      new_vertices.push_back(*vi);
    } else {
      prepare_remove_vertex(*vi, vi - _vertices.begin() - num_removed,
                            _vertices.size() - num_removed);
      num_removed++;
    }
  }

  _vertices.swap(new_vertices);
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::has_primitives
//       Access: Published, Virtual
//  Description: Returns true if there are any primitives
//               (e.g. polygons) defined within this group or below,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool EggPrimitive::
has_primitives() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::joint_has_primitives
//       Access: Published, Virtual
//  Description: Returns true if there are any primitives
//               (e.g. polygons) defined within this group or below,
//               but the search does not include nested joints.
////////////////////////////////////////////////////////////////////
bool EggPrimitive::
joint_has_primitives() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::has_normals
//       Access: Published, Virtual
//  Description: Returns true if any of the primitives (e.g. polygons)
//               defined within this group or below have either face
//               or vertex normals defined, false otherwise.
////////////////////////////////////////////////////////////////////
bool EggPrimitive::
has_normals() const {
  if (has_normal()) {
    return true;
  }

  const_iterator vi;
  for (vi = begin(); vi != end(); ++vi) {
    if ((*vi)->has_normal()) {
      return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::erase
//       Access: Public
//  Description: Part of the implementaion of the EggPrimitive as an
//               STL container.  Most of the rest of these functions
//               are inline and declared in EggPrimitive.I.
////////////////////////////////////////////////////////////////////
EggPrimitive::iterator EggPrimitive::
erase(iterator first, iterator last) {
  iterator i;
  int num_removed = 0;
  for (i = first; i != last; ++i) {
    prepare_remove_vertex(*i, first - _vertices.begin(), 
                          _vertices.size() - num_removed);
    num_removed++;
  }
  iterator result = _vertices.erase((Vertices::iterator &)first,
                                    (Vertices::iterator &)last);
  test_vref_integrity();
  return result;
}


////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::add_vertex
//       Access: Published
//  Description: Adds the indicated vertex to the end of the
//               primitive's list of vertices, and returns it.
////////////////////////////////////////////////////////////////////
EggVertex *EggPrimitive::
add_vertex(EggVertex *vertex) {
  prepare_add_vertex(vertex, _vertices.size(), _vertices.size() + 1);
  _vertices.push_back(vertex);

  vertex->test_pref_integrity();
  test_vref_integrity();

  return vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::remove_vertex
//       Access: Published
//  Description: Removes the indicated vertex from the
//               primitive and returns it.  If the vertex was not
//               already in the primitive, does nothing and returns
//               NULL.
////////////////////////////////////////////////////////////////////
EggVertex *EggPrimitive::
remove_vertex(EggVertex *vertex) {
  PT_EggVertex vpt = vertex;
  iterator i = find(begin(), end(), vpt);
  if (i == end()) {
    return PT_EggVertex();
  } else {
    // erase() calls prepare_remove_vertex().
    erase(i);

    vertex->test_pref_integrity();
    test_vref_integrity();

    return vertex;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::copy_vertices
//       Access: Published
//  Description: Replaces the current primitive's list of vertices
//               with a copy of the list of vertices on the other
//               primitive.
////////////////////////////////////////////////////////////////////
void EggPrimitive::
copy_vertices(const EggPrimitive &other) {
  clear();
  _vertices.reserve(other.size());

  iterator vi;
  for (vi = other.begin(); vi != other.end(); ++vi) {
    add_vertex(*vi);
  }

  test_vref_integrity();
  other.test_vref_integrity();
}

#ifndef NDEBUG

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::test_vref_integrity
//       Access: Published
//  Description: Verifies that each vertex in the primitive exists and
//               that it knows it is referenced by the primitive.
////////////////////////////////////////////////////////////////////
void EggPrimitive::
test_vref_integrity() const {
  test_ref_count_integrity();

  // First, we need to know how many times each vertex appears.
  // Usually, this will be only one, but it's possible for a vertex to
  // appear more than once.
  typedef pmap<const EggVertex *, int> VertexCount;
  VertexCount _count;

  // Now count up the vertices.
  iterator vi;
  for (vi = begin(); vi != end(); ++vi) {
    const EggVertex *vert = *vi;
    vert->test_ref_count_integrity();

    VertexCount::iterator vci = _count.find(vert);
    if (vci == _count.end()) {
      _count[vert] = 1;
    } else {
      (*vci).second++;
    }
  }

  // Ok, now walk through the vertices found and make sure the vertex
  // has the proper number of entries of this primitive in its pref.
  VertexCount::iterator vci;
  for (vci = _count.begin(); vci != _count.end(); ++vci) {
    const EggVertex *vert = (*vci).first;

    int count = (*vci).second;
    int vert_count = vert->has_pref(this);

    nassertv(count == vert_count);
  }
}

#endif  // NDEBUG

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::prepare_add_vertex
//       Access: Protected, Virtual
//  Description: Marks the vertex as belonging to the primitive.  This
//               is an internal function called by the STL-like
//               functions push_back() and insert(), in preparation
//               for actually adding the vertex.
//
//               i indicates the new position of the vertex in the
//               list; n indicates the new number of vertices after
//               the operation has completed.
////////////////////////////////////////////////////////////////////
void EggPrimitive::
prepare_add_vertex(EggVertex *vertex, int i, int n) {
  // We can't test integrity within this function, because it might be
  // called when the primitive is in an incomplete state.

  // The vertex must have the same vertex pool as the vertices already
  // added.
  nassertv(empty() || vertex->get_pool() == get_pool());

  // Since a given vertex might appear more than once in a particular
  // primitive, we can't conclude anything about data integrity by
  // inspecting the return value of insert().  (In fact, the vertex's
  // pref is a multiset, so the insert() will always succeed.)

  vertex->_pref.insert(this);
}


////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::prepare_remove_vertex
//       Access: Protected, Virtual
//  Description: Marks the vertex as removed from the primitive.  This
//               is an internal function called by the STL-like
//               functions pop_back() and erase(), in preparation for
//               actually doing the removal.
//
//               i indicates the former position of the vertex in the
//               list; n indicates the current number of vertices
//               before the operation has completed.
//
//               It is an error to attempt to remove a vertex that is
//               not already a vertex of this primitive.
////////////////////////////////////////////////////////////////////
void EggPrimitive::
prepare_remove_vertex(EggVertex *vertex, int i, int n) {
  // We can't test integrity within this function, because it might be
  // called when the primitive is in an incomplete state.

  // Now we must remove the primitive from the vertex's pref.  We
  // can't just use the simple erase() function, since that will
  // remove all instances of this primitive from the pref; instead, we
  // must find one instance and remove that.

  EggVertex::PrimitiveRef::iterator pri = vertex->_pref.find(this);

  // We should have found the primitive in the vertex's pref.  If we
  // did not, something's out of sync internally.
  nassertv(pri != vertex->_pref.end());

  vertex->_pref.erase(pri);
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::write_body
//       Access: Protected
//  Description: Writes the attributes and the vertices referenced by
//               the primitive to the indicated output stream in Egg
//               format.
////////////////////////////////////////////////////////////////////
void EggPrimitive::
write_body(ostream &out, int indent_level) const {
  test_vref_integrity();

  EggAttributes::write(out, indent_level);
  EggRenderMode::write(out, indent_level);

  int num_textures = get_num_textures();
  for (int i = 0; i < num_textures; i++) {
    EggTexture *texture = get_texture(i);

    indent(out, indent_level) << "<TRef> { ";
    enquote_string(out, texture->get_name())
      << " }\n";
  }

  if (has_material()) {
    EggMaterial *material = get_material();

    indent(out, indent_level) << "<MRef> { ";
    enquote_string(out, material->get_name())
      << " }\n";
  }

  if (get_bface_flag()) {
    indent(out, indent_level) << "<BFace> { 1 }\n";
  }

  if (!empty()) {
    EggVertexPool *pool = get_pool();

    // Make sure the vertices belong to some vertex pool.
    nassertv(pool != NULL);

    // Make sure the vertex pool is named.
    nassertv(pool->has_name());

    if ((int)size() < 10) {
      // A simple primitive gets all its vertex indices written on one
      // line.
      indent(out, indent_level) << "<VertexRef> {";
      const_iterator i;
      for (i = begin(); i != end(); ++i) {
        EggVertex *vert = *i;
        vert->test_pref_integrity();

        // Make sure each vertex belongs to the same pool.
        nassertv(vert->get_pool() == pool);

        out << " " << vert->get_index();
      }
      out << " <Ref> { ";
      enquote_string(out, pool->get_name()) << " } }\n";

    } else {

      // A larger primitive gets its vertex indices written as
      // multiple lines.
      vector_int indices;
      const_iterator i;
      for (i = begin(); i != end(); ++i) {
        EggVertex *vert = *i;
        vert->test_pref_integrity();

        // Make sure each vertex belongs to the same pool.
        nassertv(vert->get_pool() == pool);

        indices.push_back(vert->get_index());
      }

      indent(out, indent_level) << "<VertexRef> {\n";
      write_long_list(out, indent_level+2, indices.begin(), indices.end(),
                "", "", 72);
      indent(out, indent_level+2) << "<Ref> { ";
      enquote_string(out, pool->get_name()) << " }\n";
      indent(out, indent_level) << "}\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::egg_start_parse_body
//       Access: Protected, Virtual
//  Description: This function is called within parse_egg().  It
//               should call the appropriate function on the lexer to
//               initialize the parser into the state associated with
//               this object.  If the object cannot be parsed into
//               directly, it should return false.
////////////////////////////////////////////////////////////////////
bool EggPrimitive::
egg_start_parse_body() {
  egg_start_primitive_body();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::r_transform
//       Access: Protected, Virtual
//  Description: This is called from within the egg code by
//               transform().  It applies a transformation matrix
//               to the current node in some sensible way, then
//               continues down the tree.
//
//               The first matrix is the transformation to apply; the
//               second is its inverse.  The third parameter is the
//               coordinate system we are changing to, or CS_default
//               if we are not changing coordinate systems.
////////////////////////////////////////////////////////////////////
void EggPrimitive::
r_transform(const LMatrix4d &mat, const LMatrix4d &, CoordinateSystem) {
  EggAttributes::transform(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::r_flatten_transforms
//       Access: Protected, Virtual
//  Description: The recursive implementation of flatten_transforms().
////////////////////////////////////////////////////////////////////
void EggPrimitive::
r_flatten_transforms() {
  if (is_local_coord()) {
    LMatrix4d mat = get_vertex_frame();
    EggAttributes::transform(mat);

    // Transform each vertex by duplicating it in the vertex pool.
    size_t num_vertices = size();
    for (size_t i = 0; i < num_vertices; i++) {
      EggVertex *vertex = get_vertex(i);
      EggVertexPool *pool = vertex->get_pool();

      EggVertex new_vertex(*vertex);
      new_vertex.transform(mat);
      EggVertex *unique = pool->create_unique_vertex(new_vertex);
      unique->copy_grefs_from(*vertex);

      set_vertex(i, unique);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggPrimitive::r_apply_texmats
//       Access: Protected, Virtual
//  Description: The recursive implementation of apply_texmats().
////////////////////////////////////////////////////////////////////
void EggPrimitive::
r_apply_texmats(EggTextureCollection &textures) {
  Textures new_textures;
  Textures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    EggTexture *texture = (*ti);

    if (!texture->has_transform()) {
      new_textures.push_back(texture);

    } else if (texture->transform_is_identity()) {
      // Now, what's the point of a texture with an identity
      // transform?
      texture->clear_transform();
      new_textures.push_back(texture);

    } else {

      // We've got a texture with a matrix applied.  Save the matrix,
      // and get a new texture without the matrix.
      LMatrix3d mat = texture->get_transform();
      EggTexture new_texture(*texture);
      new_texture.clear_transform();
      EggTexture *unique = textures.create_unique_texture(new_texture, ~0);

      new_textures.push_back(unique);
      string uv_name = unique->get_uv_name();

      // Now apply the matrix to the vertex UV's.  Create new vertices
      // as necessary.
      size_t num_vertices = size();
      for (size_t i = 0; i < num_vertices; i++) {
        EggVertex *vertex = get_vertex(i);

        EggVertexUV *uv_obj = vertex->get_uv_obj(uv_name);
        if (uv_obj != (EggVertexUV *)NULL) {
          EggVertex new_vertex(*vertex);
          PT(EggVertexUV) new_uv_obj = new EggVertexUV(*uv_obj);
          new_uv_obj->set_uv(uv_obj->get_uv() * mat);
          new_vertex.set_uv_obj(new_uv_obj);
          
          EggVertexPool *pool = vertex->get_pool();
          EggVertex *unique = pool->create_unique_vertex(new_vertex);
          unique->copy_grefs_from(*vertex);
          
          set_vertex(i, unique);
        }
      }
    }
  }

  _textures.swap(new_textures);
}
