// Filename: qpfindApproxPath.cxx
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

#include "qpfindApproxPath.h"
#include "config_pgraph.h"

#include "globPattern.h"
#include "pandaNode.h"


////////////////////////////////////////////////////////////////////
//     Function: qpFindApproxPath::Component::matches
//       Access: Public
//  Description: Returns true if the indicated node matches this
//               component, false otherwise.
////////////////////////////////////////////////////////////////////
bool qpFindApproxPath::Component::
matches(PandaNode *node) const {
  string node_name;

  switch (_type) {
  case CT_match_name:
    // Match the node's name exactly.
    return (_name == node->get_name());

  case CT_match_name_glob:
    // Match the node's name according to filename globbing rules.
    {
      GlobPattern pattern(_name);
      return (pattern.matches(node->get_name()));
    }

  case CT_match_exact_type:
    // Match the node's type exactly.
    return (node->is_exact_type(_type_handle));

  case CT_match_inexact_type:
    // Match the node's type inexactly: it's a match if the node
    // is the type, or is derived from the type.
    return (node->is_of_type(_type_handle));

  case CT_match_one:
  case CT_match_many:
    // Match any node.
    return true;

  case CT_match_pointer:
    // Match only this one particular node.
    return (_pointer == node);
  }

  pgraph_cat.error()
    << "Invalid component in qpFindApproxPath\n";
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: qpFindApproxPath::Component::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void qpFindApproxPath::Component::
output(ostream &out) const {
  out << _type;
  if (_type == CT_match_name || _type == CT_match_name_glob) {
    out << " \"" << _name << "\"";

  } else if (_type == CT_match_exact_type || _type == CT_match_inexact_type) {
    out << " " << _type_handle;

  } else if (_type == CT_match_pointer) {
    out << " (" << *_pointer << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpFindApproxPath::add_string
//       Access: Public
//  Description: Adds a sequence of components separated by slashes,
//               followed optionally by a semicolon and a sequence of
//               control flags, to the path sequence.  Returns true if
//               successful, false if the string contained an error.
////////////////////////////////////////////////////////////////////
bool qpFindApproxPath::
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
//     Function: qpFindApproxPath::add_flags
//       Access: Public
//  Description: Adds a sequence of control flags.  This will be a
//               sequence of letters preceded by either '+' or '-',
//               with no intervening punctuation.  Returns true if
//               successful, false otherwise.
////////////////////////////////////////////////////////////////////
bool qpFindApproxPath::
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
//     Function: qpFindApproxPath::add_component
//       Access: Public
//  Description: Adds a single component to the path sequence, defined
//               by a string as might appear between slashes in the
//               path string.  Returns true if successful, false if
//               the string component was in some way invalid.
////////////////////////////////////////////////////////////////////
bool qpFindApproxPath::
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

    // *** for now, as a quick hack, if a type exists with the "qp"
    // prefix on the named type, we search for that type instead.
    TypeHandle handle = TypeRegistry::ptr()->find_type("qp" + type_name);
    if (handle == TypeHandle::none()) {
      handle = TypeRegistry::ptr()->find_type(type_name);
    }

    if (handle == TypeHandle::none()) {
      pgraph_cat.error()
        << "Invalid type name: " + type_name;
      return false;

    } else {
      add_match_exact_type(handle, flags);
    }

  } else if (!str_component.empty() && str_component[0] == '+') {
    string type_name = str_component.substr(1);

    // *** for now, as a quick hack, if a type exists with the "qp"
    // prefix on the named type, we search for that type instead.
    TypeHandle handle = TypeRegistry::ptr()->find_type("qp" + type_name);
    if (handle == TypeHandle::none()) {
      handle = TypeRegistry::ptr()->find_type(type_name);
    }

    if (handle == TypeHandle::none()) {
      pgraph_cat.error()
        << "Invalid type name: " + type_name;
      return false;

    } else {
      add_match_inexact_type(handle, flags);
    }

  } else {
    add_match_name_glob(str_component, flags);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: qpFindApproxPath::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void qpFindApproxPath::
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
operator << (ostream &out, qpFindApproxPath::ComponentType type) {
  switch (type) {
  case qpFindApproxPath::CT_match_name:
    return out << "match_name";

  case qpFindApproxPath::CT_match_name_glob:
    return out << "match_name_glob";

  case qpFindApproxPath::CT_match_exact_type:
    return out << "match_exact_type";

  case qpFindApproxPath::CT_match_inexact_type:
    return out << "match_inexact_type";

  case qpFindApproxPath::CT_match_one:
    return out << "match_one";

  case qpFindApproxPath::CT_match_many:
    return out << "match_many";

  case qpFindApproxPath::CT_match_pointer:
    return out << "match_pointer";
  };

  return out << "**invalid**";
};

