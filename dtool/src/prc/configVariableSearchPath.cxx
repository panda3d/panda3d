// Filename: configVariableSearchPath.cxx
// Created by:  drose (21Oct04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "configVariableSearchPath.h"
#include "executionEnvironment.h"

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableSearchPath::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
ConfigVariableSearchPath::
ConfigVariableSearchPath(const string &name, 
                   const string &description, int flags) :
  ConfigVariableBase(name, VT_search_path, description, flags),
  _value_seq(-1),
  _value_stale(true)
{
  // A SearchPath variable implicitly defines a default value of the empty
  // string.  This is just to prevent the core variable from
  // complaining should anyone ask for its solitary value.
  if (_core->get_default_value() == (ConfigDeclaration *)NULL) {
    _core->set_default_value("");
  }
  _core->set_used();
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableSearchPath::get_value
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
const DSearchPath &ConfigVariableSearchPath::
get_value() const {
  if (_value_stale || _value_seq != _core->get_value_seq()) {
    ((ConfigVariableSearchPath *)this)->reload_search_path();
  }
  return _value;
}

////////////////////////////////////////////////////////////////////
//     Function: ConfigVariableSearchPath::reload_search_path
//       Access: Private
//  Description: Recopies the config variable into the search path for
//               returning its value.
////////////////////////////////////////////////////////////////////
void ConfigVariableSearchPath::
reload_search_path() {
  _value.clear();

  _value.append_path(_prefix);
  int num_unique_references = _core->get_num_unique_references();
  for (int i = 0; i < num_unique_references; i++) {
    string dirname = _core->get_unique_reference(i)->get_string_value();
    string expanded = ExecutionEnvironment::expand_string(dirname);
    _value.append_directory(Filename::from_os_specific(expanded));
  }
  _value.append_path(_postfix);

  _value_seq = _core->get_value_seq();
  _value_stale = false;
}
