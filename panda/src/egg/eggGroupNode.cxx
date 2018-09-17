/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggGroupNode.cxx
 * @author drose
 * @date 1999-01-16
 */

#include "eggGroupNode.h"
#include "eggGroup.h"
#include "eggCoordinateSystem.h"
#include "eggData.h"
#include "eggFilenameNode.h"
#include "eggExternalReference.h"
#include "eggPrimitive.h"
#include "eggPoint.h"
#include "eggPolygon.h"
#include "eggCompositePrimitive.h"
#include "eggMesher.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "eggTextureCollection.h"
#include "eggMaterialCollection.h"
#include "epvector.h"
#include "pt_EggTexture.h"
#include "pt_EggMaterial.h"
#include "config_egg.h"

#include "dSearchPath.h"
#include "virtualFileSystem.h"
#include "deg_2_rad.h"
#include "dcast.h"
#include "bamCacheRecord.h"

#include <algorithm>

using std::string;

TypeHandle EggGroupNode::_type_handle;


/**
 *
 */
EggGroupNode::
EggGroupNode(const EggGroupNode &copy) : EggNode(copy) {
  if (!copy.empty()) {
    egg_cat.warning()
      << "The EggGroupNode copy constructor does not copy children!\n";
  }
}

/**
 *
 */
EggGroupNode &EggGroupNode::
operator =(const EggGroupNode &copy) {
  if (!copy.empty()) {
    egg_cat.warning()
      << "The EggGroupNode copy assignment does not copy children!\n";
  }
  EggNode::operator =(copy);
  return *this;
}

/**
 *
 */
EggGroupNode::
~EggGroupNode() {
}


/**
 * Writes the group and all of its children to the indicated output stream in
 * Egg format.
 */
void EggGroupNode::
write(std::ostream &out, int indent_level) const {
  iterator i;

  // Since joints tend to reference vertex pools, which sometimes appear later
  // in the file, and since generally non-joints don't reference joints, we
  // try to maximize our chance of writing out a one-pass readable egg file by
  // writing joints at the end of the list of children of a particular node.

  for (i = begin(); i != end(); ++i) {
    PT(EggNode) child = (*i);
    if (!child->is_joint()) {
      child->write(out, indent_level);
    }
  }

  for (i = begin(); i != end(); ++i) {
    PT(EggNode) child = (*i);
    if (child->is_joint()) {
      child->write(out, indent_level);
    }
  }
}

/**
 *
 */
EggGroupNode::iterator EggGroupNode::
begin() const {
  return _children.begin();
}

/**
 *
 */
EggGroupNode::iterator EggGroupNode::
end() const {
  return _children.end();
}

/**
 *
 */
EggGroupNode::reverse_iterator EggGroupNode::
rbegin() const {
  return _children.rbegin();
}

/**
 *
 */
EggGroupNode::reverse_iterator EggGroupNode::
rend() const {
  return _children.rend();
}

/**
 *
 */
EggGroupNode::iterator EggGroupNode::
insert(iterator position, PT(EggNode) x) {
  prepare_add_child(x);
  return _children.insert((Children::iterator &)position, x);
}

/**
 *
 */
EggGroupNode::iterator EggGroupNode::
erase(iterator position) {
  prepare_remove_child(*position);
  return _children.erase((Children::iterator &)position);
}

/**
 *
 */
EggGroupNode::iterator EggGroupNode::
erase(iterator first, iterator last) {
  iterator i;
  for (i = first; i != last; ++i) {
    prepare_remove_child(*i);
  }
  return _children.erase((Children::iterator &)first,
                         (Children::iterator &)last);
}

/**
 * Replaces the node at the indicated position with the indicated node.  It is
 * an error to call this with an invalid position iterator (e.g.  end()).
 */
void EggGroupNode::
replace(iterator position, PT(EggNode) x) {
  nassertv(position != end());

  prepare_remove_child(*position);
  prepare_add_child(x);
  *(Children::iterator &)position = x;
}

/**
 *
 */
bool EggGroupNode::
empty() const {
  return _children.empty();
}

/**
 *
 */
EggGroupNode::size_type EggGroupNode::
size() const {
  return _children.size();
}

/**
 *
 */
void EggGroupNode::
clear() {
  erase(begin(), end());
}

/**
 * Returns the first child in the group's list of children, or NULL if the
 * list of children is empty.  Can be used with get_next_child() to return the
 * complete list of children without using the iterator class; however, this
 * is non-thread-safe, and so is not recommended except for languages other
 * than C++ which cannot use the iterators.
 */
EggNode *EggGroupNode::
get_first_child() {
  _gnc_iterator = begin();
  return get_next_child();
}

/**
 * Returns the next child in the group's list of children since the last call
 * to get_first_child() or get_next_child(), or NULL if the last child has
 * been returned.  Can be used with get_first_child() to return the complete
 * list of children without using the iterator class; however, this is non-
 * thread-safe, and so is not recommended except for languages other than C++
 * which cannot use the iterators.
 *
 * It is an error to call this without previously calling get_first_child().
 */
EggNode *EggGroupNode::
get_next_child() {
  if (_gnc_iterator != end()) {
    return *_gnc_iterator++;
  }
  return nullptr;
}

/**
 * Adds the indicated child to the group and returns it.  If the child node is
 * already a child of some other node, removes it first.
 */
EggNode *EggGroupNode::
add_child(EggNode *node) {
  test_ref_count_integrity();
  PT(EggNode) ptnode = node;
  if (node->_parent != nullptr) {
    node->_parent->remove_child(node);
  }
  prepare_add_child(node);
  _children.push_back(node);
  return node;
}

/**
 * Removes the indicated child node from the group and returns it.  If the
 * child was not already in the group, does nothing and returns NULL.
 */
PT(EggNode) EggGroupNode::
remove_child(EggNode *node) {
  PT(EggNode) ptnode = node;
  iterator i = find(begin(), end(), ptnode);
  if (i == end()) {
    return PT(EggNode)();
  } else {
    // erase() calls prepare_remove_child().
    erase(i);
    return ptnode;
  }
}


/**
 * Moves all the children from the other node to this one.  This is especially
 * useful because the group node copy assignment operator does not copy
 * children.
 */
void EggGroupNode::
steal_children(EggGroupNode &other) {
  Children::iterator ci;
  for (ci = other._children.begin();
       ci != other._children.end();
       ++ci) {
    other.prepare_remove_child(*ci);
    prepare_add_child(*ci);
  }

  _children.splice(_children.end(), other._children);
}

/**
 * Returns the child of this node whose name is the indicated string, or NULL
 * if there is no child of this node by that name.  Does not search
 * recursively.
 */
EggNode *EggGroupNode::
find_child(const string &name) const {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggNode *child = (*ci);
    if (child->get_name() == name) {
      return child;
    }
  }

  return nullptr;
}

/**
 * Returns true if any nodes at this level and below include a reference to a
 * file via an absolute pathname, or false if all references are relative.
 */
bool EggGroupNode::
has_absolute_pathnames() const {
  Children::const_iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    EggNode *child = *ci;
    if (child->is_of_type(EggTexture::get_class_type())) {
      EggTexture *tex = DCAST(EggTexture, child);
      if (!tex->get_filename().is_local()) {
        if (egg_cat.is_debug()) {
          egg_cat.debug()
            << "Absolute pathname: " << tex->get_filename()
            << "\n";
        }
        return true;
      }

      if (tex->has_alpha_filename()) {
        if (!tex->get_alpha_filename().is_local()) {
          if (egg_cat.is_debug()) {
            egg_cat.debug()
              << "Absolute pathname: " << tex->get_alpha_filename()
              << "\n";
          }
          return true;
        }
      }

    } else if (child->is_of_type(EggFilenameNode::get_class_type())) {
      EggFilenameNode *fnode = DCAST(EggFilenameNode, child);
      if (!fnode->get_filename().is_local()) {
        if (egg_cat.is_debug()) {
          egg_cat.debug()
            << "Absolute pathname: " << fnode->get_filename()
            << "\n";
        }
        return true;
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      if (DCAST(EggGroupNode, child)->has_absolute_pathnames()) {
        return true;
      }
    }
  }

  return false;
}

/**
 * Walks the tree and attempts to resolve any filenames encountered.  This
 * looks up filenames along the specified search path; it does not
 * automatically search the model_path for missing files.
 */
void EggGroupNode::
resolve_filenames(const DSearchPath &searchpath) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Children::iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    EggNode *child = *ci;
    if (child->is_of_type(EggTexture::get_class_type())) {
      EggTexture *tex = DCAST(EggTexture, child);
      Filename tex_filename = tex->get_filename();
      vfs->resolve_filename(tex_filename, searchpath);
      tex->set_filename(tex_filename);

      if (tex->has_alpha_filename()) {
        Filename alpha_filename = tex->get_alpha_filename();
        vfs->resolve_filename(alpha_filename, searchpath);
        tex->set_alpha_filename(alpha_filename);
      }

    } else if (child->is_of_type(EggFilenameNode::get_class_type())) {
      EggFilenameNode *fnode = DCAST(EggFilenameNode, child);
      Filename filename = fnode->get_filename();
      vfs->resolve_filename(filename, searchpath, fnode->get_default_extension());
      fnode->set_filename(filename);

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      DCAST(EggGroupNode, child)->resolve_filenames(searchpath);
    }
  }
}

/**
 * Similar to resolve_filenames, but each non-absolute filename encountered is
 * arbitrarily taken to be in the indicated directory, whether or not the so-
 * named filename exists.
 */
void EggGroupNode::
force_filenames(const Filename &directory) {
  Children::iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    EggNode *child = *ci;
    if (child->is_of_type(EggTexture::get_class_type())) {
      EggTexture *tex = DCAST(EggTexture, child);
      Filename tex_filename = tex->get_filename();
      if (tex_filename.is_local()) {
        tex->set_filename(Filename(directory, tex_filename));
      }

      if (tex->has_alpha_filename()) {
        Filename alpha_filename = tex->get_alpha_filename();
        if (alpha_filename.is_local()) {
          tex->set_alpha_filename(Filename(directory, alpha_filename));
        }
      }

    } else if (child->is_of_type(EggFilenameNode::get_class_type())) {
      EggFilenameNode *fnode = DCAST(EggFilenameNode, child);
      Filename filename = fnode->get_filename();
      if (filename.is_local()) {
        fnode->set_filename(Filename(directory, filename));
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      DCAST(EggGroupNode, child)->force_filenames(directory);
    }
  }
}

/**
 * Reverses the vertex ordering of all polygons defined at this node and
 * below.  Does not change the surface normals, if any.
 */
void EggGroupNode::
reverse_vertex_ordering() {
  Children::iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    EggNode *child = *ci;
    if (child->is_of_type(EggPrimitive::get_class_type())) {
      EggPrimitive *prim = DCAST(EggPrimitive, child);
      prim->reverse_vertex_ordering();

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      DCAST(EggGroupNode, child)->reverse_vertex_ordering();
    }
  }
}

/**
 * Recomputes all the vertex normals for polygon geometry at this group node
 * and below so that they accurately reflect the vertex positions.  A shared
 * edge between two polygons (even in different groups) is considered smooth
 * if the angle between the two edges is less than threshold degrees.
 *
 * This function also removes degenerate polygons that do not have enough
 * vertices to define a normal.  It does not affect normals for other kinds of
 * primitives like Nurbs or Points.
 *
 * This function does not remove or adjust vertices in the vertex pool; it
 * only adds new vertices with the correct normals.  Thus, it is a good idea
 * to call remove_unused_vertices() after calling this.
 */
void EggGroupNode::
recompute_vertex_normals(double threshold, CoordinateSystem cs) {
  // First, collect all the vertices together with their shared polygons.
  NVertexCollection collection;
  r_collect_vertex_normals(collection, threshold, cs);

  // Now bust them into separate groups according to the edge threshold.  Two
  // polygons that share a vertex belong in the same group only if the angle
  // between their normals is within the threshold.

  double cos_angle = cos(deg_2_rad(threshold));

  NVertexCollection::iterator ci;
  for (ci = collection.begin(); ci != collection.end(); ++ci) {
    NVertexGroup &group = (*ci).second;

    // Here's a group of polygons that share a vertex.  Build up a new group
    // that consists of just the first polygon and all the ones that are
    // within threshold degrees from it.
    NVertexGroup::iterator gi;
    gi = group.begin();
    while (gi != group.end()) {
      const NVertexReference &base_ref = (*gi);
      NVertexGroup new_group;
      NVertexGroup leftover_group;
      new_group.push_back(base_ref);
      ++gi;

      while (gi != group.end()) {
        const NVertexReference &ref = (*gi);
        double dot = base_ref._normal.dot(ref._normal);
        if (dot > cos_angle) {
          // These polygons are close enough to the same angle.
          new_group.push_back(ref);
        } else {
          // These polygons are not.
          leftover_group.push_back(ref);
        }
        ++gi;
      }

      // Now new_group is a collection of connected polygons and the vertices
      // that connect them.  Smooth these vertices.
      do_compute_vertex_normals(new_group);

      // And reset the group of remaining polygons.
      group.swap(leftover_group);
      gi = group.begin();
    }
  }
}

/**
 * Recomputes all the polygon normals for polygon geometry at this group node
 * and below so that they accurately reflect the vertex positions.  Normals
 * are removed from the vertices and defined only on polygons, giving the
 * geometry a faceted appearance.
 *
 * This function also removes degenerate polygons that do not have enough
 * vertices to define a normal.  It does not affect normals for other kinds of
 * primitives like Nurbs or Points.
 *
 * This function does not remove or adjust vertices in the vertex pool; it
 * only adds new vertices with the normals removed.  Thus, it is a good idea
 * to call remove_unused_vertices() after calling this.
 */
void EggGroupNode::
recompute_polygon_normals(CoordinateSystem cs) {
  Children::iterator ci, cnext;
  ci = _children.begin();
  while (ci != _children.end()) {
    cnext = ci;
    ++cnext;
    EggNode *child = *ci;

    if (child->is_of_type(EggPolygon::get_class_type())) {
      EggPolygon *polygon = DCAST(EggPolygon, child);

      if (!polygon->recompute_polygon_normal(cs)) {
        // The polygon is degenerate.  Remove it.
        prepare_remove_child(child);
        _children.erase(ci);

      } else {
        // Remove the normal from each polygon vertex.
        size_t num_vertices = polygon->size();
        for (size_t i = 0; i < num_vertices; i++) {
          EggVertex *vertex = polygon->get_vertex(i);
          EggVertexPool *pool = vertex->get_pool();

          if (vertex->has_normal()) {
            EggVertex new_vertex(*vertex);
            new_vertex.clear_normal();
            EggVertex *unique = pool->create_unique_vertex(new_vertex);
            unique->copy_grefs_from(*vertex);

            polygon->set_vertex(i, unique);
          }
        }
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      DCAST(EggGroupNode, child)->recompute_polygon_normals(cs);
    }

    ci = cnext;
  }
}

/**
 * Removes all normals from primitives, and the vertices they reference, at
 * this node and below.
 *
 * This function does not remove or adjust vertices in the vertex pool; it
 * only adds new vertices with the normal removed.  Thus, it is a good idea to
 * call remove_unused_vertices() after calling this.
 */
void EggGroupNode::
strip_normals() {
  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggNode *child = *ci;

    if (child->is_of_type(EggPrimitive::get_class_type())) {
      EggPrimitive *prim = DCAST(EggPrimitive, child);
      prim->clear_normal();

      // Remove the normal from each prim vertex.
      size_t num_vertices = prim->size();
      for (size_t i = 0; i < num_vertices; i++) {
        EggVertex *vertex = prim->get_vertex(i);
        EggVertexPool *pool = vertex->get_pool();

        if (vertex->has_normal()) {
          EggVertex new_vertex(*vertex);
          new_vertex.clear_normal();
          EggVertex *unique = pool->create_unique_vertex(new_vertex);
          unique->copy_grefs_from(*vertex);

          prim->set_vertex(i, unique);
        }
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      DCAST(EggGroupNode, child)->strip_normals();
    }
  }
}

/**
 * This function recomputes the tangent and binormal for the named texture
 * coordinate set for all vertices at this level and below.  Use the empty
 * string for the default texture coordinate set.
 *
 * It is necessary for each vertex to already have a normal (or at least a
 * polygon normal), as well as a texture coordinate in the named texture
 * coordinate set, before calling this function.  You might precede this with
 * recompute_vertex_normals() to ensure that the normals exist.
 *
 * Like recompute_vertex_normals(), this function does not remove or adjust
 * vertices in the vertex pool; it only adds new vertices with the new
 * tangents and binormals computed.  Thus, it is a good idea to call
 * remove_unused_vertices() after calling this.
 */
bool EggGroupNode::
recompute_tangent_binormal(const GlobPattern &uv_name) {
  // First, collect all the vertices together with their shared polygons.
  TBNVertexCollection collection;
  r_collect_tangent_binormal(uv_name, collection);

  // Now compute the tangent and binormal separately for each common group of
  // vertices.
  TBNVertexCollection::const_iterator ci;
  for (ci = collection.begin(); ci != collection.end(); ++ci) {
    const TBNVertexValue &value = (*ci).first;
    const TBNVertexGroup &group = (*ci).second;

    do_compute_tangent_binormal(value, group);
  }

  return true;
}

/**
 * This function recomputes the tangent and binormal for the named texture
 * coordinate sets.  Returns true if anything was done.
 */
bool EggGroupNode::
recompute_tangent_binormal(const vector_string &names) {
  bool changed = false;

  for (vector_string::const_iterator si = names.begin();
       si != names.end();
       ++si) {
    GlobPattern uv_name(*si);
    nout << "Computing tangent and binormal for \"" << uv_name << "\"\n";
    recompute_tangent_binormal(uv_name);
    changed = true;
  }

  return changed;
}

/**
 * This function recomputes the tangent and binormal for any texture
 * coordinate set that affects a normal map.  Returns true if anything was
 * done.
 */
bool EggGroupNode::
recompute_tangent_binormal_auto() {
  vector_string names;
  EggTextureCollection texs;
  EggTextureCollection::iterator eti;
  texs.find_used_textures(this);
  for (eti = texs.begin(); eti != texs.end(); eti++) {
    EggTexture *eggtex = (*eti);
    if ((eggtex->get_env_type() == EggTexture::ET_normal)||
        (eggtex->get_env_type() == EggTexture::ET_normal_height)||
        (eggtex->get_env_type() == EggTexture::ET_normal_gloss)) {
      string uv = eggtex->get_uv_name();
      vector_string::iterator it = find(names.begin(), names.end(), uv);
      if (it == names.end()) {
        names.push_back(uv);
      }
    }
  }
  return recompute_tangent_binormal(names);
}

/**
 * Replace all higher-order polygons at this point in the scene graph and
 * below with triangles.  Returns the total number of new triangles produced,
 * less degenerate polygons removed.
 *
 * If flags contains T_polygon and T_convex, both concave and convex polygons
 * will be subdivided into triangles; with only T_polygon, only concave
 * polygons will be subdivided, and convex polygons will be largely unchanged.
 */
int EggGroupNode::
triangulate_polygons(int flags) {
  int num_produced = 0;

  Children children_copy = _children;

  Children::iterator ci;
  for (ci = children_copy.begin();
       ci != children_copy.end();
       ++ci) {
    EggNode *child = (*ci);

    if (child->is_of_type(EggPolygon::get_class_type())) {
      if ((flags & T_polygon) != 0) {
        EggPolygon *poly = DCAST(EggPolygon, child);
        poly->triangulate_in_place((flags & T_convex) != 0);
      }

    } else if (child->is_of_type(EggCompositePrimitive::get_class_type())) {
      if ((flags & T_composite) != 0) {
        EggCompositePrimitive *comp = DCAST(EggCompositePrimitive, child);
        comp->triangulate_in_place();
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      if ((flags & T_recurse) != 0) {
        num_produced += DCAST(EggGroupNode, child)->triangulate_polygons(flags);
      }
    }
  }

  num_produced += std::max(0, (int)(_children.size() - children_copy.size()));
  return num_produced;
}

/**
 * Combine triangles together into triangle strips, at this group and below.
 */
void EggGroupNode::
mesh_triangles(int flags) {
  EggMesher mesher;
  mesher.mesh(this, (flags & T_flat_shaded) != 0);

  if ((flags & T_recurse) != 0) {
    EggGroupNode::iterator ci;
    for (ci = begin(); ci != end(); ++ci) {
      if ((*ci)->is_of_type(EggGroupNode::get_class_type())) {
        EggGroupNode *group_child = DCAST(EggGroupNode, *ci);
        group_child->mesh_triangles(flags);
      }
    }
  }
}

/**
 * Creates PointLight primitives to reference any otherwise unreferences
 * vertices discovered in this group or below.
 */
void EggGroupNode::
make_point_primitives() {
  // Create a temporary node to hold the EggPoint objects we might create
  // while we iterate.  (We don't add them during the iteration to avoid
  // invalidating the iterator.)
  PT(EggGroupNode) temp = new EggGroup("temp");

  EggGroupNode::iterator ci;
  for (ci = begin(); ci != end(); ++ci) {
    if ((*ci)->is_of_type(EggGroupNode::get_class_type())) {
      EggGroupNode *group_child = DCAST(EggGroupNode, *ci);
      group_child->make_point_primitives();

    } else if ((*ci)->is_of_type(EggVertexPool::get_class_type())) {
      EggVertexPool *vpool = DCAST(EggVertexPool, *ci);
      PT(EggPrimitive) prim = new EggPoint;
      vpool->add_unused_vertices_to_prim(prim);
      if (!prim->empty()) {
        temp->add_child(prim);
      }
    }
  }

  steal_children(*temp);
}

/**
 * Rename by stripping out the prefix
 */
int EggGroupNode::
rename_nodes(vector_string strip_prefix, bool recurse) {
  int num_renamed = 0;
  for (unsigned int ni = 0; ni < strip_prefix.size(); ++ni) {
    string axe_name = strip_prefix[ni];
    if (this->get_name().substr(0, axe_name.size()) == axe_name) {
      string new_name = this->get_name().substr(axe_name.size());
      // cout << "renaming " << this->get_name() << "->" << new_name << endl;
      this->set_name(new_name);
      num_renamed += 1;
    }
  }
  if (recurse) {
    EggGroupNode::iterator ci;
    for (ci = begin(); ci != end(); ++ci) {
      if ((*ci)->is_of_type(EggGroupNode::get_class_type())) {
        EggGroupNode *group_child = DCAST(EggGroupNode, *ci);
        num_renamed += group_child->rename_nodes(strip_prefix, recurse);
      }
      else if ((*ci)->is_of_type(EggNode::get_class_type())) {
        EggNode *node_child = DCAST(EggNode, *ci);
        num_renamed += node_child->rename_node(strip_prefix);
      }
    }
  }
  return num_renamed;
}

/**
 * Removes all vertices from VertexPools within this group or below that are
 * not referenced by at least one primitive.  Also collapses together
 * equivalent vertices, and renumbers all vertices after the operation so
 * their indices are consecutive, beginning at zero.  Returns the total number
 * of vertices removed.
 *
 * Note that this operates on the VertexPools within this group level, without
 * respect to primitives that reference these vertices (unlike other functions
 * like strip_normals()).  It is therefore most useful to call this on the
 * EggData root, rather than on a subgroup within the hierarchy, since a
 * VertexPool may appear anywhere in the hierarchy.
 */
int EggGroupNode::
remove_unused_vertices(bool recurse) {
  int num_removed = 0;

  Children::iterator ci, cnext;
  ci = _children.begin();
  while (ci != _children.end()) {
    cnext = ci;
    ++cnext;
    EggNode *child = *ci;

    if (child->is_of_type(EggVertexPool::get_class_type())) {
      EggVertexPool *vpool = DCAST(EggVertexPool, child);
      num_removed += vpool->remove_unused_vertices();

      if (vpool->empty()) {
        // If, after removing all the vertices, there's nothing left in the
        // vertex pool, then remove the whole vertex pool.
        _children.erase(ci);
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      if (recurse) {
        num_removed += DCAST(EggGroupNode, child)->remove_unused_vertices(recurse);
      }
    }

    ci = cnext;
  }

  return num_removed;
}

/**
 * Removes primitives at this level and below which appear to be degenerate;
 * e.g.  polygons with fewer than 3 vertices, etc.  Returns the number of
 * primitives removed.
 */
int EggGroupNode::
remove_invalid_primitives(bool recurse) {
  int num_removed = 0;

  Children::iterator ci, cnext;
  ci = _children.begin();
  while (ci != _children.end()) {
    cnext = ci;
    ++cnext;
    EggNode *child = *ci;

    if (child->is_of_type(EggPrimitive::get_class_type())) {
      EggPrimitive *prim = DCAST(EggPrimitive, child);
      if (!prim->cleanup()) {
        _children.erase(ci);
        num_removed++;
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      if (recurse) {
        num_removed += DCAST(EggGroupNode, child)->remove_invalid_primitives(recurse);
      }
    }

    ci = cnext;
  }

  return num_removed;
}

/**
 * Resets the connected_shading information on all primitives at this node and
 * below, so that it may be accurately rederived by the next call to
 * get_connected_shading().
 *
 * It may be a good idea to call remove_unused_vertices() as well, to
 * establish the correct connectivity between common vertices.
 */
void EggGroupNode::
clear_connected_shading() {
  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggNode *child = *ci;

    if (child->is_of_type(EggPrimitive::get_class_type())) {
      EggPrimitive *prim = DCAST(EggPrimitive, child);
      prim->clear_connected_shading();
    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      DCAST(EggGroupNode, child)->clear_connected_shading();
    }
  }
}

/**
 * Queries the connected_shading information on all primitives at this node
 * and below, to ensure that it has been completely filled in before we start
 * mucking around with vertices.
 */
void EggGroupNode::
get_connected_shading() {
  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggNode *child = *ci;

    if (child->is_of_type(EggPrimitive::get_class_type())) {
      EggPrimitive *prim = DCAST(EggPrimitive, child);
      prim->get_connected_shading();
    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      DCAST(EggGroupNode, child)->get_connected_shading();
    }
  }
}

/**
 * Applies per-vertex normal and color to all vertices, if they are in fact
 * per-vertex (and different for each vertex), or moves them to the primitive
 * if they are all the same.
 *
 * After this call, either the primitive will have normals or its vertices
 * will, but not both.  Ditto for colors.
 *
 * If use_connected_shading is true, each polygon is considered in conjunction
 * with all connected polygons; otherwise, each polygon is considered
 * individually.
 *
 * If allow_per_primitive is false, S_per_face or S_overall will treated like
 * S_per_vertex: normals and colors will always be assigned to the vertices.
 * In this case, there will never be per-primitive colors or normals after
 * this call returns.  On the other hand, if allow_per_primitive is true, then
 * S_per_face means that normals and colors should be assigned to the
 * primitives, and removed from the vertices, as described above.
 *
 * This may create redundant vertices in the vertex pool, so it may be a good
 * idea to follow this up with remove_unused_vertices().
 */
void EggGroupNode::
unify_attributes(bool use_connected_shading, bool allow_per_primitive,
                 bool recurse) {
  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggNode *child = *ci;

    if (child->is_of_type(EggPrimitive::get_class_type())) {
      EggPrimitive *prim = DCAST(EggPrimitive, child);

      EggPrimitive::Shading shading = EggPrimitive::S_per_vertex;

      if (allow_per_primitive) {
        shading = prim->get_shading();
        if (use_connected_shading) {
          shading = prim->get_connected_shading();
        }
      }

      prim->unify_attributes(shading);

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      if (recurse) {
        DCAST(EggGroupNode, child)->unify_attributes
          (use_connected_shading, allow_per_primitive, recurse);
      }
    }
  }
}

/**
 * Sets the last vertex of the triangle (or each component) to the primitive
 * normal and/or color, if the primitive is flat-shaded.  This reflects the
 * OpenGL convention of storing flat-shaded properties on the last vertex,
 * although it is not usually a convention in Egg.
 *
 * This may create redundant vertices in the vertex pool, so it may be a good
 * idea to follow this up with remove_unused_vertices().
 */
void EggGroupNode::
apply_last_attribute(bool recurse) {
  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggNode *child = *ci;

    if (child->is_of_type(EggPrimitive::get_class_type())) {
      EggPrimitive *prim = DCAST(EggPrimitive, child);
      prim->apply_last_attribute();
    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      if (recurse) {
        DCAST(EggGroupNode, child)->apply_last_attribute(recurse);
      }
    }
  }
}

/**
 * Sets the first vertex of the triangle (or each component) to the primitive
 * normal and/or color, if the primitive is flat-shaded.  This reflects the
 * DirectX convention of storing flat-shaded properties on the first vertex,
 * although it is not usually a convention in Egg.
 *
 * This may create redundant vertices in the vertex pool, so it may be a good
 * idea to follow this up with remove_unused_vertices().
 */
void EggGroupNode::
apply_first_attribute(bool recurse) {
  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggNode *child = *ci;

    if (child->is_of_type(EggPrimitive::get_class_type())) {
      EggPrimitive *prim = DCAST(EggPrimitive, child);
      prim->apply_first_attribute();
    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      if (recurse) {
        DCAST(EggGroupNode, child)->apply_first_attribute(recurse);
      }
    }
  }
}

/**
 * Intended as a followup to apply_last_attribute(), this also sets an
 * attribute on the first vertices of the primitive, if they don't already
 * have an attribute set, just so they end up with *something*.
 */
void EggGroupNode::
post_apply_flat_attribute(bool recurse) {
  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggNode *child = *ci;

    if (child->is_of_type(EggPrimitive::get_class_type())) {
      EggPrimitive *prim = DCAST(EggPrimitive, child);
      prim->post_apply_flat_attribute();
    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      if (recurse) {
        DCAST(EggGroupNode, child)->post_apply_flat_attribute(recurse);
      }
    }
  }
}

/**
 * Returns true if there are any primitives (e.g.  polygons) defined within
 * this group or below, false otherwise.
 */
bool EggGroupNode::
has_primitives() const {
  Children::const_iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    if ((*ci)->has_primitives()) {
      return true;
    }
  }

  return false;
}

/**
 * Returns true if there are any primitives (e.g.  polygons) defined within
 * this group or below, but the search does not include nested joints.
 */
bool EggGroupNode::
joint_has_primitives() const {
  Children::const_iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    EggNode *child = (*ci);

    if (!child->is_joint()) {
      if (child->joint_has_primitives()) {
        return true;
      }
    }
  }

  return false;
}

/**
 * Returns true if any of the primitives (e.g.  polygons) defined within this
 * group or below have either face or vertex normals defined, false otherwise.
 */
bool EggGroupNode::
has_normals() const {
  Children::const_iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    if ((*ci)->has_normals()) {
      return true;
    }
  }

  return false;
}

/**
 * Copies vertices used by the primitives at this group node (and below, if
 * recurse is true) into one or more new vertex pools, and updates the
 * primitives to reference these pools.  It is up to the caller to parent the
 * newly-created vertex pools somewhere appropriate in the egg hierarchy.
 *
 * No more than max_vertices will be placed into any one vertex pool.  This is
 * the sole criteria for splitting vertex pools.
 */
void EggGroupNode::
rebuild_vertex_pools(EggVertexPools &vertex_pools, unsigned int max_vertices,
                     bool recurse) {
  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggNode *child = *ci;

    if (child->is_of_type(EggPrimitive::get_class_type())) {
      typedef pvector< PT(EggVertex) > Vertices;
      Vertices vertices;
      EggPrimitive *prim = DCAST(EggPrimitive, child);

      // Copy all of the vertices out.
      EggPrimitive::const_iterator pi;
      for (pi = prim->begin(); pi != prim->end(); ++pi) {
        vertices.push_back(*pi);
      }

      typedef epvector<EggAttributes> Attributes;
      Attributes attributes;

      if (prim->is_of_type(EggCompositePrimitive::get_class_type())) {
        // A compositive primitive has the additional complication of dealing
        // with its attributes.
        EggCompositePrimitive *cprim = DCAST(EggCompositePrimitive, prim);
        int i;
        int num_components = cprim->get_num_components();
        for (i = 0; i < num_components; i++) {
          attributes.push_back(*cprim->get_component(i));
        }
      }

      prim->clear();

      // Now look for a new home for the vertices.  First, see if any of the
      // vertex pools we've already created already have a copy of each one of
      // the vertices.
      bool found_pool = false;
      EggVertexPool *best_pool = nullptr;
      int best_new_vertices = 0;

      Vertices new_vertices;
      EggVertexPools::iterator vpi;
      for (vpi = vertex_pools.begin();
           vpi != vertex_pools.end() && !found_pool;
           ++vpi) {
        EggVertexPool *vertex_pool = (*vpi);
        int num_new_vertices = 0;

        new_vertices.clear();
        new_vertices.reserve(vertices.size());

        Vertices::const_iterator vi;
        for (vi = vertices.begin();
             vi != vertices.end() && !found_pool;
             ++vi) {
          EggVertex *vertex = (*vi);
          EggVertex *new_vertex = vertex_pool->find_matching_vertex(*vertex);
          new_vertices.push_back(new_vertex);
          if (new_vertex == nullptr) {
            ++num_new_vertices;
          }
        }

        if (num_new_vertices == 0) {
          // Great, we found a vertex pool that already shares all these
          // vertices.  No need to look any further.
          found_pool = true;

        } else if (vertex_pool->size() + num_new_vertices <= max_vertices) {
          // We would have to add some vertices to this pool, so this vertex
          // pool qualifies only if the number of vertices we have to add
          // would still keep it within our limit.
          if (best_pool == nullptr ||
              num_new_vertices < best_new_vertices) {
            // This is currently our most favorable vertex pool.
            best_pool = vertex_pool;
            best_new_vertices = num_new_vertices;
          }
        }
      }

      if (!found_pool) {
        if (best_pool == nullptr) {
          // There was no vertex pool that qualified.  We will have to create
          // a new vertex pool.
          best_pool = new EggVertexPool("");
          vertex_pools.push_back(best_pool);
        }

        new_vertices.clear();
        new_vertices.reserve(vertices.size());

        Vertices::const_iterator vi;
        for (vi = vertices.begin(); vi != vertices.end(); ++vi) {
          EggVertex *vertex = (*vi);
          EggVertex *new_vertex = best_pool->create_unique_vertex(*vertex);
          new_vertex->copy_grefs_from(*vertex);
          new_vertices.push_back(new_vertex);
        }
      }

      Vertices::const_iterator vi;
      nassertv(new_vertices.size() == vertices.size());
      for (vi = new_vertices.begin(); vi != new_vertices.end(); ++vi) {
        EggVertex *new_vertex = (*vi);
        nassertv(new_vertex != nullptr);
        prim->add_vertex(new_vertex);
      }

      if (prim->is_of_type(EggCompositePrimitive::get_class_type())) {
        // Now restore the composite attributes.
        EggCompositePrimitive *cprim = DCAST(EggCompositePrimitive, prim);
        int i;
        int num_components = cprim->get_num_components();
        nassertv(num_components == (int)attributes.size());
        for (i = 0; i < num_components; i++) {
          cprim->set_component(i, &attributes[i]);
        }
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      if (recurse) {
        DCAST(EggGroupNode, child)->rebuild_vertex_pools(vertex_pools, max_vertices, recurse);
      }
    }
  }
}

/**
 * This function is called from within EggGroupNode whenever the parentage of
 * the node has changed.  It should update the depth and under_instance flags
 * accordingly.
 *
 * Offset is the difference between the old depth value and the new value.  It
 * should be consistent with the supplied depth value.  If it is not, we have
 * some error.
 */
void EggGroupNode::
update_under(int depth_offset) {
  EggNode::update_under(depth_offset);

  Children::iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    nassertv((*ci)->get_parent() == this);
    (*ci)->update_under(depth_offset);
  }
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
void EggGroupNode::
r_transform(const LMatrix4d &mat, const LMatrix4d &inv,
            CoordinateSystem to_cs) {
  Children::iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    (*ci)->r_transform(mat, inv, to_cs);
  }
}

/**
 * This is called from within the egg code by transform_vertices_only()().  It
 * applies a transformation matrix to the current node in some sensible way
 * (if the current node is a vertex pool with vertices), then continues down
 * the tree.
 */
void EggGroupNode::
r_transform_vertices(const LMatrix4d &mat) {
  Children::iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    (*ci)->r_transform_vertices(mat);
  }
}

/**
 * This is only called immediately after loading an egg file from disk, to
 * propagate the value found in the CoordinateSystem entry (or the default
 * Y-up coordinate system) to all nodes that care about what the coordinate
 * system is.
 */
void EggGroupNode::
r_mark_coordsys(CoordinateSystem cs) {
  Children::iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    (*ci)->r_mark_coordsys(cs);
  }
}

/**
 * The recursive implementation of flatten_transforms().
 */
void EggGroupNode::
r_flatten_transforms() {
  Children::iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    (*ci)->r_flatten_transforms();
  }
}

/**
 * The recursive implementation of apply_texmats().
 */
void EggGroupNode::
r_apply_texmats(EggTextureCollection &textures) {
  Children::iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    (*ci)->r_apply_texmats(textures);
  }
}

/**
 * Walks the tree, looking for an EggCoordinateSystem entry.  If one is found,
 * extracts it and returns its value.  If multiple entries are found, extracts
 * all of them and returns CS_invalid if they disagree.
 */
CoordinateSystem EggGroupNode::
find_coordsys_entry() {
  CoordinateSystem coordsys = CS_default;

  // We can do this cicnext iteration through the list as we modify it, only
  // because we know this works with an STL list type container.  If this were
  // a vector or a set, this wouldn't necessarily work.

  Children::iterator ci, cnext;
  ci = _children.begin();
  while (ci != _children.end()) {
    cnext = ci;
    ++cnext;
    EggNode *child = *ci;

    if (child->is_of_type(EggCoordinateSystem::get_class_type())) {
      CoordinateSystem new_cs =
        DCAST(EggCoordinateSystem, child)->get_value();

      // Now remove the CoordinateSystem entry from our child list.
      prepare_remove_child(child);
      _children.erase(ci);

      if (new_cs != CS_default) {
        if (coordsys != CS_default && coordsys != new_cs) {
          coordsys = CS_invalid;
        } else {
          coordsys = new_cs;
        }
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      CoordinateSystem new_cs =
        DCAST(EggGroupNode, child)->find_coordsys_entry();
      if (new_cs != CS_default) {
        if (coordsys != CS_default && coordsys != new_cs) {
          coordsys = CS_invalid;
        } else {
          coordsys = new_cs;
        }
      }
    }

    ci = cnext;
  }

  return coordsys;
}

/**
 * Walks the tree, looking for EggTextures.  Each EggTexture that is found is
 * removed from the hierarchy and added to the EggTextureCollection.  Returns
 * the number of EggTextures found.
 */
int EggGroupNode::
find_textures(EggTextureCollection *collection) {
  int num_found = 0;

  // We can do this cicnext iteration through the list as we modify it, only
  // because we know this works with an STL list type container.  If this were
  // a vector or a set, this wouldn't necessarily work.

  Children::iterator ci, cnext;
  ci = _children.begin();
  while (ci != _children.end()) {
    cnext = ci;
    ++cnext;
    EggNode *child = *ci;

    if (child->is_of_type(EggTexture::get_class_type())) {
      PT_EggTexture tex = DCAST(EggTexture, child);

      // Now remove the EggTexture entry from our child list.
      prepare_remove_child(tex);
      _children.erase(ci);

      // And add it to the collection.
      collection->add_texture(tex);
      num_found++;

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      num_found +=
        DCAST(EggGroupNode, child)->find_textures(collection);
    }

    ci = cnext;
  }

  return num_found;
}

/**
 * Walks the tree, looking for EggMaterials.  Each EggMaterial that is found
 * is removed from the hierarchy and added to the EggMaterialCollection.
 * Returns the number of EggMaterials found.
 */
int EggGroupNode::
find_materials(EggMaterialCollection *collection) {
  int num_found = 0;

  // We can do this cicnext iteration through the list as we modify it, only
  // because we know this works with an STL list type container.  If this were
  // a vector or a set, this wouldn't necessarily work.

  Children::iterator ci, cnext;
  ci = _children.begin();
  while (ci != _children.end()) {
    cnext = ci;
    ++cnext;
    EggNode *child = *ci;

    if (child->is_of_type(EggMaterial::get_class_type())) {
      PT_EggMaterial tex = DCAST(EggMaterial, child);

      // Now remove the EggMaterial entry from our child list.
      prepare_remove_child(tex);
      _children.erase(ci);

      // And add it to the collection.
      collection->add_material(tex);
      num_found++;

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      num_found +=
        DCAST(EggGroupNode, child)->find_materials(collection);
    }

    ci = cnext;
  }

  return num_found;
}

/**
 * Walks the tree and locates unloaded external reference nodes, which it
 * attempts to locate and load in.  The reference node is replaced with the
 * entire subtree loaded.  This is intended to be called from
 * EggData::load_externals().
 */
bool EggGroupNode::
r_load_externals(const DSearchPath &searchpath, CoordinateSystem coordsys,
                 BamCacheRecord *record) {
  bool success = true;

  Children::iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    EggNode *child = *ci;
    if (child->is_of_type(EggExternalReference::get_class_type())) {
      PT(EggExternalReference) ref = DCAST(EggExternalReference, child);

      // Replace the reference with an empty group node.  When we load the
      // external file successfully, we'll put its contents here.
      Filename filename = ref->get_filename();
      EggGroupNode *new_node =
        new EggGroupNode(filename.get_basename_wo_extension());
      replace(ci, new_node);

      if (!EggData::resolve_egg_filename(filename, searchpath)) {
        egg_cat.error()
          << "Could not locate " << filename << " in "
          << searchpath << "\n";
      } else {
        // Now define a new EggData structure to hold the external reference,
        // and load it.
        EggData ext_data;
        ext_data.set_coordinate_system(coordsys);
        ext_data.set_auto_resolve_externals(true);
        if (ext_data.read(filename)) {
          // The external file was read correctly.  Add its contents into the
          // tree at this point.
          if (record != nullptr) {
            record->add_dependent_file(filename);
          }

          success =
            ext_data.load_externals(searchpath, record)
            && success;
          new_node->steal_children(ext_data);
        }
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      EggGroupNode *group_child = DCAST(EggGroupNode, child);
      success =
        group_child->r_load_externals(searchpath, coordsys, record)
        && success;
    }
  }
  return success;
}


/**
 * Marks the node as a child of the group.  This is an internal function
 * called by the STL-like functions push_back() and insert(), in preparation
 * for actually adding the child.
 *
 * It is an error to add a node that is already a child of this group or some
 * other group.
 */
void EggGroupNode::
prepare_add_child(EggNode *node) {
  nassertv(node != nullptr);
  test_ref_count_integrity();
  node->test_ref_count_integrity();
  // Make sure the node is not already a child of some other group.
  nassertv(node->get_parent() == nullptr);
  nassertv(node->get_depth() == 0);
  node->_parent = this;

  node->update_under(get_depth() + 1);
}


/**
 * Marks the node as removed from the group.  This is an internal function
 * called by the STL-like functions pop_back() and erase(), in preparation for
 * actually doing the removal.
 *
 * It is an error to attempt to remove a node that is not already a child of
 * this group.
 */
void EggGroupNode::
prepare_remove_child(EggNode *node) {
  nassertv(node != nullptr);
  // Make sure the node is in fact a child of this group.
  nassertv(node->get_parent() == this);
  nassertv(node->get_depth() == get_depth() + 1);
  node->_parent = nullptr;

  node->update_under(-(get_depth() + 1));
}



/**
 * This is part of the implementation of recompute_vertex_normals().  It walks
 * the scene graph at this group node and below, identifying all the polygons
 * and the vertices they have in common.
 */
void EggGroupNode::
r_collect_vertex_normals(EggGroupNode::NVertexCollection &collection,
                         double threshold, CoordinateSystem cs) {
  // We can do this cicnext iteration through the list as we modify it, only
  // because we know this works with an STL list type container.  If this were
  // a vector or a set, this wouldn't necessarily work.

  Children::iterator ci, cnext;
  ci = _children.begin();
  while (ci != _children.end()) {
    cnext = ci;
    ++cnext;
    EggNode *child = *ci;

    if (child->is_of_type(EggPolygon::get_class_type())) {
      EggPolygon *polygon = DCAST(EggPolygon, child);
      polygon->clear_normal();

      NVertexReference ref;
      ref._polygon = polygon;
      if (!polygon->calculate_normal(ref._normal, cs)) {
        // The polygon is degenerate.  Remove it.

        prepare_remove_child(child);
        _children.erase(ci);

      } else {
        // Now add each vertex from the polygon separately to our collection.
        size_t num_vertices = polygon->size();
        for (size_t i = 0; i < num_vertices; i++) {
          EggVertex *vertex = polygon->get_vertex(i);
          ref._vertex = i;
          collection[vertex->get_pos3()].push_back(ref);
        }
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      EggGroupNode *group = DCAST(EggGroupNode, child);

      // We can't share vertices across an Instance node.  Don't even bother
      // trying.  Instead, just restart.
      if (group->is_under_instance()) {
        group->recompute_vertex_normals(threshold, cs);
      } else {
        group->r_collect_vertex_normals(collection, threshold, cs);
      }
    }

    ci = cnext;
  }
}

/**
 * This is part of the implementation of recompute_vertex_normals().  It
 * accepts a group of polygons and their common normals, and computes the same
 * normal for all their shared vertices.
 */
void EggGroupNode::
do_compute_vertex_normals(const NVertexGroup &group) {
  nassertv(!group.empty());

  // Determine the common normal.  This is simply the average of all the
  // polygon normals that share this vertex.
  LNormald normal(0.0, 0.0, 0.0);
  NVertexGroup::const_iterator gi;
  for (gi = group.begin(); gi != group.end(); ++gi) {
    const NVertexReference &ref = (*gi);
    normal += ref._normal;
  }

  normal /= (double)group.size();
  normal.normalize();

  // Now we have the common normal; apply it to all the vertices.

  for (gi = group.begin(); gi != group.end(); ++gi) {
    const NVertexReference &ref = (*gi);
    EggVertex *vertex = ref._polygon->get_vertex(ref._vertex);
    EggVertexPool *pool = vertex->get_pool();

    EggVertex new_vertex(*vertex);
    new_vertex.set_normal(normal);
    EggVertex *unique = pool->create_unique_vertex(new_vertex);
    unique->copy_grefs_from(*vertex);

    ref._polygon->set_vertex(ref._vertex, unique);
  }
}

/**
 * This is part of the implementation of recompute_tangent_binormal().  It
 * walks the scene graph at this group node and below, identifying all the
 * polygons and the vertices they have in common.
 */
void EggGroupNode::
r_collect_tangent_binormal(const GlobPattern &uv_name,
                           EggGroupNode::TBNVertexCollection &collection) {
  Children::iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    EggNode *child = *ci;

    if (child->is_of_type(EggPolygon::get_class_type())) {
      EggPolygon *polygon = DCAST(EggPolygon, child);

      TBNVertexReference ref;
      ref._polygon = polygon;

      // Now add each vertex from the polygon separately to our collection.
      size_t num_vertices = polygon->size();
      for (size_t i = 0; i < num_vertices; i++) {
        // We look at the triangle formed by each three consecutive vertices
        // to determine the s direction and t direction at each vertex.  v1 is
        // the key vertex, the one at position i; v2 is vertex i + 1, and v3
        // is vertex i - 1.
        EggVertex *v1 = polygon->get_vertex(i);
        EggVertex *v2 = polygon->get_vertex((i + 1) % num_vertices);
        EggVertex *v3 = polygon->get_vertex((i + num_vertices - 1) % num_vertices);
        if (v1->has_normal() || polygon->has_normal()) {
          // Go through all of the UV names on the vertex, looking for one
          // that matches the glob pattern.
          EggVertex::const_uv_iterator uvi;
          for (uvi = v1->uv_begin(); uvi != v1->uv_end(); ++uvi) {
            EggVertexUV *uv_obj = (*uvi);
            string name = uv_obj->get_name();
            if (uv_name.matches(name) &&
                v2->has_uv(name) && v3->has_uv(name)) {
              TBNVertexValue value;
              value._uv_name = name;
              value._pos = v1->get_pos3();
              if (v1->has_normal()) {
                value._normal = v1->get_normal();
              } else {
                value._normal = polygon->get_normal();
              }
              value._uv = v1->get_uv(name);

              // Compute the s direction and t direction for this vertex.
              LPoint3d p1 = v1->get_pos3();
              LPoint3d p2 = v2->get_pos3();
              LPoint3d p3 = v3->get_pos3();

              LTexCoordd w1 = v1->get_uv(name);
              LTexCoordd w2 = v2->get_uv(name);
              LTexCoordd w3 = v3->get_uv(name);

              // Check the facing of the texture; we will have to split
              // vertices whose UV's are mirrored along a seam.  The facing is
              // determined by the winding order of the texcoords on the
              // polygon.  A front-facing polygon should not contribute to the
              // tangent and binormal of a back-facing polygon, and vice-
              // versa.
              value._facing = is_right(w1 - w2, w3 - w1);

              double x1 = p2[0] - p1[0];
              double x2 = p3[0] - p1[0];
              double y1 = p2[1] - p1[1];
              double y2 = p3[1] - p1[1];
              double z1 = p2[2] - p1[2];
              double z2 = p3[2] - p1[2];

              double s1 = w2[0] - w1[0];
              double s2 = w3[0] - w1[0];
              double t1 = w2[1] - w1[1];
              double t2 = w3[1] - w1[1];

              double denom = (s1 * t2 - s2 * t1);
              if (denom == 0.0) {
                ref._sdir.set(0.0, 0.0, 0.0);
                ref._tdir.set(0.0, 0.0, 0.0);
              } else {
                double r = 1.0 / denom;
                ref._sdir.set((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
                              (t2 * z1 - t1 * z2) * r);
                ref._tdir.set((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
                              (s1 * z2 - s2 * z1) * r);
              }

              // Store the vertex referenced to the polygon.
              ref._vertex = i;
              collection[value].push_back(ref);
            }
          }
        }
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      EggGroupNode *group = DCAST(EggGroupNode, child);

      // We can't share vertices across an Instance node.  Don't even bother
      // trying.  Instead, just restart.
      if (group->is_under_instance()) {
        group->recompute_tangent_binormal(uv_name);
      } else {
        group->r_collect_tangent_binormal(uv_name, collection);
      }
    }
  }
}

/**
 * This is part of the implementation of recompute_tangent_binormal().  It
 * accepts a group of polygons and their common normals and UV's, and computes
 * the tangent and binormal for all their shared vertices.
 */
void EggGroupNode::
do_compute_tangent_binormal(const TBNVertexValue &value,
                            const TBNVertexGroup &group) {
  nassertv(!group.empty());

  // Accumulate together all of the s vectors and t vectors computed for the
  // different vertices that are together here.
  LNormald sdir(0.0, 0.0, 0.0);
  LNormald tdir(0.0, 0.0, 0.0);

  TBNVertexGroup::const_iterator gi;
  for (gi = group.begin(); gi != group.end(); ++gi) {
    const TBNVertexReference &ref = (*gi);
    sdir += ref._sdir;
    tdir += ref._tdir;
  }

  // If sdir andor tdir are zero, choose an arbitrary vector instead.  (This
  // is really the only reason we normalize sdir and tdir, though it also
  // helps stabilize the math below in case the vectors are very small but not
  // quite zero.)
  if (!sdir.normalize()) {
    sdir.set(1.0, 0.0, 0.0);
  }
  if (!tdir.normalize()) {
    tdir = sdir.cross(LNormald(0.0, 0.0, -1.0));
  }

  LNormald tangent = (sdir - value._normal * value._normal.dot(sdir));
  tangent.normalize();

  LNormald binormal = cross(value._normal, tangent);
  if (dot(binormal, tdir) < 0.0f) {
    binormal = -binormal;
  }
  // Shouldn't need to normalize this, but we do just for good measure.
  binormal.normalize();

  // Now we have the common tangent and binormal; apply them to all the
  // vertices.

  for (gi = group.begin(); gi != group.end(); ++gi) {
    const TBNVertexReference &ref = (*gi);
    EggVertex *vertex = ref._polygon->get_vertex(ref._vertex);
    EggVertexPool *pool = vertex->get_pool();

    EggVertex new_vertex(*vertex);
    EggVertexUV *uv_obj = new_vertex.modify_uv_obj(value._uv_name);
    nassertv(uv_obj != nullptr);
    uv_obj->set_tangent(tangent);
    uv_obj->set_binormal(binormal);

    EggVertex *unique = pool->create_unique_vertex(new_vertex);
    unique->copy_grefs_from(*vertex);

    ref._polygon->set_vertex(ref._vertex, unique);
  }
}
