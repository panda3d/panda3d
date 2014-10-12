// Filename: executionEnvironment.I
// Created by:  drose (15May00)
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
//     Function: ExecutionEnvironment::shadow_environment_variable
//       Access: Public, Static
//  Description: Changes the apparent definition of the indicated
//               environment variable by masking it within this class
//               with a new value.  This does not change the actual
//               environment variable, but future calls to
//               get_environment_variable() will return this new
//               value.
////////////////////////////////////////////////////////////////////
INLINE void ExecutionEnvironment::
shadow_environment_variable(const string &var, const string &value) {
  get_ptr()->ns_shadow_environment_variable(var, value);
}

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::clear_shadow
//       Access: Public, Static
//  Description: Removes a value set by a previous call to
//               shadow_environment_variable(), and lets the actual
//               value of the variable show again.
////////////////////////////////////////////////////////////////////
INLINE void ExecutionEnvironment::
clear_shadow(const string &var) {
  get_ptr()->ns_clear_shadow(var);
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

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::get_dtool_name
//       Access: Public, Static
//  Description: Returns the name of the libdtool DLL that
//               is used in this program, if it can be determined.
////////////////////////////////////////////////////////////////////
INLINE string ExecutionEnvironment::
get_dtool_name() {
  return get_ptr()->ns_get_dtool_name();
}

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::set_binary_name
//       Access: Public, Static
//  Description: Do not use.
////////////////////////////////////////////////////////////////////
INLINE void ExecutionEnvironment::
set_binary_name(const string &name) {
  get_ptr()->_binary_name = name;
}

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::set_dtool_name
//       Access: Public, Static
//  Description: Do not use.
////////////////////////////////////////////////////////////////////
INLINE void ExecutionEnvironment::
set_dtool_name(const string &name) {
  get_ptr()->_dtool_name = name;
}
