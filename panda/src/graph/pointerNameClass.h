// Filename: pointerNameClass.h
// Created by:  drose (23Mar00)
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

#ifndef POINTERNAMECLASS_H
#define POINTERNAMECLASS_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
//       Class : PointerNameClass
// Description : This is a stupid little class that's used by
//               MultiNodeTransition to define the name of its
//               PT(Node) class, so the MultiTransition it
//               inherits from can initialize itself properly.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PointerNameClass {
public:
  static string get_class_name() {
    return "PT(Node)";
  }
};

#endif


