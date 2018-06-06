/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file executionEnvironment.h
 * @author drose
 * @date 2000-05-15
 */

#ifndef EXECUTIONENVIRONMENT_H
#define EXECUTIONENVIRONMENT_H

#include "dtoolbase.h"

#include "vector_string.h"
#include "filename.h"

#include <map>

/**
 * Encapsulates access to the environment variables and command-line arguments
 * at the time of execution.  This is encapsulated to support accessing these
 * things during static init time, which seems to be risky at best.
 */
class EXPCL_DTOOL_DTOOLUTIL ExecutionEnvironment {
private:
  ExecutionEnvironment();

PUBLISHED:
  INLINE static bool has_environment_variable(const std::string &var);
  INLINE static std::string get_environment_variable(const std::string &var);
  INLINE static void set_environment_variable(const std::string &var, const std::string &value);

  INLINE static void shadow_environment_variable(const std::string &var, const std::string &value);
  INLINE static void clear_shadow(const std::string &var);

  static std::string expand_string(const std::string &str);

  INLINE static size_t get_num_args();
  INLINE static std::string get_arg(size_t n);

  INLINE static std::string get_binary_name();
  INLINE static std::string get_dtool_name();

  INLINE static void set_binary_name(const std::string &name);
  INLINE static void set_dtool_name(const std::string &name);

  static Filename get_cwd();

PUBLISHED:
  MAKE_MAP_PROPERTY(environment_variables, has_environment_variable,
                    get_environment_variable, set_environment_variable);

  MAKE_SEQ_PROPERTY(args, get_num_args, get_arg);
  MAKE_PROPERTY(binary_name, get_binary_name, set_binary_name);
  MAKE_PROPERTY(dtool_name, get_dtool_name, set_dtool_name);
  MAKE_PROPERTY(cwd, get_cwd);

private:
  bool ns_has_environment_variable(const std::string &var) const;
  std::string ns_get_environment_variable(const std::string &var) const;
  void ns_set_environment_variable(const std::string &var, const std::string &value);
  void ns_shadow_environment_variable(const std::string &var, const std::string &value);
  void ns_clear_shadow(const std::string &var);

  size_t ns_get_num_args() const;
  std::string ns_get_arg(size_t n) const;

  std::string ns_get_binary_name() const;
  std::string ns_get_dtool_name() const;

  static ExecutionEnvironment *get_ptr();

  void read_environment_variables();
  void read_args();

private:
  typedef std::map<std::string, std::string> EnvironmentVariables;
  EnvironmentVariables _variables;

  typedef vector_string CommandArguments;
  CommandArguments _args;

  std::string _binary_name;
  std::string _dtool_name;

  static ExecutionEnvironment *_global_ptr;
};

#include "executionEnvironment.I"

#endif
