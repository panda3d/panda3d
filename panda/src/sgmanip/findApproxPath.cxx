// Filename: findApproxPath.cxx
// Created by:  drose (18Feb00)
// 
////////////////////////////////////////////////////////////////////

#include "findApproxPath.h"
#include "config_sgmanip.h"

#include <globPattern.h>
#include <node.h>
#include <namedNode.h>
#include <nodeRelation.h>


////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::Component::matches
//       Access: Public
//  Description: Returns true if the indicated node matches this
//               component, false otherwise.
////////////////////////////////////////////////////////////////////
bool FindApproxPath::Component::
matches(NodeRelation *arc) const {
  Node *node = arc->get_child();
  string node_name;

  switch (_type) {
  case CT_match_name:
    // Match the node's name exactly.
    if (node->is_of_type(NamedNode::get_class_type())) {
      node_name = DCAST(NamedNode, node)->get_name();
    }
    return (_name == node_name);

  case CT_match_name_glob:
    // Match the node's name according to filename globbing rules.
    if (node->is_of_type(NamedNode::get_class_type())) {
      node_name = DCAST(NamedNode, node)->get_name();
    }
    {
      GlobPattern pattern(_name);
      return (pattern.matches(node_name));
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

  sgmanip_cat.error()
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
  if (_type == CT_match_name || _type == CT_match_name_glob) {
    out << " \"" << _name << "\"";

  } else if (_type == CT_match_exact_type || _type == CT_match_inexact_type) {
    out << " " << _type_handle;

  } else if (_type == CT_match_pointer) {
    out << " (" << *_pointer << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FindApproxPath::add_string
//       Access: Public
//  Description: Adds a sequence of components separated by slashes to
//               the path sequence.  Returns true if successful, false
//               if the string contained an error.
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
  return add_component(str_path.substr(start));
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
      sgmanip_cat.warning()
	<< "@@** is ambiguous; use @@*/** or **/@@* instead.\n";
    }
    add_match_many(flags);

  } else if (!str_component.empty() && str_component[0] == '-') {
    string type_name = str_component.substr(1);
    TypeHandle handle = TypeRegistry::ptr()->find_type(type_name);

    if (handle == TypeHandle::none()) {
      sgmanip_cat.error()
	<< "Invalid type name: " + type_name;
      return false;

    } else {
      add_match_exact_type(handle, flags);
    }

  } else if (!str_component.empty() && str_component[0] == '+') {
    string type_name = str_component.substr(1);
    TypeHandle handle = TypeRegistry::ptr()->find_type(type_name);

    if (handle == TypeHandle::none()) {
      sgmanip_cat.error()
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

  case FindApproxPath::CT_match_one:
    return out << "match_one";

  case FindApproxPath::CT_match_many:
    return out << "match_many";

  case FindApproxPath::CT_match_pointer:
    return out << "match_pointer";
  };

  return out << "**invalid**";
};

