// Filename: py_panda.cxx
// Created by:  drose (04Jul05)
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

#include "py_panda.h"

PyMemberDef standard_type_members[] = {
	{"this", T_INT, offsetof(Dtool_PyInstDef,_ptr_to_object),READONLY,"C++ This if any"},
	{"this_ownership", T_INT, offsetof(Dtool_PyInstDef, _memory_rules), READONLY,"C++ 'this' ownership rules"},
	{"this_signiture", T_INT, offsetof(Dtool_PyInstDef, _signiture), READONLY,"A type check signiture"},
	{"this_metatype", T_OBJECT, offsetof(Dtool_PyInstDef, _My_Type), READONLY,"The dtool meta object"},
	{NULL}	/* Sentinel */
};
