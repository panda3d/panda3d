// Filename: eggCompositePrimitive.cxx
// Created by:  drose (13Mar05)
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

#include "eggCompositePrimitive.h"
#include "eggGroupNode.h"

TypeHandle EggCompositePrimitive::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::triangulate_in_place
//       Access: Published
//  Description: Subdivides the composite primitive into triangles and
//               adds those triangles to the parent group node in
//               place of the original primitive.  Returns a pointer
//               to the original primitive, which is likely about to
//               be destructed.
//
//               If convex_also is true, both concave and convex
//               polygons will be subdivided into triangles;
//               otherwise, only concave polygons will be subdivided,
//               and convex polygons will be copied unchanged into the
//               container.
////////////////////////////////////////////////////////////////////
PT(EggCompositePrimitive) EggCompositePrimitive::
triangulate_in_place() {
  EggGroupNode *parent = get_parent();
  nassertr(parent != (EggGroupNode *)NULL, this);

  PT(EggCompositePrimitive) save_me = this;
  parent->remove_child(this);
  do_triangulate(parent);

  return save_me;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::unify_attributes
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
void EggCompositePrimitive::
unify_attributes() {
  // A composite primitive wants to do the same sort of thing with
  // components that the fundamental primitive does with vertices.
  // But first, we want to make sure that the primitive overall
  // attributes have been correctly moved from the vertices, so call
  // up to the base class.
  EggPrimitive::unify_attributes();

  // Now the rest of this body is more or less what an EggPrimitive
  // does for vertices, modified to work on components.

  Components components;

  // First, go through the components and apply the primitive overall
  // attributes to any component that omits them.
  bool all_have_normal = true;
  bool all_have_color = true;
  Components::iterator ci;
  for (ci = _components.begin(); ci != _components.end(); ++ci) {
    EggAttributes *orig_component = (*ci);
    EggAttributes *component = new EggAttributes(*orig_component);

    if (!component->has_normal()) {
      if (has_normal()) {
        component->set_normal(get_normal());
        component->_dnormals = _dnormals;
      } else {
        all_have_normal = false;
      }
    }
    if (!component->has_color()) {
      if (has_color()) {
        component->set_color(get_color());
        component->_drgbas = _drgbas;
      } else {
        all_have_color = false;
      }
    }

    components.push_back(component);
  }

  clear_normal();
  clear_color();

  // Now see if any ended up different.
  EggAttributes overall;
  bool normal_different = false;
  bool color_different = false;

  Components::iterator vi;
  for (vi = components.begin(); vi != components.end(); ++vi) {
    EggAttributes *component = (*vi);
    if (!all_have_normal) {
      component->clear_normal();
    } else if (!normal_different) {
      if (!overall.has_normal()) {
        overall.set_normal(component->get_normal());
        overall._dnormals = component->_dnormals;
      } else if (overall.get_normal() != component->get_normal() ||
                 overall._dnormals.compare_to(component->_dnormals) != 0) {
        normal_different = true;
      }
    }
    if (!all_have_color) {
      component->clear_color();
    } else if (!color_different) {
      if (!overall.has_color()) {
        overall.set_color(component->get_color());
        overall._drgbas = component->_drgbas;
      } else if (overall.get_color() != component->get_color() ||
                 overall._drgbas.compare_to(component->_drgbas) != 0) {
        color_different = true;
      }
    }
  }

  if (!color_different || !normal_different) {
    // Ok, it's flat-shaded after all.  Go back through and remove
    // this stuff from the components.
    if (!normal_different) {
      set_normal(overall.get_normal());
      _dnormals = overall._dnormals;
    }
    if (!color_different) {
      set_color(overall.get_color());
      _drgbas = overall._drgbas;
    }
    for (vi = components.begin(); vi != components.end(); ++vi) {
      EggAttributes *component = (*vi);
      if (!normal_different) {
        component->clear_normal();
      }
      if (!color_different) {
        component->clear_color();
      }
    }
  }

  // Finally, assign the new components.
  for (size_t i = 0; i < components.size(); i++) {
    EggAttributes *component = components[i];
    set_component(i, component);
    delete component;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::apply_last_attribute
//       Access: Published, Virtual
//  Description: Sets the last vertex of the triangle (or each
//               component) to the primitive normal and/or color, if
//               they exist.  This reflects the Panda convention of
//               storing flat-shaded properties on the last vertex,
//               although it is not usually a convention in Egg.
//
//               This may introduce redundant vertices to the vertex
//               pool.
////////////////////////////////////////////////////////////////////
void EggCompositePrimitive::
apply_last_attribute() {
  // The first component gets applied to the third vertex, and so on
  // from there.
  for (int i = 0; i < get_num_components(); i++) {
    EggAttributes *component = get_component(i);

    // The significant_change flag is set if we have changed the
    // vertex in some important way, that will invalidate it for other
    // primitives that might share it.  We don't consider *adding* a
    // normal where there wasn't one before to be significant, but we
    // do consider it significant to change a vertex's normal to
    // something different.  Similarly for color.
    bool significant_change = false;

    EggVertex *orig_vertex = get_vertex(i + 2);
    nassertv(orig_vertex != (EggVertex *)NULL);
    PT(EggVertex) new_vertex = new EggVertex(*orig_vertex);

    if (component->has_normal() || has_normal()) {
      if (component->has_normal()) {
        new_vertex->set_normal(component->get_normal());
        new_vertex->_dnormals = component->_dnormals;
      } else {
        new_vertex->set_normal(get_normal());
        new_vertex->_dnormals = _dnormals;
      }

      if (orig_vertex->has_normal() &&
          (orig_vertex->get_normal() != new_vertex->get_normal() ||
           orig_vertex->_dnormals.compare_to(new_vertex->_dnormals) != 0)) {
        significant_change = true;
      }
    }

    if (component->has_color() || has_color()) {
      if (component->has_color()) {
        new_vertex->set_color(component->get_color());
        new_vertex->_drgbas = component->_drgbas;
      } else {
        new_vertex->set_color(get_color());
        new_vertex->_drgbas = _drgbas;
      }

      if (orig_vertex->has_color() &&
          (orig_vertex->get_color() != new_vertex->get_color() ||
           orig_vertex->_drgbas.compare_to(new_vertex->_drgbas) != 0)) {
        significant_change = true;
      }
    }

    if (significant_change) {
      new_vertex = get_pool()->create_unique_vertex(*new_vertex);
      set_vertex(i + 2, new_vertex);
    } else {
      // Just copy the new attributes back into the pool.
      ((EggAttributes *)orig_vertex)->operator = (*new_vertex);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::post_apply_last_attribute
//       Access: Published, Virtual
//  Description: Intended as a followup to apply_last_attribute(),
//               this also sets an attribute on the first vertices of
//               the primitive, if they don't already have an
//               attribute set, just so they end up with *something*.
////////////////////////////////////////////////////////////////////
void EggCompositePrimitive::
post_apply_last_attribute() {
  if (!empty()) {
    for (int i = 0; i < (int)size(); i++) {
      EggVertex *vertex = get_vertex(i);
      EggAttributes *component = get_component(max(i - 2, 0));

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

////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::cleanup
//       Access: Published, Virtual
//  Description: Cleans up modeling errors in whatever context this
//               makes sense.  For instance, for a polygon, this calls
//               remove_doubled_verts(true).  For a point, it calls
//               remove_nonunique_verts().  Returns true if the
//               primitive is valid, or false if it is degenerate.
////////////////////////////////////////////////////////////////////
bool EggCompositePrimitive::
cleanup() {
  return size() >= 3;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::prepare_add_vertex
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
void EggCompositePrimitive::
prepare_add_vertex(EggVertex *vertex, int i, int n) {
  EggPrimitive::prepare_add_vertex(vertex, i, n);

  if (n >= 3) {
    i = max(i - 2, 0);
    nassertv(i <= (int)_components.size());
    _components.insert(_components.begin() + i, new EggAttributes(*this));
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::prepare_remove_vertex
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
void EggCompositePrimitive::
prepare_remove_vertex(EggVertex *vertex, int i, int n) {
  EggPrimitive::prepare_remove_vertex(vertex, i, n);

  if (n >= 3) {
    i = max(i - 2, 0);
    nassertv(i < (int)_components.size());
    delete _components[i];
    _components.erase(_components.begin() + i);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggCompositePrimitive::write_body
//       Access: Protected
//  Description: Writes the attributes and the vertices referenced by
//               the primitive to the indicated output stream in Egg
//               format.
////////////////////////////////////////////////////////////////////
void EggCompositePrimitive::
write_body(ostream &out, int indent_level) const {
  EggPrimitive::write_body(out, indent_level);

  for (int i = 0; i < get_num_components(); i++) {
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
