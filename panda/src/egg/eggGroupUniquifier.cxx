// Filename: eggGroupUniquifier.cxx
// Created by:  drose (22Feb01)
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

#include "eggGroupUniquifier.h"
#include "eggGroup.h"

#include "notify.h"

#include <ctype.h>

TypeHandle EggGroupUniquifier::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: EggGroupUniquifier::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
EggGroupUniquifier::
EggGroupUniquifier() {
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupUniquifier::get_category
//       Access: Public
//  Description: Returns the category name into which the given node
//               should be collected, or the empty string if the
//               node's name should be left alone.
////////////////////////////////////////////////////////////////////
string EggGroupUniquifier::
get_category(EggNode *node) {
  if (node->is_of_type(EggGroup::get_class_type()) && node->has_name()) {
    return "group";
  }

  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupUniquifier::filter_name
//       Access: Public, Virtual
//  Description: Returns the name of the given node, or at least the
//               name it should be.  This provides a hook to adjust
//               the name before attempting to uniquify it, if
//               desired, for instance to remove invalid characters.
////////////////////////////////////////////////////////////////////
string EggGroupUniquifier::
filter_name(EggNode *node) {
  string name = node->get_name();
  nassertr(!name.empty(), string());

  string result;

  // First, replace characters not A-Z, a-z, 0-9, or '_' with
  // underscore, and remove consecutive underscores.
  string::const_iterator pi;
  bool last_underscore = false;
  for (pi = name.begin(); pi != name.end(); ++pi) {
    if (isalnum(*pi)) {
      result += *pi;
      last_underscore = false;

    } else if (!last_underscore) {
      result += '_';
      last_underscore = true;
    }
  }

  // Next, ensure the name does not begin with a digit.
  nassertr(!result.empty(), string());
  if (isdigit(result[0])) {
    result = "_" + result;
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: EggGroupUniquifier::generate_name
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
string EggGroupUniquifier::
generate_name(EggNode *node, const string &category, int index) {
  ostringstream str;
  str << node->get_name() << "_group" << index;
  return str.str();
}
