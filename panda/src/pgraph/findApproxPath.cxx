/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file findApproxPath.cxx
 * @author drose
 * @date 2002-03-13
 */

#include "findApproxPath.h"
#include "config_pgraph.h"

#include "string_utils.h"
#include "pandaNode.h"

using std::ostream;
using std::string;


/**
 * Returns true if the indicated node matches this component, false otherwise.
 */
bool FindApproxPath::Component::
matches(PandaNode *node) const {
  string node_name;

  switch (_type) {
  case CT_match_name:
    // Match the node's name exactly.
    return (_name == node->get_name());

  case CT_match_name_insensitive:
    // Match the node's name exactly, with case-insensitive comparison.
    return cmp_nocase(_name, node->get_name()) == 0;

  case CT_match_name_glob:
    // Match the node's name according to filename globbing rules.
    return (_glob.matches(node->get_name()));

  case CT_match_exact_type:
    // Match the node's type exactly.
    return (node->is_exact_type(_type_handle));

  case CT_match_inexact_type:
    // Match the node's type inexactly: it's a match if the node is the type,
    // or is derived from the type.
    return (node->is_of_type(_type_handle));

  case CT_match_tag:
    // Match the node's tag only.
    return (node->has_tag(_name));

  case CT_match_tag_value:
    // Match the node's tag and value.
    if (node->has_tag(_name)) {
      return _glob.matches(node->get_tag(_name));
    }
    return false;

  case CT_match_one:
  case CT_match_many:
    // Match any node.
    return true;

  case CT_match_pointer:
    // Match only this one particular node.
    return (_pointer == node);
  }

  pgraph_cat.error()
    << "Invalid component in FindApproxPath\n";
  return false;
}

/**
 *
 */
void FindApproxPath::Component::
output(ostream &out) const {
  out << _type;
  switch (_type) {
  case CT_match_name:
  case CT_match_name_insensitive:
  case CT_match_name_glob:
  case CT_match_tag:
    out << " \"" << _name << "\"";
    break;

  case CT_match_tag_value:
    out << " \"" << _name << "\"=\"" << _glob << "\"";
    break;

  case CT_match_exact_type:
  case CT_match_inexact_type:
    out << " " << _type_handle;
    break;

  case CT_match_pointer:
    out << " (" << *_pointer << ")";
    break;

  default:
    break;
  }
}

/**
 * Adds a sequence of components separated by slashes, followed optionally by
 * a semicolon and a sequence of control flags, to the path sequence.  Returns
 * true if successful, false if the string contained an error.
 */
bool FindApproxPath::
add_string(const string &str_path) {
  // First, chop the string up by slashes into its components.
  vector_string components;

  size_t start = 0;
  size_t slash = str_path.find('/');
  while (slash != string::npos) {
    components.push_back(str_path.substr(start, slash - start));
    start = slash + 1;
    slash = str_path.find('/', start);
  }

  size_t semicolon = str_path.rfind(';');

  // We want to find the *last* semicolon at start or later, if there happens
  // to be more than one.  rfind will find the rightmost semicolon in the
  // entire string; if this is less than start, there is no semicolon right of
  // start.
  if (semicolon < start) {
    semicolon = string::npos;
  }

  components.push_back(str_path.substr(start, semicolon - start));

  if (semicolon != string::npos) {
    if (!add_flags(str_path.substr(semicolon + 1))) {
      return false;
    }
  }

  // Now decode each component and add it to the path.
  vector_string::const_iterator ci;
  for (ci = components.begin(); ci != components.end(); ++ci) {
    if (!add_component(*ci)) {
      return false;
    }
  }

  return true;
}

/**
 * Adds a sequence of control flags.  This will be a sequence of letters
 * preceded by either '+' or '-', with no intervening punctuation.  Returns
 * true if successful, false otherwise.
 */
bool FindApproxPath::
add_flags(const string &str_flags) {
  string::const_iterator pi = str_flags.begin();
  while (pi != str_flags.end()) {
    bool on;
    switch (*pi) {
    case '+':
      on = true;
      break;
    case '-':
      on = false;
      break;
    default:
      pgraph_cat.error()
        << "Invalid control flag string: " << str_flags << "\n";
      return false;
    }

    ++pi;
    if (pi == str_flags.end()) {
      pgraph_cat.error()
        << "Invalid control flag string: " << str_flags << "\n";
      return false;
    }

    switch (*pi) {
    case 'h':
      _return_hidden = on;
      break;

    case 's':
      _return_stashed = on;
      break;

    case 'i':
      _case_insensitive = on;
      break;

    default:
      pgraph_cat.error()
        << "Invalid control flag string: " << str_flags << "\n";
      return false;
    }

    ++pi;
  }

  return true;
}


/**
 * Adds a single component to the path sequence, defined by a string as might
 * appear between slashes in the path string.  Returns true if successful,
 * false if the string component was in some way invalid.
 */
bool FindApproxPath::
add_component(string str_component) {
  int flags = 0;
  if (str_component.size() >= 2 && str_component.substr(0, 2) == "@@") {
    flags |= CF_stashed;
    str_component = str_component.substr(2);
  }

  if (str_component == "*") {
    add_match_one(flags);

  } else if (str_component == "**") {
    if ((flags & CF_stashed) != 0) {
      pgraph_cat.error()
        << "@@** is undefined; use @@*/** or **/@@* instead.\n";
      return false;
    }
    add_match_many(flags);

  } else if (!str_component.empty() && str_component[0] == '-') {
    string type_name = str_component.substr(1);
    TypeHandle handle = TypeRegistry::ptr()->find_type(type_name);

    if (handle == TypeHandle::none()) {
      pgraph_cat.error()
        << "Invalid type name: " << type_name << "\n";
      return false;

    } else {
      add_match_exact_type(handle, flags);
    }

  } else if (!str_component.empty() && str_component[0] == '+') {
    string type_name = str_component.substr(1);
    TypeHandle handle = TypeRegistry::ptr()->find_type(type_name);

    if (handle == TypeHandle::none()) {
      pgraph_cat.error()
        << "Invalid type name: " << type_name << "\n";
      return false;

    } else {
      add_match_inexact_type(handle, flags);
    }

  } else if (!str_component.empty() && str_component[0] == '=') {
    size_t equals = str_component.find('=', 1);
    if (equals != string::npos) {
      // =key=value
      string tag_key = str_component.substr(1, equals - 1);
      string tag_value = str_component.substr(equals + 1);
      add_match_tag_value(tag_key, tag_value, flags);
    } else {
      // =key
      string tag_key = str_component.substr(1);
      add_match_tag(tag_key, flags);
    }

  } else {
    add_match_name_glob(str_component, flags);
  }

  return true;
}

/**
 * Adds a component that must match the name of a node exactly.
 */
void FindApproxPath::
add_match_name(const string &name, int flags) {
  Component comp;
  comp._type = _case_insensitive ? CT_match_name_insensitive : CT_match_name;
  comp._name = name;
  comp._flags = flags;
  _path.push_back(comp);
}

/**
 * Adds a component that must match the name of a node using standard shell
 * globbing rules, with wildcard characters accepted.
 */
void FindApproxPath::
add_match_name_glob(const string &name, int flags) {
  Component comp;
  comp._type = CT_match_name_glob;
  comp._name = name;
  comp._glob.set_pattern(name);
  comp._glob.set_case_sensitive(!_case_insensitive);
  comp._flags = flags;
  if (!comp._glob.has_glob_characters()) {
    // The glob pattern contains no special characters; make it a literal
    // match for efficiency.
    add_match_name(name, flags);
  } else {
    _path.push_back(comp);
  }
}

/**
 * Adds a component that must match the type of a node exactly, with no
 * derived types matching.
 */
void FindApproxPath::
add_match_exact_type(TypeHandle type, int flags) {
  Component comp;
  comp._type = CT_match_exact_type;
  comp._type_handle = type;
  comp._flags = flags;
  _path.push_back(comp);
}

/**
 * Adds a component that must match the type of a node or be a base class of
 * the node's type.
 */
void FindApproxPath::
add_match_inexact_type(TypeHandle type, int flags) {
  Component comp;
  comp._type = CT_match_inexact_type;
  comp._type_handle = type;
  comp._flags = flags;
  _path.push_back(comp);
}

/**
 * Adds a component that will match a node that has a tag with the indicated
 * key, no matter what the value is.
 */
void FindApproxPath::
add_match_tag(const string &name, int flags) {
  Component comp;
  comp._type = CT_match_tag;
  comp._name = name;
  comp._flags = flags;
  _path.push_back(comp);
}

/**
 * Adds a component that will match a node that has a tag with the indicated
 * key.  The value may be "*" to match any value, or a particular glob pattern
 * to match only those nodes with the indicated value.
 */
void FindApproxPath::
add_match_tag_value(const string &name, const string &value, int flags) {
  Component comp;
  comp._type = CT_match_tag_value;
  comp._name = name;
  comp._glob.set_pattern(value);
  comp._flags = flags;
  _path.push_back(comp);
}

/**
 * Adds a component that will match any node (but not a chain of many nodes).
 */
void FindApproxPath::
add_match_one(int flags) {
  Component comp;
  comp._type = CT_match_one;
  comp._flags = flags;
  _path.push_back(comp);
}

/**
 * Adds a component that will match a chain of zero or more consecutive nodes.
 */
void FindApproxPath::
add_match_many(int flags) {
  Component comp;
  comp._type = CT_match_many;
  comp._flags = flags;
  _path.push_back(comp);
}

/**
 * Adds a component that must match a particular node exactly, by pointer.
 */
void FindApproxPath::
add_match_pointer(PandaNode *pointer, int flags) {
  Component comp;
  comp._type = CT_match_pointer;
  comp._pointer = pointer;
  comp._flags = flags;
  _path.push_back(comp);
}

/**
 *
 */
void FindApproxPath::
output(ostream &out) const {
  out << "(";
  if (!_path.empty()) {
    Path::const_iterator pi = _path.begin();
    out << *pi;
    ++pi;
    while (pi != _path.end()) {
      out << " / " << *pi;
      ++pi;
    }
  }
  out << ")";
}

ostream &
operator << (ostream &out, FindApproxPath::ComponentType type) {
  switch (type) {
  case FindApproxPath::CT_match_name:
    return out << "match_name";

  case FindApproxPath::CT_match_name_insensitive:
    return out << "match_name_insensitive";

  case FindApproxPath::CT_match_name_glob:
    return out << "match_name_glob";

  case FindApproxPath::CT_match_exact_type:
    return out << "match_exact_type";

  case FindApproxPath::CT_match_inexact_type:
    return out << "match_inexact_type";

  case FindApproxPath::CT_match_tag:
    return out << "match_tag";

  case FindApproxPath::CT_match_tag_value:
    return out << "match_tag_value";

  case FindApproxPath::CT_match_one:
    return out << "match_one";

  case FindApproxPath::CT_match_many:
    return out << "match_many";

  case FindApproxPath::CT_match_pointer:
    return out << "match_pointer";
  };

  return out << "**invalid**";
};
