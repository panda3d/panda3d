// Filename: writable.cxx
// Created by:  jason (19Jun00)
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


#include "writable.h"

TypeHandle Writable::_type_handle;
Writable* const Writable::Null = (Writable*)0L;

////////////////////////////////////////////////////////////////////
//     Function: Writable::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
Writable::~Writable()
{
}
