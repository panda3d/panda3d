// Filename: xFileTemplates.h
// Created by:  drose (21Jun01)
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

#ifndef XFILETEMPLATES_H
#define XFILETEMPLATES_H

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

#endif

