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
#include "winStatsFlameGraph.h"
#include "winStatsTimeline.h"
#include "winStatsChartMenu.h"
#include "winStatsMenuId.h"
#include "pStatFrameData.h"
#include "pStatGraph.h"
#include "pStatCollectorDef.h"

#include "convert_srgb.h"

#include <algorithm>

#include <commctrl.h>
#include <commdlg.h>
#include <uxtheme.h>

/**
 *
 */
WinStatsMonitor::
WinStatsMonitor(WinStatsServer *server) : PStatMonitor(server) {
  _window = server->get_window();
  _menu_bar = server->get_menu_bar();
  _status_bar = server->get_status_bar();

  // These will be filled in later when the menu is created.
  _scroll_speed = 0.0;
  _pause = false;

  setup_speed_menu();
  setup_frame_rate_label();
}

/**
 *
 */
WinStatsMonitor::
~WinStatsMonitor() {
  close();
}

/**
 * Closes the client connection if it is active.
 */
void WinStatsMonitor::
close() {
  PStatMonitor::close();

  remove_all_graphs();

  RemoveMenu(_menu_bar, 2, MF_BYPOSITION);
  RemoveMenu(_menu_bar, 2, MF_BYPOSITION);

  for (WinStatsChartMenu *chart_menu : _chart_menus) {
    RemoveMenu(_menu_bar, 2, MF_BYPOSITION);
    delete chart_menu;
  }
  _chart_menus.clear();

  DrawMenuBar(_window);
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
  for (WinStatsGraph *graph : _graphs) {
    graph->new_collector(collector_index);
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

  if (thread_index == 0) {
    update_status_bar();
  }
}

/**
 * Called when a thread should be removed from the list of threads.
 */
void WinStatsMonitor::
remove_thread(int thread_index) {
  for (ChartMenus::iterator it = _chart_menus.begin(); it != _chart_menus.end(); ++it) {
    WinStatsChartMenu *chart_menu = *it;
    if (chart_menu->get_thread_index() == thread_index) {
      chart_menu->remove_from_menu_bar(_menu_bar);
      delete chart_menu;
      _chart_menus.erase(it);
      return;
    }
  }
}

/**
 * Called as each frame's data is made available.  There is no guarantee the
 * frames will arrive in order, or that all of them will arrive at all.  The
 * monitor should be prepared to accept frames received out-of-order or
 * missing.
 */
void WinStatsMonitor::
new_data(int thread_index, int frame_number) {
  PStatMonitor::new_data(thread_index, frame_number);

  for (WinStatsGraph *graph : _graphs) {
    graph->new_data(thread_index, frame_number);
  }

  if (thread_index == 0) {
    update_status_bar();
  }

  if (!_have_data) {
    open_default_graphs();
    _have_data = true;
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
}

/**
 * If has_idle() returns true, this will be called periodically to allow the
 * monitor to update its display or whatever it needs to do.
 */
void WinStatsMonitor::
idle() {
  // Check if any of our chart menus need updating.
  for (WinStatsChartMenu *chart_menu : _chart_menus) {
    chart_menu->check_update();
  }

  // Update the frame rate label from the main thread (thread 0).
  const PStatThreadData *thread_data = get_client_data()->get_thread_data(0);
  double frame_rate = thread_data->get_frame_rate();
  if (frame_rate != 0.0f) {
    // The leading tab centers the text in the status bar.
    char buffer[128];
    sprintf(buffer, "\t%0.1f ms / %0.1f Hz", 1000.0f / frame_rate, frame_rate);

    MENUITEMINFO mii;
    memset(&mii, 0, sizeof(mii));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STRING;
    mii.dwTypeData = buffer + 1; // chop off leading tab
    SetMenuItemInfo(_menu_bar, MI_frame_rate_label, FALSE, &mii);
    DrawMenuBar(_window);

    if (_status_bar) {
      SendMessage(_status_bar, WM_SETTEXT, 0, (LPARAM)buffer);
    }
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
  for (WinStatsGraph *graph : _graphs) {
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
 * Returns the font that should be used for rendering text.
 */
HFONT WinStatsMonitor::
get_font() const {
  return ((WinStatsServer *)_server)->get_font();
}

/**
 * Returns the system DPI scaling as a fraction where 4 = no scaling.
 */
int WinStatsMonitor::
get_pixel_scale() const {
  return ((WinStatsServer *)_server)->get_pixel_scale();
}

/**
 * Returns an amount by which to offset the next window position.
 */
POINT WinStatsMonitor::
get_new_window_pos() {
  WinStatsServer *server = (WinStatsServer *)_server;
  int offset = _graphs.size() * 10 * server->get_pixel_scale();
  POINT client_origin = server->get_client_origin();
  POINT pt;
  pt.x = offset + client_origin.x;
  pt.y = offset + client_origin.y;
  return pt;
}

/**
 * Opens a new timeline.
 */
PStatGraph *WinStatsMonitor::
open_timeline() {
  WinStatsTimeline *graph = new WinStatsTimeline(this);
  add_graph(graph);
  return graph;
}

/**
 * Opens a new strip chart showing the indicated data.
 */
PStatGraph *WinStatsMonitor::
open_strip_chart(int thread_index, int collector_index, bool show_level) {
  WinStatsStripChart *graph =
    new WinStatsStripChart(this, thread_index, collector_index, show_level);
  add_graph(graph);
  return graph;
}

/**
 * Opens a new flame graph showing the indicated data.
 */
PStatGraph *WinStatsMonitor::
open_flame_graph(int thread_index, int collector_index, int frame_number) {
  WinStatsFlameGraph *graph = new WinStatsFlameGraph(this, thread_index, collector_index, frame_number);
  add_graph(graph);
  return graph;
}

/**
 * Opens a new piano roll showing the indicated data.
 */
PStatGraph *WinStatsMonitor::
open_piano_roll(int thread_index) {
  WinStatsPianoRoll *graph = new WinStatsPianoRoll(this, thread_index);
  add_graph(graph);
  return graph;
}

/**
 * Opens a dialog to change the given collector color.
 */
void WinStatsMonitor::
choose_collector_color(int collector_index) {
  const LRGBColor &current = get_collector_color(collector_index);
  static COLORREF custom_colors[16] = {0};

  CHOOSECOLORA cc = {
    sizeof(CHOOSECOLORA),
    _window,
    0,
    RGB(encode_sRGB_uchar((float)current[0]),
        encode_sRGB_uchar((float)current[1]),
        encode_sRGB_uchar((float)current[2])),
    (LPDWORD)custom_colors,
    CC_FULLOPEN | CC_RGBINIT,
    0,
    nullptr,
    nullptr,
  };

  if (ChooseColorA(&cc)) {
    LRGBColor result(
      decode_sRGB_float(GetRValue(cc.rgbResult)),
      decode_sRGB_float(GetGValue(cc.rgbResult)),
      decode_sRGB_float(GetBValue(cc.rgbResult)));

    set_collector_color(collector_index, result);

    for (WinStatsGraph *graph : _graphs) {
      graph->reset_collector_color(collector_index);
    }
  }
}

/**
 * Resets the color of the given collector to the default.
 */
void WinStatsMonitor::
reset_collector_color(int collector_index) {
  clear_collector_color(collector_index);

  for (WinStatsGraph *graph : _graphs) {
    graph->reset_collector_color(collector_index);
  }
}

/**
 * Returns the MenuDef properties associated with the indicated menu ID.  This
 * specifies what we expect to do when the given menu has been selected.
 */
const WinStatsMonitor::MenuDef &WinStatsMonitor::
lookup_menu(int menu_id) const {
  static MenuDef invalid(0, 0, CT_strip_chart, false);
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
  for (WinStatsGraph *graph : _graphs) {
    graph->set_time_units(unit_mask);
  }
}

/**
 * Called when the user selects a new scroll speed from the monitor pulldown
 * menu, this should adjust the speeds for all graphs to the indicated value.
 */
void WinStatsMonitor::
set_scroll_speed(double scroll_speed) {
  _scroll_speed = scroll_speed;

  // First, change all of the open graphs appropriately.
  for (WinStatsGraph *graph : _graphs) {
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
  for (WinStatsGraph *graph : _graphs) {
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

  graph->set_time_units(((WinStatsServer *)_server)->get_time_units());
  graph->set_scroll_speed(_scroll_speed);
  graph->set_pause(_pause);
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
 * Deletes all open graphs.
 */
void WinStatsMonitor::
remove_all_graphs() {
  for (WinStatsGraph *graph : _graphs) {
    delete graph;
  }
  _graphs.clear();
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
 * Updates the status bar.
 */
void WinStatsMonitor::
update_status_bar() {
  const PStatClientData *client_data = get_client_data();
  if (client_data == nullptr) {
    return;
  }

  const PStatThreadData *thread_data = get_client_data()->get_thread_data(0);
  if (thread_data == nullptr || thread_data->is_empty()) {
    return;
  }
  int frame_number = thread_data->get_latest_frame_number();
  const PStatFrameData &frame_data = thread_data->get_latest_frame();

  // Gather the top-level collector list.
  pvector<std::string> parts;
  pvector<int> collectors;
  size_t total_chars = 0;

  int num_toplevel_collectors = client_data->get_num_toplevel_collectors();
  for (int tc = 0; tc < num_toplevel_collectors; tc++) {
    int collector = client_data->get_toplevel_collector(tc);
    if (client_data->has_collector(collector) &&
        client_data->get_collector_has_level(collector, 0)) {
      PStatView &view = get_level_view(collector, 0);
      view.set_to_frame(frame_data);
      double value = view.get_net_value();
      if (value == 0.0) {
        // Don't include it unless we've included it before.
        if (std::find(_status_bar_collectors.begin(), _status_bar_collectors.end(), collector) == _status_bar_collectors.end()) {
          continue;
        }
      }

      // Add the value for other threads that have this collector.
      for (int thread_index = 1; thread_index < client_data->get_num_threads(); ++thread_index) {
        PStatView &view = get_level_view(collector, thread_index);
        view.set_to_frame(frame_number);
        value += view.get_net_value();
      }

      const PStatCollectorDef &def = client_data->get_collector_def(collector);
      std::string text = "\t" + def._name;
      text += ": " + PStatGraph::format_number(value, PStatGraph::GBU_named | PStatGraph::GBU_show_units, def._level_units);
      total_chars += text.size();
      parts.push_back(text);
      collectors.push_back(collector);
    }
  }

  size_t cur_size = _status_bar_collectors.size();
  _status_bar_collectors = std::move(collectors);

  // Allocate an array for holding the right edge coordinates.
  HLOCAL hloc = LocalAlloc(LHND, sizeof(int) * (parts.size() + 1));
  PINT sizes = (PINT)LocalLock(hloc);

  int pixel_scale = get_pixel_scale();

  // Allocate the left-most slot for the framerate indicator.
  double offset = 28.0 * pixel_scale;
  sizes[0] = (int)(offset + 0.5);

  if (!parts.empty()) {
    // Distribute the sizes roughly based on the number of characters.  It's not
    // as good as measuring the text, but it's good enough.
    RECT rect;
    double width_per_char = 0;
    GetClientRect(_status_bar, &rect);
    // Leave room for the grip.
    rect.right -= pixel_scale * 4;
    width_per_char = (rect.right - rect.left - offset) / (double)total_chars;

    // If we get below a minimum width, start chopping parts off.
    while (!parts.empty() && width_per_char < pixel_scale * 1.5) {
      total_chars -= parts.back().size();
      parts.pop_back();
      width_per_char = (rect.right - rect.left - offset) / (double)total_chars;
    }

    if (!parts.empty()) {
      for (size_t i = 0; i < parts.size(); ++i) {
        offset += (parts[i].size()) * width_per_char;
        sizes[i + 1] = (int)(offset + 0.5);
      }
    } else {
      // No room for any collectors; the framerate can take up the whole width.
      sizes[0] = rect.right - rect.left;
    }
  }

  SendMessage(_status_bar, SB_SETPARTS, (WPARAM)(parts.size() + 1), (LPARAM)sizes);

  LocalUnlock(hloc);
  LocalFree(hloc);

  for (size_t i = 0; i < parts.size(); ++i) {
    SendMessage(_status_bar, SB_SETTEXT, i + 1, (LPARAM)parts[i].c_str());
  }
}

/**
 * Called when someone right-clicks on a part of the status bar.
 */
void WinStatsMonitor::
show_popup_menu(int collector) {
  POINT point;
  if (!GetCursorPos(&point)) {
    return;
  }

  const PStatClientData *client_data = get_client_data();
  if (client_data == nullptr) {
    return;
  }

  PStatView &level_view = get_level_view(collector, 0);
  const PStatViewLevel *view_level = level_view.get_top_level();
  int num_children = view_level->get_num_children();
  if (num_children == 0) {
    return;
  }

  HMENU popup = CreatePopupMenu();

  // Reverse the order since the menus are listed from the top down; we want
  // to be visually consistent with the graphs, which list these labels from
  // the bottom up.
  for (int c = num_children - 1; c >= 0; c--) {
    const PStatViewLevel *child_level = view_level->get_child(c);

    int child_collector = child_level->get_collector();
    MenuDef menu_def(0, child_collector, CT_strip_chart, true);
    int menu_id = get_menu_id(menu_def);

    double value = child_level->get_net_value();

    const PStatCollectorDef &def = client_data->get_collector_def(child_collector);
    std::string text = def._name;
    text += ": " + PStatGraph::format_number(value, PStatGraph::GBU_named | PStatGraph::GBU_show_units, def._level_units);
    AppendMenu(popup, MF_STRING, menu_id, text.c_str());
  }

  TrackPopupMenu(popup, TPM_LEFTBUTTON, point.x, point.y, 0, _window, nullptr);
}

/**
 * Called when a graph window is iconified or moved in the iconified state.
 */
void WinStatsMonitor::
calc_iconic_graph_window_pos(WinStatsGraph *moved_graph, int &x, int &y) {
  MINIMIZEDMETRICS metrics;
  metrics.cbSize = sizeof(metrics);
  SystemParametersInfoA(SPI_GETMINIMIZEDMETRICS, 0, &metrics, 0);

  int height = GetThemeSysSize(nullptr, SM_CYSIZE) + GetThemeSysSize(nullptr, SM_CXPADDEDBORDER) * 2;

  RECT client_rect;
  GetClientRect(_window, &client_rect);
  MapWindowPoints(_window, nullptr, (POINT *)&client_rect, 2);

  // Remove the status bar from the client rectangle.
  RECT status_bar_rect;
  GetWindowRect(_status_bar, &status_bar_rect);
  client_rect.bottom -= (status_bar_rect.bottom - status_bar_rect.top);

  int iconic_offset = 0;

  for (WinStatsGraph *graph : _graphs) {
    RECT child_rect;
    HWND window = graph->get_window();
    if (graph == moved_graph) {
      x = client_rect.left + iconic_offset;
      y = client_rect.bottom - height;
      iconic_offset += metrics.iWidth + metrics.iHorzGap;
    }
    else if (IsIconic(window) && GetWindowRect(window, &child_rect)) {
      // Keep it glued to the bottom-left corner of the parent window.
      child_rect.left = client_rect.left + iconic_offset;
      child_rect.top = client_rect.bottom - height;
      iconic_offset += metrics.iWidth + metrics.iHorzGap;

      SetWindowPos(window, 0, child_rect.left, child_rect.top, 0, 0,
                   SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOSENDCHANGING);
    }
  }
}

/**
 * Called when the server window is moved.
 */
void WinStatsMonitor::
handle_window_moved(const RECT &client_rect, int delta_x, int delta_y) {
  if (_graphs.empty()) {
    return;
  }

  MINIMIZEDMETRICS metrics;
  metrics.cbSize = sizeof(metrics);
  SystemParametersInfoA(SPI_GETMINIMIZEDMETRICS, 0, &metrics, 0);

  int height = GetThemeSysSize(nullptr, SM_CYSIZE) + GetThemeSysSize(nullptr, SM_CXPADDEDBORDER) * 2;

  int iconic_offset = 0;

  for (WinStatsGraph *graph : _graphs) {
    WINDOWPLACEMENT wp;
    HWND window = graph->get_window();
    if (GetWindowPlacement(window, &wp)) {
      if (wp.showCmd == SW_SHOWMINIMIZED) {
        // Keep the minimized window title bar glued to the bottom-left
        // corner of the parent window.
        wp.flags |= WPF_SETMINPOSITION;
        wp.ptMinPosition.x = client_rect.left + iconic_offset;
        wp.ptMinPosition.y = client_rect.bottom - height;
        iconic_offset += metrics.iWidth + metrics.iHorzGap;
      }

      // Move the "restored" window (even when it's currently min/maximized).
      wp.rcNormalPosition.left += delta_x;
      wp.rcNormalPosition.top += delta_y;
      wp.rcNormalPosition.right += delta_x;
      wp.rcNormalPosition.bottom += delta_y;
      SetWindowPlacement(window, &wp);
    }
  }
}

/**
 * Called when a menu item is clicked.
 */
void WinStatsMonitor::
handle_menu_command(int menu_id) {
  switch (menu_id) {
  case MI_none:
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

  case MI_graphs_close_all:
    remove_all_graphs();
    break;

  case MI_graphs_reopen_default:
    remove_all_graphs();
    open_default_graphs();
    break;

  case MI_graphs_save_default:
    save_default_graphs();
    break;

  default:
    if (menu_id >= MI_new_chart) {
      const MenuDef &menu_def = lookup_menu(menu_id);
      switch (menu_def._chart_type) {
      case CT_timeline:
        open_timeline();
        break;

      case CT_strip_chart:
        open_strip_chart(menu_def._thread_index, menu_def._collector_index,
                         menu_def._show_level);
        break;

      case CT_flame_graph:
        open_flame_graph(menu_def._thread_index, menu_def._collector_index);
        break;

      case CT_piano_roll:
        open_piano_roll(menu_def._thread_index);
        break;
      }
    }
  }
}

/**
 * Called when a status bar item is double-clicked.
 */
void WinStatsMonitor::
handle_status_bar_click(int item) {
  if (item == 0) {
    open_strip_chart(0, 0, false);
  }
  else if (item >= 1 && item <= _status_bar_collectors.size()) {
    int collector = _status_bar_collectors[item - 1];
    open_strip_chart(0, collector, true);

    // Also open a strip chart for other threads with data for this
    // collector.
    const PStatClientData *client_data = get_client_data();
    for (int thread_index = 1; thread_index < client_data->get_num_threads(); ++thread_index) {
      PStatView &view = get_level_view(collector, thread_index);
      if (view.get_net_value() > 0.0) {
        open_strip_chart(thread_index, collector, true);
      }
    }
  }
}

/**
 * Called when a status bar item is right-clicked.
 */
void WinStatsMonitor::
handle_status_bar_popup(int item) {
  if (item >= 1 && item <= _status_bar_collectors.size()) {
    int collector = _status_bar_collectors[item - 1];
    show_popup_menu(collector);
  }
}
