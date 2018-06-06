/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file executionEnvironment.I
 * @author drose
 * @date 2000-05-15
 */

/**
 * Returns true if the indicated environment variable is defined.
 */
INLINE bool ExecutionEnvironment::
has_environment_variable(const std::string &var) {
  return get_ptr()->ns_has_environment_variable(var);
}

/**
 * Returns the definition of the indicated environment variable, or the empty
 * string if the variable is undefined.
 */
INLINE std::string ExecutionEnvironment::
get_environment_variable(const std::string &var) {
  return get_ptr()->ns_get_environment_variable(var);
}

/**
 * Changes the definition of the indicated environment variable.
 */
INLINE void ExecutionEnvironment::
set_environment_variable(const std::string &var, const std::string &value) {
  get_ptr()->ns_set_environment_variable(var, value);
}

/**
 * Changes the apparent definition of the indicated environment variable by
 * masking it within this class with a new value.  This does not change the
 * actual environment variable, but future calls to get_environment_variable()
 * will return this new value.
 */
INLINE void ExecutionEnvironment::
shadow_environment_variable(const std::string &var, const std::string &value) {
  get_ptr()->ns_shadow_environment_variable(var, value);
}

/**
 * Removes a value set by a previous call to shadow_environment_variable(),
 * and lets the actual value of the variable show again.
 */
INLINE void ExecutionEnvironment::
clear_shadow(const std::string &var) {
  get_ptr()->ns_clear_shadow(var);
}

/**
 * Returns the number of command-line arguments available, not counting arg 0,
 * the binary name.
 */
INLINE size_t ExecutionEnvironment::
get_num_args() {
  return get_ptr()->ns_get_num_args();
}

/**
 * Returns the nth command-line argument.  The index n must be in the range [0
 * .. get_num_args()).  The first parameter, n == 0, is the first actual
 * parameter, not the binary name.
 */
INLINE std::string ExecutionEnvironment::
get_arg(size_t n) {
  return get_ptr()->ns_get_arg(n);
}

/**
 * Returns the name of the binary executable that started this program, if it
 * can be determined.
 */
INLINE std::string ExecutionEnvironment::
get_binary_name() {
  return get_ptr()->ns_get_binary_name();
}

/**
 * Returns the name of the libdtool DLL that is used in this program, if it
 * can be determined.
 */
INLINE std::string ExecutionEnvironment::
get_dtool_name() {
  return get_ptr()->ns_get_dtool_name();
}

/**
 * Do not use.
 */
INLINE void ExecutionEnvironment::
set_binary_name(const std::string &name) {
  get_ptr()->_binary_name = name;
}

/**
 * Do not use.
 */
INLINE void ExecutionEnvironment::
set_dtool_name(const std::string &name) {
  get_ptr()->_dtool_name = name;
}
