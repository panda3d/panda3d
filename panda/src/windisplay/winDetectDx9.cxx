/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winDetectDx9.cxx
 * @author aignacio
 * @date 2007-01-18
 */

#include "pandabase.h"

#ifdef HAVE_DX9

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#undef Configure
#include <d3d9.h>
#include "graphicsStateGuardian.h"
#include "graphicsPipe.h"
#include "displaySearchParameters.h"

#define Direct3DCreate Direct3DCreate9

typedef LPDIRECT3D9 (WINAPI *DIRECT3DCREATE9)(UINT SDKVersion);

typedef LPDIRECT3D9 DIRECT_3D;
typedef D3DCAPS9 D3DCAPS;
typedef D3DADAPTER_IDENTIFIER9 D3DADAPTER_IDENTIFIER;
typedef LPDIRECT3DDEVICE9 DIRECT_3D_DEVICE;
typedef DIRECT3DCREATE9 DIRECT_3D_CREATE;

static char *d3d_dll_name = "d3d9.dll";
static char *direct_3d_create_function_name = "Direct3DCreate9";


// include common source code
#include "winDetectDx.h"


int dx9_display_information (DisplaySearchParameters &display_search_parameters, DisplayInformation *display_information) {
  return get_display_information (display_search_parameters, display_information);
}

#endif
