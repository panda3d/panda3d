// Filename: clipPlaneAttrib.cxx
// Created by:  drose (11Jul02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "clipPlaneAttrib.h"
#include "pandaNode.h"
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "config_pgraph.h"
#include "attribNodeRegistry.h"
#include <iterator>

CPT(RenderAttrib) ClipPlaneAttrib::_empty_attrib;
CPT(RenderAttrib) ClipPlaneAttrib::_all_off_attrib;
TypeHandle ClipPlaneAttrib::_type_handle;
int ClipPlaneAttrib::_attrib_slot;

// This STL Function object is used in filter_to_max(), below, to sort
// a list of PlaneNodes in reverse order by priority.
class ComparePlaneNodePriorities {
public:
  bool operator ()(const NodePath &a, const NodePath &b) const {
    nassertr(!a.is_empty() && !b.is_empty(), a < b);
    PlaneNode *pa = DCAST(PlaneNode, a.node());
    PlaneNode *pb = DCAST(PlaneNode, b.node());
    nassertr(pa != (PlaneNode *)NULL && pb != (PlaneNode *)NULL, a < b);
             
    return pa->get_priority() > pb->get_priority();
  }
};

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::make
//       Access: Published, Static
//  Description: Constructs a new ClipPlaneAttrib object that enables (or
//               disables, according to op) the indicated plane(s).
//
//               This method is now deprecated.  Use add_on_plane() or
//               add_off_plane() instead.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
make(ClipPlaneAttrib::Operation op, PlaneNode *plane) {
  pgraph_cat.warning()
    << "Using deprecated ClipPlaneAttrib interface.\n";

  CPT(RenderAttrib) attrib;

  switch (op) {
  case O_set:
    attrib = make_all_off();
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane));
    return attrib;
   
  case O_add:
    attrib = make();
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane));
    return attrib;

  case O_remove:
    attrib = make();
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_off_plane(NodePath(plane));
    return attrib;
  }

  nassertr(false, make());
  return make();
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::make
//       Access: Published, Static
//  Description: Constructs a new ClipPlaneAttrib object that turns on (or
//               off, according to op) the indicate plane(s).
//
//               This method is now deprecated.  Use add_on_plane() or
//               add_off_plane() instead.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
make(ClipPlaneAttrib::Operation op, PlaneNode *plane1, PlaneNode *plane2) {
  pgraph_cat.warning()
    << "Using deprecated ClipPlaneAttrib interface.\n";

  CPT(RenderAttrib) attrib;

  switch (op) {
  case O_set:
    attrib = make_all_off();
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane1));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane2));
    return attrib;
   
  case O_add:
    attrib = make();
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane1));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane2));
    return attrib;

  case O_remove:
    attrib = make();
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_off_plane(NodePath(plane1));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_off_plane(NodePath(plane2));
    return attrib;
  }

  nassertr(false, make());
  return make();
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::make
//       Access: Published, Static
//  Description: Constructs a new ClipPlaneAttrib object that turns on (or
//               off, according to op) the indicate plane(s).
//
//               This method is now deprecated.  Use add_on_plane() or
//               add_off_plane() instead.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
make(ClipPlaneAttrib::Operation op, PlaneNode *plane1, PlaneNode *plane2,
     PlaneNode *plane3) {
  pgraph_cat.warning()
    << "Using deprecated ClipPlaneAttrib interface.\n";

  CPT(RenderAttrib) attrib;

  switch (op) {
  case O_set:
    attrib = make_all_off();
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane1));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane2));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane3));
    return attrib;
   
  case O_add:
    attrib = make();
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane1));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane2));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane3));
    return attrib;

  case O_remove:
    attrib = make();
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_off_plane(NodePath(plane1));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_off_plane(NodePath(plane2));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_off_plane(NodePath(plane3));
    return attrib;
  }

  nassertr(false, make());
  return make();
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::make
//       Access: Published, Static
//  Description: Constructs a new ClipPlaneAttrib object that turns on (or
//               off, according to op) the indicate plane(s).
//
//               This method is now deprecated.  Use add_on_plane() or
//               add_off_plane() instead.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
make(ClipPlaneAttrib::Operation op, PlaneNode *plane1, PlaneNode *plane2,
     PlaneNode *plane3, PlaneNode *plane4) {
  pgraph_cat.warning()
    << "Using deprecated ClipPlaneAttrib interface.\n";

  CPT(RenderAttrib) attrib;

  switch (op) {
  case O_set:
    attrib = make_all_off();
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane1));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane2));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane3));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane4));
    return attrib;
   
  case O_add:
    attrib = make();
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane1));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane2));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane3));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_on_plane(NodePath(plane4));
    return attrib;

  case O_remove:
    attrib = make();
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_off_plane(NodePath(plane1));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_off_plane(NodePath(plane2));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_off_plane(NodePath(plane3));
    attrib = DCAST(ClipPlaneAttrib, attrib)->add_off_plane(NodePath(plane4));
    return attrib;
  }

  nassertr(false, make());
  return make();
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::make_default
//       Access: Published, Static
//  Description: Returns a RenderAttrib that corresponds to whatever
//               the standard default properties for render attributes
//               of this type ought to be.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
make_default() {
  return return_new(new ClipPlaneAttrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::get_operation
//       Access: Published
//  Description: Returns the basic operation type of the ClipPlaneAttrib.
//               If this is O_set, the planes listed here completely
//               replace any planes that were already on.  If this is
//               O_add, the planes here are added to the set of of
//               planes that were already on, and if O_remove, the
//               planes here are removed from the set of planes that
//               were on.
//
//               This method is now deprecated.  ClipPlaneAttribs
//               nowadays have a separate list of on_planes and
//               off_planes, so this method doesn't make sense.  Query
//               the lists independently.
////////////////////////////////////////////////////////////////////
ClipPlaneAttrib::Operation ClipPlaneAttrib::
get_operation() const {
  pgraph_cat.warning()
    << "Using deprecated ClipPlaneAttrib interface.\n";

  if (has_all_off()) {
    return O_set;

  } else if (get_num_off_planes() == 0) {
    return O_add;

  } else {
    return O_remove;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::get_num_planes
//       Access: Published
//  Description: Returns the number of planes listed in the attribute.
//
//               This method is now deprecated.  ClipPlaneAttribs
//               nowadays have a separate list of on_planes and
//               off_planes, so this method doesn't make sense.  Query
//               the lists independently.
////////////////////////////////////////////////////////////////////
int ClipPlaneAttrib::
get_num_planes() const {
  pgraph_cat.warning()
    << "Using deprecated ClipPlaneAttrib interface.\n";

  if (get_num_off_planes() == 0) {
    return get_num_on_planes();
  } else {
    return get_num_off_planes();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::get_plane
//       Access: Published
//  Description: Returns the nth plane listed in the attribute.
//
//               This method is now deprecated.  ClipPlaneAttribs
//               nowadays have a separate list of on_planes and
//               off_planes, so this method doesn't make sense.  Query
//               the lists independently.
////////////////////////////////////////////////////////////////////
PlaneNode *ClipPlaneAttrib::
get_plane(int n) const {
  pgraph_cat.warning()
    << "Using deprecated ClipPlaneAttrib interface.\n";

  if (get_num_off_planes() == 0) {
    return DCAST(PlaneNode, get_on_plane(n).node());
  } else {
    return DCAST(PlaneNode, get_off_plane(n).node());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::has_plane
//       Access: Published
//  Description: Returns true if the indicated plane is listed in the
//               attrib, false otherwise.
//
//               This method is now deprecated.  ClipPlaneAttribs
//               nowadays have a separate list of on_planes and
//               off_planes, so this method doesn't make sense.  Query
//               the lists independently.
////////////////////////////////////////////////////////////////////
bool ClipPlaneAttrib::
has_plane(PlaneNode *plane) const {
  pgraph_cat.warning()
    << "Using deprecated ClipPlaneAttrib interface.\n";

  if (get_num_off_planes() == 0) {
    return has_on_plane(NodePath(plane));
  } else {
    return has_off_plane(NodePath(plane));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::add_plane
//       Access: Published
//  Description: Returns a new ClipPlaneAttrib, just like this one, but
//               with the indicated plane added to the list of planes.
//
//               This method is now deprecated.  Use add_on_plane() or
//               add_off_plane() instead.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
add_plane(PlaneNode *plane) const {
  pgraph_cat.warning()
    << "Using deprecated ClipPlaneAttrib interface.\n";

  if (get_num_off_planes() == 0) {
    return add_on_plane(NodePath(plane));
  } else {
    return add_off_plane(NodePath(plane));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::remove_plane
//       Access: Published
//  Description: Returns a new ClipPlaneAttrib, just like this one, but
//               with the indicated plane removed from the list of
//               planes.
//
//               This method is now deprecated.  Use remove_on_plane()
//               or remove_off_plane() instead.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
remove_plane(PlaneNode *plane) const {
  pgraph_cat.warning()
    << "Using deprecated ClipPlaneAttrib interface.\n";

  if (get_num_off_planes() == 0) {
    return remove_on_plane(NodePath(plane));
  } else {
    return remove_off_plane(NodePath(plane));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::make
//       Access: Published, Static
//  Description: Constructs a new ClipPlaneAttrib object that does
//               nothing.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
make() {
  // We make it a special case and store a pointer to the empty attrib
  // forever once we find it the first time, as an optimization.
  if (_empty_attrib == (RenderAttrib *)NULL) {
    _empty_attrib = return_new(new ClipPlaneAttrib);
  }

  return _empty_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::make_all_off
//       Access: Published, Static
//  Description: Constructs a new ClipPlaneAttrib object that disables 
//               all planes (and hence disables clipping).
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
make_all_off() {
  // We make it a special case and store a pointer to the off attrib
  // forever once we find it the first time, as an optimization.
  if (_all_off_attrib == (RenderAttrib *)NULL) {
    ClipPlaneAttrib *attrib = new ClipPlaneAttrib;
    attrib->_off_all_planes = true;
    _all_off_attrib = return_new(attrib);
  }

  return _all_off_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::add_on_plane
//       Access: Published
//  Description: Returns a new ClipPlaneAttrib, just like this one, but
//               with the indicated plane added to the list of planes
//               enabled by this attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
add_on_plane(const NodePath &plane) const {
  nassertr(!plane.is_empty() && plane.node()->is_of_type(PlaneNode::get_class_type()), this);
  ClipPlaneAttrib *attrib = new ClipPlaneAttrib(*this);
  attrib->_on_planes.insert(plane);
  attrib->_off_planes.erase(plane);

  pair<Planes::iterator, bool> insert_result = 
    attrib->_on_planes.insert(Planes::value_type(plane));
  if (insert_result.second) {
    // Also ensure it is removed from the off_planes list.
    attrib->_off_planes.erase(plane);
  }

  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::remove_on_plane
//       Access: Published
//  Description: Returns a new ClipPlaneAttrib, just like this one, but
//               with the indicated plane removed from the list of
//               planes enabled by this attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
remove_on_plane(const NodePath &plane) const {
  nassertr(!plane.is_empty() && plane.node()->is_of_type(PlaneNode::get_class_type()), this);
  ClipPlaneAttrib *attrib = new ClipPlaneAttrib(*this);
  attrib->_on_planes.erase(plane);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::add_off_plane
//       Access: Published
//  Description: Returns a new ClipPlaneAttrib, just like this one, but
//               with the indicated plane added to the list of planes
//               disabled by this attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
add_off_plane(const NodePath &plane) const {
  nassertr(!plane.is_empty() && plane.node()->is_of_type(PlaneNode::get_class_type()), this);
  ClipPlaneAttrib *attrib = new ClipPlaneAttrib(*this);
  if (!_off_all_planes) {
    attrib->_off_planes.insert(plane);
  }
  attrib->_on_planes.erase(plane);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::remove_off_plane
//       Access: Published
//  Description: Returns a new ClipPlaneAttrib, just like this one, but
//               with the indicated plane removed from the list of
//               planes disabled by this attrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
remove_off_plane(const NodePath &plane) const {
  nassertr(!plane.is_empty() && plane.node()->is_of_type(PlaneNode::get_class_type()), this);
  ClipPlaneAttrib *attrib = new ClipPlaneAttrib(*this);
  attrib->_off_planes.erase(plane);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::filter_to_max
//       Access: Public
//  Description: Returns a new ClipPlaneAttrib, very much like this one,
//               but with the number of on_planes reduced to be no
//               more than max_clip_planes.  The number of off_planes in
//               the new ClipPlaneAttrib is undefined.
////////////////////////////////////////////////////////////////////
CPT(ClipPlaneAttrib) ClipPlaneAttrib::
filter_to_max(int max_clip_planes) const {
  if (max_clip_planes < 0 || (int)_on_planes.size() <= max_clip_planes) {
    // Trivial case: this ClipPlaneAttrib qualifies.
    return this;
  }

  // Since check_filtered() will clear the _filtered list if we are out
  // of date, we should call it first.
  check_filtered();

  Filtered::const_iterator fi;
  fi = _filtered.find(max_clip_planes);
  if (fi != _filtered.end()) {
    // Easy case: we have already computed this for this particular
    // ClipPlaneAttrib.
    return (*fi).second;
  }

  // Harder case: we have to compute it now.  We must choose the n
  // planeNodes with the highest priority in our list of planeNodes.
  Planes priority_planes = _on_planes;

  // This sort function uses the STL function object defined above.
  sort(priority_planes.begin(), priority_planes.end(), 
       ComparePlaneNodePriorities());

  // Now lop off all of the planeNodes after the first max_clip_planes.
  priority_planes.erase(priority_planes.begin() + max_clip_planes,
                        priority_planes.end());

  // And re-sort the ov_set into its proper order.
  priority_planes.sort();

  // Now create a new attrib reflecting these planeNodes.
  PT(ClipPlaneAttrib) attrib = new ClipPlaneAttrib;
  attrib->_on_planes.swap(priority_planes);

  CPT(RenderAttrib) new_attrib = return_new(attrib);

  // Finally, record this newly-created attrib in the map for next
  // time.
  CPT(ClipPlaneAttrib) planeNode_attrib = (const ClipPlaneAttrib *)new_attrib.p();
  ((ClipPlaneAttrib *)this)->_filtered[max_clip_planes] = planeNode_attrib;
  return planeNode_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::compose_off
//       Access: Public
//  Description: This is a special method which composes two
//               ClipPlaneAttribs with regard only to their set of
//               "off" clip planes, for the purposes of deriving
//               PandaNode::get_off_clip_planes().
//
//               The result will be a ClipPlaneAttrib that represents
//               the union of all of the clip planes turned off in
//               either attrib.  The set of on planes in the result is
//               undefined and should be ignored.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
compose_off(const RenderAttrib *other) const {
  const ClipPlaneAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  if (_off_all_planes || (!ta->_off_all_planes && ta->_off_planes.empty())) {
    // If we turn off all planes, or the other turns none off, the
    // result is the same as this one.
    return this;
  }

  if (ta->_off_all_planes || _off_planes.empty()) {
    // And contrariwise.
    return ta;
  }

  Planes::const_iterator ai = _off_planes.begin();
  Planes::const_iterator bi = ta->_off_planes.begin();

  // Create a new ClipPlaneAttrib that will hold the result.
  ClipPlaneAttrib *new_attrib = new ClipPlaneAttrib;
  back_insert_iterator<Planes> result = 
    back_inserter(new_attrib->_on_planes);

  while (ai != _off_planes.end() && 
         bi != ta->_off_planes.end()) {
    if ((*ai) < (*bi)) {
      // Here is a plane that we have in the original, which is not
      // present in the secondary.
      *result = *ai;
      ++ai;
      ++result;

    } else if ((*bi) < (*ai)) {
      // Here is a new plane we have in the secondary, that was not
      // present in the original.
      *result = *bi;
      ++bi;
      ++result;

    } else {  // (*bi) == (*ai)
      // Here is a plane we have in both.
      *result = *bi;
      ++ai;
      ++bi;
      ++result;
    }
  }

  while (ai != _off_planes.end()) {
    *result = *ai;
    ++ai;
    ++result;
  }

  while (bi != ta->_off_planes.end()) {
    *result = *bi;
    ++bi;
    ++result;
  }

  return return_new(new_attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void ClipPlaneAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  if (_off_planes.empty()) {
    if (_on_planes.empty()) {
      if (_off_all_planes) {
        out << "all off";
      } else {
        out << "identity";
      }
    } else {
      if (_off_all_planes) {
        out << "set";
      } else {
        out << "on";
      }
    }

  } else {
    out << "off";
    Planes::const_iterator fi;
    for (fi = _off_planes.begin(); fi != _off_planes.end(); ++fi) {
      NodePath plane = (*fi);
      out << " " << plane;
    }

    if (!_on_planes.empty()) {
      out << " on";
    }
  }
    
  Planes::const_iterator li;
  for (li = _on_planes.begin(); li != _on_planes.end(); ++li) {
    NodePath plane = (*li);
    out << " " << plane;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived ClipPlaneAttrib
//               types to return a unique number indicating whether
//               this ClipPlaneAttrib is equivalent to the other one.
//
//               This should return 0 if the two ClipPlaneAttrib
//               objects are equivalent, a number less than zero if
//               this one should be sorted before the other one, and a
//               number greater than zero otherwise.
//
//               This will only be called with two ClipPlaneAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int ClipPlaneAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const ClipPlaneAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  if (_off_all_planes != ta->_off_all_planes) {
    return (int)_off_all_planes - (int)ta->_off_all_planes;
  }

  Planes::const_iterator li = _on_planes.begin();
  Planes::const_iterator oli = ta->_on_planes.begin();

  while (li != _on_planes.end() && oli != ta->_on_planes.end()) {
    NodePath plane = (*li);
    NodePath other_plane = (*oli);

    int compare = plane.compare_to(other_plane);
    if (compare != 0) {
      return compare;
    }

    ++li;
    ++oli;
  }

  if (li != _on_planes.end()) {
    return 1;
  }
  if (oli != ta->_on_planes.end()) {
    return -1;
  }

  Planes::const_iterator fi = _off_planes.begin();
  Planes::const_iterator ofi = ta->_off_planes.begin();

  while (fi != _off_planes.end() && ofi != ta->_off_planes.end()) {
    NodePath plane = (*fi);
    NodePath other_plane = (*ofi);

    int compare = plane.compare_to(other_plane);
    if (compare != 0) {
      return compare;
    }

    ++fi;
    ++ofi;
  }

  if (fi != _off_planes.end()) {
    return 1;
  }
  if (ofi != ta->_off_planes.end()) {
    return -1;
  }
  
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::get_hash_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to return a unique hash for these particular
//               properties.  RenderAttribs that compare the same with
//               compare_to_impl(), above, should return the same
//               hash; RenderAttribs that compare differently should
//               return a different hash.
////////////////////////////////////////////////////////////////////
size_t ClipPlaneAttrib::
get_hash_impl() const {
  size_t hash = 0;

  Planes::const_iterator li;
  for (li = _on_planes.begin(); li != _on_planes.end(); ++li) {
    NodePath plane = (*li);
    hash = plane.add_hash(hash);
  }

  // This bool value goes here, between the two lists, to
  // differentiate between the two.
  hash = int_hash::add_hash(hash, (int)_off_all_planes);

  for (li = _off_planes.begin(); li != _off_planes.end(); ++li) {
    NodePath plane = (*li);
    hash = plane.add_hash(hash);
  }

  return hash;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               This should return the result of applying the other
//               RenderAttrib to a node in the scene graph below this
//               RenderAttrib, which was already applied.  In most
//               cases, the result is the same as the other
//               RenderAttrib (that is, a subsequent RenderAttrib
//               completely replaces the preceding one).  On the other
//               hand, some kinds of RenderAttrib (for instance,
//               ColorTransformAttrib) might combine in meaningful
//               ways.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
compose_impl(const RenderAttrib *other) const {
  const ClipPlaneAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  if (ta->_off_all_planes) {
    // If the other type turns off all planes, it doesn't matter what
    // we are.
    return ta;
  }

  // This is a three-way merge between ai, bi, and ci, except that bi
  // and ci should have no intersection and therefore needn't be
  // compared to each other.
  Planes::const_iterator ai = _on_planes.begin();
  Planes::const_iterator bi = ta->_on_planes.begin();
  Planes::const_iterator ci = ta->_off_planes.begin();

  // Create a new ClipPlaneAttrib that will hold the result.
  ClipPlaneAttrib *new_attrib = new ClipPlaneAttrib;
  back_insert_iterator<Planes> result = 
    back_inserter(new_attrib->_on_planes);

  while (ai != _on_planes.end() && 
         bi != ta->_on_planes.end() && 
         ci != ta->_off_planes.end()) {
    if ((*ai) < (*bi)) {
      if ((*ai) < (*ci)) {
        // Here is a plane that we have in the original, which is not
        // present in the secondary.
        *result = *ai;
        ++ai;
        ++result;

      } else if ((*ci) < (*ai)) {
        // Here is a plane that is disabled in the secondary, but
        // was not present in the original.
        ++ci;

      } else { // (*ci) == (*ai)
        // Here is a plane that is disabled in the secondary, and
        // was present in the original.
        ++ai;
        ++ci;
      }

    } else if ((*bi) < (*ai)) {
      // Here is a new plane we have in the secondary, that was not
      // present in the original.
      *result = *bi;
      ++bi;
      ++result;

    } else {  // (*bi) == (*ai)
      // Here is a plane we have in both.
      *result = *bi;
      ++ai;
      ++bi;
      ++result;
    }
  }

  while (ai != _on_planes.end() && bi != ta->_on_planes.end()) {
    if ((*ai) < (*bi)) {
      // Here is a plane that we have in the original, which is not
      // present in the secondary.
      *result = *ai;
      ++ai;
      ++result;

    } else if ((*bi) < (*ai)) {
      // Here is a new plane we have in the secondary, that was not
      // present in the original.
      *result = *bi;
      ++bi;
      ++result;

    } else {
      // Here is a plane we have in both.
      *result = *bi;
      ++ai;
      ++bi;
      ++result;
    }
  }

  while (ai != _on_planes.end() && ci != ta->_off_planes.end()) {
    if ((*ai) < (*ci)) {
      // Here is a plane that we have in the original, which is not
      // present in the secondary.
      *result = *ai;
      ++ai;
      ++result;
      
    } else if ((*ci) < (*ai)) {
      // Here is a plane that is disabled in the secondary, but
      // was not present in the original.
      ++ci;
      
    } else { // (*ci) == (*ai)
      // Here is a plane that is disabled in the secondary, and
      // was present in the original.
      ++ai;
      ++ci;
    }
  }

  while (ai != _on_planes.end()) {
    *result = *ai;
    ++ai;
    ++result;
  }

  while (bi != ta->_on_planes.end()) {
    *result = *bi;
    ++bi;
    ++result;
  }

  return return_new(new_attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::invert_compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               See invert_compose() and compose_impl().
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  // I think in this case the other attrib always wins.  Maybe this
  // needs a bit more thought.  It's hard to imagine that it's even
  // important to compute this properly.
  return other;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::get_auto_shader_attrib_impl
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
get_auto_shader_attrib_impl(const RenderState *state) const {
  return this;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::sort_on_planes
//       Access: Private
//  Description: This is patterned after
//               TextureAttrib::sort_on_stages(), but since planeNodes
//               don't actually require sorting, this only empties the
//               _filtered map.
////////////////////////////////////////////////////////////////////
void ClipPlaneAttrib::
sort_on_planes() {
  _sort_seq = PlaneNode::get_sort_seq();
  _filtered.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               ClipPlaneAttrib.
////////////////////////////////////////////////////////////////////
void ClipPlaneAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void ClipPlaneAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_bool(_off_all_planes);

  // write the number of off_planes
  dg.add_uint16(get_num_off_planes());
  // write the off planes pointers if any
  Planes::const_iterator fi;
  for (fi = _off_planes.begin(); fi != _off_planes.end(); ++fi) {
    NodePath plane = (*fi);

    // Since we can't write out a NodePath, we write out just the
    // plain PandaNode.  The user can use the AttribNodeRegistry on
    // re-read if there is any ambiguity that needs to be resolved.
    manager->write_pointer(dg, plane.node());
  }

  // write the number of on planes
  dg.add_uint16(get_num_on_planes());
  // write the on planes pointers if any
  Planes::const_iterator nti;
  for (nti = _on_planes.begin(); nti != _on_planes.end(); ++nti) {
    NodePath plane = (*nti);
    manager->write_pointer(dg, plane.node());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int ClipPlaneAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);
  AttribNodeRegistry *areg = AttribNodeRegistry::get_global_ptr();

  Planes::iterator ci = _off_planes.begin();
  while (ci != _off_planes.end()) {
    PandaNode *node;
    DCAST_INTO_R(node, p_list[pi++], pi);
    
    // We go through some effort to look up the node in the registry
    // without creating a NodePath around it first (which would up,
    // and then down, the reference count, possibly deleting the
    // node).
    int ni = areg->find_node(node->get_type(), node->get_name());
    if (ni != -1) {
      (*ci) = areg->get_node(ni);
    } else {
      (*ci) = NodePath(node);
    }
    ++ci;
  }
  _off_planes.sort();

  ci = _on_planes.begin();
  while (ci != _on_planes.end()) {
    PandaNode *node;
    DCAST_INTO_R(node, p_list[pi++], pi);

    int ni = areg->find_node(node->get_type(), node->get_name());
    if (ni != -1) {
      (*ci) = areg->get_node(ni);
    } else {
      (*ci) = NodePath(node);
    }
    ++ci;
  }
  _on_planes.sort();

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::require_fully_complete
//       Access: Public, Virtual
//  Description: Some objects require all of their nested pointers to
//               have been completed before the objects themselves can
//               be completed.  If this is the case, override this
//               method to return true, and be careful with circular
//               references (which would make the object unreadable
//               from a bam file).
////////////////////////////////////////////////////////////////////
bool ClipPlaneAttrib::
require_fully_complete() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type ClipPlaneAttrib is encountered
//               in the Bam file.  It should create the ClipPlaneAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *ClipPlaneAttrib::
make_from_bam(const FactoryParams &params) {
  ClipPlaneAttrib *attrib = new ClipPlaneAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new ClipPlaneAttrib.
////////////////////////////////////////////////////////////////////
void ClipPlaneAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  // We cheat a little bit here.  In the middle of bam version 4.10,
  // we completely redefined the bam storage definition for
  // ClipPlaneAttribs, without bothering to up the bam version or even to
  // attempt to read the old definition.  We get away with this,
  // knowing that the egg loader doesn't create ClipPlaneAttribs, and
  // hence no old bam files have the old definition for ClipPlaneAttrib
  // within them.

  _off_all_planes = scan.get_bool();

  int num_off_planes = scan.get_uint16();
    
  // Push back an empty NodePath for each off Plane for now, until we
  // get the actual list of pointers later in complete_pointers().
  _off_planes.reserve(num_off_planes);
  int i;
  for (i = 0; i < num_off_planes; i++) {
    manager->read_pointer(scan);
    _off_planes.push_back(NodePath());
  }
    
  int num_on_planes = scan.get_uint16();
  _on_planes.reserve(num_on_planes);
  for (i = 0; i < num_on_planes; i++) {
    manager->read_pointer(scan);
    _on_planes.push_back(NodePath());
  }
}
