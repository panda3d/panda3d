// Filename: xFileTemplates.cxx
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

#include "xFileTemplates.h"

// This must be included only in exactly one .cxx file, since
// including defines the structure!
#include <rmxftmpl.h>

const int d3drm_xtemplates_length = D3DRM_XTEMPLATE_BYTES;

// I don't know what the official Windows way to declare this thing
// is.  This is my way.  What a strange coding environment this is.
#define DECLARE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
  const GUID name \
      = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

DECLARE_GUID(mydef_TID_D3DRMHeader,
0x3d82ab43, 0x62da, 0x11cf, 0xab, 0x39, 0x0, 0x20, 0xaf, 0x71, 0xe4, 0x33);
