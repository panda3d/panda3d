/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFileMaker.cxx
 * @author drose
 * @date 2001-06-19
 */

#include "xFileMaker.h"
#include "xFileMesh.h"
#include "xFileMaterial.h"
#include "config_xfile.h"

#include "pnotify.h"
#include "eggGroupNode.h"
#include "eggGroup.h"
#include "eggBin.h"
#include "eggPolysetMaker.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "eggPolygon.h"
#include "eggData.h"
#include "pvector.h"
#include "vector_int.h"
#include "string_utils.h"
#include "datagram.h"

/**
 *
 */
XFileMaker::
XFileMaker() {
  _mesh_index = 0;
  _x_file = new XFile;
}

/**
 *
 */
XFileMaker::
~XFileMaker() {
}

/**
 * Writes the .x file data to the indicated filename; returns true on success,
 * false otherwise.
 */
bool XFileMaker::
write(const Filename &filename) {
  return _x_file->write(filename);
}

/**
 * Adds the egg tree rooted at the indicated node to the X structure.  This
 * may be somewhat destructive of the egg tree.  Returns true on success,
 * false on failure.
 */
bool XFileMaker::
add_tree(EggData *egg_data) {
  _meshes.clear();

  // Now collect all the polygons together into polysets.
  EggPolysetMaker pmaker;
  pmaker.make_bins(egg_data);

  // And now we're ready to traverse the egg hierarchy.
  if (!recurse_nodes(egg_data, _x_file)) {
    return false;
  }

  // Create X structures for all of the meshes we built up.
  Meshes::iterator mi;
  for (mi = _meshes.begin(); mi != _meshes.end(); ++mi) {
    if (!finalize_mesh((*mi).first, (*mi).second)) {
      return false;
    }
  }
  _meshes.clear();

  return true;
}

/**
 * Adds the node to the DX structure, in whatever form it is supported.
 */
bool XFileMaker::
add_node(EggNode *egg_node, XFileNode *x_parent) {
  if (egg_node->is_of_type(EggBin::get_class_type())) {
    return add_bin(DCAST(EggBin, egg_node), x_parent);

  } else if (egg_node->is_of_type(EggGroup::get_class_type())) {
    return add_group(DCAST(EggGroup, egg_node), x_parent);

  } else if (egg_node->is_of_type(EggGroupNode::get_class_type())) {
    // A grouping node of some kind.
    EggGroupNode *egg_group = DCAST(EggGroupNode, egg_node);

    if (xfile_one_mesh) {
      // Don't create any additional frames representing the egg hierarchy.
      if (!recurse_nodes(egg_group, x_parent)) {
        return false;
      }

    } else {
      // Create a Frame for each EggGroup.
      XFileDataNode *x_frame = x_parent->add_Frame(egg_group->get_name());

      if (!recurse_nodes(egg_group, x_frame)) {
        return false;
      }
    }

    return true;
  }

  // Some unsupported node type.  Ignore it.
  return true;
}

/**
 * Adds a frame for the indicated group node.
 */
bool XFileMaker::
add_group(EggGroup *egg_group, XFileNode *x_parent) {
  if (xfile_one_mesh) {
    // Don't create any additional frames representing the egg hierarchy.
    if (!recurse_nodes(egg_group, x_parent)) {
      return false;
    }

  } else {
    // Create a frame for each EggGroup.
    XFileDataNode *x_frame = x_parent->add_Frame(egg_group->get_name());

    // Set the transform on the frame, if we have one.
    if (egg_group->has_transform()) {
      x_frame->add_FrameTransformMatrix(egg_group->get_transform3d());
    }

    if (!recurse_nodes(egg_group, x_frame)) {
      return false;
    }
  }

  return true;
}

/**
 * Determines what kind of object needs to be added for the indicated bin
 * node.
 */
bool XFileMaker::
add_bin(EggBin *egg_bin, XFileNode *x_parent) {
  switch (egg_bin->get_bin_number()) {
  case EggPolysetMaker::BN_polyset:
    return add_polyset(egg_bin, x_parent);
  }

  xfile_cat.error()
    << "Unexpected bin type " << egg_bin->get_bin_number() << "\n";
  return false;
}

/**
 * Adds a mesh object corresponding to the collection of polygons within the
 * indicated bin.
 */
bool XFileMaker::
add_polyset(EggBin *egg_bin, XFileNode *x_parent) {
  // Make sure that all our polygons are reasonable.
  egg_bin->remove_invalid_primitives(true);

  XFileMesh *mesh = get_mesh(x_parent);

  EggGroupNode::iterator ci;
  for (ci = egg_bin->begin(); ci != egg_bin->end(); ++ci) {
    EggPolygon *poly;
    DCAST_INTO_R(poly, *ci, false);

    mesh->add_polygon(poly);
  }

  return true;
}


/**
 * Adds each child of the indicated Node as a child of the indicated DX
 * object.
 */
bool XFileMaker::
recurse_nodes(EggGroupNode *egg_node, XFileNode *x_parent) {
  EggGroupNode::iterator ci;
  for (ci = egg_node->begin(); ci != egg_node->end(); ++ci) {
    EggNode *child = (*ci);
    if (!add_node(child, x_parent)) {
      return false;
    }
  }

  return true;
}

/**
 * Returns a suitable XFileMesh object for creating meshes within the
 * indicated x_parent object.
 */
XFileMesh *XFileMaker::
get_mesh(XFileNode *x_parent) {
  Meshes::iterator mi = _meshes.find(x_parent);
  if (mi != _meshes.end()) {
    // We've already started working on this x_parent before; use the same
    // mesh object.
    return (*mi).second;
  }

  // We haven't seen this x_parent before; create a new mesh object.
  XFileMesh *mesh = new XFileMesh;
  _meshes.insert(Meshes::value_type(x_parent, mesh));
  return mesh;
}


/**
 * Creates the actual X structures corresponding to the indicated XFileMesh
 * object.
 */
bool XFileMaker::
finalize_mesh(XFileNode *x_parent, XFileMesh *mesh) {
  // Get a unique number for each mesh.
  _mesh_index++;
  std::string mesh_index = format_string(_mesh_index);

  // Finally, create the Mesh object.
  mesh->make_x_mesh(x_parent, mesh_index);

  return true;
}
