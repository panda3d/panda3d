// Filename: winDetectDx8.cxx
// Created by:  aignacio (18Jan07)
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

#include "pandabase.h"

#ifdef HAVE_DX8

#define WIN32_LEAN_AND_MEAN
#include <d3d8.h>
#include "graphicsStateGuardian.h"
#include "graphicsPipe.h"
#include "displaySearchParameters.h"


#define DX8 1
#define Direct3DCreate Direct3DCreate8

typedef LPDIRECT3D8 (WINAPI *DIRECT3DCREATE8)(UINT SDKVersion);
typedef LPDIRECT3D8 DIRECT_3D;
typedef D3DCAPS8 D3DCAPS;
typedef D3DADAPTER_IDENTIFIER8 D3DADAPTER_IDENTIFIER;
typedef LPDIRECT3DDEVICE8 DIRECT_3D_DEVICE;
typedef DIRECT3DCREATE8 DIRECT_3D_CREATE;

static char *d3d_dll_name = "d3d8.dll";
static char *direct_3d_create_function_name = "Direct3DCreate8";


// include common source code
#include "winDetectDx.h"


int dx8_display_information (DisplaySearchParameters &display_search_parameters, DisplayInformation *display_information) {
  return get_display_information (display_search_parameters, display_information);
}

#endif
