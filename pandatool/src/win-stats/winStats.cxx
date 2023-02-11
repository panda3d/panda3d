/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStats.cxx
 * @author drose
 * @date 2003-12-02
 */

#include "pandatoolbase.h"

#include "winStatsServer.h"
#include "config_pstatclient.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include <windows.h>
#include <commctrl.h>
#include <shellscalingapi.h>

// Enable common controls version 6, necessary for modern visual styles
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

int main(int argc, char *argv[]) {
  // Initialize commctl32.dll.
  INITCOMMONCONTROLSEX icc;
  icc.dwICC = ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES;
  icc.dwSize = sizeof(INITCOMMONCONTROLSEX);
  InitCommonControlsEx(&icc);

  // Signal DPI awareness.
  SetProcessDPIAware();

  // Create the server window.
  WinStatsServer *server = new WinStatsServer;
  if (server->parse_command_line(argc, argv, false) == ProgramBase::EC_failure) {
    MessageBox(nullptr, "Failed to parse command-line options.",
               "PStats Error", MB_OK | MB_ICONEXCLAMATION);
    return 1;
  }

  // Now get lost in the Windows message loop.
  MSG msg;
  int retval;
  retval = GetMessage(&msg, nullptr, 0, 0);
  while (retval != 0) {
    if (retval == -1) {
      nout << "Error processing message queue.\n";
      exit(1);
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    retval = GetMessage(&msg, nullptr, 0, 0);
  }

  return 0;
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
  return main(__argc, __argv);
}
