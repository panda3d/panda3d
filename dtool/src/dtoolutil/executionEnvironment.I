// Filename: executionEnvironment.I
// Created by:  drose (15May00)
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


////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::has_environment_variable
//       Access: Public, Static
//  Description: Returns true if the indicated environment variable
//               is defined.
////////////////////////////////////////////////////////////////////
INLINE bool ExecutionEnvironment::
has_environment_variable(const string &var) {
  return get_ptr()->ns_has_environment_variable(var);
}

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::get_environment_variable
//       Access: Public, Static
//  Description: Returns the definition of the indicated environment
//               variable, or the empty string if the variable is
//               undefined.
////////////////////////////////////////////////////////////////////
INLINE string ExecutionEnvironment::
get_environment_variable(const string &var) {
  return get_ptr()->ns_get_environment_variable(var);
}

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::set_environment_variable
//       Access: Public, Static
//  Description: Changes the definition of the indicated environment
//               variable.
////////////////////////////////////////////////////////////////////
INLINE void ExecutionEnvironment::
set_environment_variable(const string &var, const string &value) {
  get_ptr()->ns_set_environment_variable(var, value);
}

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::get_num_args
//       Access: Public, Static
//  Description: Returns the number of command-line arguments
//               available, not counting arg 0, the binary name.
////////////////////////////////////////////////////////////////////
INLINE int ExecutionEnvironment::
get_num_args() {
  return get_ptr()->ns_get_num_args();
}

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::get_arg
//       Access: Public, Static
//  Description: Returns the nth command-line argument.  The index n
//               must be in the range [0 .. get_num_args()).  The
//               first parameter, n == 0, is the first actual
//               parameter, not the binary name.
////////////////////////////////////////////////////////////////////
INLINE string ExecutionEnvironment::
get_arg(int n) {
  return get_ptr()->ns_get_arg(n);
}

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::get_binary_name
//       Access: Public, Static
//  Description: Returns the name of the binary executable that
//               started this program, if it can be determined.
////////////////////////////////////////////////////////////////////
INLINE string ExecutionEnvironment::
get_binary_name() {
  return get_ptr()->ns_get_binary_name();
}
