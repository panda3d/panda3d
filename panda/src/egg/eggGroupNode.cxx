// Filename: eggGroupNode.cxx
// Created by:  drose (16Jan99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "eggGroupNode.h"
#include "eggCoordinateSystem.h"
#include "eggData.h"
#include "eggFilenameNode.h"
#include "eggExternalReference.h"
#include "eggPrimitive.h"
#include "eggPolygon.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "eggTextureCollection.h"
#include "eggMaterialCollection.h"
#include "pt_EggTexture.h"
#include "pt_EggMaterial.h"
#include "config_egg.h"

#include "dSearchPath.h"
#include "deg_2_rad.h"

#include <algorithm>

TypeHandle EggGroupNode::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::Copy constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggGroupNode::
EggGroupNode(const EggGroupNode &copy) : EggNode(copy) {
  if (!copy.empty()) {
    egg_cat.warning()
      << "The EggGroupNode copy constructor does not copy children!\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::Copy assignment operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggGroupNode &EggGroupNode::
operator =(const EggGroupNode &copy) {
  if (!copy.empty()) {
    egg_cat.warning()
      << "The EggGroupNode copy assignment does not copy children!\n";
  }
  EggNode::operator =(copy);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
EggGroupNode::
~EggGroupNode() {
}


////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::write
//       Access: Public, Virtual
//  Description: Writes the group and all of its children to the
//               indicated output stream in Egg format.
////////////////////////////////////////////////////////////////////
void EggGroupNode::
write(ostream &out, int indent_level) const {
  iterator i;
  for (i = begin(); i != end(); ++i) {
    (*i)->write(out, indent_level);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::begin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggGroupNode::iterator EggGroupNode::
begin() const {
  return _children.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::end
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggGroupNode::iterator EggGroupNode::
end() const {
  return _children.end();
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::rbegin
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggGroupNode::reverse_iterator EggGroupNode::
rbegin() const {
  return _children.rbegin();
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::rend
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggGroupNode::reverse_iterator EggGroupNode::
rend() const {
  return _children.rend();
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::empty
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool EggGroupNode::
empty() const {
  return _children.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::size
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggGroupNode::size_type EggGroupNode::
size() const {
  return _children.size();
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::insert
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggGroupNode::iterator EggGroupNode::
insert(iterator position, PT(EggNode) x) {
  prepare_add_child(x);
  return _children.insert((Children::iterator &)position, x);
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::erase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggGroupNode::iterator EggGroupNode::
erase(iterator position) {
  prepare_remove_child(*position);
  return _children.erase((Children::iterator &)position);
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::erase
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggGroupNode::iterator EggGroupNode::
erase(iterator first, iterator last) {
  iterator i;
  for (i = first; i != last; ++i) {
    prepare_remove_child(*i);
  }
  return _children.erase((Children::iterator &)first,
                         (Children::iterator &)last);
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::replace
//       Access: Public
//  Description: Replaces the node at the indicated position with
//               the indicated node.  It is an error to call this
//               with an invalid position iterator (e.g. end()).
////////////////////////////////////////////////////////////////////
void EggGroupNode::
replace(iterator position, PT(EggNode) x) {
  nassertv(position != end());

  prepare_remove_child(*position);
  prepare_add_child(x);
  *(Children::iterator &)position = x;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::clear
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void EggGroupNode::
clear() {
  erase(begin(), end());
}


////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::add_child
//       Access: Public
//  Description: Adds the indicated child to the group and returns it.
//               If the child node is already a child of some other
//               node, removes it first.
////////////////////////////////////////////////////////////////////
PT(EggNode) EggGroupNode::
add_child(PT(EggNode) node) {
  if (node->_parent != NULL) {
    node->_parent->remove_child(node);
  }
  prepare_add_child(node);
  _children.push_back(node);
  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::remove_child
//       Access: Public
//  Description: Removes the indicated child node from the group and
//               returns it.  If the child was not already in the
//               group, does nothing and returns NULL.
////////////////////////////////////////////////////////////////////
PT(EggNode) EggGroupNode::
remove_child(PT(EggNode) node) {
  iterator i = find(begin(), end(), node);
  if (i == end()) {
    return PT(EggNode)();
  } else {
    // erase() calls prepare_remove_child().
    erase(i);
    return node;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::steal_children
//       Access: Public
//  Description: Moves all the children from the other node to this
//               one.  This is especially useful because the group
//               node copy assignment operator does not copy children.
////////////////////////////////////////////////////////////////////
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


////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::resolve_filenames
//       Access: Public
//  Description: Walks the tree and attempts to resolve any filenames
//               encountered.  This looks up filenames in the search
//               path, etc.  It does not automatically search the
//               egg_path for missing files.
////////////////////////////////////////////////////////////////////
void EggGroupNode::
resolve_filenames(const DSearchPath &searchpath) {
  Children::iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    EggNode *child = *ci;
    if (child->is_of_type(EggTexture::get_class_type())) {
      EggTexture *tex = DCAST(EggTexture, child);
      tex->update_filename().
        resolve_filename(searchpath, tex->get_default_extension());
      if (tex->has_alpha_file()) {
        tex->update_alpha_file().
          resolve_filename(searchpath, tex->get_default_extension());
      }

    } else if (child->is_of_type(EggFilenameNode::get_class_type())) {
      EggFilenameNode *fnode = DCAST(EggFilenameNode, child);
      fnode->update_filename().
        resolve_filename(searchpath, fnode->get_default_extension());

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      DCAST(EggGroupNode, child)->resolve_filenames(searchpath);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::reverse_vertex_ordering
//       Access: Public
//  Description: Reverses the vertex ordering of all polygons defined
//               at this node and below.  Does not change the surface
//               normals, if any.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::recompute_vertex_normals
//       Access: Public
//  Description: Recomputes all the vertex normals for polygon
//               geometry at this group node and below so that they
//               accurately reflect the vertex positions.  A shared
//               edge between two polygons (even in different groups)
//               is considered smooth if the angle between the two
//               edges is less than threshold degrees.
//
//               This function also removes degenerate polygons that
//               do not have enough vertices to define a normal.  It
//               does not affect normals for other kinds of primitives
//               like Nurbs or Points.
//
//               This function does not remove or adjust vertices in
//               the vertex pool; it only adds new vertices with the
//               correct normals.  Thus, it is a good idea to call
//               remove_unused_vertices() after calling this.
////////////////////////////////////////////////////////////////////
void EggGroupNode::
recompute_vertex_normals(double threshold, CoordinateSystem cs) {
  // First, collect all the vertices together with their shared
  // polygons.
  NVertexCollection collection;
  r_collect_vertex_normals(collection, threshold, cs);

  // Now bust them into separate groups according to the edge
  // threshold.  Two polygons that share a vertex belong in the same
  // group only if the angle between their normals is within the
  // threshold.

  double cos_angle = cos(deg_2_rad(threshold));

  NVertexCollection::iterator ci;
  for (ci = collection.begin(); ci != collection.end(); ++ci) {
    NVertexGroup &group = (*ci).second;

    // Here's a group of polygons that share a vertex.  Build up a new
    // group that consists of just the first polygon and all the ones
    // that are within threshold degrees from it.
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

      // Now new_group is a collection of connected polygons and the
      // vertices that connect them.  Smooth these vertices.
      do_compute_vertex_normals(new_group);

      // And reset the group of remaining polygons.
      group.swap(leftover_group);
      gi = group.begin();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::recompute_polygon_normals
//       Access: Public
//  Description: Recomputes all the polygon normals for polygon
//               geometry at this group node and below so that they
//               accurately reflect the vertex positions.  Normals are
//               removed from the vertices and defined only on
//               polygons, giving the geometry a faceted appearance.
//
//               This function also removes degenerate polygons that
//               do not have enough vertices to define a normal.  It
//               does not affect normals for other kinds of primitives
//               like Nurbs or Points.
//
//               This function does not remove or adjust vertices in
//               the vertex pool; it only adds new vertices with the
//               normals removed.  Thus, it is a good idea to call
//               remove_unused_vertices() after calling this.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::strip_normals
//       Access: Public
//  Description: Removes all normals from primitives, and the vertices
//               they reference, at this node and below.
//
//               This function does not remove or adjust vertices in
//               the vertex pool; it only adds new vertices with the
//               normal removed.  Thus, it is a good idea to call
//               remove_unused_vertices() after calling this.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::triangulate_polygons
//       Access: Public
//  Description: Replace all higher-order polygons at this point in
//               the scene graph and below with triangles.  Returns
//               the total number of new triangles produced, less
//               degenerate polygons removed.
//
//               If convex_also is true, both concave and convex
//               polygons will be subdivided into triangles;
//               otherwise, only concave polygons will be subdivided,
//               and convex polygons will be largely unchanged.
////////////////////////////////////////////////////////////////////
int EggGroupNode::
triangulate_polygons(bool convex_also) {
  int num_produced = 0;

  Children children_copy = _children;

  Children::iterator ci;
  for (ci = children_copy.begin();
       ci != children_copy.end();
       ++ci) {
    EggNode *child = (*ci);

    if (child->is_of_type(EggPolygon::get_class_type())) {
      EggPolygon *poly = DCAST(EggPolygon, child);
      poly->triangulate_in_place(convex_also);

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      num_produced += DCAST(EggGroupNode, child)->triangulate_polygons(convex_also);
    }
  }

  num_produced += max(0, (int)(_children.size() - children_copy.size()));
  return num_produced;
}


////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::remove_unused_vertices
//       Access: Public
//  Description: Removes all vertices from VertexPools within this
//               group or below that are not referenced by at least
//               one primitive.  Also renumbers all vertices after the
//               operation so their indices are consecutive, beginning
//               at zero.  Returns the total number of vertices removed.
//
//               Note that this operates on the VertexPools within
//               this group level, without respect to primitives that
//               reference these vertices (unlike other functions like
//               strip_normals()).  It is therefore most useful to
//               call this on the EggData root, rather than on a
//               subgroup within the hierarchy, since a VertexPool may
//               appear anywhere in the hierarchy.
////////////////////////////////////////////////////////////////////
int EggGroupNode::
remove_unused_vertices() {
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
        // If, after removing all the vertices, there's nothing left
        // in the vertex pool, then remove the whole vertex pool.
        _children.erase(ci);
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      num_removed += DCAST(EggGroupNode, child)->remove_unused_vertices();
    }

    ci = cnext;
  }

  return num_removed;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::remove_invalid_primitives
//       Access: Public
//  Description: Removes primitives at this level and below which
//               appear to be degenerate; e.g. polygons with fewer
//               than 3 vertices, etc.  Returns the number of
//               primitives removed.
////////////////////////////////////////////////////////////////////
int EggGroupNode::
remove_invalid_primitives() {
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
      num_removed += DCAST(EggGroupNode, child)->remove_invalid_primitives();
    }

    ci = cnext;
  }

  return num_removed;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::update_under
//       Access: Protected, Virtual
//  Description: This function is called from within EggGroupNode
//               whenever the parentage of the node has changed.  It
//               should update the depth and under_instance flags
//               accordingly.
//
//               Offset is the difference between the old depth value
//               and the new value.  It should be consistent with the
//               supplied depth value.  If it is not, we have some
//               error.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::r_transform
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

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::r_transform_vertices
//       Access: Protected, Virtual
//  Description: This is called from within the egg code by
//               transform_vertices_only()().  It applies a
//               transformation matrix to the current node in some
//               sensible way (if the current node is a vertex pool
//               with vertices), then continues down the tree.
////////////////////////////////////////////////////////////////////
void EggGroupNode::
r_transform_vertices(const LMatrix4d &mat) {
  Children::iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    (*ci)->r_transform_vertices(mat);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::r_mark_coordsys
//       Access: Protected, Virtual
//  Description: This is only called immediately after loading an egg
//               file from disk, to propagate the value found in the
//               CoordinateSystem entry (or the default Y-up
//               coordinate system) to all nodes that care about what
//               the coordinate system is.
////////////////////////////////////////////////////////////////////
void EggGroupNode::
r_mark_coordsys(CoordinateSystem cs) {
  Children::iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    (*ci)->r_mark_coordsys(cs);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::r_flatten_transforms
//       Access: Protected, Virtual
//  Description: The recursive implementation of flatten_transforms().
////////////////////////////////////////////////////////////////////
void EggGroupNode::
r_flatten_transforms() {
  Children::iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    (*ci)->r_flatten_transforms();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::r_apply_texmats
//       Access: Protected, Virtual
//  Description: The recursive implementation of apply_texmats().
////////////////////////////////////////////////////////////////////
void EggGroupNode::
r_apply_texmats(EggTextureCollection &textures) {
  Children::iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    (*ci)->r_apply_texmats(textures);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::find_coordsys_entry()
//       Access: Protected
//  Description: Walks the tree, looking for an EggCoordinateSystem
//               entry.  If one is found, extracts it and returns its
//               value.  If multiple entries are found, extracts all
//               of them and returns CS_invalid if they disagree.
////////////////////////////////////////////////////////////////////
CoordinateSystem EggGroupNode::
find_coordsys_entry() {
  CoordinateSystem coordsys = CS_default;

  // We can do this ci/cnext iteration through the list as we modify
  // it, only because we know this works with an STL list type
  // container.  If this were a vector or a set, this wouldn't
  // necessarily work.

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

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::find_textures()
//       Access: Protected
//  Description: Walks the tree, looking for EggTextures.  Each
//               EggTexture that is found is removed from the
//               hierarchy and added to the EggTextureCollection.
//               Returns the number of EggTextures found.
////////////////////////////////////////////////////////////////////
int EggGroupNode::
find_textures(EggTextureCollection *collection) {
  int num_found = 0;

  // We can do this ci/cnext iteration through the list as we modify
  // it, only because we know this works with an STL list type
  // container.  If this were a vector or a set, this wouldn't
  // necessarily work.

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

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::find_materials()
//       Access: Protected
//  Description: Walks the tree, looking for EggMaterials.  Each
//               EggMaterial that is found is removed from the
//               hierarchy and added to the EggMaterialCollection.
//               Returns the number of EggMaterials found.
////////////////////////////////////////////////////////////////////
int EggGroupNode::
find_materials(EggMaterialCollection *collection) {
  int num_found = 0;

  // We can do this ci/cnext iteration through the list as we modify
  // it, only because we know this works with an STL list type
  // container.  If this were a vector or a set, this wouldn't
  // necessarily work.

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

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::r_load_externals
//       Access: Protected
//  Description: Walks the tree and locates unloaded external
//               reference nodes, which it attempts to locate and load
//               in.  The reference node is replaced with the entire
//               subtree loaded.  This is intended to be called from
//               EggData::load_externals().
////////////////////////////////////////////////////////////////////
bool EggGroupNode::
r_load_externals(const DSearchPath &searchpath, CoordinateSystem coordsys) {
  bool success = true;

  Children::iterator ci;
  for (ci = _children.begin();
       ci != _children.end();
       ++ci) {
    EggNode *child = *ci;
    if (child->is_of_type(EggExternalReference::get_class_type())) {
      PT(EggExternalReference) ref = DCAST(EggExternalReference, child);

      // Replace the reference with an empty group node.  When we load
      // the external file successfully, we'll put its contents here.
      Filename filename = ref->get_filename();
      EggGroupNode *new_node =
        new EggGroupNode(filename.get_basename_wo_extension());
      replace(ci, new_node);

      if (!EggData::resolve_egg_filename(filename, searchpath)) {
        egg_cat.error()
          << "Could not locate " << filename << " in "
          << searchpath << "\n";
      } else {
        // Now define a new EggData structure to hold the external
        // reference, and load it.
        EggData ext_data;
        ext_data.set_coordinate_system(coordsys);
        ext_data.set_auto_resolve_externals(true);
        if (ext_data.read(filename)) {
          // The external file was read correctly.  Add its contents
          // into the tree at this point.
          success =
            ext_data.load_externals(searchpath)
            && success;
          new_node->steal_children(ext_data);
        }
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      EggGroupNode *group_child = DCAST(EggGroupNode, child);
      success =
        group_child->r_load_externals(searchpath, coordsys)
        && success;
    }
  }
  return success;
}


////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::prepare_add_child
//       Access: Private
//  Description: Marks the node as a child of the group.  This is an
//               internal function called by the STL-like functions
//               push_back() and insert(), in preparation for actually
//               adding the child.
//
//               It is an error to add a node that is already a child
//               of this group or some other group.
////////////////////////////////////////////////////////////////////
void EggGroupNode::
prepare_add_child(EggNode *node) {
  nassertv(node != (EggNode *)NULL);
  // Make sure the node is not already a child of some other group.
  nassertv(node->get_parent() == NULL);
  nassertv(node->get_depth() == 0);
  node->_parent = this;

  node->update_under(get_depth() + 1);
}


////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::prepare_remove_child
//       Access: Private
//  Description: Marks the node as removed from the group.  This is an
//               internal function called by the STL-like functions
//               pop_back() and erase(), in preparation for actually
//               doing the removal.
//
//               It is an error to attempt to remove a node that is
//               not already a child of this group.
////////////////////////////////////////////////////////////////////
void EggGroupNode::
prepare_remove_child(EggNode *node) {
  nassertv(node != (EggNode *)NULL);
  // Make sure the node is in fact a child of this group.
  nassertv(node->get_parent() == this);
  nassertv(node->get_depth() == get_depth() + 1);
  node->_parent = NULL;

  node->update_under(-(get_depth() + 1));
}



////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::r_collect_vertex_normals
//       Access: Private
//  Description: This is part of the implementation of
//               recompute_vertex_normals().  It walks the scene graph
//               at this group node and below, identifying all the
//               polygons and the vertices they have in common.
////////////////////////////////////////////////////////////////////
void EggGroupNode::
r_collect_vertex_normals(EggGroupNode::NVertexCollection &collection,
                         double threshold, CoordinateSystem cs) {
  // We can do this ci/cnext iteration through the list as we modify
  // it, only because we know this works with an STL list type
  // container.  If this were a vector or a set, this wouldn't
  // necessarily work.

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
        // Now add each vertex from the polygon separately to our
        // collection.
        size_t num_vertices = polygon->size();
        for (size_t i = 0; i < num_vertices; i++) {
          EggVertex *vertex = polygon->get_vertex(i);
          ref._vertex = i;
          collection[vertex->get_pos3()].push_back(ref);
        }
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      EggGroupNode *group = DCAST(EggGroupNode, child);

      // We can't share vertices across an Instance node.  Don't
      // even bother trying.  Instead, just restart.
      if (group->is_under_instance()) {
        group->recompute_vertex_normals(threshold, cs);
      } else {
        group->r_collect_vertex_normals(collection, threshold, cs);
      }
    }

    ci = cnext;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupNode::r_collect_vertex_normals
//       Access: Private
//  Description: This is part of the implementation of
//               recompute_vertex_normals().  It accepts a group of
//               polygons and their common normals, and computes the
//               same normal for all their shared vertices.
////////////////////////////////////////////////////////////////////
void EggGroupNode::
do_compute_vertex_normals(const NVertexGroup &group) {
  nassertv(!group.empty());

  // Determine the common normal.  This is simply the average of all
  // the polygon normals that share this vertex.
  Normald normal(0.0, 0.0, 0.0);
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

