// Filename: qpnodePathCollection.cxx
// Created by:  drose (06Mar02)
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

#include "qpnodePathCollection.h"
//#include "findApproxPath.h"
//#include "findApproxLevel.h"

#include "indent.h"

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
qpNodePathCollection::
qpNodePathCollection() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
qpNodePathCollection::
qpNodePathCollection(const qpNodePathCollection &copy) :
  _node_paths(copy._node_paths)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void qpNodePathCollection::
operator = (const qpNodePathCollection &copy) {
  _node_paths = copy._node_paths;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::add_path
//       Access: Published
//  Description: Adds a new qpNodePath to the collection.
////////////////////////////////////////////////////////////////////
void qpNodePathCollection::
add_path(const qpNodePath &node_path) {
  // If the pointer to our internal array is shared by any other
  // NodePathCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren NodePathCollection
  // objects.

  if (_node_paths.get_ref_count() > 1) {
    PTA(qpNodePath) old_node_paths = _node_paths;
    _node_paths = PTA(qpNodePath)::empty_array(0);
    _node_paths.v() = old_node_paths.v();
  }

  _node_paths.push_back(node_path);
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::remove_path
//       Access: Published
//  Description: Removes the indicated qpNodePath from the collection.
//               Returns true if the path was removed, false if it was
//               not a member of the collection.
////////////////////////////////////////////////////////////////////
bool qpNodePathCollection::
remove_path(const qpNodePath &node_path) {
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
    PTA(qpNodePath) old_node_paths = _node_paths;
    _node_paths = PTA(qpNodePath)::empty_array(0);
    _node_paths.v() = old_node_paths.v();
  }

  _node_paths.erase(_node_paths.begin() + path_index);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::add_paths_from
//       Access: Published
//  Description: Adds all the qpNodePaths indicated in the other
//               collection to this path.  The other paths are simply
//               appended to the end of the paths in this list;
//               duplicates are not automatically removed.
////////////////////////////////////////////////////////////////////
void qpNodePathCollection::
add_paths_from(const qpNodePathCollection &other) {
  int other_num_paths = other.get_num_paths();
  for (int i = 0; i < other_num_paths; i++) {
    add_path(other.get_path(i));
  }
}


////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::remove_paths_from
//       Access: Published
//  Description: Removes from this collection all of the qpNodePaths
//               listed in the other collection.
////////////////////////////////////////////////////////////////////
void qpNodePathCollection::
remove_paths_from(const qpNodePathCollection &other) {
  NodePaths new_paths;
  int num_paths = get_num_paths();
  for (int i = 0; i < num_paths; i++) {
    qpNodePath path = get_path(i);
    if (!other.has_path(path)) {
      new_paths.push_back(path);
    }
  }
  _node_paths = new_paths;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::remove_duplicate_paths
//       Access: Published
//  Description: Removes any duplicate entries of the same NodePaths
//               on this collection.  If a qpNodePath appears multiple
//               times, the first appearance is retained; subsequent
//               appearances are removed.
////////////////////////////////////////////////////////////////////
void qpNodePathCollection::
remove_duplicate_paths() {
  NodePaths new_paths;

  int num_paths = get_num_paths();
  for (int i = 0; i < num_paths; i++) {
    qpNodePath path = get_path(i);
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
//     Function: qpNodePathCollection::has_path
//       Access: Published
//  Description: Returns true if the indicated qpNodePath appears in
//               this collection, false otherwise.
////////////////////////////////////////////////////////////////////
bool qpNodePathCollection::
has_path(const qpNodePath &path) const {
  for (int i = 0; i < get_num_paths(); i++) {
    if (path == get_path(i)) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::clear
//       Access: Published
//  Description: Removes all NodePaths from the collection.
////////////////////////////////////////////////////////////////////
void qpNodePathCollection::
clear() {
  _node_paths.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::is_empty
//       Access: Published
//  Description: Returns true if there are no NodePaths in the
//               collection, false otherwise.
////////////////////////////////////////////////////////////////////
bool qpNodePathCollection::
is_empty() const {
  return _node_paths.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::get_num_paths
//       Access: Published
//  Description: Returns the number of NodePaths in the collection.
////////////////////////////////////////////////////////////////////
int qpNodePathCollection::
get_num_paths() const {
  return _node_paths.size();
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::get_path
//       Access: Published
//  Description: Returns the nth qpNodePath in the collection.
////////////////////////////////////////////////////////////////////
qpNodePath qpNodePathCollection::
get_path(int index) const {
  nassertr(index >= 0 && index < (int)_node_paths.size(), qpNodePath());

  return _node_paths[index];
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::operator []
//       Access: Published
//  Description: Returns the nth qpNodePath in the collection.  This is
//               the same as get_path(), but it may be a more
//               convenient way to access it.
////////////////////////////////////////////////////////////////////
qpNodePath qpNodePathCollection::
operator [] (int index) const {
  nassertr(index >= 0 && index < (int)_node_paths.size(), qpNodePath());

  return _node_paths[index];
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::ls
//       Access: Published
//  Description: Lists all the nodes at and below each node in the
//               collection hierarchically.
////////////////////////////////////////////////////////////////////
void qpNodePathCollection::
ls(ostream &out, int indent_level) const {
  for (int i = 0; i < get_num_paths(); i++) {
    qpNodePath path = get_path(i);
    indent(out, indent_level) << path << "\n";
    path.ls(out, indent_level + 2);
    out << "\n";
  }
}

/*
////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::find_all_matches
//       Access: Published
//  Description: Returns the complete set of all NodePaths that begin
//               with any qpNodePath in this collection and can be
//               extended by path.  The shortest paths will be listed
//               first.
////////////////////////////////////////////////////////////////////
qpNodePathCollection qpNodePathCollection::
find_all_matches(const string &path) const {
  qpNodePathCollection result;

  FindApproxPath approx_path;
  if (approx_path.add_string(path)) {
    if (!is_empty()) {
      FindApproxLevel level;
      for (int i = 0; i < get_num_paths(); i++) {
        FindApproxLevelEntry start(get_path(i), approx_path);
        level.add_entry(start);
      }
      get_path(0).r_find_matches(result, level, -1,
                                 qpNodePath::get_max_search_depth());
    }
  }

  return result;
}
*/

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::reparent_to
//       Access: Published
//  Description: Reparents all the NodePaths in the collection to the
//               indicated node.
////////////////////////////////////////////////////////////////////
void qpNodePathCollection::
reparent_to(const qpNodePath &other) {
  for (int i = 0; i < get_num_paths(); i++) {
    get_path(i).reparent_to(other);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::wrt_reparent_to
//       Access: Published
//  Description: Reparents all the NodePaths in the collection to the
//               indicated node, adjusting each transform so as not to
//               move in world coordinates.
////////////////////////////////////////////////////////////////////
void qpNodePathCollection::
wrt_reparent_to(const qpNodePath &other) {
  for (int i = 0; i < get_num_paths(); i++) {
    get_path(i).wrt_reparent_to(other);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::show
//       Access: Published
//  Description: Shows all NodePaths in the collection.
////////////////////////////////////////////////////////////////////
void qpNodePathCollection::
show() {
  for (int i = 0; i < get_num_paths(); i++) {
    get_path(i).show();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::show
//       Access: Published
//  Description: Hides all NodePaths in the collection.
////////////////////////////////////////////////////////////////////
void qpNodePathCollection::
hide() {
  for (int i = 0; i < get_num_paths(); i++) {
    get_path(i).hide();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::stash
//       Access: Published
//  Description: Stashes all NodePaths in the collection.
////////////////////////////////////////////////////////////////////
void qpNodePathCollection::
stash() {
  for (int i = 0; i < get_num_paths(); i++) {
    get_path(i).stash();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::unstash
//       Access: Published
//  Description: Unstashes all NodePaths in the collection.
////////////////////////////////////////////////////////////////////
void qpNodePathCollection::
unstash() {
  for (int i = 0; i < get_num_paths(); i++) {
    get_path(i).unstash();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::output
//       Access: Published
//  Description: Writes a brief one-line description of the
//               qpNodePathCollection to the indicated output stream.
////////////////////////////////////////////////////////////////////
void qpNodePathCollection::
output(ostream &out) const {
  if (get_num_paths() == 1) {
    out << "1 NodePath";
  } else {
    out << get_num_paths() << " NodePaths";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpNodePathCollection::write
//       Access: Published
//  Description: Writes a complete multi-line description of the
//               qpNodePathCollection to the indicated output stream.
////////////////////////////////////////////////////////////////////
void qpNodePathCollection::
write(ostream &out, int indent_level) const {
  for (int i = 0; i < get_num_paths(); i++) {
    indent(out, indent_level) << get_path(i) << "\n";
  }
}
