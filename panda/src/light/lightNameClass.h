// Filename: lightNameClass.h
// Created by:  drose (24Mar00)
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

#ifndef LIGHTNAMECLASS_H
#define LIGHTNAMECLASS_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
//       Class : LightNameClass
// Description : This is a stupid little class that's used by
//               LightTransition to define the name of its
//               PT(Light) class, so the MultiTransition it
//               inherits from can initialize itself properly.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LightNameClass {
public:
  static string get_class_name() {
    return "PT(Light)";
  }
};

#endif


