// Filename: configVariableBase.h
// Created by:  drose (21Oct04)
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

#ifndef CONFIGVARIABLEBASE_H
#define CONFIGVARIABLEBASE_H

#include "dtoolbase.h"
#include "configFlags.h"
#include "configVariableCore.h"
#include "configDeclaration.h"
#include "configVariableManager.h"
#include "vector_string.h"
#include "pset.h"

// Use this macro to wrap around a description passed to a
// ConfigVariable constructor.  This allows the description to be
// completely compiled out, so that it won't even be a part of the
// string table, in the absence of PRC_SAVE_DESCRIPTIONS.
#ifdef PRC_SAVE_DESCRIPTIONS
#define PRC_DESC(description) description
#else
#define PRC_DESC(description) ""
#endif

////////////////////////////////////////////////////////////////////
//       Class : ConfigVariableBase
// Description : This class is the base class for both
//               ConfigVariableList and ConfigVariable (and hence for
//               all of the ConfigVariableBool, ConfigVaribleString,
//               etc. classes).  It collects together the common
//               interface for all generic ConfigVariables.
//
//               Mostly, this class serves as a thin wrapper around
//               ConfigVariableCore and/or ConfigDeclaration, more or
//               less duplicating the interface presented there.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ConfigVariableBase : public ConfigFlags {
protected:
  INLINE ConfigVariableBase(const string &name, ValueType type);
  ConfigVariableBase(const string &name, ValueType type,
                     const string &description, int flags);
  INLINE ~ConfigVariableBase();

PUBLISHED:
  INLINE const string &get_name() const;

  INLINE ValueType get_value_type() const;
  INLINE const string &get_description() const;
  INLINE int get_flags() const;
  INLINE bool is_closed() const;
  INLINE int get_trust_level() const;
  INLINE bool is_dynamic() const;

  INLINE bool clear_local_value();
  INLINE bool has_local_value() const;
  INLINE bool has_value() const;
  
  INLINE void output(ostream &out) const;
  INLINE void write(ostream &out) const;

protected:
  void record_unconstructed() const;
  bool was_unconstructed() const;

  ConfigVariableCore *_core;

  typedef pset<const ConfigVariableBase *> Unconstructed;
  static Unconstructed *_unconstructed;
};

INLINE ostream &operator << (ostream &out, const ConfigVariableBase &variable);

#include "configVariableBase.I"

#endif
