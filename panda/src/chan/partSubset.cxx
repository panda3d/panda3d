/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file partSubset.cxx
 * @author drose
 * @date 2006-01-19
 */

#include "partSubset.h"

/**
 *
 */
PartSubset::
PartSubset() {
}

/**
 *
 */
PartSubset::
PartSubset(const PartSubset &copy) :
  _include_joints(copy._include_joints),
  _exclude_joints(copy._exclude_joints)
{
}

/**
 *
 */
void PartSubset::
operator = (const PartSubset &copy) {
  _include_joints = copy._include_joints;
  _exclude_joints = copy._exclude_joints;
}

/**
 * Adds the named joint to the list of joints that will be explicitly included
 * in the subset.  Any joint at or below a named node will be included in the
 * subset (unless a lower node is also listed in the exclude list).
 *
 * Since the name is a GlobPattern, it may of course include filename globbing
 * characters like * and ?.
 */
void PartSubset::
add_include_joint(const GlobPattern &name) {
  _include_joints.push_back(name);
}

/**
 * Adds the named joint to the list of joints that will be explicitly
 * exlcluded from the subset.  Any joint at or below a named node will not be
 * included in the subset (unless a lower node is also listed in the include
 * list).
 *
 * Since the name is a GlobPattern, it may of course include filename globbing
 * characters like * and ?.
 */
void PartSubset::
add_exclude_joint(const GlobPattern &name) {
  _exclude_joints.push_back(name);
}

/**
 * Appends the include and exclude list from the other object onto this
 * object's lists.
 */
void PartSubset::
append(const PartSubset &other) {
  Joints::const_iterator ji;
  for (ji = other._include_joints.begin();
       ji != other._include_joints.end();
       ++ji) {
    _include_joints.push_back(*ji);
  }
  for (ji = other._exclude_joints.begin();
       ji != other._exclude_joints.end();
       ++ji) {
    _exclude_joints.push_back(*ji);
  }
}

/**
 *
 */
void PartSubset::
output(std::ostream &out) const {
  if (_include_joints.empty() && _exclude_joints.empty()) {
    out << "PartSubset, empty";
  } else {
    out << "PartSubset, include: [";
    Joints::const_iterator ji;
    for (ji = _include_joints.begin(); ji != _include_joints.end(); ++ji) {
      out << " " << (*ji);
    }
    out << " ], exclude: [";
    for (ji = _exclude_joints.begin(); ji != _exclude_joints.end(); ++ji) {
      out << " " << (*ji);
    }
    out << " ]";
  }
}

/**
 * Returns true if the include list is completely empty, false otherwise.  If
 * it is empty, it is the same thing as including all joints.
 */
bool PartSubset::
is_include_empty() const {
  return _include_joints.empty();
}

/**
 * Returns true if the indicated name matches a name on the include list,
 * false otherwise.
 */
bool PartSubset::
matches_include(const std::string &joint_name) const {
  Joints::const_iterator ji;
  for (ji = _include_joints.begin(); ji != _include_joints.end(); ++ji) {
    if ((*ji).matches(joint_name)) {
      return true;
    }
  }

  return false;
}


/**
 * Returns true if the indicated name matches a name on the exclude list,
 * false otherwise.
 */
bool PartSubset::
matches_exclude(const std::string &joint_name) const {
  Joints::const_iterator ji;
  for (ji = _exclude_joints.begin(); ji != _exclude_joints.end(); ++ji) {
    if ((*ji).matches(joint_name)) {
      return true;
    }
  }

  return false;
}
