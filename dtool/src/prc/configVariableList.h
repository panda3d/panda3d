// Filename: configVariableList.h
// Created by:  drose (20Oct04)
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

#ifndef CONFIGVARIABLELIST_H
#define CONFIGVARIABLELIST_H

#include "dtoolbase.h"
#include "configVariableBase.h"

////////////////////////////////////////////////////////////////////
//       Class : ConfigVariableList
// Description : This class is similar to ConfigVariable, but it
//               reports its value as a list of strings.  In this
//               special case, all of the declarations of the variable
//               are returned as the elements of this list, in order.
//
//               Note that this is different from a normal
//               ConfigVariableString, which just returns its topmost
//               value, which can optionally be treated as a number of
//               discrete words by dividing it at the spaces.
//
//               A ConfigVariableList cannot be modified locally.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ConfigVariableList : public ConfigVariableBase {
PUBLISHED:
  ConfigVariableList(const string &name, 
                     const string &description = string(), int flags = 0);
  INLINE ~ConfigVariableList();

  INLINE int get_num_values() const;
  INLINE string get_string_value(int n) const;

  INLINE int get_num_unique_values() const;
  INLINE string get_unique_value(int n) const;

  INLINE int size() const;
  INLINE string operator [] (int n) const;
};

#include "configVariableList.I"

#endif
