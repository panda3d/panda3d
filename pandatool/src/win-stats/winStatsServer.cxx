/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file winStatsServer.cxx
 * @author drose
 * @date 2003-12-02
 */

#include "winStatsServer.h"
#include "winStatsMenuId.h"
#include "winStatsMonitor.h"
#include "pandaVersion.h"
#include "pStatGraph.h"
#include "config_pstatclient.h"

#include <commctrl.h>
#include <commdlg.h>

bool WinStatsServer::_window_class_registered = false;
const char *const WinStatsServer::_window_class_name = "server";

/**
 *
 */
WinStatsServer::
WinStatsServer() : _port(pstats_port) {
  set_program_brief("Windows PStats client");
  add_option("p", "port", 0, "", &ProgramBase::dispatch_int, nullptr, &_port);

  _last_session = Filename::expand_from(
    "$USER_APPDATA/Panda3D-" PANDA_ABI_VERSION_STR "/last-session.pstats");
  _last_session.set_binary();

  // Create the fonts used for rendering the UI.
  NONCLIENTMETRICS metrics = {0};
  metrics.cbSize = sizeof(NONCLIENTMETRICS);
  if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, 0, &metrics, 0)) {
    _font = CreateFontIndirect(&metrics.lfMenuFont);
  } else {
    _font = (HFONT)GetStockObject(ANSI_VAR_FONT);
  }

  create_window();
}

/**
 * Does something with the additional arguments on the command line (after all
 * the -options have been parsed).  Returns true if the arguments are good,
 * false otherwise.
 */
bool WinStatsServer::
handle_args(ProgramBase::Args &args) {
  if (args.empty()) {
    new_session();
    return true;
  }
  else if (args.size() == 1) {
    Filename fn = Filename::from_os_specific(args[0]);
    fn.set_binary();
    WinStatsMonitor *monitor = new WinStatsMonitor(this);
    if (!monitor->read(fn)) {
      delete monitor;

      std::ostringstream stream;
      stream << "Failed to load session file: " << fn;
      std::string str = stream.str();
      MessageBox(_window, str.c_str(), "PStats Error",
                 MB_OK | MB_ICONEXCLAMATION);
      return true;
    }

    // Enable the "New Session", "Save Session" and "Close Session" menu items.
    MENUITEMINFO mii;
    memset(&mii, 0, sizeof(mii));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    mii.fState = MFS_ENABLED;
    SetMenuItemInfoA(_session_menu, MI_session_new, FALSE, &mii);
    SetMenuItemInfoA(_session_menu, MI_session_save, FALSE, &mii);
    SetMenuItemInfoA(_session_menu, MI_session_close, FALSE, &mii);
    SetMenuItemInfoA(_session_menu, MI_session_export_json, FALSE, &mii);

    _monitor = monitor;
    return true;
  }
  else {
    nout << "At most one filename may be specified on the command-line.\n";
    return false;
  }
}

/**
 *
 */
PStatMonitor *WinStatsServer::
make_monitor(const NetAddress &address) {
  // Enable the "New Session", "Save Session" and "Close Session" menu items.
  MENUITEMINFO mii;
  memset(&mii, 0, sizeof(mii));
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STATE;
  mii.fState = MFS_ENABLED;
  SetMenuItemInfoA(_session_menu, MI_session_new, FALSE, &mii);
  SetMenuItemInfoA(_session_menu, MI_session_save, FALSE, &mii);
  SetMenuItemInfoA(_session_menu, MI_session_close, FALSE, &mii);
  SetMenuItemInfoA(_session_menu, MI_session_export_json, FALSE, &mii);

  std::ostringstream strm;
  strm << "PStats Server (connected to " << address << ")";
  std::string title = strm.str();
  SetWindowTextA(_window, title.c_str());

  _monitor = new WinStatsMonitor(this);
  return _monitor;
}

/**
 * Called when connection has been lost.
 */
void WinStatsServer::
lost_connection(PStatMonitor *monitor) {
  if (_monitor != nullptr && !_monitor->_have_data) {
    // We didn't have any data yet.  Just silently restart the session.
    _monitor->close();
    _monitor = nullptr;
    if (new_session()) {
      return;
    }
  } else {
    // Store a backup now, in case PStats crashes or something.
    _last_session.make_dir();
    if (monitor->write(_last_session)) {
      nout << "Wrote to " << _last_session << "\n";
    } else {
      nout << "Failed to write to " << _last_session << "\n";
    }
  }

  stop_listening();

  SetWindowTextA(_window, "PStats Server (disconnected)");
}

/**
 * Starts a new session.
 */
bool WinStatsServer::
new_session() {
  if (!close_session()) {
    return false;
  }

  if (listen(_port)) {
    {
      std::ostringstream strm;
      strm << "PStats Server (listening on port " << _port << ")";
      std::string title = strm.str();
      SetWindowTextA(_window, title.c_str());
    }
    {
      std::ostringstream strm;
      strm << "Waiting for client to connect on port " << _port << "...";
      std::string title = strm.str();
      int part = -1;
      SendMessage(_status_bar, SB_SETPARTS, 1, (LPARAM)&part);
      SendMessage(_status_bar, WM_SETTEXT, 0, (LPARAM)title.c_str());
    }

    // Disable the "New Session" menu item.
    MENUITEMINFO mii;
    memset(&mii, 0, sizeof(mii));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    mii.fState = MFS_DISABLED;
    SetMenuItemInfoA(_session_menu, MI_session_new, FALSE, &mii);

    // Disable the "Save Session" menu item.
    mii.fState = MFS_DISABLED;
    SetMenuItemInfoA(_session_menu, MI_session_save, FALSE, &mii);

    // Enable the "Close Session" menu item.
    mii.fState = MFS_ENABLED;
    SetMenuItemInfoA(_session_menu, MI_session_close, FALSE, &mii);

    // Disable the "Export Session" menu item.
    mii.fState = MFS_DISABLED;
    SetMenuItemInfoA(_session_menu, MI_session_export_json, FALSE, &mii);

    return true;
  }

  SetWindowTextA(_window, "PStats Server");

  std::ostringstream stream;
  stream
    << "Unable to open port " << _port << ".  Try specifying a different "
    << "port number using pstats-port in your Config file or the -p option on "
    << "the command-line.";
  std::string str = stream.str();
  MessageBox(_window, str.c_str(), "PStats Error",
             MB_OK | MB_ICONEXCLAMATION);
  return false;
}

/**
 * Offers to open an existing session.
 */
bool WinStatsServer::
open_session() {
  if (!close_session()) {
    return false;
  }

  char buffer[4096];
  buffer[0] = '\0';

  OPENFILENAMEA ofn = {
    sizeof(OPENFILENAMEA),
    _window,
    nullptr,
    "PStats Session Files (*.pstats)\0*.pstats\0All Files (*.*)\0*.*\0",
    nullptr,
    0,
    0,
    buffer,
    sizeof(buffer),
    nullptr,
    0,
    nullptr,
    "Open Session",
    OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
    0,
  };

  if (GetOpenFileNameA(&ofn)) {
    Filename fn = Filename::from_os_specific(buffer);
    fn.set_binary();

    WinStatsMonitor *monitor = new WinStatsMonitor(this);
    if (!monitor->read(fn)) {
      delete monitor;

      std::ostringstream stream;
      stream << "Failed to load session file: " << fn;
      std::string str = stream.str();
      MessageBox(_window, str.c_str(), "PStats Error",
                 MB_OK | MB_ICONEXCLAMATION);
      return false;
    }
    _monitor = monitor;

    // Enable the "New Session", "Save Session" and "Close Session" menu items.
    MENUITEMINFO mii;
    memset(&mii, 0, sizeof(mii));
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE;
    mii.fState = MFS_ENABLED;
    SetMenuItemInfoA(_session_menu, MI_session_new, FALSE, &mii);
    SetMenuItemInfoA(_session_menu, MI_session_save, FALSE, &mii);
    SetMenuItemInfoA(_session_menu, MI_session_close, FALSE, &mii);
    SetMenuItemInfoA(_session_menu, MI_session_export_json, FALSE, &mii);

    return true;
  }

  return false;
}

/**
 * Opens the last session, if any.
 */
bool WinStatsServer::
open_last_session() {
  if (!close_session()) {
    return false;
  }

  Filename fn = _last_session;
  WinStatsMonitor *monitor = new WinStatsMonitor(this);
  if (!monitor->read(fn)) {
    delete monitor;

    std::ostringstream stream;
    stream << "Failed to load session file: " << fn;
    std::string str = stream.str();
    MessageBox(_window, str.c_str(), "PStats Error",
               MB_OK | MB_ICONEXCLAMATION);
    return false;
  }
  _monitor = monitor;

  // Enable the "New Session", "Save Session" and "Close Session" menu items.
  MENUITEMINFO mii;
  memset(&mii, 0, sizeof(mii));
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STATE;
  mii.fState = MFS_ENABLED;
  SetMenuItemInfoA(_session_menu, MI_session_new, FALSE, &mii);
  SetMenuItemInfoA(_session_menu, MI_session_save, FALSE, &mii);
  SetMenuItemInfoA(_session_menu, MI_session_close, FALSE, &mii);
  SetMenuItemInfoA(_session_menu, MI_session_export_json, FALSE, &mii);

  // If the file contained no graphs, open the default graphs.
  if (monitor->_graphs.empty()) {
    monitor->open_default_graphs();
  }

  return true;
}

/**
 * Offers to save the current session.
 */
bool WinStatsServer::
save_session() {
  nassertr_always(_monitor != nullptr, true);

  char buffer[4096];
  buffer[0] = '\0';

  OPENFILENAMEA ofn = {
    sizeof(OPENFILENAMEA),
    _window,
    0,
    "PStats Session Files\0*.pstats\0",
    nullptr,
    0,
    0,
    buffer,
    sizeof(buffer),
    nullptr,
    0,
    nullptr,
    "Save Session",
    OFN_OVERWRITEPROMPT,
    0,
    0,
    "pstats",
    0,
  };

  if (GetSaveFileNameA(&ofn)) {
    Filename fn = Filename::from_os_specific(buffer);
    fn.set_binary();

    if (!_monitor->write(fn)) {
      std::ostringstream stream;
      stream << "Failed to save session file: " << fn;
      std::string str = stream.str();
      MessageBox(_window, str.c_str(), "PStats Error",
                 MB_OK | MB_ICONEXCLAMATION);
      return false;
    }
    _monitor->get_client_data()->clear_dirty();
    return true;
  }

  return false;
}

/**
 * Offers to export the current session as a JSON file.
 */
bool WinStatsServer::
export_session() {
  nassertr_always(_monitor != nullptr, true);

  char buffer[4096];
  buffer[0] = '\0';

  OPENFILENAMEA ofn = {
    sizeof(OPENFILENAMEA),
    _window,
    0,
    "JSON files\0*.json\0",
    nullptr,
    0,
    0,
    buffer,
    sizeof(buffer),
    nullptr,
    0,
    nullptr,
    "Export Session",
    OFN_OVERWRITEPROMPT,
    0,
    0,
    "json",
    0,
  };

  if (GetSaveFileNameA(&ofn)) {
    Filename fn = Filename::from_os_specific(buffer);
    fn.set_text();

    std::ofstream stream;
    if (!fn.open_write(stream)) {
      std::ostringstream stream;
      stream << "Failed to open file for export: " << fn;
      std::string str = stream.str();
      MessageBox(_window, str.c_str(), "PStats Error",
                 MB_OK | MB_ICONEXCLAMATION);
      return false;
    }

    int pid = _monitor->get_client_pid();
    _monitor->get_client_data()->write_json(stream, std::max(0, pid));
    stream.close();
    return true;
  }

  return false;
}

/**
 * Closes the current session.
 */
bool WinStatsServer::
close_session() {
  bool wrote_last_session = false;

  if (_monitor != nullptr) {
    const PStatClientData *client_data = _monitor->get_client_data();
    if (client_data != nullptr && client_data->is_dirty()) {
      if (!_monitor->has_read_filename()) {
        _last_session.make_dir();
        if (_monitor->write(_last_session)) {
          nout << "Wrote to " << _last_session << "\n";
          wrote_last_session = true;
        }
        else {
          nout << "Failed to write to " << _last_session << "\n";
        }
      }

      int result = MessageBox(_window,
                              "Would you like to save the currently open session?",
                              "Unsaved Data", MB_YESNOCANCEL | MB_ICONQUESTION);
      if (result == IDCANCEL || (result == IDYES && !save_session())) {
        return false;
      }
    }

    _monitor->close();
    _monitor = nullptr;
  }

  stop_listening();

  SetWindowTextA(_window, "PStats Server");

  int part = -1;
  SendMessage(_status_bar, SB_SETPARTS, 1, (LPARAM)&part);
  SendMessage(_status_bar, WM_SETTEXT, 0, (LPARAM)"");

  // Enable the "New Session" menu item.
  MENUITEMINFO mii;
  memset(&mii, 0, sizeof(mii));
  mii.cbSize = sizeof(mii);
  mii.fMask = MIIM_STATE;
  mii.fState = MFS_ENABLED;
  SetMenuItemInfoA(_session_menu, MI_session_new, FALSE, &mii);

  if (wrote_last_session) {
    // And the "Open Last Session" menu item.
    mii.fState = MFS_ENABLED;
    SetMenuItemInfoA(_session_menu, MI_session_open_last, FALSE, &mii);
  }

  // Disable the "Save Session" menu item.
  mii.fState = MFS_DISABLED;
  SetMenuItemInfoA(_session_menu, MI_session_save, FALSE, &mii);

  // Disable the "Close Session" menu item.
  mii.fState = MFS_DISABLED;
  SetMenuItemInfoA(_session_menu, MI_session_close, FALSE, &mii);

  // Disable the "Export Session" menu item.
  mii.fState = MFS_DISABLED;
  SetMenuItemInfoA(_session_menu, MI_session_export_json, FALSE, &mii);

  return true;
}

/**
 * Returns the window handle to the server's window.
 */
HWND WinStatsServer::
get_window() const {
  return _window;
}

/**
 * Returns the menu handle to the server's menu bar.
 */
HMENU WinStatsServer::
get_menu_bar() const {
  return _menu_bar;
}

/**
 * Returns the window handle to the server's status bar.
 */
HWND WinStatsServer::
get_status_bar() const {
  return _status_bar;
}

/**
 * Returns the font that should be used for rendering text.
 */
HFONT WinStatsServer::
get_font() const {
  return _font;
}

/**
 * Returns the system DPI scaling as a fraction where 4 = no scaling.
 */
int WinStatsServer::
get_pixel_scale() const {
  return _pixel_scale;
}

/**
 * Returns the origin of the window's client area.
 */
POINT WinStatsServer::
get_client_origin() const {
  return _client_origin;
}

/**
 *
 */
int WinStatsServer::
get_time_units() const {
  return _time_units;
}


/**
 * Called when the user selects a new time units from the monitor pulldown
 * menu, this should adjust the units for all graphs to the indicated mask if
 * it is a time-based graph.
 */
void WinStatsServer::
set_time_units(int unit_mask) {
  _time_units = unit_mask;

  if (_monitor != nullptr) {
    _monitor->set_time_units(unit_mask);
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
 * Creates the window for this server.
 */
void WinStatsServer::
create_window() {
  HINSTANCE application = GetModuleHandle(nullptr);
  register_window_class(application);

  _menu_bar = CreateMenu();

  setup_session_menu();
  setup_options_menu();

  DWORD window_style = WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN |
    WS_CLIPSIBLINGS | WS_VISIBLE;

  _window =
    CreateWindow(_window_class_name, "PStats Server", window_style,
                 CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                 nullptr, _menu_bar, application, 0);
  if (!_window) {
    nout << "Could not create server window!\n";
    exit(1);
  }

  SetWindowLongPtr(_window, 0, (LONG_PTR)this);

  create_status_bar(application);

  // For some reason, SW_SHOWNORMAL doesn't always work, but SW_RESTORE seems
  // to.
  ShowWindow(_window, SW_RESTORE);
  SetForegroundWindow(_window);

  HDC dc = GetDC(_window);
  _pixel_scale = 0;
  if (dc) {
    _pixel_scale = GetDeviceCaps(dc, LOGPIXELSX) / (96 / 4);
  }
  if (_pixel_scale <= 0) {
    _pixel_scale = 4;
  }
  ReleaseDC(_window, dc);

  _client_origin.x = 0;
  _client_origin.y = 0;
  ClientToScreen(_window, &_client_origin);

  // Set up a timer to poll the pstats every so often.
  SetTimer(_window, 1, 200, nullptr);
}

/**
 * Creates the "Session" pulldown menu.
 */
void WinStatsServer::
setup_session_menu() {
  _session_menu = CreatePopupMenu();

  MENUITEMINFO mii;
  memset(&mii, 0, sizeof(mii));
  mii.cbSize = sizeof(mii);

  mii.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_SUBMENU;
  mii.fType = MFT_STRING;
  mii.hSubMenu = _session_menu;

  mii.dwTypeData = "&Session";
  InsertMenuItem(_menu_bar, GetMenuItemCount(_menu_bar), TRUE, &mii);

  AppendMenu(_session_menu, MF_STRING, MI_session_new, "&New Session\tCtrl+N");
  AppendMenu(_session_menu, MF_STRING, MI_session_open, "&Open Session...\tCtrl+O");

  if (_last_session.exists()) {
    AppendMenu(_session_menu, MF_STRING, MI_session_open_last, "Open &Last Session");
  } else {
    AppendMenu(_session_menu, MF_STRING | MF_DISABLED, MI_session_open_last, "Open &Last Session");
  }

  AppendMenu(_session_menu, MF_STRING | MF_DISABLED, MI_session_save, "&Save Session...\tCtrl+S");
  AppendMenu(_session_menu, MF_STRING | MF_DISABLED, MI_session_close, "&Close Session\tCtrl+W");

  AppendMenu(_session_menu, MF_SEPARATOR, 0, nullptr);
  AppendMenu(_session_menu, MF_STRING | MF_DISABLED, MI_session_export_json, "&Export as JSON...");

  AppendMenu(_session_menu, MF_SEPARATOR, 0, nullptr);
  AppendMenu(_session_menu, MF_STRING, MI_exit, "E&xit");
}

/**
 * Creates the "Options" pulldown menu.
 */
void WinStatsServer::
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
 * Sets up a status bar at the bottom of the screen showing assorted level
 * values.
 */
void WinStatsServer::
create_status_bar(HINSTANCE application) {
  _status_bar = CreateWindow(STATUSCLASSNAME, nullptr,
                             SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE,
                             0, 0, 0, 0,
                             _window, (HMENU)0, application, nullptr);

  ShowWindow(_status_bar, SW_SHOW);
  UpdateWindow(_status_bar);

  InvalidateRect(_status_bar, NULL, TRUE);
}

/**
 * Registers the window class for the server window, if it has not already
 * been registered.
 */
void WinStatsServer::
register_window_class(HINSTANCE application) {
  if (_window_class_registered) {
    return;
  }

  HMODULE imageres = LoadLibraryExA("imageres.dll", 0, LOAD_LIBRARY_AS_DATAFILE);

  WNDCLASS wc;

  ZeroMemory(&wc, sizeof(WNDCLASS));
  wc.style = 0;
  wc.lpfnWndProc = (WNDPROC)static_window_proc;
  wc.hInstance = application;
  wc.hIcon = LoadIcon(imageres, MAKEINTRESOURCE(150));
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
  wc.lpszMenuName = nullptr;
  wc.lpszClassName = _window_class_name;

  // Reserve space to associate the this pointer with the window.
  wc.cbWndExtra = sizeof(WinStatsServer *);

  if (!RegisterClass(&wc)) {
    nout << "Could not register server window class!\n";
    exit(1);
  }

  _window_class_registered = true;
}

/**
 *
 */
LONG WINAPI WinStatsServer::
static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  WinStatsServer *self = (WinStatsServer *)GetWindowLongPtr(hwnd, 0);
  if (self != nullptr && self->_window == hwnd) {
    return self->window_proc(hwnd, msg, wparam, lparam);
  } else {
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
}

/**
 *
 */
LONG WinStatsServer::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  switch (msg) {
  case WM_TIMER:
    poll();
    break;

  case WM_CLOSE:
    if (!close_session()) {
      return 0;
    }
    break;

  case WM_DESTROY:
    PostQuitMessage(0);
    break;

  case WM_WINDOWPOSCHANGED:
    {
      RECT client_rect;
      GetClientRect(_window, &client_rect);
      MapWindowPoints(_window, nullptr, (POINT *)&client_rect, 2);

      if (_monitor != nullptr) {
        // Remove the status bar from the client rectangle.
        RECT status_bar_rect;
        GetWindowRect(_status_bar, &status_bar_rect);
        client_rect.bottom -= (status_bar_rect.bottom - status_bar_rect.top);

        int delta_x = client_rect.left - _client_origin.x;
        int delta_y = client_rect.top - _client_origin.y;
        _client_origin.x = client_rect.left;
        _client_origin.y = client_rect.top;

        _monitor->handle_window_moved(client_rect, delta_x, delta_y);
      }
      else {
        _client_origin.x = client_rect.left;
        _client_origin.y = client_rect.top;
      }
    }
    break;

  case WM_SIZE:
    if (_status_bar) {
      SendMessage(_status_bar, WM_SIZE, 0, 0);
      if (_monitor != nullptr) {
        _monitor->update_status_bar();
      }
    }
    break;

  case WM_NOTIFY:
    if (_monitor != nullptr) {
      if (((LPNMHDR)lparam)->code == NM_DBLCLK) {
        NMMOUSE &mouse = *(NMMOUSE *)lparam;
        _monitor->handle_status_bar_click(mouse.dwItemSpec);
        return TRUE;
      }
      else if (((LPNMHDR)lparam)->code == NM_RCLICK) {
        NMMOUSE &mouse = *(NMMOUSE *)lparam;
        _monitor->handle_status_bar_popup(mouse.dwItemSpec);
        return TRUE;
      }
    }
    break;

  case WM_KEYDOWN:
    if ((lparam & 0x40000000) == 0 && GetKeyState(VK_CONTROL) < 0) {
      switch (wparam) {
      case 'N':
        new_session();
        return 0;

      case 'O':
        open_session();
        return 0;

      case 'S':
        save_session();
        return 0;

      case 'W':
        close_session();
        return 0;

      default:
        break;
      }
    }
    break;

  case WM_APPCOMMAND:
    switch (GET_APPCOMMAND_LPARAM(lparam)) {
    case APPCOMMAND_OPEN:
      open_session();
      return TRUE;

    case APPCOMMAND_SAVE:
      save_session();
      return TRUE;
    }
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
void WinStatsServer::
handle_menu_command(int menu_id) {
  switch (menu_id) {
  case MI_none:
    break;

  case MI_session_new:
    new_session();
    break;

  case MI_session_open:
    open_session();
    break;

  case MI_session_open_last:
    open_last_session();
    break;

  case MI_session_save:
    save_session();
    break;

  case MI_session_close:
    close_session();
    break;

  case MI_session_export_json:
    export_session();
    break;

  case MI_exit:
    if (close_session()) {
      exit(0);
    }
    break;

  case MI_time_ms:
    set_time_units(PStatGraph::GBU_ms);
    break;

  case MI_time_hz:
    set_time_units(PStatGraph::GBU_hz);
    break;

  default:
    if (_monitor != nullptr) {
      _monitor->handle_menu_command(menu_id);
    }
    break;
  }
}
