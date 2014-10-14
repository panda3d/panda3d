// Filename: executionEnvironment.h
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

#ifndef EXECUTIONENVIRONMENT_H
#define EXECUTIONENVIRONMENT_H

#include "dtoolbase.h"

#include "vector_string.h"
#include "filename.h"

#include <map>

////////////////////////////////////////////////////////////////////
//       Class : ExecutionEnvironment
// Description : Encapsulates access to the environment variables and
//               command-line arguments at the time of execution.
//               This is encapsulated to support accessing these
//               things during static init time, which seems to be
//               risky at best.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL ExecutionEnvironment {
private:
  ExecutionEnvironment();

PUBLISHED:
  INLINE static bool has_environment_variable(const string &var);
  INLINE static string get_environment_variable(const string &var);
  INLINE static void set_environment_variable(const string &var, const string &value);

  INLINE static void shadow_environment_variable(const string &var, const string &value);
  INLINE static void clear_shadow(const string &var);

  static string expand_string(const string &str);

  INLINE static int get_num_args();
  INLINE static string get_arg(int n);

  INLINE static string get_binary_name();
  INLINE static string get_dtool_name();

  INLINE static void set_binary_name(const string &name);
  INLINE static void set_dtool_name(const string &name);

  static Filename get_cwd();

private:
  bool ns_has_environment_variable(const string &var) const;
  string ns_get_environment_variable(const string &var) const;
  void ns_set_environment_variable(const string &var, const string &value);
  void ns_shadow_environment_variable(const string &var, const string &value);
  void ns_clear_shadow(const string &var);

  int ns_get_num_args() const;
  string ns_get_arg(int n) const;

  string ns_get_binary_name() const;
  string ns_get_dtool_name() const;

  static ExecutionEnvironment *get_ptr();

  void read_environment_variables();
  void read_args();

private:
  typedef map<string, string> EnvironmentVariables;
  EnvironmentVariables _variables;

  typedef vector_string CommandArguments;
  CommandArguments _args;

  string _binary_name;
  string _dtool_name;

  static ExecutionEnvironment *_global_ptr;
};

#include "executionEnvironment.I"

#endif
