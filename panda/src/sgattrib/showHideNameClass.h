// Filename: showHideNameClass.h
// Created by:  drose (26Apr00)
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

#ifndef SHOWHIDENAMECLASS_H
#define SHOWHIDENAMECLASS_H

#include <pandabase.h>

////////////////////////////////////////////////////////////////////
//       Class : ShowHideNameClass
// Description : This is a stupid little class that's used by
//               ShowHideTransition to define the name of its
//               PT(Camera) class, so the MultiTransition it
//               inherits from can initialize itself properly.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ShowHideNameClass {
public:
  static string get_class_name() {
    return "PT(Camera)";
  }
};

#endif


