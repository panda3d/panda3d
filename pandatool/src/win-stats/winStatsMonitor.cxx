/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsMonitor.cxx
 * @author drose
 * @date 2003-12-02
 */

#include "winStatsMonitor.h"
#include "winStatsServer.h"
#include "winStatsStripChart.h"
#include "winStatsPianoRoll.h"
#include "winStatsChartMenu.h"
#include "winStatsMenuId.h"
#include "pStatGraph.h"
#include "pStatCollectorDef.h"
#include "indent.h"

bool WinStatsMonitor::_window_class_registered = false;
const char * const WinStatsMonitor::_window_class_name = "monitor";

/**
 *
 */
WinStatsMonitor::
WinStatsMonitor(WinStatsServer *server) : PStatMonitor(server) {
  _window = 0;
  _menu_bar = 0;
  _options_menu = 0;

  // These will be filled in later when the menu is created.
  _time_units = 0;
  _scroll_speed = 0.0;
  _pause = false;
}

/**
 *
 */
WinStatsMonitor::
~WinStatsMonitor() {
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

#ifdef DEVELOP_WINSTATS
  // For Winstats developers, exit when the first monitor closes.
  exit(0);
#endif
}

/**
 * Should be redefined to return a descriptive name for the type of
 * PStatsMonitor this is.
 */
std::string WinStatsMonitor::
get_monitor_name() {
  return "WinStats";
}

/**
 * Called after the monitor has been fully set up.  At this time, it will have
 * a valid _client_data pointer, and things like is_alive() and close() will
 * be meaningful.  However, we may not yet know who we're connected to
 * (is_client_known() may return false), and we may not know anything about
 * the threads or collectors we're about to get data on.
 */
void WinStatsMonitor::
initialized() {
}

/**
 * Called when the "hello" message has been received from the client.  At this
 * time, the client's hostname and program name will be known.
 */
void WinStatsMonitor::
got_hello() {
  create_window();
  open_strip_chart(0, 0, false);
}

/**
 * Like got_hello(), this is called when the "hello" message has been received
 * from the client.  At this time, the client's hostname and program name will
 * be known.  However, the client appears to be an incompatible version and
 * the connection will be terminated; the monitor should issue a message to
 * that effect.
 */
void WinStatsMonitor::
got_bad_version(int client_major, int client_minor,
                int server_major, int server_minor) {
  std::ostringstream str;
  str << "Unable to honor connection attempt from "
      << get_client_progname() << " on " << get_client_hostname()
      << ": unsupported PStats version "
      << client_major << "." << client_minor;

  if (server_minor == 0) {
    str << " (server understands version " << server_major
        << "." << server_minor << " only).";
  } else {
    str << " (server understands versions " << server_major
        << ".0 through " << server_major << "." << server_minor << ").";
  }

  std::string message = str.str();
  MessageBox(nullptr, message.c_str(), "Bad version",
             MB_OK | MB_ICONINFORMATION | MB_SETFOREGROUND);
}

/**
 * Called whenever a new Collector definition is received from the client.
 * Generally, the client will send all of its collectors over shortly after
 * connecting, but there's no guarantee that they will all be received before
 * the first frames are received.  The monitor should be prepared to accept
 * new Collector definitions midstream.
 */
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

/**
 * Called whenever a new Thread definition is received from the client.
 * Generally, the client will send all of its threads over shortly after
 * connecting, but there's no guarantee that they will all be received before
 * the first frames are received.  The monitor should be prepared to accept
 * new Thread definitions midstream.
 */
void WinStatsMonitor::
new_thread(int thread_index) {
  WinStatsChartMenu *chart_menu = new WinStatsChartMenu(this, thread_index);
  chart_menu->add_to_menu_bar(_menu_bar, MI_frame_rate_label);
  _chart_menus.push_back(chart_menu);
  DrawMenuBar(_window);
}

/**
 * Called as each frame's data is made available.  There is no guarantee the
 * frames will arrive in order, or that all of them will arrive at all.  The
 * monitor should be prepared to accept frames received out-of-order or
 * missing.
 */
void WinStatsMonitor::
new_data(int thread_index, int frame_number) {
  Graphs::iterator gi;
  for (gi = _graphs.begin(); gi != _graphs.end(); ++gi) {
    WinStatsGraph *graph = (*gi);
    graph->new_data(thread_index, frame_number);
  }
}


/**
 * Called whenever the connection to the client has been lost.  This is a
 * permanent state change.  The monitor should update its display to represent
 * this, and may choose to close down automatically.
 */
void WinStatsMonitor::
lost_connection() {
  nout << "Lost connection to " << get_client_hostname() << "\n";

  if (_window) {
    DestroyWindow(_window);
    _window = 0;
  }
}

/**
 * If has_idle() returns true, this will be called periodically to allow the
 * monitor to update its display or whatever it needs to do.
 */
void WinStatsMonitor::
idle() {
  // Check if any of our chart menus need updating.
  ChartMenus::iterator mi;
  for (mi = _chart_menus.begin(); mi != _chart_menus.end(); ++mi) {
    (*mi)->check_update();
  }

  // Update the frame rate label from the main thread (thread 0).
  const PStatThreadData *thread_data = get_client_data()->get_thread_data(0);
  double frame_rate = thread_data->get_frame_rate();
  if (frame_rate != 0.0f) {
    char buffer[128];
    sprintf(buffer, "%0.1f ms / %0.1f Hz", 1000.0f / frame_rate, frame_rate);

    MENUITEMINFO mii;
    memset(&mii, 0, sizeof(mii));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STRING;
    mii.dwTypeData = buffer;
    SetMenuItemInfo(_menu_bar, MI_frame_rate_label, FALSE, &mii);
    DrawMenuBar(_window);
  }
}

/**
 * Should be redefined to return true if you want to redefine idle() and
 * expect it to be called.
 */
bool WinStatsMonitor::
has_idle() {
  return true;
}

/**
 * Called when the user guide bars have been changed.
 */
void WinStatsMonitor::
user_guide_bars_changed() {
  Graphs::iterator gi;
  for (gi = _graphs.begin(); gi != _graphs.end(); ++gi) {
    WinStatsGraph *graph = (*gi);
    graph->user_guide_bars_changed();
  }
}

/**
 * Returns the window handle to the monitor's window.
 */
HWND WinStatsMonitor::
get_window() const {
  return _window;
}

/**
 * Opens a new strip chart showing the indicated data.
 */
void WinStatsMonitor::
open_strip_chart(int thread_index, int collector_index, bool show_level) {
  WinStatsStripChart *graph =
    new WinStatsStripChart(this, thread_index, collector_index, show_level);
  add_graph(graph);

  graph->set_time_units(_time_units);
  graph->set_scroll_speed(_scroll_speed);
  graph->set_pause(_pause);
}

/**
 * Opens a new piano roll showing the indicated data.
 */
void WinStatsMonitor::
open_piano_roll(int thread_index) {
  WinStatsPianoRoll *graph = new WinStatsPianoRoll(this, thread_index);
  add_graph(graph);

  graph->set_time_units(_time_units);
  graph->set_scroll_speed(_scroll_speed);
  graph->set_pause(_pause);
}

/**
 * Returns the MenuDef properties associated with the indicated menu ID.  This
 * specifies what we expect to do when the given menu has been selected.
 */
const WinStatsMonitor::MenuDef &WinStatsMonitor::
lookup_menu(int menu_id) const {
  static MenuDef invalid(0, 0, false);
  int menu_index = menu_id - MI_new_chart;
  nassertr(menu_index >= 0 && menu_index < (int)_menu_by_id.size(), invalid);
  return _menu_by_id[menu_index];
}

/**
 * Returns the menu ID that is reserved for the indicated MenuDef properties.
 * If this is the first time these particular properties have been requested,
 * a new menu ID is returned; otherwise, the existing menu ID is returned.
 */
int WinStatsMonitor::
get_menu_id(const MenuDef &menu_def) {
  MenuByDef::iterator mi;
  mi = _menu_by_def.find(menu_def);
  if (mi != _menu_by_def.end()) {
    return (*mi).second;
  }

  // Slot a new id.
  int menu_id = (int)_menu_by_id.size() + MI_new_chart;
  _menu_by_id.push_back(menu_def);
  _menu_by_def[menu_def] = menu_id;

  return menu_id;
}

/**
 * Called when the user selects a new time units from the monitor pulldown
 * menu, this should adjust the units for all graphs to the indicated mask if
 * it is a time-based graph.
 */
void WinStatsMonitor::
set_time_units(int unit_mask) {
  _time_units = unit_mask;

  // First, change all of the open graphs appropriately.
  Graphs::iterator gi;
  for (gi = _graphs.begin(); gi != _graphs.end(); ++gi) {
    WinStatsGraph *graph = (*gi);
    graph->set_time_units(_time_units);
  }

  // Now change the checkmark on the pulldown menu.
  MENUITEMINFO mii;
  memset(&mii, 0, sizeof(mii));
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STATE;

  mii.fState = ((_time_units & PStatGraph::GBU_ms) != 0) ?
    MFS_CHECKED : MFS_UNCHECKED;
  SetMenuItemInfo(_options_menu, MI_time_ms, FALSE, &mii);

  mii.fState = ((_time_units & PStatGraph::GBU_hz) != 0) ?
    MFS_CHECKED : MFS_UNCHECKED;
  SetMenuItemInfo(_options_menu, MI_time_hz, FALSE, &mii);
}

/**
 * Called when the user selects a new scroll speed from the monitor pulldown
 * menu, this should adjust the speeds for all graphs to the indicated value.
 */
void WinStatsMonitor::
set_scroll_speed(double scroll_speed) {
  _scroll_speed = scroll_speed;

  // First, change all of the open graphs appropriately.
  Graphs::iterator gi;
  for (gi = _graphs.begin(); gi != _graphs.end(); ++gi) {
    WinStatsGraph *graph = (*gi);
    graph->set_scroll_speed(_scroll_speed);
  }

  // Now change the checkmark on the pulldown menu.
  MENUITEMINFO mii;
  memset(&mii, 0, sizeof(mii));
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STATE;

  mii.fState = IS_THRESHOLD_EQUAL(_scroll_speed, 1.0, 0.1) ?
    MFS_CHECKED : MFS_UNCHECKED;
  SetMenuItemInfo(_speed_menu, MI_speed_1, FALSE, &mii);

  mii.fState = IS_THRESHOLD_EQUAL(_scroll_speed, 2.0, 0.1) ?
    MFS_CHECKED : MFS_UNCHECKED;
  SetMenuItemInfo(_speed_menu, MI_speed_2, FALSE, &mii);

  mii.fState = IS_THRESHOLD_EQUAL(_scroll_speed, 3.0, 0.1) ?
    MFS_CHECKED : MFS_UNCHECKED;
  SetMenuItemInfo(_speed_menu, MI_speed_3, FALSE, &mii);

  mii.fState = IS_THRESHOLD_EQUAL(_scroll_speed, 6.0, 0.1) ?
    MFS_CHECKED : MFS_UNCHECKED;
  SetMenuItemInfo(_speed_menu, MI_speed_6, FALSE, &mii);

  mii.fState = IS_THRESHOLD_EQUAL(_scroll_speed, 12.0, 0.1) ?
    MFS_CHECKED : MFS_UNCHECKED;
  SetMenuItemInfo(_speed_menu, MI_speed_12, FALSE, &mii);
}

/**
 * Called when the user selects a pause on or pause off option from the menu.
 */
void WinStatsMonitor::
set_pause(bool pause) {
  _pause = pause;

  // First, change all of the open graphs appropriately.
  Graphs::iterator gi;
  for (gi = _graphs.begin(); gi != _graphs.end(); ++gi) {
    WinStatsGraph *graph = (*gi);
    graph->set_pause(_pause);
  }

  // Now change the checkmark on the pulldown menu.
  MENUITEMINFO mii;
  memset(&mii, 0, sizeof(mii));
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STATE;

  mii.fState = _pause ? MFS_CHECKED : MFS_UNCHECKED;
  SetMenuItemInfo(_speed_menu, MI_pause, FALSE, &mii);
}

/**
 * Adds the newly-created graph to the list of managed graphs.
 */
void WinStatsMonitor::
add_graph(WinStatsGraph *graph) {
  _graphs.insert(graph);
}

/**
 * Deletes the indicated graph.
 */
void WinStatsMonitor::
remove_graph(WinStatsGraph *graph) {
  Graphs::iterator gi = _graphs.find(graph);
  if (gi != _graphs.end()) {
    _graphs.erase(gi);
    delete graph;
  }
}

/**
 * Creates the window for this monitor.
 */
void WinStatsMonitor::
create_window() {
  if (_window) {
    return;
  }

  HINSTANCE application = GetModuleHandle(nullptr);
  register_window_class(application);

  _menu_bar = CreateMenu();

  setup_options_menu();
  setup_speed_menu();
  setup_frame_rate_label();

  ChartMenus::iterator mi;
  for (mi = _chart_menus.begin(); mi != _chart_menus.end(); ++mi) {
    (*mi)->add_to_menu_bar(_menu_bar, MI_frame_rate_label);
  }

  _window_title = get_client_progname() + " on " + get_client_hostname();
  DWORD window_style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN |
    WS_CLIPSIBLINGS | WS_VISIBLE;

  _window =
    CreateWindow(_window_class_name, _window_title.c_str(), window_style,
                 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                 nullptr, _menu_bar, application, 0);
  if (!_window) {
    nout << "Could not create monitor window!\n";
    exit(1);
  }

  SetWindowLongPtr(_window, 0, (LONG_PTR)this);

  // For some reason, SW_SHOWNORMAL doesn't always work, but SW_RESTORE seems
  // to.
  ShowWindow(_window, SW_RESTORE);
  SetForegroundWindow(_window);
}

/**
 * Creates the "Options" pulldown menu.
 */
void WinStatsMonitor::
setup_options_menu() {
  _options_menu = CreatePopupMenu();

  MENUITEMINFO mii;
  memset(&mii, 0, sizeof(mii));
  mii.cbSize = sizeof(mii);

  mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_SUBMENU;
  mii.fType = MFT_STRING;
  mii.hSubMenu = _options_menu;

  // One day, when there is more than one option here, we will actually
  // present this to the user as the "Options" menu.  For now, the only option
  // we have is time units.  mii.dwTypeData = "Options";
  mii.dwTypeData = "Units";
  InsertMenuItem(_menu_bar, GetMenuItemCount(_menu_bar), TRUE, &mii);


  mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_CHECKMARKS | MIIM_STATE;
  mii.fType = MFT_STRING | MFT_RADIOCHECK;
  mii.hbmpChecked = nullptr;
  mii.hbmpUnchecked = nullptr;
  mii.fState = MFS_UNCHECKED;
  mii.wID = MI_time_ms;
  mii.dwTypeData = "ms";
  InsertMenuItem(_options_menu, GetMenuItemCount(_options_menu), TRUE, &mii);

  mii.wID = MI_time_hz;
  mii.dwTypeData = "Hz";
  InsertMenuItem(_options_menu, GetMenuItemCount(_options_menu), TRUE, &mii);

  set_time_units(PStatGraph::GBU_ms);
}

/**
 * Creates the "Speed" pulldown menu.
 */
void WinStatsMonitor::
setup_speed_menu() {
  _speed_menu = CreatePopupMenu();

  MENUITEMINFO mii;
  memset(&mii, 0, sizeof(mii));
  mii.cbSize = sizeof(mii);

  mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_SUBMENU;
  mii.fType = MFT_STRING;
  mii.hSubMenu = _speed_menu;
  mii.dwTypeData = "Speed";
  InsertMenuItem(_menu_bar, GetMenuItemCount(_menu_bar), TRUE, &mii);


  mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_CHECKMARKS | MIIM_STATE;
  mii.fType = MFT_STRING | MFT_RADIOCHECK;
  mii.hbmpChecked = nullptr;
  mii.hbmpUnchecked = nullptr;
  mii.fState = MFS_UNCHECKED;
  mii.wID = MI_speed_1;
  mii.dwTypeData = "1";
  InsertMenuItem(_speed_menu, GetMenuItemCount(_speed_menu), TRUE, &mii);

  mii.wID = MI_speed_2;
  mii.dwTypeData = "2";
  InsertMenuItem(_speed_menu, GetMenuItemCount(_speed_menu), TRUE, &mii);

  mii.wID = MI_speed_3;
  mii.dwTypeData = "3";
  InsertMenuItem(_speed_menu, GetMenuItemCount(_speed_menu), TRUE, &mii);

  mii.wID = MI_speed_6;
  mii.dwTypeData = "6";
  InsertMenuItem(_speed_menu, GetMenuItemCount(_speed_menu), TRUE, &mii);

  mii.wID = MI_speed_12;
  mii.dwTypeData = "12";
  InsertMenuItem(_speed_menu, GetMenuItemCount(_speed_menu), TRUE, &mii);

  mii.fMask = MIIM_FTYPE;
  mii.fType = MFT_SEPARATOR;
  InsertMenuItem(_speed_menu, GetMenuItemCount(_speed_menu), TRUE, &mii);

  mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_CHECKMARKS | MIIM_STATE;
  mii.fType = MFT_STRING;
  mii.wID = MI_pause;
  mii.dwTypeData = "pause";
  InsertMenuItem(_speed_menu, GetMenuItemCount(_speed_menu), TRUE, &mii);

  set_scroll_speed(3);
  set_pause(false);
}

/**
 * Creates the frame rate label on the right end of the menu bar.  This is
 * used as a text label to display the main thread's frame rate to the user,
 * although it is implemented as a right-justified toplevel menu item that
 * doesn't open to anything.
 */
void WinStatsMonitor::
setup_frame_rate_label() {
  MENUITEMINFO mii;
  memset(&mii, 0, sizeof(mii));
  mii.cbSize = sizeof(mii);

  mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID;
  mii.fType = MFT_STRING | MFT_RIGHTJUSTIFY;
  mii.wID = MI_frame_rate_label;
  mii.dwTypeData = "";
  InsertMenuItem(_menu_bar, GetMenuItemCount(_menu_bar), TRUE, &mii);
}

/**
 * Registers the window class for the monitor window, if it has not already
 * been registered.
 */
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
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
  wc.lpszMenuName = nullptr;
  wc.lpszClassName = _window_class_name;

  // Reserve space to associate the this pointer with the window.
  wc.cbWndExtra = sizeof(WinStatsMonitor *);

  if (!RegisterClass(&wc)) {
    nout << "Could not register monitor window class!\n";
    exit(1);
  }

  _window_class_registered = true;
}

/**
 *
 */
LONG WINAPI WinStatsMonitor::
static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  WinStatsMonitor *self = (WinStatsMonitor *)GetWindowLongPtr(hwnd, 0);
  if (self != nullptr && self->_window == hwnd) {
    return self->window_proc(hwnd, msg, wparam, lparam);
  } else {
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
}

/**
 *
 */
LONG WinStatsMonitor::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_DESTROY:
    close();
    break;

  case WM_COMMAND:
    if (HIWORD(wparam) <= 1) {
      int menu_id = LOWORD(wparam);
      handle_menu_command(menu_id);
      return 0;
    }
    break;

  default:
    break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}

/**
 *
 */
void WinStatsMonitor::
handle_menu_command(int menu_id) {
  switch (menu_id) {
  case MI_none:
    break;

  case MI_time_ms:
    set_time_units(PStatGraph::GBU_ms);
    break;

  case MI_time_hz:
    set_time_units(PStatGraph::GBU_hz);
    break;

  case MI_speed_1:
    set_scroll_speed(1);
    break;

  case MI_speed_2:
    set_scroll_speed(2);
    break;

  case MI_speed_3:
    set_scroll_speed(3);
    break;

  case MI_speed_6:
    set_scroll_speed(6);
    break;

  case MI_speed_12:
    set_scroll_speed(12);
    break;

  case MI_pause:
    set_pause(!_pause);
    break;

  default:
    if (menu_id >= MI_new_chart) {
      const MenuDef &menu_def = lookup_menu(menu_id);
      if (menu_def._collector_index < 0) {
        open_piano_roll(menu_def._thread_index);
      } else {
        open_strip_chart(menu_def._thread_index, menu_def._collector_index,
                         menu_def._show_level);
      }
    }
  }
}
