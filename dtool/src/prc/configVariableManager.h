// Filename: configVariableManager.h
// Created by:  drose (15Oct04)
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

#ifndef CONFIGVARIABLEMANAGER_H
#define CONFIGVARIABLEMANAGER_H

#include "dtoolbase.h"
#include "notify.h"
#include "pvector.h"
#include "pmap.h"

class ConfigVariableCore;

////////////////////////////////////////////////////////////////////
//       Class : ConfigVariableManager
// Description : A global object that maintains the set of ConfigVariableCores
//               everywhere in the world, and keeps them in sorted
//               order.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ConfigVariableManager {
protected:
  ConfigVariableManager();
  ~ConfigVariableManager();

PUBLISHED:
  ConfigVariableCore *make_variable(const string &name);

  INLINE int get_num_variables() const;
  INLINE ConfigVariableCore *get_variable(int n) const;
  string get_variable_name(int n) const;
  bool is_variable_used(int n) const;

  void output(ostream &out) const;
  void write(ostream &out) const;

  void list_unused_variables() const;
  void list_variables() const;
  void list_dynamic_variables() const;

  static ConfigVariableManager *get_global_ptr();

private:
  void list_variable(const ConfigVariableCore *variable,
                     bool include_descriptions) const;

  typedef pvector<ConfigVariableCore *> Variables;
  Variables _variables;

  typedef pmap<string, ConfigVariableCore *> VariablesByName;
  VariablesByName _variables_by_name;

  static ConfigVariableManager *_global_ptr;
};

INLINE ostream &operator << (ostream &out, const ConfigVariableManager &variableMgr);

#include "configVariableManager.I"

#endif
