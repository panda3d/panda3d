// Filename: winGraphicsPipe.cxx
// Created by:  drose (20Dec02)
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

#include "winGraphicsPipe.h"
#include "config_windisplay.h"
#include "displaySearchParameters.h"
#include "dtool_config.h"

#include "psapi.h"

TypeHandle WinGraphicsPipe::_type_handle;

typedef BOOL (WINAPI *GetProcessMemoryInfoType) (HANDLE Process, PROCESS_MEMORY_COUNTERS *ppsmemCounters, DWORD cb);
typedef BOOL (WINAPI *GlobalMemoryStatusExType) (LPMEMORYSTATUSEX lpBuffer);

static int initialize = false;
static HMODULE psapi_dll = 0;
static HMODULE kernel32_dll = 0;
static GetProcessMemoryInfoType GetProcessMemoryInfoFunction = 0;
static GlobalMemoryStatusExType GlobalMemoryStatusExFunction = 0;

void get_memory_information (DisplayInformation *display_information)
{
  if (initialize == false) {
    psapi_dll = LoadLibrary ("psapi.dll");
    if (psapi_dll) {
      GetProcessMemoryInfoFunction = (GetProcessMemoryInfoType) GetProcAddress (psapi_dll, "GetProcessMemoryInfo");
    }

    kernel32_dll = LoadLibrary ("kernel32.dll");
    if (kernel32_dll) {
      GlobalMemoryStatusExFunction = (GlobalMemoryStatusExType) GetProcAddress (kernel32_dll, "GlobalMemoryStatusEx");
    }
  
    initialize = true;
  }

  if (GlobalMemoryStatusExFunction) {
    MEMORYSTATUSEX memory_status;

    memory_status.dwLength = sizeof (MEMORYSTATUSEX);
    if (GlobalMemoryStatusExFunction (&memory_status)) {
      display_information -> _physical_memory = memory_status.ullTotalPhys;
      display_information -> _available_physical_memory = memory_status.ullAvailPhys;
      display_information -> _page_file_size = memory_status.ullTotalPageFile;
      display_information -> _available_page_file_size = memory_status.ullAvailPageFile;
      display_information -> _process_virtual_memory = memory_status.ullTotalVirtual;
      display_information -> _available_process_virtual_memory = memory_status.ullAvailVirtual;
      display_information -> _memory_load = memory_status.dwMemoryLoad;
    }    
  }
  else {
    MEMORYSTATUS memory_status;

    memory_status.dwLength = sizeof (MEMORYSTATUS);
    GlobalMemoryStatus (&memory_status);

    display_information -> _physical_memory = memory_status.dwTotalPhys;
    display_information -> _available_physical_memory = memory_status.dwAvailPhys;
    display_information -> _page_file_size = memory_status.dwTotalPageFile;
    display_information -> _available_page_file_size = memory_status.dwAvailPageFile;
    display_information -> _process_virtual_memory = memory_status.dwTotalVirtual;
    display_information -> _available_process_virtual_memory = memory_status.dwAvailVirtual;
    display_information -> _memory_load = memory_status.dwMemoryLoad;
  }

  if (GetProcessMemoryInfoFunction) {
    HANDLE process;
    DWORD process_id;
    PROCESS_MEMORY_COUNTERS process_memory_counters;

    process_id = GetCurrentProcessId();
    process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
    if (process) {
      if (GetProcessMemoryInfoFunction (process, &process_memory_counters, sizeof (PROCESS_MEMORY_COUNTERS))) {
        display_information -> _process_memory =  process_memory_counters.WorkingSetSize;
        display_information -> _peak_process_memory = process_memory_counters.PeakWorkingSetSize;
        display_information -> _page_file_usage = process_memory_counters.PagefileUsage;
        display_information -> _peak_page_file_usage = process_memory_counters.PeakPagefileUsage;
      }

      CloseHandle(process);
    }
  }  
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsPipe::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
WinGraphicsPipe::
WinGraphicsPipe() {

  bool state;
  
  state = false;
  _supported_types = OT_window | OT_fullscreen_window;

  // these fns arent defined on win95, so get dynamic ptrs to them
  // to avoid ugly DLL loader failures on w95
  _pfnTrackMouseEvent = NULL;

  _hUser32 = (HINSTANCE)LoadLibrary("user32.dll");
  if (_hUser32 != NULL) {
    _pfnTrackMouseEvent = 
      (PFN_TRACKMOUSEEVENT)GetProcAddress(_hUser32, "TrackMouseEvent");
  }

  
#ifdef HAVE_DX9
  DisplaySearchParameters display_search_parameters_dx9;
  int dx9_display_information (DisplaySearchParameters &display_search_parameters_dx9, DisplayInformation *display_information);

  if (state == false && dx9_display_information (display_search_parameters_dx9, _display_information)) {
    state = true;
  }
#endif

#ifdef HAVE_DX8
  DisplaySearchParameters display_search_parameters_dx8;
  int dx8_display_information (DisplaySearchParameters &display_search_parameters_dx8, DisplayInformation *display_information);
  
  if (state == false && dx8_display_information (display_search_parameters_dx8, _display_information)) {
    state = true;    
  }
#endif

  // set callback for memory function
  _display_information -> _get_memory_information_function = get_memory_information;
  
  if (state) {

  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsPipe::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
WinGraphicsPipe::
~WinGraphicsPipe() {
  if (_hUser32 != NULL) {
    FreeLibrary(_hUser32);
    _hUser32 = NULL;
  }
}

bool MyGetProcAddr(HINSTANCE hDLL, FARPROC *pFn, const char *szExportedFnName) {
  *pFn = (FARPROC) GetProcAddress(hDLL, szExportedFnName);
  if (*pFn == NULL) {
    windisplay_cat.error() << "GetProcAddr failed for " << szExportedFnName << ", error=" << GetLastError() <<endl;
    return false;
  }
  return true;
}

bool MyLoadLib(HINSTANCE &hDLL, const char *DLLname) {
  hDLL = LoadLibrary(DLLname);
  if(hDLL == NULL) {
    windisplay_cat.error() << "LoadLibrary failed for " << DLLname << ", error=" << GetLastError() <<endl;
    return false;
  }
  return true;
}
