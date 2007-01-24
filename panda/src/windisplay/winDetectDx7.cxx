// Filename: winDetectDx7.h
// Created by:  aignacio (18Jan07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2007, Disney Enterprises, Inc.  All rights 
// reserved.
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

#include "pandabase.h"

#if HAVE_DX

#include <ddraw.h>

#include "graphicsStateGuardian.h"
#include "graphicsPipe.h"
#include "displaySearchParameters.h"


typedef HRESULT (WINAPI *DIRECTDRAWCREATEEX) (GUID FAR * lpGuid, LPVOID *lplpDD, REFIID iid, IUnknown FAR *pUnkOuter);

static GUID REFIID_IDirectDraw7 = { 0x15e65ec0, 0x3b9c, 0x11d2, 0xb9, 0x2f, 0x00, 0x60, 0x97, 0x97, 0xea, 0x5b };

int dx7_display_information (DisplaySearchParameters &display_search_parameters, DisplayInformation *display_information) {
  int debug = false;
  int state;
  
  HMODULE ddraw_dll;
  LPDIRECTDRAW7 direct_draw;
  DDSCAPS2 dds_caps; 
  DWORD total_video_memory; 
  DWORD free_video_memory;
  HRESULT hr; 

  state = false;
  ddraw_dll = LoadLibrary ("ddraw.dll");
  if (ddraw_dll) {
    DIRECTDRAWCREATEEX DirectDrawCreateEx;

    DirectDrawCreateEx = (DIRECTDRAWCREATEEX) GetProcAddress (ddraw_dll, "DirectDrawCreateEx");
    if (DirectDrawCreateEx) {
      hr = DirectDrawCreateEx (NULL, (LPVOID *) &direct_draw, REFIID_IDirectDraw7, NULL); 
      if (hr == DD_OK) { 
        hr = direct_draw -> QueryInterface (REFIID_IDirectDraw7, (LPVOID *)&direct_draw); 
        if (hr == DD_OK) {
          ZeroMemory (&dds_caps, sizeof (DDSCAPS2));
          dds_caps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
          hr = direct_draw -> GetAvailableVidMem (&dds_caps, &total_video_memory, &free_video_memory); 
          if (hr == DD_OK) {
            display_information -> _video_memory = total_video_memory;
            if (debug) {
              printf ("video_memory %d \n", total_video_memory);
            }
            
            state = true;
          }

          direct_draw -> Release ( );
        }
      }
    }
    FreeLibrary (ddraw_dll);
  }

  return state;
}

#endif
