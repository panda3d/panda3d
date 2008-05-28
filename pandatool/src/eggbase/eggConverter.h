// Filename: eggConverter.h
// Created by:  drose (15Feb00)
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


