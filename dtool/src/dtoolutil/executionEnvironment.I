// Filename: executionEnvironment.I
// Created by:  drose (15May00)
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
