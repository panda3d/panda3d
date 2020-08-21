/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggCompositePrimitive.cxx
 * @author drose
 * @date 2005-03-13
 */

#include "eggCompositePrimitive.h"
#include "eggGroupNode.h"
#include "eggVertexPool.h"

TypeHandle EggCompositePrimitive::_type_handle;


/**
 *
 */
EggCompositePrimitive::
~EggCompositePrimitive() {
  // Every derived class of EggCompositePrimitive must call clear() in its
  // destructor.
  nassertv(_components.empty());
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
EggPrimitive::Shading EggCompositePrimitive::
get_shading() const {
  Shading basic_shading = EggPrimitive::get_shading();
  if (basic_shading == S_per_vertex) {
    return basic_shading;
  }
  if (_components.empty()) {
    return S_overall;
  }

  // Check if the components all have the same normal.
  {
    const EggAttributes *first_component = get_component(0);
    if (!first_component->has_normal()) {
      first_component = this;
    }
    for (size_t i = 1; i < get_num_components(); ++i) {
      const EggAttributes *component = get_component(i);
      if (!component->has_normal()) {
        component = this;
      }
      if (!component->matches_normal(*first_component)) {
        return S_per_face;
      }
    }
  }

  // Check if the components all have the same color.
  {
    const EggAttributes *first_component = get_component(0);
    if (!first_component->has_color()) {
      first_component = this;
    }
    for (size_t i = 1; i < get_num_components(); ++i) {
      const EggAttributes *component = get_component(i);
      if (!component->has_color()) {
        component = this;
      }
      if (!component->matches_color(*first_component)) {
        return S_per_face;
      }
    }
  }

  return S_overall;
}

/**
 * Subdivides the composite primitive into triangles and adds those triangles
 * to the parent group node in place of the original primitive.  Returns a
 * pointer to the original primitive, which is likely about to be destructed.
 *
 * If convex_also is true, both concave and convex polygons will be subdivided
 * into triangles; otherwise, only concave polygons will be subdivided, and
 * convex polygons will be copied unchanged into the container.
 */
PT(EggCompositePrimitive) EggCompositePrimitive::
triangulate_in_place() {
  EggGroupNode *parent = get_parent();
  nassertr(parent != nullptr, this);

  PT(EggCompositePrimitive) save_me = this;
  parent->remove_child(this);
  do_triangulate(parent);

  return save_me;
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
void EggCompositePrimitive::
unify_attributes(EggPrimitive::Shading shading) {
  if (shading == S_unknown) {
    shading = get_shading();
  }

  switch (shading) {
  case S_per_vertex:
    // Propagate everything to the vertices.
    {
      Components::iterator ci;
      for (ci = _components.begin(); ci != _components.end(); ++ci) {
        EggAttributes *component = (*ci);
        if (component->has_normal()) {
          if (!has_normal()) {
            copy_normal(*component);
          }
          component->clear_normal();
        }
        if (component->has_color()) {
          if (!has_color()) {
            copy_color(*component);
          }
          component->clear_color();
        }
      }

      // Not having a color is implicitly white.
      if (!has_color()) {
        set_color(LColor(1.0f, 1.0f, 1.0f, 1.0f));
      }

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
    // Propagate everything to the components.
    {
      iterator pi;
      for (pi = begin(); pi != end(); ++pi) {
        EggVertex *orig_vertex = (*pi);
        if (orig_vertex->has_normal() || orig_vertex->has_color()) {
          if (orig_vertex->has_normal() && !has_normal()) {
            copy_normal(*orig_vertex);
          }
          if (orig_vertex->has_color() && !has_color()) {
            copy_color(*orig_vertex);
          }

          PT(EggVertex) vertex = new EggVertex(*orig_vertex);
          vertex->clear_normal();
          vertex->clear_color();

          EggVertexPool *vertex_pool = orig_vertex->get_pool();
          nassertv(vertex_pool != nullptr);
          vertex = vertex_pool->create_unique_vertex(*vertex);
          vertex->copy_grefs_from(*orig_vertex);
          replace(pi, vertex);
        }
      }

      // Not having a color is implicitly white.
      if (!has_color()) {
        set_color(LColor(1.0f, 1.0f, 1.0f, 1.0f));
      }

      Components::iterator ci;
      for (ci = _components.begin(); ci != _components.end(); ++ci) {
        EggAttributes *component = (*ci);
        if (!component->has_normal() && has_normal()) {
          component->copy_normal(*this);
        }
        if (!component->has_color() && has_color()) {
          component->copy_color(*this);
        }
      }
      clear_normal();
      clear_color();
    }
    break;

  case S_overall:
    // Remove everything from the vertices and components.
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
      Components::iterator ci;
      for (ci = _components.begin(); ci != _components.end(); ++ci) {
        EggAttributes *component = (*ci);
        if (component->has_normal()) {
          if (!has_normal()) {
            copy_normal(*component);
          }
          component->clear_normal();
        }
        if (component->has_color()) {
          if (!has_color()) {
            copy_color(*component);
          }
          component->clear_color();
        }
      }

      // Not having a color is implicitly white.
      if (!has_color()) {
        set_color(LColor(1.0f, 1.0f, 1.0f, 1.0f));
      }
    }
    break;

  case S_unknown:
    break;
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
void EggCompositePrimitive::
apply_last_attribute() {
  // The first component gets applied to the third vertex, and so on from
  // there.
  int num_lead_vertices = get_num_lead_vertices();
  for (size_t i = 0; i < get_num_components(); ++i) {
    EggAttributes *component = get_component(i);
    do_apply_flat_attribute(i + num_lead_vertices, component);
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
void EggCompositePrimitive::
apply_first_attribute() {
  // The first component gets applied to the first vertex, and so on from
  // there.
  for (size_t i = 0; i < get_num_components(); ++i) {
    EggAttributes *component = get_component(i);
    do_apply_flat_attribute(i, component);
  }
}

/**
 * Intended as a followup to apply_last_attribute(), this also sets an
 * attribute on the first vertices of the primitive, if they don't already
 * have an attribute set, just so they end up with *something*.
 */
void EggCompositePrimitive::
post_apply_flat_attribute() {
  if (!empty()) {
    int num_lead_vertices = get_num_lead_vertices();
    for (int i = 0; i < (int)size(); i++) {
      EggVertex *vertex = get_vertex(i);
      EggAttributes *component = get_component(std::max(i - num_lead_vertices, 0));

      // Use set_normal() instead of copy_normal(), to avoid getting the
      // morphs--we don't want them here, since we're just putting a bogus
      // value on the normal anyway.

      if (component->has_normal() && !vertex->has_normal()) {
        vertex->set_normal(component->get_normal());
      } else if (has_normal() && !vertex->has_normal()) {
        vertex->set_normal(get_normal());
      }

      if (component->has_color() && !vertex->has_color()) {
        vertex->set_color(component->get_color());
      } else if (has_color() && !vertex->has_color()) {
        vertex->set_color(get_color());
      }
    }
  }
}

/**
 * Cleans up modeling errors in whatever context this makes sense.  For
 * instance, for a polygon, this calls remove_doubled_verts(true).  For a
 * point, it calls remove_nonunique_verts().  Returns true if the primitive is
 * valid, or false if it is degenerate.
 */
bool EggCompositePrimitive::
cleanup() {
  return (int)size() >= get_num_lead_vertices() + 1;
}

/**
 * Marks the vertex as belonging to the primitive.  This is an internal
 * function called by the STL-like functions push_back() and insert(), in
 * preparation for actually adding the vertex.
 *
 * i indicates the new position of the vertex in the list; n indicates the new
 * number of vertices after the operation has completed.
 */
void EggCompositePrimitive::
prepare_add_vertex(EggVertex *vertex, int i, int n) {
  EggPrimitive::prepare_add_vertex(vertex, i, n);

  int num_lead_vertices = get_num_lead_vertices();
  if (n >= num_lead_vertices + 1) {
    i = std::max(i - num_lead_vertices, 0);
    nassertv(i <= (int)_components.size());
    _components.insert(_components.begin() + i, new EggAttributes(*this));
  }
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
void EggCompositePrimitive::
prepare_remove_vertex(EggVertex *vertex, int i, int n) {
  EggPrimitive::prepare_remove_vertex(vertex, i, n);

  int num_lead_vertices = get_num_lead_vertices();
  if (n >= num_lead_vertices + 1) {
    i = std::max(i - num_lead_vertices, 0);
    nassertv(i < (int)_components.size());
    delete _components[i];
    _components.erase(_components.begin() + i);
  }
}

/**
 * Fills the container up with EggPolygons that represent the component
 * triangles of this triangle strip.
 *
 * It is assumed that the EggCompositePrimitive is not already a child of any
 * other group when this function is called.
 *
 * Returns true if the triangulation is successful, or false if there was some
 * error (in which case the container may contain some partial triangulation).
 */
bool EggCompositePrimitive::
do_triangulate(EggGroupNode *container) const {
  container->add_child((EggCompositePrimitive *)this);
  return true;
}

/**
 * Writes the attributes and the vertices referenced by the primitive to the
 * indicated output stream in Egg format.
 */
void EggCompositePrimitive::
write_body(std::ostream &out, int indent_level) const {
  EggPrimitive::write_body(out, indent_level);

  for (size_t i = 0; i < get_num_components(); ++i) {
    const EggAttributes *attrib = get_component(i);
    if (attrib->compare_to(*this) != 0 &&
        (attrib->has_color() || attrib->has_normal())) {
      indent(out, indent_level)
        << "<Component> " << i << " {\n";
      attrib->write(out, indent_level + 2);
      indent(out, indent_level) << "}\n";
    }
  }
}
