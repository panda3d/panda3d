// Filename: eggGroupNode.cxx
// Created by:  drose (16Jan99)
// 
////////////////////////////////////////////////////////////////////

#include "eggGroupNode.h"
#include "eggCoordinateSystem.h"
#include "eggData.h"
#include "eggFilenameNode.h"
#include "eggExternalReference.h"
#include "eggPrimitive.h"
#include "eggTextureCollection.h"
#include "config_egg.h"

#include <dSearchPath.h>

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
//     Function: EggGroupNode::erase
//       Access: Public
//  Description: Part of the implementaion of the EggGroupNode as an
//               STL container.  Most of the rest of these functions
//               are inline and declared in eggGroupNode.I.
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
    if (child->is_of_type(EggFilenameNode::get_class_type())) {
      EggFilenameNode *filename = DCAST(EggFilenameNode, child);

      filename->resolve_filename(searchpath, 
				 filename->get_default_extension());

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
      PT(EggTexture) tex = DCAST(EggTexture, child);

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
//     Function: EggGroupNode::r_resolve_externals
//       Access: Protected
//  Description: Walks the tree and locates unloaded external
//               reference nodes, which it attempts to locate and load
//               in.  The reference node is replaced with the entire
//               subtree loaded.  This is intended to be called from
//               EggData::resolve_externals().
////////////////////////////////////////////////////////////////////
bool EggGroupNode::
r_resolve_externals(const DSearchPath &searchpath, 
		    CoordinateSystem coordsys) {
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
      EggGroupNode *new_node = 
	new EggGroupNode(ref->get_basename_wo_extension());
      replace(ci, new_node);

      if (!EggData::resolve_egg_filename(*ref, searchpath)) {
	egg_cat.error()
	  << "Could not locate " << ref->get_fullpath() << " in "
	  << searchpath << "\n";
      } else {
	// Now define a new EggData structure to hold the external
	// reference, and load it.
	EggData ext_data;
	ext_data.set_coordinate_system(coordsys);
	if (ext_data.read(*ref)) {
	  // The external file was read correctly.  Add its contents
	  // into the tree at this point.
	  success =
	    ext_data.resolve_externals(searchpath)
	    && success;
	  new_node->steal_children(ext_data);
	}
      }

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      EggGroupNode *group_child = DCAST(EggGroupNode, child);
      success =
	group_child->r_resolve_externals(searchpath, coordsys)
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
  // Make sure the node is in fact a child of this group.
  nassertv(node->get_parent() == this);
  nassertv(node->get_depth() == get_depth() + 1);
  node->_parent = NULL;

  node->update_under(-(get_depth() + 1));
}

  
