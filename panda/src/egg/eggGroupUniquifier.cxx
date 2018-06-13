/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggGroupUniquifier.cxx
 * @author drose
 * @date 2001-02-22
 */

#include "eggGroupUniquifier.h"
#include "eggGroup.h"

#include "pnotify.h"

#include <ctype.h>

using std::string;

TypeHandle EggGroupUniquifier::_type_handle;


/**
 * If filter_names is true, then the group names will be coerced into a fairly
 * safe, standard convention that uses no characters other than a-z, A-Z, 0-9,
 * and underscore.  If filter_names is false, the group names will be left
 * unchanged.
 */
EggGroupUniquifier::
EggGroupUniquifier(bool filter_names)
  : _filter_names(filter_names)
{
}

/**
 * Returns the category name into which the given node should be collected, or
 * the empty string if the node's name should be left alone.
 */
string EggGroupUniquifier::
get_category(EggNode *node) {
  if (node->is_of_type(EggGroup::get_class_type()) && node->has_name()) {
    return "group";
  }

  return string();
}

/**
 * Returns the name of the given node, or at least the name it should be.
 * This provides a hook to adjust the name before attempting to uniquify it,
 * if desired, for instance to remove invalid characters.
 */
string EggGroupUniquifier::
filter_name(EggNode *node) {
  string name = node->get_name();
  if (!_filter_names) {
    return name;
  }
  nassertr(!name.empty(), string());

  string result;

  // First, replace characters not A-Z, a-z, 0-9, or '_' with underscore, and
  // remove consecutive underscores.
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

/**
 * Generates a new name for the given node when its existing name clashes with
 * some other node.  This function will be called repeatedly, if necessary,
 * until it returns a name that actually is unique.
 *
 * The category is the string returned by get_category(), and index is a
 * uniquely-generated number that may be useful for synthesizing the name.
 */
string EggGroupUniquifier::
generate_name(EggNode *node, const string &category, int index) {
  std::ostringstream str;
  str << node->get_name() << "_group" << index;
  return str.str();
}
