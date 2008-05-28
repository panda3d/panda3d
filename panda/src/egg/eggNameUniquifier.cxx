// Filename: eggNameUniquifier.cxx
// Created by:  drose (09Nov00)
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

#include "eggNameUniquifier.h"
#include "eggNode.h"
#include "eggGroupNode.h"
#include "config_egg.h"
#include "dcast.h"

#include "pnotify.h"

TypeHandle EggNameUniquifier::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggNameUniquifier::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggNameUniquifier::
EggNameUniquifier() {
  _index = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggNameUniquifier::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggNameUniquifier::
~EggNameUniquifier() {
}

////////////////////////////////////////////////////////////////////
//     Function: EggNameUniquifier::clear
//       Access: Public
//  Description: Empties the table of used named and prepares the
//               Uniquifier for a new tree.
////////////////////////////////////////////////////////////////////
void EggNameUniquifier::
clear() {
  _categories.clear();
  _index = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggNameUniquifier::uniquify
//       Access: Public
//  Description: Begins the traversal from the indicated node.
////////////////////////////////////////////////////////////////////
void EggNameUniquifier::
uniquify(EggNode *node) {
  string category = get_category(node);
  if (egg_cat.is_debug()) {
    egg_cat.debug()
      << "Uniquifying " << node->get_name() << ", category = " << category
      << "\n";
  }

  if (!category.empty()) {
    string name = filter_name(node);

    UsedNames &names = _categories[category];
    bool inserted = false;
    if (!name.empty()) {
      inserted = names.insert(UsedNames::value_type(name, node)).second;
    }

    while (!inserted) {
      _index++;
      name = generate_name(node, category, _index);
      inserted = names.insert(UsedNames::value_type(name, node)).second;
    }

    if (egg_cat.is_debug()) {
      egg_cat.debug()
        << "Uniquifying " << node->get_name() << " to "
        << name << "\n";
    }

    node->set_name(name);
  }

  if (node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *group;
    DCAST_INTO_V(group, node);

    EggGroupNode::iterator ci;
    for (ci = group->begin(); ci != group->end(); ++ci) {
      EggNode *child = (*ci);
      nassertv(child != (EggNode *)NULL);
      uniquify(child);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: EggNameUniquifier::get_node
//       Access: Public
//  Description: Returns the node associated with the given category
//               and name, or NULL if the name has not been used.
////////////////////////////////////////////////////////////////////
EggNode *EggNameUniquifier::
get_node(const string &category, const string &name) const {
  Categories::const_iterator ci;
  ci = _categories.find(category);
  if (ci == _categories.end()) {
    return (EggNode *)NULL;
  }

  const UsedNames &names = (*ci).second;
  UsedNames::const_iterator ni;
  ni = names.find(name);
  if (ni == names.end()) {
    return (EggNode *)NULL;
  }

  return (*ni).second;
}

////////////////////////////////////////////////////////////////////
//     Function: EggNameUniquifier::has_name
//       Access: Public
//  Description: Returns true if the name has been used for the
//               indicated category already, false otherwise.
////////////////////////////////////////////////////////////////////
bool EggNameUniquifier::
has_name(const string &category, const string &name) const {
  Categories::const_iterator ci;
  ci = _categories.find(category);
  if (ci == _categories.end()) {
    return false;
  }

  const UsedNames &names = (*ci).second;
  UsedNames::const_iterator ni;
  ni = names.find(name);
  if (ni == names.end()) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggNameUniquifier::add_name
//       Access: Public
//  Description: Adds the name to the indicated category.  This name
//               will not be used for any other egg node within this
//               category.  Returns true if the name was added, or
//               false if it was already in use for the category.
////////////////////////////////////////////////////////////////////
bool EggNameUniquifier::
add_name(const string &category, const string &name, EggNode *node) {
  UsedNames &names = _categories[category];
  bool inserted = names.insert(UsedNames::value_type(name, node)).second;
  return inserted;
}

////////////////////////////////////////////////////////////////////
//     Function: EggNameUniquifier::filter_name
//       Access: Public, Virtual
//  Description: Returns the name of the given node, or at least the
//               name it should be.  This provides a hook to adjust
//               the name before attempting to uniquify it, if
//               desired, for instance to remove invalid characters.
////////////////////////////////////////////////////////////////////
string EggNameUniquifier::
filter_name(EggNode *node) {
  return node->get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: EggNameUniquifier::generate_name
//       Access: Public, Virtual
//  Description: Generates a new name for the given node when its
//               existing name clashes with some other node.  This
//               function will be called repeatedly, if necessary,
//               until it returns a name that actually is unique.
//
//               The category is the string returned by
//               get_category(), and index is a uniquely-generated
//               number that may be useful for synthesizing the name.
////////////////////////////////////////////////////////////////////
string EggNameUniquifier::
generate_name(EggNode *node, const string &category, int index) {
  string name = filter_name(node);

  ostringstream str;
  if (name.empty()) {
    str << category << index;
  } else {
    str << name << "." << category << index;
  }
  return str.str();
}
