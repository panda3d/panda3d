// Filename: winStatsMonitor.cxx
// Created by:  drose (02Dec03)
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

#include "winStatsMonitor.h"
#include "winStatsStripChart.h"
#include "winStatsChartMenu.h"

#include "pStatCollectorDef.h"
#include "indent.h"

bool WinStatsMonitor::_window_class_registered = false;
const char * const WinStatsMonitor::_window_class_name = "monitor";

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
WinStatsMonitor::
WinStatsMonitor() {
  _window = 0;
  _menu_bar = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
WinStatsMonitor::
~WinStatsMonitor() {
  cerr << "WinStatsMonitor destructor\n";
  Graphs::iterator gi;
  for (gi = _graphs.begin(); gi != _graphs.end(); ++gi) {
    delete (*gi);
  }
  _graphs.clear();

  ChartMenus::iterator mi;
  for (mi = _chart_menus.begin(); mi != _chart_menus.end(); ++mi) {
    delete (*mi);
  }
  _chart_menus.clear();

  if (_window) {
    DestroyWindow(_window);
    _window = 0;
  }

  // For now, exit when the first monitor closes.
  exit(0);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::get_monitor_name
//       Access: Public, Virtual
//  Description: Should be redefined to return a descriptive name for
//               the type of PStatsMonitor this is.
////////////////////////////////////////////////////////////////////
string WinStatsMonitor::
get_monitor_name() {
  return "WinStats";
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::initialized
//       Access: Public, Virtual
//  Description: Called after the monitor has been fully set up.  At
//               this time, it will have a valid _client_data pointer,
//               and things like is_alive() and close() will be
//               meaningful.  However, we may not yet know who we're
//               connected to (is_client_known() may return false),
//               and we may not know anything about the threads or
//               collectors we're about to get data on.
////////////////////////////////////////////////////////////////////
void WinStatsMonitor::
initialized() {
  cerr << "Monitor initialized (refcount = " << get_ref_count() << ")\n";
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::got_hello
//       Access: Public, Virtual
//  Description: Called when the "hello" message has been received
//               from the client.  At this time, the client's hostname
//               and program name will be known.
////////////////////////////////////////////////////////////////////
void WinStatsMonitor::
got_hello() {
  create_window();

  add_graph(new WinStatsStripChart(this, get_view(0), 0));
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::got_bad_version
//       Access: Public, Virtual
//  Description: Like got_hello(), this is called when the "hello"
//               message has been received from the client.  At this
//               time, the client's hostname and program name will be
//               known.  However, the client appears to be an
//               incompatible version and the connection will be
//               terminated; the monitor should issue a message to
//               that effect.
////////////////////////////////////////////////////////////////////
void WinStatsMonitor::
got_bad_version(int client_major, int client_minor,
                int server_major, int server_minor) {
  cerr << "Got bad version " << client_major << "." << client_minor 
       << " from " << get_client_progname() << " on " 
       << get_client_hostname() << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::new_collector
//       Access: Public, Virtual
//  Description: Called whenever a new Collector definition is
//               received from the client.  Generally, the client will
//               send all of its collectors over shortly after
//               connecting, but there's no guarantee that they will
//               all be received before the first frames are received.
//               The monitor should be prepared to accept new Collector
//               definitions midstream.
////////////////////////////////////////////////////////////////////
void WinStatsMonitor::
new_collector(int collector_index) {
  Graphs::iterator gi;
  for (gi = _graphs.begin(); gi != _graphs.end(); ++gi) {
    WinStatsGraph *graph = (*gi);
    graph->new_collector(collector_index);
  }

  // We might need to update our menus.
  ChartMenus::iterator mi;
  for (mi = _chart_menus.begin(); mi != _chart_menus.end(); ++mi) {
    (*mi)->do_update();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::new_thread
//       Access: Public, Virtual
//  Description: Called whenever a new Thread definition is
//               received from the client.  Generally, the client will
//               send all of its threads over shortly after
//               connecting, but there's no guarantee that they will
//               all be received before the first frames are received.
//               The monitor should be prepared to accept new Thread
//               definitions midstream.
////////////////////////////////////////////////////////////////////
void WinStatsMonitor::
new_thread(int thread_index) {
  WinStatsChartMenu *chart_menu = new WinStatsChartMenu(this, thread_index);
  chart_menu->add_to_menu_bar(_menu_bar);
  _chart_menus.push_back(chart_menu);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::new_data
//       Access: Public, Virtual
//  Description: Called as each frame's data is made available.  There
//               is no guarantee the frames will arrive in order, or
//               that all of them will arrive at all.  The monitor
//               should be prepared to accept frames received
//               out-of-order or missing.
////////////////////////////////////////////////////////////////////
void WinStatsMonitor::
new_data(int thread_index, int frame_number) {
  Graphs::iterator gi;
  for (gi = _graphs.begin(); gi != _graphs.end(); ++gi) {
    WinStatsGraph *graph = (*gi);
    graph->new_data(thread_index, frame_number);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::lost_connection
//       Access: Public, Virtual
//  Description: Called whenever the connection to the client has been
//               lost.  This is a permanent state change.  The monitor
//               should update its display to represent this, and may
//               choose to close down automatically.
////////////////////////////////////////////////////////////////////
void WinStatsMonitor::
lost_connection() {
  cerr << "Lost connection to " << get_client_hostname()
       << " (refcount = " << get_ref_count() << ")\n";

  if (_window) {
    DestroyWindow(_window);
    _window = 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::idle
//       Access: Public, Virtual
//  Description: If has_idle() returns true, this will be called
//               periodically to allow the monitor to update its
//               display or whatever it needs to do.
////////////////////////////////////////////////////////////////////
void WinStatsMonitor::
idle() {
  // Check if any of our chart menus need updating.
  ChartMenus::iterator mi;
  for (mi = _chart_menus.begin(); mi != _chart_menus.end(); ++mi) {
    (*mi)->check_update();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::has_idle
//       Access: Public, Virtual
//  Description: Should be redefined to return true if you want to
//               redefine idle() and expect it to be called.
////////////////////////////////////////////////////////////////////
bool WinStatsMonitor::
has_idle() {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::get_window
//       Access: Public
//  Description: Returns the window handle to the monitor's window.
////////////////////////////////////////////////////////////////////
HWND WinStatsMonitor::
get_window() const {
  return _window;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::add_graph
//       Access: Private
//  Description: Adds the newly-created graph to the list of managed
//               graphs.
////////////////////////////////////////////////////////////////////
void WinStatsMonitor::
add_graph(WinStatsGraph *graph) {
  _graphs.insert(graph);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::remove_graph
//       Access: Private
//  Description: Deletes the indicated graph.
////////////////////////////////////////////////////////////////////
void WinStatsMonitor::
remove_graph(WinStatsGraph *graph) {
  Graphs::iterator gi = _graphs.find(graph);
  if (gi != _graphs.end()) {
    _graphs.erase(gi);
    delete graph;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::create_window
//       Access: Private
//  Description: Creates the window for this monitor.
////////////////////////////////////////////////////////////////////
void WinStatsMonitor::
create_window() {
  if (_window) {
    return;
  }

  HINSTANCE application = GetModuleHandle(NULL);
  register_window_class(application);

  _menu_bar = CreateMenu();

  ChartMenus::iterator mi;
  for (mi = _chart_menus.begin(); mi != _chart_menus.end(); ++mi) {
    (*mi)->add_to_menu_bar(_menu_bar);
  }

  _window_title = get_client_progname() + " on " + get_client_hostname();
  DWORD window_style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | 
    WS_CLIPSIBLINGS | WS_VISIBLE;

  _window = 
    CreateWindow(_window_class_name, _window_title.c_str(), window_style,
                 CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                 NULL, _menu_bar, application, 0);
  if (!_window) {
    nout << "Could not create monitor window!\n";
    exit(1);
  }

  SetWindowLongPtr(_window, 0, (LONG_PTR)this);
  ShowWindow(_window, SW_SHOWNORMAL);
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::register_window_class
//       Access: Private, Static
//  Description: Registers the window class for the monitor window, if
//               it has not already been registered.
////////////////////////////////////////////////////////////////////
void WinStatsMonitor::
register_window_class(HINSTANCE application) {
  if (_window_class_registered) {
    return;
  }

  WNDCLASS wc;

  ZeroMemory(&wc, sizeof(WNDCLASS));
  wc.style = 0;
  wc.lpfnWndProc = (WNDPROC)static_window_proc;
  wc.hInstance = application;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = _window_class_name;

  // Reserve space to associate the this pointer with the window.
  wc.cbWndExtra = sizeof(WinStatsMonitor *);
  
  if (!RegisterClass(&wc)) {
    nout << "Could not register monitor window class!\n";
    exit(1);
  }

  _window_class_registered = true;
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::static_window_proc
//       Access: Private, Static
//  Description: 
////////////////////////////////////////////////////////////////////
LONG WINAPI WinStatsMonitor::
static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  WinStatsMonitor *self = (WinStatsMonitor *)GetWindowLongPtr(hwnd, 0);
  if (self != (WinStatsMonitor *)NULL && self->_window == hwnd) {
    return self->window_proc(hwnd, msg, wparam, lparam);
  } else {
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinStatsMonitor::window_proc
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
LONG WinStatsMonitor::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_DESTROY:
    close();
    break;

  default:
    break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}
