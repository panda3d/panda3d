// Filename: xFileTemplates.h
// Created by:  drose (21Jun01)
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

#ifndef XFILETEMPLATES_H
#define XFILETEMPLATES_H

#include "pandatoolbase.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <d3d.h>
#include <dxfile.h>
#include <rmxfguid.h>
#undef WIN32_LEAN_AND_MEAN

////////////////////////////////////////////////////////////////////
//
// This file exists to get an external handle to the table defined in
// the Windows header file rmxftmpl.h.  Since the table is actually
// defined in the Windows header file, we can't include that header
// file in multiple .cxx files, or we'll define the table multiple
// times.
//
// Unfortunately, the length of the table is defined within that
// header file with a #define, so there's no way to extern *that*.
// Instead, we define our own variable that references the length.
//
////////////////////////////////////////////////////////////////////

extern unsigned char D3DRM_XTEMPLATES[];
extern const int d3drm_xtemplates_length;

// For some reason, this definition does not appear in rmxfguid.h. 
/* {3D82AB43-62DA-11cf-AB39-0020AF71E433} */
DEFINE_GUID(mydef_TID_D3DRMHeader,
0x3d82ab43, 0x62da, 0x11cf, 0xab, 0x39, 0x0, 0x20, 0xaf, 0x71, 0xe4, 0x33);

#endif

