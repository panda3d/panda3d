// Filename: nodePathCollection.cxx
// Created by:  drose (06Mar02)
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

#include "nodePathCollection.h"
#include "findApproxPath.h"
#include "findApproxLevelEntry.h"
#include "colorAttrib.h"

#include "indent.h"

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
NodePathCollection::
NodePathCollection() {
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
NodePathCollection::
NodePathCollection(const NodePathCollection &copy) :
  _node_paths(copy._node_paths)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void NodePathCollection::
operator = (const NodePathCollection &copy) {
  _node_paths = copy._node_paths;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::add_path
//       Access: Published
//  Description: Adds a new NodePath to the collection.
////////////////////////////////////////////////////////////////////
void NodePathCollection::
add_path(const NodePath &node_path) {
  // If the pointer to our internal array is shared by any other
  // NodePathCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren NodePathCollection
  // objects.

  if (_node_paths.get_ref_count() > 1) {
    NodePaths old_node_paths = _node_paths;
    _node_paths = NodePaths::empty_array(0);
    _node_paths.v() = old_node_paths.v();
  }

  _node_paths.push_back(node_path);
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::remove_path
//       Access: Published
//  Description: Removes the indicated NodePath from the collection.
//               Returns true if the path was removed, false if it was
//               not a member of the collection.
////////////////////////////////////////////////////////////////////
bool NodePathCollection::
remove_path(const NodePath &node_path) {
  int path_index = -1;
  for (int i = 0; path_index == -1 && i < (int)_node_paths.size(); i++) {
    if (_node_paths[i] == node_path) {
      path_index = i;
    }
  }

  if (path_index == -1) {
    // The indicated path was not a member of the collection.
    return false;
  }

  // If the pointer to our internal array is shared by any other
  // NodePathCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren NodePathCollection
  // objects.

  if (_node_paths.get_ref_count() > 1) {
    NodePaths old_node_paths = _node_paths;
    _node_paths = NodePaths::empty_array(0);
    _node_paths.v() = old_node_paths.v();
  }

  _node_paths.erase(_node_paths.begin() + path_index);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::add_paths_from
//       Access: Published
//  Description: Adds all the NodePaths indicated in the other
//               collection to this path.  The other paths are simply
//               appended to the end of the paths in this list;
//               duplicates are not automatically removed.
////////////////////////////////////////////////////////////////////
void NodePathCollection::
add_paths_from(const NodePathCollection &other) {
  int other_num_paths = other.get_num_paths();
  for (int i = 0; i < other_num_paths; i++) {
    add_path(other.get_path(i));
  }
}


////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::remove_paths_from
//       Access: Published
//  Description: Removes from this collection all of the NodePaths
//               listed in the other collection.
////////////////////////////////////////////////////////////////////
void NodePathCollection::
remove_paths_from(const NodePathCollection &other) {
  NodePaths new_paths;
  int num_paths = get_num_paths();
  for (int i = 0; i < num_paths; i++) {
    NodePath path = get_path(i);
    if (!other.has_path(path)) {
      new_paths.push_back(path);
    }
  }
  _node_paths = new_paths;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::remove_duplicate_paths
//       Access: Published
//  Description: Removes any duplicate entries of the same NodePaths
//               on this collection.  If a NodePath appears multiple
//               times, the first appearance is retained; subsequent
//               appearances are removed.
////////////////////////////////////////////////////////////////////
void NodePathCollection::
remove_duplicate_paths() {
  NodePaths new_paths;

  int num_paths = get_num_paths();
  for (int i = 0; i < num_paths; i++) {
    NodePath path = get_path(i);
    bool duplicated = false;

    for (int j = 0; j < i && !duplicated; j++) {
      duplicated = (path == get_path(j));
    }

    if (!duplicated) {
      new_paths.push_back(path);
    }
  }

  _node_paths = new_paths;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::has_path
//       Access: Published
//  Description: Returns true if the indicated NodePath appears in
//               this collection, false otherwise.
////////////////////////////////////////////////////////////////////
bool NodePathCollection::
has_path(const NodePath &path) const {
  for (int i = 0; i < get_num_paths(); i++) {
    if (path == get_path(i)) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::clear
//       Access: Published
//  Description: Removes all NodePaths from the collection.
////////////////////////////////////////////////////////////////////
void NodePathCollection::
clear() {
  _node_paths.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::is_empty
//       Access: Published
//  Description: Returns true if there are no NodePaths in the
//               collection, false otherwise.
////////////////////////////////////////////////////////////////////
bool NodePathCollection::
is_empty() const {
  return _node_paths.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::get_num_paths
//       Access: Published
//  Description: Returns the number of NodePaths in the collection.
////////////////////////////////////////////////////////////////////
int NodePathCollection::
get_num_paths() const {
  return _node_paths.size();
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::get_path
//       Access: Published
//  Description: Returns the nth NodePath in the collection.
////////////////////////////////////////////////////////////////////
NodePath NodePathCollection::
get_path(int index) const {
  nassertr(index >= 0 && index < (int)_node_paths.size(), NodePath());

  return _node_paths[index];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::operator []
//       Access: Published
//  Description: Returns the nth NodePath in the collection.  This is
//               the same as get_path(), but it may be a more
//               convenient way to access it.
////////////////////////////////////////////////////////////////////
NodePath NodePathCollection::
operator [] (int index) const {
  nassertr(index >= 0 && index < (int)_node_paths.size(), NodePath());

  return _node_paths[index];
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::ls
//       Access: Published
//  Description: Lists all the nodes at and below each node in the
//               collection hierarchically.
////////////////////////////////////////////////////////////////////
void NodePathCollection::
ls(ostream &out, int indent_level) const {
  for (int i = 0; i < get_num_paths(); i++) {
    NodePath path = get_path(i);
    indent(out, indent_level) << path << "\n";
    path.ls(out, indent_level + 2);
    out << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::find_all_matches
//       Access: Published
//  Description: Returns the complete set of all NodePaths that begin
//               with any NodePath in this collection and can be
//               extended by path.  The shortest paths will be listed
//               first.
////////////////////////////////////////////////////////////////////
NodePathCollection NodePathCollection::
find_all_matches(const string &path) const {
  NodePathCollection result;

  FindApproxPath approx_path;
  if (approx_path.add_string(path)) {
    if (!is_empty()) {
      FindApproxLevelEntry *level = NULL;
      for (int i = 0; i < get_num_paths(); i++) {
        FindApproxLevelEntry *start = 
          new FindApproxLevelEntry(get_path(i), approx_path);
        start->_next = level;
        level = start;
      }
      get_path(0).find_matches(result, level, -1);
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::reparent_to
//       Access: Published
//  Description: Reparents all the NodePaths in the collection to the
//               indicated node.
////////////////////////////////////////////////////////////////////
void NodePathCollection::
reparent_to(const NodePath &other) {
  for (int i = 0; i < get_num_paths(); i++) {
    get_path(i).reparent_to(other);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::wrt_reparent_to
//       Access: Published
//  Description: Reparents all the NodePaths in the collection to the
//               indicated node, adjusting each transform so as not to
//               move in world coordinates.
////////////////////////////////////////////////////////////////////
void NodePathCollection::
wrt_reparent_to(const NodePath &other) {
  for (int i = 0; i < get_num_paths(); i++) {
    get_path(i).wrt_reparent_to(other);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::show
//       Access: Published
//  Description: Shows all NodePaths in the collection.
////////////////////////////////////////////////////////////////////
void NodePathCollection::
show() {
  for (int i = 0; i < get_num_paths(); i++) {
    get_path(i).show();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::show
//       Access: Published
//  Description: Hides all NodePaths in the collection.
////////////////////////////////////////////////////////////////////
void NodePathCollection::
hide() {
  for (int i = 0; i < get_num_paths(); i++) {
    get_path(i).hide();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::stash
//       Access: Published
//  Description: Stashes all NodePaths in the collection.
////////////////////////////////////////////////////////////////////
void NodePathCollection::
stash() {
  for (int i = 0; i < get_num_paths(); i++) {
    get_path(i).stash();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::unstash
//       Access: Published
//  Description: Unstashes all NodePaths in the collection.
////////////////////////////////////////////////////////////////////
void NodePathCollection::
unstash() {
  for (int i = 0; i < get_num_paths(); i++) {
    get_path(i).unstash();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::detach
//       Access: Published
//  Description: Detaches all NodePaths in the collection.
////////////////////////////////////////////////////////////////////
void NodePathCollection::
detach() {
  for (int i = 0; i < get_num_paths(); i++) {
    get_path(i).detach_node();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::set_color
//       Access: Published
//  Description: Colors all NodePaths in the collection
////////////////////////////////////////////////////////////////////
void NodePathCollection::
set_color(float r, float g, float b, float a, int priority) {
  for (int i = 0; i < get_num_paths(); i++) {
    get_path(i).set_color(Colorf(r, g, b, a), priority);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::set_color
//       Access: Published
//  Description: Colors all NodePaths in the collection
////////////////////////////////////////////////////////////////////
void NodePathCollection::
set_color(const Colorf &color, int priority) {
  for (int i = 0; i < get_num_paths(); i++) {
    get_path(i).node()->set_attrib(ColorAttrib::make_flat(color), priority);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::output
//       Access: Published
//  Description: Writes a brief one-line description of the
//               NodePathCollection to the indicated output stream.
////////////////////////////////////////////////////////////////////
void NodePathCollection::
output(ostream &out) const {
  if (get_num_paths() == 1) {
    out << "1 NodePath";
  } else {
    out << get_num_paths() << " NodePaths";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathCollection::write
//       Access: Published
//  Description: Writes a complete multi-line description of the
//               NodePathCollection to the indicated output stream.
////////////////////////////////////////////////////////////////////
void NodePathCollection::
write(ostream &out, int indent_level) const {
  for (int i = 0; i < get_num_paths(); i++) {
    indent(out, indent_level) << get_path(i) << "\n";
  }
}
