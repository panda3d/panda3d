// Filename: findApproxPath.cxx
// Created by:  drose (13Mar02)
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

#include "findApproxPath.h"
#include "config_pgraph.h"

#include "pandaNode.h"


////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::Component::matches
//       Access: Public
//  Description: Returns true if the indicated node matches this
//               component, false otherwise.
////////////////////////////////////////////////////////////////////
bool FindApproxPath::Component::
matches(PandaNode *node) const {
  string node_name;

  switch (_type) {
  case CT_match_name:
    // Match the node's name exactly.
    return (_name == node->get_name());

  case CT_match_name_glob:
    // Match the node's name according to filename globbing rules.
    return (_glob.matches(node->get_name()));

  case CT_match_exact_type:
    // Match the node's type exactly.
    return (node->is_exact_type(_type_handle));

  case CT_match_inexact_type:
    // Match the node's type inexactly: it's a match if the node
    // is the type, or is derived from the type.
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

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::Component::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void FindApproxPath::Component::
output(ostream &out) const {
  out << _type;
  switch (_type) {
  case CT_match_name:
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

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::add_string
//       Access: Public
//  Description: Adds a sequence of components separated by slashes,
//               followed optionally by a semicolon and a sequence of
//               control flags, to the path sequence.  Returns true if
//               successful, false if the string contained an error.
////////////////////////////////////////////////////////////////////
bool FindApproxPath::
add_string(const string &str_path) {
  size_t start = 0;
  size_t slash = str_path.find('/');
  while (slash != string::npos) {
    if (!add_component(str_path.substr(start, slash - start))) {
      return false;
    }
    start = slash + 1;
    slash = str_path.find('/', start);
  }

  size_t semicolon = str_path.rfind(';');

  // We want to find the *last* semicolon at start or later, if there
  // happens to be more than one.  rfind will find the rightmost
  // semicolon in the entire string; if this is less than start, there
  // is no semicolon right of start.
  if (semicolon < start) {
    semicolon = string::npos;
  }

  if (!add_component(str_path.substr(start, semicolon - start))) {
    return false;
  }

  if (semicolon != string::npos) {
    return add_flags(str_path.substr(semicolon + 1));
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::add_flags
//       Access: Public
//  Description: Adds a sequence of control flags.  This will be a
//               sequence of letters preceded by either '+' or '-',
//               with no intervening punctuation.  Returns true if
//               successful, false otherwise.
////////////////////////////////////////////////////////////////////
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

    default:
      pgraph_cat.error()
        << "Invalid control flag string: " << str_flags << "\n";
      return false;
    }

    ++pi;
  }

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::add_component
//       Access: Public
//  Description: Adds a single component to the path sequence, defined
//               by a string as might appear between slashes in the
//               path string.  Returns true if successful, false if
//               the string component was in some way invalid.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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

