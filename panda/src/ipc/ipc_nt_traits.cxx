// Filename: ipc_nt_traits.cxx
// Created by:  mike (23Oct98)
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

#ifdef WIN32_VC

#include "ipc_nt_traits.h"

const int ipc_traits::days_in_preceeding_months[12] = { 0, 31, 59, 90,
                                                120, 151, 181, 212,
                                                243, 273, 304, 334 };
const int ipc_traits::days_in_preceeding_months_leap[12] = { 0, 31, 60,
                                                91, 121, 152, 182, 213,
                                                244, 274, 305, 335 };

#endif
