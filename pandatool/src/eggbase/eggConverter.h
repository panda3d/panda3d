// Filename: eggConverter.h
// Created by:  drose (15Feb00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef EGGCONVERTER_H
#define EGGCONVERTER_H

#include "pandatoolbase.h"

#include "eggFilter.h"

////////////////////////////////////////////////////////////////////
//       Class : EggConverter
// Description : This is a general base class for programs that
//               convert between egg files and some other format.  See
//               EggToSomething and SomethingToEgg.
////////////////////////////////////////////////////////////////////
class EggConverter : public EggFilter {
public:
  EggConverter(const string &format_name,
               const string &preferred_extension = string(),
               bool allow_last_param = true,
               bool allow_stdout = true);

protected:
  string _format_name;
};

#endif


