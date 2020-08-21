/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggPrimitive.cxx
 * @author drose
 * @date 1999-01-16
 */

#include "eggPrimitive.h"
#include "eggVertexPool.h"
#include "eggMiscFuncs.h"
#include "eggTextureCollection.h"
#include "lexerDefs.h"
#include "config_egg.h"

#include "indent.h"
#include "vector_int.h"

TypeHandle EggPrimitive::_type_handle;


/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this primitive that has an
 * alpha_mode other than AM_unspecified.  Returns a valid EggRenderMode
 * pointer if one is found, or NULL otherwise.
 */
EggRenderMode *EggPrimitive::
determine_alpha_mode() {
  if (get_alpha_mode() != AM_unspecified) {
    return this;
  }

  EggRenderMode *result = EggNode::determine_alpha_mode();
  if (result == nullptr) {
    int num_textures = get_num_textures();
    for (int i = 0; i < num_textures && result == nullptr; i++) {
      EggTexture *egg_tex = get_texture(i);

      // We only want to consider the alpha mode on those textures that can
      // affect the transparency of the polygon.  This mostly depends on the
      // envtype flag.
      if (egg_tex->affects_polygon_alpha()) {
        // This texture might affect the polygon alpha, so it gets to decide
        // the polygon transparency mode.
        if (egg_tex->get_alpha_mode() != AM_unspecified) {
          result = get_texture(i);
        }
      }
    }
  }
  return result;
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this node that has a
 * depth_write_mode other than DWM_unspecified.  Returns a valid EggRenderMode
 * pointer if one is found, or NULL otherwise.
 */
EggRenderMode *EggPrimitive::
determine_depth_write_mode() {
  if (get_depth_write_mode() != DWM_unspecified) {
    return this;
  }

  EggRenderMode *result = EggNode::determine_depth_write_mode();
  if (result == nullptr) {
    int num_textures = get_num_textures();
    for (int i = 0; i < num_textures && result == nullptr; i++) {
      if (get_texture(i)->get_depth_write_mode() != DWM_unspecified) {
        result = get_texture(i);
      }
    }
  }
  return result;
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this node that has a
 * depth_test_mode other than DTM_unspecified.  Returns a valid EggRenderMode
 * pointer if one is found, or NULL otherwise.
 */
EggRenderMode *EggPrimitive::
determine_depth_test_mode() {
  if (get_depth_test_mode() != DTM_unspecified) {
    return this;
  }

  EggRenderMode *result = EggNode::determine_depth_test_mode();
  if (result == nullptr) {
    int num_textures = get_num_textures();
    for (int i = 0; i < num_textures && result == nullptr; i++) {
      if (get_texture(i)->get_depth_test_mode() != DTM_unspecified) {
        result = get_texture(i);
      }
    }
  }
  return result;
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this node that has a
 * visibility_mode other than VM_unspecified.  Returns a valid EggRenderMode
 * pointer if one is found, or NULL otherwise.
 */
EggRenderMode *EggPrimitive::
determine_visibility_mode() {
  if (get_visibility_mode() != VM_unspecified) {
    return this;
  }

  EggRenderMode *result = EggNode::determine_visibility_mode();
  if (result == nullptr) {
    int num_textures = get_num_textures();
    for (int i = 0; i < num_textures && result == nullptr; i++) {
      if (get_texture(i)->get_visibility_mode() != VM_unspecified) {
        result = get_texture(i);
      }
    }
  }
  return result;
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this primitive that has a
 * depth_offset specified.  Returns a valid EggRenderMode pointer if one is
 * found, or NULL otherwise.
 */
EggRenderMode *EggPrimitive::
determine_depth_offset() {
  if (has_depth_offset()) {
    return this;
  }

  EggRenderMode *result = EggNode::determine_depth_offset();
  if (result == nullptr) {
    int num_textures = get_num_textures();
    for (int i = 0; i < num_textures && result == nullptr; i++) {
      if (get_texture(i)->has_depth_offset()) {
        result = get_texture(i);
      }
    }
  }
  return result;
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this primitive that has a
 * draw_order specified.  Returns a valid EggRenderMode pointer if one is
 * found, or NULL otherwise.
 */
EggRenderMode *EggPrimitive::
determine_draw_order() {
  if (has_draw_order()) {
    return this;
  }

  EggRenderMode *result = EggNode::determine_draw_order();
  if (result == nullptr) {
    int num_textures = get_num_textures();
    for (int i = 0; i < num_textures && result == nullptr; i++) {
      if (get_texture(i)->has_draw_order()) {
        result = get_texture(i);
      }
    }
  }
  return result;
}

/**
 * Walks back up the hierarchy, looking for an EggGroup or EggPrimitive or
 * some such object at this level or above this primitive that has a bin
 * specified.  Returns a valid EggRenderMode pointer if one is found, or NULL
 * otherwise.
 */
EggRenderMode *EggPrimitive::
determine_bin() {
  if (has_bin()) {
    return this;
  }

  EggRenderMode *result = EggNode::determine_bin();
  if (result == nullptr) {
    int num_textures = get_num_textures();
    for (int i = 0; i < num_textures && result == nullptr; i++) {
      if (get_texture(i)->has_bin()) {
        result = get_texture(i);
      }
    }
  }
  return result;
}


/**
 * Returns the shading properties apparent on this particular primitive.  This
 * returns S_per_vertex if the vertices have colors or normals (and they are
 * not all the same values), or for a simple primitive, S_overall otherwise.
 * A composite primitive may also return S_per_face if the individual
 * component primitives have colors or normals that are not all the same
 * values.
 *
 * To get the most accurate results, you should call clear_shading() on all
 * connected primitives (or on all primitives in the egg file), followed by
 * get_shading() on each primitive.  You may find it easiest to call these
 * methods on the EggData root node (they are defined on EggGroupNode).
 */
EggPrimitive::Shading EggPrimitive::
get_shading() const {
  if (empty()) {
    return S_overall;
  }

  if (has_vertex_normal()) {
    // Check if the vertices all have the same normal.
    const EggAttributes *first_vertex = get_vertex(0);
    if (!first_vertex->has_normal()) {
      first_vertex = this;
    }
    for (size_t i = 1; i < get_num_vertices(); ++i) {
      const EggAttributes *vertex = get_vertex(i);
      if (!vertex->has_normal()) {
        vertex = this;
      }
      if (!vertex->matches_normal(*first_vertex)) {
        return S_per_vertex;
      }
    }
  }

  if (has_vertex_color()) {
    // Check if the vertices all have the same color.
    const EggAttributes *first_vertex = get_vertex(0);
    if (!first_vertex->has_color()) {
      first_vertex = this;
    }
    for (size_t i = 1; i < get_num_vertices(); ++i) {
      const EggAttributes *vertex = get_vertex(i);
      if (!vertex->has_color()) {
        vertex = this;
      }
      if (!vertex->matches_color(*first_vertex)) {
        return S_per_vertex;
      }
    }
  }

  return S_overall;
}

/**
 * Copies the rendering attributes from the indicated primitive.
 */
void EggPrimitive::
copy_attributes(const EggAttributes &other) {
  EggAttributes::operator = (other);
}

/**
 * Copies the rendering attributes from the indicated primitive.
 */
void EggPrimitive::
copy_attributes(const EggPrimitive &other) {
  EggAttributes::operator = (other);
  _textures = other._textures;
  set_material(other.get_material());
  set_bface_flag(other.get_bface_flag());
}

/**
 * Returns true if any vertex on the primitive has a specific normal set,
 * false otherwise.
 *
 * If you call unify_attributes() first, this will also return false even if
 * all the vertices were set to the same value (since unify_attributes()
 * removes redundant vertex properties).
 */
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

/**
 * Returns true if any vertex on the primitive has a specific color set, false
 * otherwise.
 *
 * If you call unify_attributes() first, this will also return false even if
 * all the vertices were set to the same value (since unify_attributes()
 * removes redundant vertex properties).
 */
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

/**
 * If the shading property is S_per_vertex, ensures that all vertices have a
 * normal and a color, and the overall primitive does not.
 *
 * If the shading property is S_per_face, and this is a composite primitive,
 * ensures that all components have a normal and a color, and the vertices and
 * overall primitive do not.  (If this is a simple primitive, S_per_face works
 * the same as S_overall, below).
 *
 * If the shading property is S_overall, ensures that no vertices or
 * components have a normal or a color, and the overall primitive does (if any
 * exists at all).
 *
 * After this call, either the primitive will have normals or its vertices
 * will, but not both.  Ditto for colors.
 *
 * This may create redundant vertices in the vertex pool.
 */
void EggPrimitive::
unify_attributes(EggPrimitive::Shading shading) {
  if (shading == S_unknown) {
    shading = get_shading();
  }

  // Does the primitive have an explicit color?
  if (!has_color() && shading != S_overall) {
    if (shading != S_per_vertex) {
      // If there is no color set, first we check the vertices.  If the
      // vertices have a color, we inherit the color from there.
      iterator pi;
      for (pi = begin(); pi != end() && !has_color(); ++pi) {
        EggVertex *vertex = (*pi);
        if (vertex->has_color()) {
          set_color(vertex->get_color());
        }
      }
    }
    if (!has_color()) {
      // If we still don't have a color, the implicit color is white.
      set_color(LColor(1.0f, 1.0f, 1.0f, 1.0f));
    }
  }

  switch (shading) {
  case S_per_vertex:
    // Propagate everything to the vertices.
    {
      iterator pi;
      for (pi = begin(); pi != end(); ++pi) {
        EggVertex *orig_vertex = (*pi);
        PT(EggVertex) vertex = new EggVertex(*orig_vertex);
        if (!vertex->has_normal() && has_normal()) {
          vertex->copy_normal(*this);
        }
        if (!vertex->has_color() && has_color()) {
          vertex->copy_color(*this);
        }

        EggVertexPool *vertex_pool = orig_vertex->get_pool();
        nassertv(vertex_pool != nullptr);
        vertex = vertex_pool->create_unique_vertex(*vertex);
        vertex->copy_grefs_from(*orig_vertex);
        replace(pi, vertex);
      }
      clear_normal();
      clear_color();
    }
    break;

  case S_per_face:
  case S_overall:
    // Remove everything from the vertices.
    {
      iterator pi;
      for (pi = begin(); pi != end(); ++pi) {
        EggVertex *orig_vertex = (*pi);
        PT(EggVertex) vertex = new EggVertex(*orig_vertex);
        if (vertex->has_normal()) {
          if (!has_normal()) {
            copy_normal(*vertex);
          }
          vertex->clear_normal();
        }
        if (vertex->has_color()) {
          if (!has_color()) {
            copy_color(*vertex);
          }
          vertex->clear_color();
        }

        EggVertexPool *vertex_pool = orig_vertex->get_pool();
        nassertv(vertex_pool != nullptr);
        vertex = vertex_pool->create_unique_vertex(*vertex);
        vertex->copy_grefs_from(*orig_vertex);
        replace(pi, vertex);
      }
    }
    break;

  case S_unknown:
    break;
  }

  if (!has_color() && shading == S_overall) {
    set_color(LColor(1.0f, 1.0f, 1.0f, 1.0f));
  }
}

/**
 * Sets the last vertex of the triangle (or each component) to the primitive
 * normal and/or color, if the primitive is flat-shaded.  This reflects the
 * OpenGL convention of storing flat-shaded properties on the last vertex,
 * although it is not usually a convention in Egg.
 *
 * This may introduce redundant vertices to the vertex pool.
 */
void EggPrimitive::
apply_last_attribute() {
  if (!empty()) {
    do_apply_flat_attribute(size() - 1, this);
  }
}

/**
 * Sets the first vertex of the triangle (or each component) to the primitive
 * normal and/or color, if the primitive is flat-shaded.  This reflects the
 * DirectX convention of storing flat-shaded properties on the first vertex,
 * although it is not usually a convention in Egg.
 *
 * This may introduce redundant vertices to the vertex pool.
 */
void EggPrimitive::
apply_first_attribute() {
  if (!empty()) {
    do_apply_flat_attribute(0, this);
  }
}

/**
 * Intended as a followup to apply_last_attribute(), this also sets an
 * attribute on the first vertices of the primitive, if they don't already
 * have an attribute set, just so they end up with *something*.
 */
void EggPrimitive::
post_apply_flat_attribute() {
  if (!empty()) {
    for (EggVertex *vertex : _vertices) {
      // Use set_normal() instead of copy_normal(), to avoid getting the
      // morphs--we don't want them here, since we're just putting a bogus
      // value on the normal anyway.

      if (has_normal() && !vertex->has_normal()) {
        vertex->set_normal(get_normal());
      }
      if (has_color() && !vertex->has_color()) {
        vertex->set_color(get_color());
      }
    }
  }
}

/**
 * Reverses the ordering of the vertices in this primitive, if appropriate, in
 * order to change the direction the polygon appears to be facing.  Does not
 * adjust the surface normal, if any.
 */
void EggPrimitive::
reverse_vertex_ordering() {
  // This really only makes sense for polygons.  Lights don't care about
  // vertex ordering, and NURBS surfaces have to do a bit more work in
  // addition to this.
  reverse(_vertices.begin(), _vertices.end());
}

/**
 * Cleans up modeling errors in whatever context this makes sense.  For
 * instance, for a polygon, this calls remove_doubled_verts(true).  For a
 * point, it calls remove_nonunique_verts().  Returns true if the primitive is
 * valid, or false if it is degenerate.
 */
bool EggPrimitive::
cleanup() {
  return !empty();
}

/**
 * Certain kinds of primitives, particularly polygons, don't like to have the
 * same vertex repeated consecutively.  Unfortunately, some modeling programs
 * (like MultiGen) make this an easy mistake to make.
 *
 * It's handy to have a function to remove these redundant vertices.  If
 * closed is true, it also checks that the first and last vertices are not the
 * same.
 *
 * This function identifies repeated vertices by position only; it does not
 * consider any other properties, such as color or UV, significant in
 * differentiating vertices.
 */
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
      if ((*vi)->get_pos4() != (*vlast)->get_pos4()) {
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
    // Then, if this is a polygon (which will be closed anyway), remove the
    // vertex from the end if it's a repeat of the beginning.
    while (_vertices.size() > 1 &&
           _vertices.back()->get_pos4() == _vertices.front()->get_pos4()) {
      prepare_remove_vertex(_vertices.back(), _vertices.size() - 1,
                            _vertices.size());
      _vertices.pop_back();
    }
  }
}

/**
 * Removes any multiple appearances of the same vertex from the primitive.
 * This primarily makes sense for a point primitive, which is really a
 * collection of points and which doesn't make sense to include the same point
 * twice, in any order.
 */
void EggPrimitive::
remove_nonunique_verts() {
  Vertices::iterator vi;
  Vertices new_vertices;
  int num_removed = 0;

  pset<EggVertex *> unique_vertices;
  for (vi = _vertices.begin(); vi != _vertices.end(); ++vi) {
    bool inserted = unique_vertices.insert(*vi).second;
    if (inserted) {
      new_vertices.push_back(*vi);
    } else {
      prepare_remove_vertex(*vi, vi - _vertices.begin() - num_removed,
                            _vertices.size() - num_removed);
      num_removed++;
    }
  }

  _vertices.swap(new_vertices);
}

/**
 * Returns true if there are any primitives (e.g.  polygons) defined within
 * this group or below, false otherwise.
 */
bool EggPrimitive::
has_primitives() const {
  return true;
}

/**
 * Returns true if there are any primitives (e.g.  polygons) defined within
 * this group or below, but the search does not include nested joints.
 */
bool EggPrimitive::
joint_has_primitives() const {
  return true;
}

/**
 * Returns true if any of the primitives (e.g.  polygons) defined within this
 * group or below have either face or vertex normals defined, false otherwise.
 */
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


/**
 * Part of the implementaion of the EggPrimitive as an STL container.  Most of
 * the rest of these functions are inline and declared in EggPrimitive.I.
 */
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

/**
 * Returns the iterator pointing to the indicated vertex, or end() if the
 * vertex is not part of the primitive.
 */
EggPrimitive::iterator EggPrimitive::
find(EggVertex *vertex) {
  PT_EggVertex vpt = vertex;
  return std::find(begin(), end(), vpt);
}


/**
 * Adds the indicated vertex to the end of the primitive's list of vertices,
 * and returns it.
 */
EggVertex *EggPrimitive::
add_vertex(EggVertex *vertex) {
  prepare_add_vertex(vertex, _vertices.size(), _vertices.size() + 1);
  _vertices.push_back(vertex);

  vertex->test_pref_integrity();
  test_vref_integrity();

  return vertex;
}

/**
 * Removes the indicated vertex from the primitive and returns it.  If the
 * vertex was not already in the primitive, does nothing and returns NULL.
 */
EggVertex *EggPrimitive::
remove_vertex(EggVertex *vertex) {
  PT_EggVertex vpt = vertex;
  iterator i = std::find(begin(), end(), vpt);
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

/**
 * Removes the indicated vertex from the primitive.
 */
void EggPrimitive::
remove_vertex(size_t index) {
  nassertv(index < size());
  iterator i = begin() + index;

  // erase() calls prepare_remove_vertex().
  erase(i);

  test_vref_integrity();
}

/**
 * Replaces the current primitive's list of vertices with a copy of the list
 * of vertices on the other primitive.
 */
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

#ifdef _DEBUG

/**
 * Verifies that each vertex in the primitive exists and that it knows it is
 * referenced by the primitive.
 */
void EggPrimitive::
test_vref_integrity() const {
  test_ref_count_integrity();

  if ((int)size() <= egg_test_vref_integrity) {
    // First, we need to know how many times each vertex appears.  Usually,
    // this will be only one, but it's possible for a vertex to appear more
    // than once.
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

    // Ok, now walk through the vertices found and make sure the vertex has
    // the proper number of entries of this primitive in its pref.
    VertexCount::iterator vci;
    for (vci = _count.begin(); vci != _count.end(); ++vci) {
      const EggVertex *vert = (*vci).first;

      int count = (*vci).second;
      int vert_count = vert->has_pref(this);

      nassertv(count == vert_count);
    }
  }
}

#endif  // _DEBUG

/**
 * Marks the vertex as belonging to the primitive.  This is an internal
 * function called by the STL-like functions push_back() and insert(), in
 * preparation for actually adding the vertex.
 *
 * i indicates the new position of the vertex in the list; n indicates the new
 * number of vertices after the operation has completed.
 */
void EggPrimitive::
prepare_add_vertex(EggVertex *vertex, int i, int n) {
  // We can't test integrity within this function, because it might be called
  // when the primitive is in an incomplete state.

  // The vertex must have the same vertex pool as the vertices already added.
  nassertv(empty() || vertex->get_pool() == get_pool());

  // Since a given vertex might appear more than once in a particular
  // primitive, we can't conclude anything about data integrity by inspecting
  // the return value of insert().  (In fact, the vertex's pref is a multiset,
  // so the insert() will always succeed.)

  vertex->_pref.insert(this);
}


/**
 * Marks the vertex as removed from the primitive.  This is an internal
 * function called by the STL-like functions pop_back() and erase(), in
 * preparation for actually doing the removal.
 *
 * i indicates the former position of the vertex in the list; n indicates the
 * current number of vertices before the operation has completed.
 *
 * It is an error to attempt to remove a vertex that is not already a vertex
 * of this primitive.
 */
void EggPrimitive::
prepare_remove_vertex(EggVertex *vertex, int i, int n) {
  // We can't test integrity within this function, because it might be called
  // when the primitive is in an incomplete state.

  // Now we must remove the primitive from the vertex's pref.  We can't just
  // use the simple erase() function, since that will remove all instances of
  // this primitive from the pref; instead, we must find one instance and
  // remove that.

  EggVertex::PrimitiveRef::iterator pri = vertex->_pref.find(this);

  // We should have found the primitive in the vertex's pref.  If we did not,
  // something's out of sync internally.
  nassertv(pri != vertex->_pref.end());

  vertex->_pref.erase(pri);
}

/**
 * Writes the attributes and the vertices referenced by the primitive to the
 * indicated output stream in Egg format.
 */
void EggPrimitive::
write_body(std::ostream &out, int indent_level) const {
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
    nassertv(pool != nullptr);

    // Make sure the vertex pool is named.
    nassertv(pool->has_name());

    if ((int)size() < 10) {
      // A simple primitive gets all its vertex indices written on one line.
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

      // A larger primitive gets its vertex indices written as multiple lines.
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

/**
 * This function is called within parse_egg().  It should call the appropriate
 * function on the lexer to initialize the parser into the state associated
 * with this object.  If the object cannot be parsed into directly, it should
 * return false.
 */
bool EggPrimitive::
egg_start_parse_body() {
  egg_start_primitive_body();
  return true;
}

/**
 * This is called from within the egg code by transform().  It applies a
 * transformation matrix to the current node in some sensible way, then
 * continues down the tree.
 *
 * The first matrix is the transformation to apply; the second is its inverse.
 * The third parameter is the coordinate system we are changing to, or
 * CS_default if we are not changing coordinate systems.
 */
void EggPrimitive::
r_transform(const LMatrix4d &mat, const LMatrix4d &, CoordinateSystem) {
  EggAttributes::transform(mat);
}

/**
 * The recursive implementation of flatten_transforms().
 */
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

/**
 * The recursive implementation of apply_texmats().
 */
void EggPrimitive::
r_apply_texmats(EggTextureCollection &textures) {
  Textures new_textures;
  Textures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    EggTexture *texture = (*ti);

    if (!texture->has_transform()) {
      new_textures.push_back(texture);

    } else if (texture->transform_is_identity()) {
      // Now, what's the point of a texture with an identity transform?
      texture->clear_transform();
      new_textures.push_back(texture);

    } else {

      // We've got a texture with a matrix applied.  Save the matrix, and get
      // a new texture without the matrix.
      LMatrix4d mat = texture->get_transform3d();
      EggTexture new_texture(*texture);
      new_texture.clear_transform();
      EggTexture *unique = textures.create_unique_texture(new_texture, ~0);

      new_textures.push_back(unique);
      std::string uv_name = unique->get_uv_name();

      // Now apply the matrix to the vertex UV's.  Create new vertices as
      // necessary.
      size_t num_vertices = size();
      for (size_t i = 0; i < num_vertices; i++) {
        EggVertex *vertex = get_vertex(i);

        const EggVertexUV *uv_obj = vertex->get_uv_obj(uv_name);
        if (uv_obj != nullptr) {
          EggVertex new_vertex(*vertex);
          PT(EggVertexUV) new_uv_obj = new EggVertexUV(*uv_obj);
          LTexCoord3d uvw = uv_obj->get_uvw() * mat;
          if (uv_obj->has_w() || texture->has_transform3d()) {
            new_uv_obj->set_uvw(uvw);
          } else {
            new_uv_obj->set_uv(LTexCoordd(uvw[0], uvw[1]));
          }
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

/**
 * This is used to implement apply_first_attribute() and
 * apply_last_attribute().  It copies the indicated attributes to the
 * specified vertex.
 */
void EggPrimitive::
do_apply_flat_attribute(int vertex_index, EggAttributes *attrib) {
  // The significant_change flag is set if we have changed the vertex in some
  // important way, that will invalidate it for other primitives that might
  // share it.  We don't consider *adding* a normal where there wasn't one
  // before to be significant, but we do consider it significant to change a
  // vertex's normal to something different.  Similarly for color.
  bool significant_change = false;

  EggVertex *orig_vertex = get_vertex(vertex_index);
  PT(EggVertex) new_vertex = new EggVertex(*orig_vertex);

  if (attrib->has_normal()) {
    new_vertex->copy_normal(*attrib);

    if (orig_vertex->has_normal() &&
        !orig_vertex->matches_normal(*new_vertex)) {
      significant_change = true;
    }
  } else if (has_normal()) {
    new_vertex->copy_normal(*this);

    if (orig_vertex->has_normal() &&
        !orig_vertex->matches_normal(*new_vertex)) {
      significant_change = true;
    }
  }

  if (attrib->has_color()) {
    new_vertex->copy_color(*attrib);

    if (orig_vertex->has_color() &&
        !orig_vertex->matches_color(*new_vertex)) {
      significant_change = true;
    }
  } else if (has_color()) {
    new_vertex->copy_color(*this);

    if (orig_vertex->has_color() &&
        !orig_vertex->matches_color(*new_vertex)) {
      significant_change = true;
    }
  }

  if (significant_change) {
    new_vertex = get_pool()->create_unique_vertex(*new_vertex);
    new_vertex->copy_grefs_from(*orig_vertex);
    set_vertex(vertex_index, new_vertex);
  } else {
    // Just copy the new attributes back into the pool.
    ((EggAttributes *)orig_vertex)->operator = (*new_vertex);
  }
}

/**
 * Recursively updates the connected_shading member in all connected
 * primitives.
 */
void EggPrimitive::
set_connected_shading(EggPrimitive::Shading shading,
                      const EggAttributes *neighbor) {
  ConnectedShadingNodes connected_nodes;

  r_set_connected_shading(0, shading, neighbor, connected_nodes);

  // Pick up any additional nodes we couldn't visit because of the stack depth
  // restrictions.
  while (!connected_nodes.empty()) {
    ConnectedShadingNodes next_nodes;
    next_nodes.swap(connected_nodes);

    ConnectedShadingNodes::iterator ni;
    for (ni = next_nodes.begin(); ni != next_nodes.end(); ++ni) {
      r_set_connected_shading(0, (*ni)._shading, (*ni)._neighbor, connected_nodes);
    }
  }
}

/**
 * Implements set_connected_shading, with some restrictions to prevent stack
 * overflow.
 */
void EggPrimitive::
r_set_connected_shading(int stack_depth, EggPrimitive::Shading shading,
                        const EggAttributes *neighbor,
                        ConnectedShadingNodes &next_nodes) {
  if (stack_depth > egg_recursion_limit) {
    // Too deep.  Limit recursion.
    ConnectedShadingNode next;
    next._shading = shading;
    next._neighbor = neighbor;
    next_nodes.push_back(next);
    return;
  }

  bool propagate = false;

  if (_connected_shading == S_unknown) {
    // We haven't visited this node before; propagate now.
    _connected_shading = get_shading();
    propagate = true;
  }

  if (shading > _connected_shading) {
    // More specific information just came in.  Save it, and propagate it to
    // all connected primitives.
    _connected_shading = shading;
    propagate = true;

  } else if (shading == S_overall && _connected_shading == S_overall) {
    // If both neighbors are overall shaded, check if the two neighbors have
    // different properties.  If they do, elevate to per_face.
    bool matches_normal = this->matches_normal(*neighbor);
    bool matches_color = this->matches_color(*neighbor);

    if (!matches_color) {
      // Make a special case for not having an overall color: that's
      // implicitly white.
      if (!neighbor->has_color() && has_color() && _drgbas.empty() &&
          get_color() == LColor(1.0f, 1.0f, 1.0f, 1.0f)) {
        matches_color = true;
      } else if (!has_color() && neighbor->has_color() && neighbor->_drgbas.empty() &&
          neighbor->get_color() == LColor(1.0f, 1.0f, 1.0f, 1.0f)) {
        matches_color = true;
      }
    }
    if (!matches_normal || !matches_color) {
      _connected_shading = S_per_face;
      propagate = true;
    }
  }

  if (propagate) {
    Vertices::const_iterator vi;
    for (vi = _vertices.begin(); vi != _vertices.end(); ++vi) {
      EggVertex *vertex = (*vi);
      EggVertex::PrimitiveRef::const_iterator pi;
      for (pi = vertex->pref_begin();
           pi != vertex->pref_end();
           ++pi) {
        (*pi)->r_set_connected_shading(stack_depth + 1, _connected_shading, this,
                                       next_nodes);
      }
    }
  }
}
