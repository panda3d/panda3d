// Filename: winStats.cxx
// Created by:  drose (02Dec03)
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

#include "pandatoolbase.h"

#include "winStatsServer.h"
#include "config_pstats.h"
#include "pystub.h"

#include <windows.h>

static const char *toplevel_class_name = "pstats";
static WinStatsServer *server = NULL;

////////////////////////////////////////////////////////////////////
//     Function: toplevel_window_proc
//  Description: 
////////////////////////////////////////////////////////////////////
static LONG WINAPI
toplevel_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_TIMER:
    server->poll();
    break;

  case WM_DESTROY:
    PostQuitMessage(0);
    break;

  default:
    break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}

  
////////////////////////////////////////////////////////////////////
//     Function: create_toplevel_window
//  Description: Creates the initial, toplevel window for the
//               application.
////////////////////////////////////////////////////////////////////
static HWND
create_toplevel_window(HINSTANCE application) {
  WNDCLASS wc;

  ZeroMemory(&wc, sizeof(WNDCLASS));
  wc.lpfnWndProc = (WNDPROC)toplevel_window_proc;
  wc.hInstance = application;
  wc.lpszClassName = toplevel_class_name;

  if (!RegisterClass(&wc)) {
    nout << "Could not register window class!\n";
    exit(1);
  }

  DWORD window_style = WS_POPUP | WS_SYSMENU | WS_ICONIC;

  ostringstream strm;
  strm << "PStats " << pstats_port;
  string window_name = strm.str();

  HWND toplevel_window = 
    CreateWindow(toplevel_class_name, window_name.c_str(), window_style,
                 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                 NULL, NULL, application, 0);
  if (!toplevel_window) {
    nout << "Could not create toplevel window!\n";
    exit(1);
  }
  
  return toplevel_window;
}


// WinMain() is the correct way to start a Windows-only application,
// but it is sometimes more convenient during development to use
// main() instead, which doesn't squelch the stderr output.

#ifndef DEVELOP_WINSTATS
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) 
#else
int main(int argc, char *argv[])
#endif
{
  HINSTANCE application = GetModuleHandle(NULL);
  HWND toplevel_window = create_toplevel_window(application);

  ShowWindow(toplevel_window, SW_SHOWMINIMIZED);

  // Create the server object.
  server = new WinStatsServer;
  if (!server->listen()) {
    ostringstream stream;
    stream 
      << "Unable to open port " << pstats_port
      << ".  Try specifying a different\n"
      << "port number using pstats-port in your Config file.";
    string str = stream.str();
    MessageBox(toplevel_window, str.c_str(), "PStats error", 
               MB_OK | MB_ICONEXCLAMATION);
    exit(1);
  }

  // Set up a timer to poll the pstats every so often.
  SetTimer(toplevel_window, 1, 200, NULL);

  // Now get lost in the Windows message loop.
  MSG msg;
  int retval;
  retval = GetMessage(&msg, NULL, 0, 0);
  while (retval != 0) {
    if (retval == -1) {
      nout << "Error processing message queue.\n";
      exit(1);
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    retval = GetMessage(&msg, NULL, 0, 0);
  }

  return (0);
}
