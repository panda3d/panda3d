// Filename: winGraphicsWindow.cxx
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

#include "winGraphicsWindow.h"
#include "config_windisplay.h"
#include "winGraphicsPipe.h"

#include "graphicsPipe.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "clockObject.h"

#include <tchar.h>

TypeHandle WinGraphicsWindow::_type_handle;

bool WinGraphicsWindow::_loaded_custom_cursor;
HCURSOR WinGraphicsWindow::_mouse_cursor;
const char * const WinGraphicsWindow::_window_class_name = "WinGraphicsWindow";
bool WinGraphicsWindow::_window_class_registered = false;

WinGraphicsWindow::WindowHandles WinGraphicsWindow::_window_handles;
WinGraphicsWindow *WinGraphicsWindow::_creating_window = NULL;

WinGraphicsWindow *WinGraphicsWindow::_cursor_window = NULL;
bool WinGraphicsWindow::_cursor_hidden = false;

// These are used to save the previous state of the fancy Win2000
// effects that interfere with rendering when the mouse wanders into a
// window's client area.
bool WinGraphicsWindow::_got_saved_params = false;
int WinGraphicsWindow::_saved_mouse_trails;
BOOL WinGraphicsWindow::_saved_cursor_shadow;
BOOL WinGraphicsWindow::_saved_mouse_vanish;

static const char * const errorbox_title = "Panda3D Error";

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
WinGraphicsWindow::
WinGraphicsWindow(GraphicsPipe *pipe, GraphicsStateGuardian *gsg,
                  const string &name) :
  GraphicsWindow(pipe, gsg, name) 
{
  GraphicsWindowInputDevice device =
  GraphicsWindowInputDevice::pointer_and_keyboard("keyboard/mouse");
  _input_devices.push_back(device);
  _hWnd = (HWND)0;
  _ime_open = false;
  _ime_active = false;
  _ime_composition_w = false;
  _tracking_mouse_leaving = false;
  _maximized = false;
  memset(_keyboard_state, 0, sizeof(BYTE) * num_virtual_keys);
  _lost_keypresses = false;
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
WinGraphicsWindow::
~WinGraphicsWindow() {
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::move_pointer
//       Access: Published, Virtual
//  Description: Forces the pointer to the indicated position within
//               the window, if possible.  
//
//               Returns true if successful, false on failure.  This
//               may fail if the mouse is not currently within the
//               window, or if the API doesn't support this operation.
////////////////////////////////////////////////////////////////////
bool WinGraphicsWindow::
move_pointer(int device, int x, int y) {
  // Note: this is not thread-safe; it should be called only from App.
  // Probably not an issue.
  nassertr(device == 0, false);
  if (!_properties.get_foreground() ||
      !_input_devices[0].get_pointer().get_in_window()) {
    // If the window doesn't have input focus, or the mouse isn't
    // currently within the window, forget it.
    return false;
  }

  RECT view_rect;
  get_client_rect_screen(_hWnd, &view_rect);

  SetCursorPos(view_rect.left + x, view_rect.top + y);
  _input_devices[0].set_pointer_in_window(x, y);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::begin_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after end_frame() has been called on all windows, to
//               initiate the exchange of the front and back buffers.
//
//               This should instruct the window to prepare for the
//               flip at the next video sync, but it should not wait.
//
//               We have the two separate functions, begin_flip() and
//               end_flip(), to make it easier to flip all of the
//               windows at the same time.
////////////////////////////////////////////////////////////////////
void WinGraphicsWindow::
begin_flip() {
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::process_events
//       Access: Public, Virtual
//  Description: Do whatever processing is necessary to ensure that
//               the window responds to user events.  Also, honor any
//               requests recently made via request_properties()
//
//               This function is called only within the window
//               thread.
////////////////////////////////////////////////////////////////////
void WinGraphicsWindow::
process_events() {
  GraphicsWindow::process_events();

  // We can't treat the message loop specially just because the window
  // is minimized, because we might be reading messages queued up for
  // some other window, which is not minimized.
  /*
  if (!_window_active) {
      // Get 1 msg at a time until no more are left and we block and sleep,
      // or message changes _return_control_to_app or !_window_active status

      while(!_window_active && (!_return_control_to_app)) {
          process_1_event();
      }
      _return_control_to_app = false;

  } else 
  */

  MSG msg;
    
  // Handle all the messages on the queue in a row.  Some of these
  // might be for another window, but they will get dispatched
  // appropriately.
  while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
    process_1_event();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::set_properties_now
//       Access: Public, Virtual
//  Description: Applies the requested set of properties to the
//               window, if possible, for instance to request a change
//               in size or minimization status.
//
//               The window properties are applied immediately, rather
//               than waiting until the next frame.  This implies that
//               this method may *only* be called from within the
//               window thread.
//
//               The properties that have been applied are cleared
//               from the structure by this function; so on return,
//               whatever remains in the properties structure are
//               those that were unchanged for some reason (probably
//               because the underlying interface does not support
//               changing that property on an open window).
////////////////////////////////////////////////////////////////////
void WinGraphicsWindow::
set_properties_now(WindowProperties &properties) {
  GraphicsWindow::set_properties_now(properties);
  if (!properties.is_any_specified()) {
    // The base class has already handled this case.
    return;
  }

  if (properties.has_title()) {
    string title = properties.get_title();
    _properties.set_title(title);
    SetWindowText(_hWnd, title.c_str());
    properties.clear_title();
  }

  if (properties.has_cursor_hidden()) {
    bool hide_cursor = properties.get_cursor_hidden();
    _properties.set_cursor_hidden(hide_cursor);
    if (_cursor_window == this) {
      hide_or_show_cursor(hide_cursor);
    }

    properties.clear_cursor_hidden();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::close_window
//       Access: Protected, Virtual
//  Description: Closes the window right now.  Called from the window
//               thread.
////////////////////////////////////////////////////////////////////
void WinGraphicsWindow::
close_window() {
  set_cursor_out_of_window();
  DestroyWindow(_hWnd);

  // Remove the window handle from our global map.
  _window_handles.erase(_hWnd);
  _hWnd = (HWND)0;

  if (is_fullscreen()) {
    // revert to default display mode.
    ChangeDisplaySettings(NULL, 0x0);
  }

  GraphicsWindow::close_window();
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::open_window
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool WinGraphicsWindow::
open_window() {
  // Store the current window pointer in _creating_window, so we can
  // call CreateWindow() and know which window it is sending events to
  // even before it gives us a handle.  Warning: this is not thread
  // safe!
  _creating_window = this;
  bool opened;
  if (is_fullscreen()) {
    opened = open_fullscreen_window();
  } else {
    opened = open_regular_window();
  }
  _creating_window = (WinGraphicsWindow *)NULL;

  if (!opened) {
    return false;
  }

  // Now that we have a window handle, store it in our global map, so
  // future messages for this window can be routed properly.
  _window_handles.insert(WindowHandles::value_type(_hWnd, this));
  
  // move window to top of zorder.
  SetWindowPos(_hWnd, HWND_TOP, 0,0,0,0, 
               SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOSIZE);
  
  // need to do twice to override any minimized flags in StartProcessInfo
  ShowWindow(_hWnd, SW_SHOWNORMAL);
  ShowWindow(_hWnd, SW_SHOWNORMAL);

  if (!SetForegroundWindow(_hWnd)) {
    windisplay_cat.warning()
      << "SetForegroundWindow() failed!\n";
  }

  // Determine the initial open status of the IME.
  _ime_open = false;
  _ime_active = false;
  HIMC hIMC = ImmGetContext(_hWnd);
  if (hIMC != 0) {
    _ime_open = (ImmGetOpenStatus(hIMC) != 0);
    ImmReleaseContext(_hWnd, hIMC);
  }

  // Check the version of the OS we are running.  If we are running
  // win2000, we must use ImmGetCompositionStringW() to report the
  // characters returned by the IME, since WM_CHAR and
  // ImmGetCompositionStringA() both just return question marks.
  // However, this function doesn't work for Win98; on this OS, we
  // have to use ImmGetCompositionStringA() instead, which returns an
  // encoded string in shift-jis (which we then have to decode).

  // For now, this is user-configurable, to allow testing of this code
  // on both OS's.  After we verify that truth of the above claim, we
  // should base this decision on GetVersionEx() or maybe
  // VerifyVersionInfo().
  _ime_composition_w = ime_composition_w;
  
  // need to re-evaluate above in light of this, it seems that
  // ImmGetCompositionStringW should work on both:
  //     The Input Method Editor and Unicode Windows 98/Me, Windows
  //     NT/2000/XP: Windows supports a Unicode interface for the
  //     IME, in addition to the ANSI interface originally supported.
  //     Windows 98/Me supports all the Unicode functions except
  //     ImmIsUIMessage.  Also, all the messages in Windows 98/Me are
  //     ANSI based.  Since Windows 98/Me does not support Unicode
  //     messages, applications can use ImmGetCompositionString to
  //     receive Unicode characters from a Unicode based IME on
  //     Windows 98/Me.  There are two issues involved with Unicode
  //     handling and the IME.  One is that the Unicode versions of
  //     IME routines return the size of a buffer in bytes rather
  //     than 16-bit Unicode characters,and the other is the IME
  //     normally returns Unicode characters (rather than DBCS) in
  //     the WM_CHAR and WM_IME_CHAR messages.  Use RegisterClassW
  //     to cause the WM_CHAR and WM_IME_CHAR messages to return
  //     Unicode characters in the wParam parameter rather than DBCS
  //     characters.  This is only available under Windows NT; it is
  //     stubbed out in Windows 95/98/Me.

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::fullscreen_minimized
//       Access: Protected, Virtual
//  Description: This is a hook for derived classes to do something
//               special, if necessary, when a fullscreen window has
//               been minimized.  The given WindowProperties struct
//               will be applied to this window's properties after
//               this function returns.
////////////////////////////////////////////////////////////////////
void WinGraphicsWindow::
fullscreen_minimized(WindowProperties &) {
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::fullscreen_restored
//       Access: Protected, Virtual
//  Description: This is a hook for derived classes to do something
//               special, if necessary, when a fullscreen window has
//               been restored after being minimized.  The given
//               WindowProperties struct will be applied to this
//               window's properties after this function returns.
////////////////////////////////////////////////////////////////////
void WinGraphicsWindow::
fullscreen_restored(WindowProperties &) {
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::do_reshape_request
//       Access: Protected, Virtual
//  Description: Called from the window thread in response to a request
//               from within the code (via request_properties()) to
//               change the size and/or position of the window.
//               Returns true if the window is successfully changed,
//               or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool WinGraphicsWindow::
do_reshape_request(int x_origin, int y_origin, int x_size, int y_size) {
  windisplay_cat.info()
    << "Got reshape request (" << x_origin << ", " << y_origin
    << ", " << x_size << ", " << y_size << ")\n";
  if (!is_fullscreen()) {
    // Compute the appropriate size and placement for the window,
    // including decorations.
    RECT view_rect;
    SetRect(&view_rect, x_origin, y_origin,
            x_origin + x_size, y_origin + y_size);
    WINDOWINFO wi;
    GetWindowInfo(_hWnd, &wi);
    AdjustWindowRectEx(&view_rect, wi.dwStyle, false, wi.dwExStyle);

    SetWindowPos(_hWnd, NULL, view_rect.left, view_rect.top,
                 view_rect.right - view_rect.left,
                 view_rect.bottom - view_rect.top,
                 SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING);

    // This isn't quite right, because handle_reshape() calls
    // system_changed_properties(), generating the event indicating
    // the window has changed size externally--even though it changed
    // due to an internal request.
    handle_reshape();
    return true;
  }

  // Resizing a fullscreen window is a little trickier.
  return do_fullscreen_resize(x_size, y_size);
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::handle_reshape
//       Access: Protected, Virtual
//  Description: Called in the window thread when the window size or
//               location is changed, this updates the properties
//               structure accordingly.
////////////////////////////////////////////////////////////////////
void WinGraphicsWindow::
handle_reshape() {
  RECT view_rect;
  GetClientRect(_hWnd, &view_rect);
  ClientToScreen(_hWnd, (POINT*)&view_rect.left);   // translates top,left pnt
  ClientToScreen(_hWnd, (POINT*)&view_rect.right);  // translates right,bottom pnt
  
  WindowProperties properties;
  properties.set_size((view_rect.right - view_rect.left), 
                      (view_rect.bottom - view_rect.top));

  // _props origin should reflect upper left of view rectangle
  properties.set_origin(view_rect.left, view_rect.top);
  
  if (windisplay_cat.is_spam()) {
    windisplay_cat.spam()
      << "reshape to origin: (" << properties.get_x_origin() << "," 
      << properties.get_y_origin() << "), size: (" << properties.get_x_size()
      << "," << properties.get_y_size() << ")\n";
  }

  system_changed_properties(properties);
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::do_fullscreen_resize
//       Access: Protected, Virtual
//  Description: Called in the window thread to resize a fullscreen
//               window.
////////////////////////////////////////////////////////////////////
bool WinGraphicsWindow::
do_fullscreen_resize(int x_size, int y_size) {
  HWND hDesktopWindow = GetDesktopWindow();
  HDC scrnDC = GetDC(hDesktopWindow);
  DWORD dwFullScreenBitDepth = GetDeviceCaps(scrnDC, BITSPIXEL);
  ReleaseDC(hDesktopWindow, scrnDC);

  // resize will always leave screen bitdepth unchanged

  // allowing resizing of lowvidmem cards to > 640x480.  why?  I'll
  // assume check was already done by caller, so he knows what he
  // wants

  DEVMODE dm;
  if (!find_acceptable_display_mode(x_size, y_size,
                                    dwFullScreenBitDepth, dm)) {
    windisplay_cat.error()
      << "window resize(" << x_size << ", " << y_size 
      << ") failed, no compatible fullscreen display mode found!\n";
    return false;
  }

  // this causes WM_SIZE msg to be produced
  SetWindowPos(_hWnd, NULL, 0,0, x_size, y_size, 
               SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING);
  int chg_result = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);

  if (chg_result != DISP_CHANGE_SUCCESSFUL) {
    windisplay_cat.error()
      << "resize ChangeDisplaySettings failed (error code: " 
      << chg_result << ") for specified res (" << x_size << " x "
      << y_size << " x " << dwFullScreenBitDepth << "), " 
      << dm.dmDisplayFrequency << "Hz\n";
    return false;
  }

  _fullscreen_display_mode = dm;

  windisplay_cat.info()
    << "Resized fullscreen window to " << x_size << ", " << y_size 
    << " bitdepth " << dwFullScreenBitDepth << ", "
    << dm.dmDisplayFrequency << "Hz\n";

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::reconsider_fullscreen_size
//       Access: Protected, Virtual
//  Description: Called before creating a fullscreen window to give
//               the driver a chance to adjust the particular
//               resolution request, if necessary.
////////////////////////////////////////////////////////////////////
void WinGraphicsWindow::
reconsider_fullscreen_size(DWORD &, DWORD &, DWORD &) {
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::support_overlay_window
//       Access: Protected, Virtual
//  Description: Some windows graphics contexts (e.g. DirectX)
//               require special support to enable the displaying of
//               an overlay window (particularly the IME window) over
//               the fullscreen graphics window.  This is a hook for
//               the window to enable or disable that mode when
//               necessary.
////////////////////////////////////////////////////////////////////
void WinGraphicsWindow::
support_overlay_window(bool) {
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::open_fullscreen_window
//       Access: Private
//  Description: Creates a fullscreen-style window.
////////////////////////////////////////////////////////////////////
bool WinGraphicsWindow::
open_fullscreen_window() {
  //  from MSDN:
  //  An OpenGL window has its own pixel format. Because of this, only
  //  device contexts retrieved for the client area of an OpenGL
  //  window are allowed to draw into the window. As a result, an
  //  OpenGL window should be created with the WS_CLIPCHILDREN and
  //  WS_CLIPSIBLINGS styles. Additionally, the window class attribute
  //  should not include the CS_PARENTDC style.
  DWORD window_style = 
    WS_POPUP | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

  if (!_properties.has_size()) {
    // Just pick a stupid default size if one isn't specified.
    _properties.set_size(640, 480);
  }

  HWND hDesktopWindow = GetDesktopWindow();
  HDC scrnDC = GetDC(hDesktopWindow);
  DWORD cur_bitdepth = GetDeviceCaps(scrnDC, BITSPIXEL);
  //  DWORD drvr_ver = GetDeviceCaps(scrnDC, DRIVERVERSION);
  //  DWORD cur_scrnwidth = GetDeviceCaps(scrnDC, HORZRES);
  //  DWORD cur_scrnheight = GetDeviceCaps(scrnDC, VERTRES);
  ReleaseDC(hDesktopWindow, scrnDC);

  DWORD dwWidth = _properties.get_x_size();
  DWORD dwHeight = _properties.get_y_size();
  DWORD dwFullScreenBitDepth = cur_bitdepth;
  
  reconsider_fullscreen_size(dwWidth, dwHeight, dwFullScreenBitDepth);
  if (!find_acceptable_display_mode(dwWidth, dwHeight, dwFullScreenBitDepth,
                                    _fullscreen_display_mode)) {
    windisplay_cat.error() 
      << "Videocard has no supported display resolutions at specified res ("
      << dwWidth << " x " << dwHeight << " x " << dwFullScreenBitDepth <<")\n";
    return false;
  }

  string title;
  if (_properties.has_title()) {
    title = _properties.get_title();
  }

  // I'd prefer to CreateWindow after DisplayChange in case it messes
  // up GL somehow, but I need the window's black background to cover
  // up the desktop during the mode change
  register_window_class();
  HINSTANCE hinstance = GetModuleHandle(NULL);
  _hWnd = CreateWindow(_window_class_name, title.c_str(), window_style,
                          0, 0, dwWidth, dwHeight, 
                          hDesktopWindow, NULL, hinstance, 0);
  if (!_hWnd) {
    windisplay_cat.error()
      << "CreateWindow() failed!" << endl;
    show_error_message();
    return false;
  }
   
  int chg_result = ChangeDisplaySettings(&_fullscreen_display_mode, 
                                         CDS_FULLSCREEN);
  if (chg_result != DISP_CHANGE_SUCCESSFUL) {
    windisplay_cat.error()
      << "ChangeDisplaySettings failed (error code: "
      << chg_result << ") for specified res (" << dwWidth
      << " x " << dwHeight << " x " << dwFullScreenBitDepth
      << "), " << _fullscreen_display_mode.dmDisplayFrequency  << "Hz\n";
    return false;
  }

  _properties.set_origin(0, 0);
  _properties.set_size(dwWidth, dwHeight);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::open_regular_window
//       Access: Private
//  Description: Creates a non-fullscreen window, on the desktop.
////////////////////////////////////////////////////////////////////
bool WinGraphicsWindow::
open_regular_window() {
  //  from MSDN:
  //  An OpenGL window has its own pixel format. Because of this, only
  //  device contexts retrieved for the client area of an OpenGL
  //  window are allowed to draw into the window. As a result, an
  //  OpenGL window should be created with the WS_CLIPCHILDREN and
  //  WS_CLIPSIBLINGS styles. Additionally, the window class attribute
  //  should not include the CS_PARENTDC style.
  DWORD window_style = 
    WS_POPUP | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

  if (!_properties.get_undecorated()) {
    window_style |= (WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX);

    if (!_properties.get_fixed_size()) {
      window_style |= (WS_SIZEBOX | WS_MAXIMIZEBOX);
    } else {
      window_style |= WS_BORDER;
    }
  }

  if (!_properties.has_origin()) {
    _properties.set_origin(0, 0);
  }
  if (!_properties.has_size()) {
    _properties.set_size(100, 100);
  }

  RECT win_rect;
  SetRect(&win_rect, 
          _properties.get_x_origin(),
          _properties.get_y_origin(),
          _properties.get_x_origin() + _properties.get_x_size(),
          _properties.get_y_origin() + _properties.get_y_size());
  
  // compute window size based on desired client area size
  if (!AdjustWindowRect(&win_rect, window_style, FALSE)) {
    windisplay_cat.error()
      << "AdjustWindowRect failed!" << endl;
    return false;
  }
  
  // make sure origin is on screen; slide far bounds over if necessary
  if (win_rect.left < 0) {
    win_rect.right += abs(win_rect.left); 
    win_rect.left = 0;
  }
  if (win_rect.top < 0) {
    win_rect.bottom += abs(win_rect.top); 
    win_rect.top = 0;
  }

  string title;
  if (_properties.has_title()) {
    title = _properties.get_title();
  }

  register_window_class();
  HINSTANCE hinstance = GetModuleHandle(NULL);
  _hWnd = CreateWindow(_window_class_name, title.c_str(), window_style, 
                          win_rect.left, win_rect.top,
                          win_rect.right - win_rect.left,
                          win_rect.bottom - win_rect.top,
                          NULL, NULL, hinstance, 0);

  if (!_hWnd) {
    windisplay_cat.error()
      << "CreateWindow() failed!" << endl;
    show_error_message();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::track_mouse_leaving
//       Access: Private
//  Description: Intended to be called whenever mouse motion is
//               detected within the window, this indicates that the
//               mouse is within the window and tells Windows that we
//               want to be told when the mouse leaves the window.
////////////////////////////////////////////////////////////////////
void WinGraphicsWindow::
track_mouse_leaving(HWND hwnd) {
  // Note: could use _TrackMouseEvent in comctrl32.dll (part of IE
  // 3.0+) which emulates TrackMouseEvent on w95, but that requires
  // another 500K of memory to hold that DLL, which is lame just to
  // support w95, which probably has other issues anyway
  WinGraphicsPipe *winpipe;
  DCAST_INTO_V(winpipe, _pipe);

  if (winpipe->_pfnTrackMouseEvent != NULL) {
    TRACKMOUSEEVENT tme = {
      sizeof(TRACKMOUSEEVENT),
      TME_LEAVE,
      hwnd,
      0
    };

    // tell win32 to post WM_MOUSELEAVE msgs
    BOOL bSucceeded = winpipe->_pfnTrackMouseEvent(&tme);  
    
    if ((!bSucceeded) && windisplay_cat.is_debug()) {
      windisplay_cat.debug()
        << "TrackMouseEvent failed!, LastError=" << GetLastError() << endl;
    }
    
    _tracking_mouse_leaving = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::window_proc
//       Access: Private
//  Description: This is the nonstatic window_proc function.  It is
//               called to handle window events for this particular
//               window.
////////////////////////////////////////////////////////////////////
LONG WinGraphicsWindow::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  if (windisplay_cat.is_spam()) {
    windisplay_cat.spam()
      << ClockObject::get_global_clock()->get_real_time() 
      << " window_proc(" << (void *)this << ", " << hwnd << ", "
      << msg << ", " << wparam << ", " << lparam << ")\n";
  }
  WindowProperties properties;
  int button = -1;

  switch (msg) {
      case WM_MOUSEMOVE: 
        if (!_tracking_mouse_leaving) {
          // need to re-call TrackMouseEvent every time mouse re-enters window
          track_mouse_leaving(hwnd);
        }
        set_cursor_in_window();
        if(handle_mouse_motion(translate_mouse(LOWORD(lparam)), translate_mouse(HIWORD(lparam))))
            return 0;
        break;
    
      case WM_MOUSELEAVE:
        _tracking_mouse_leaving = false;
        handle_mouse_exit();
        set_cursor_out_of_window();
        break;
    
      // if cursor is invisible, make it visible when moving in the window bars & menus, so user can use click in them
      case WM_NCMOUSEMOVE: {
            if(!_properties.get_cursor_hidden()) {
                if(!_bCursor_in_WindowClientArea) {
                    // SetCursor(_pParentWindowGroup->_hMouseCursor);
                    hide_or_show_cursor(false);
                    _bCursor_in_WindowClientArea=true;
                }
            }
            break;
      }
    
      case WM_NCMOUSELEAVE: {
            if(!_properties.get_cursor_hidden()) {
                hide_or_show_cursor(true);
                // SetCursor(NULL);
                _bCursor_in_WindowClientArea=false;
            }
            break;
      }
    
      case WM_CREATE: {
        track_mouse_leaving(hwnd);
        _bCursor_in_WindowClientArea=false;
        ClearToBlack(hwnd,_properties);
    
        POINT cpos;
        GetCursorPos(&cpos);
        ScreenToClient(hwnd,&cpos);
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        if(PtInRect(&clientRect,cpos))
           set_cursor_in_window();  // should window focus be true as well?
        else set_cursor_out_of_window();
    
        break;
      }
    
      case WM_CLOSE:
        //GraphicsWindow::close_window();  //M.A. changed to the following line
        close_window();
        properties.set_open(false);
        system_changed_properties(properties);
    
        // TODO: make sure we release the GSG properly.
        windisplay_cat.debug() << "ta ta" << endl;
        break;
    
      case WM_ACTIVATE:
        properties.set_minimized((wparam & 0xffff0000) != 0);
        if ((wparam & 0xffff) != WA_INACTIVE) {
          properties.set_foreground(true);
          if (is_fullscreen()) {
            // When a fullscreen window goes active, it automatically gets
            // un-minimized.
            ChangeDisplaySettings(&_fullscreen_display_mode, CDS_FULLSCREEN);
            GdiFlush();
            SetWindowPos(_hWnd, HWND_TOP, 0,0,0,0, 
                         SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOOWNERZORDER);
            fullscreen_restored(properties);
          }
        } else {
          properties.set_foreground(false);
          if (is_fullscreen()) {
            // When a fullscreen window goes inactive, it automatically
            // gets minimized.
            properties.set_minimized(true);
    
            // It seems order is important here.  We must minimize the
            // window before restoring the display settings, or risk
            // losing the graphics context.
            ShowWindow(_hWnd, SW_MINIMIZE);
            GdiFlush();
            ChangeDisplaySettings(NULL, 0x0);
            fullscreen_minimized(properties);
          }
        }
        system_changed_properties(properties);
        break;
    
      case WM_SIZE:
        // for maximized, unmaximize, need to call resize code
        // artificially since no WM_EXITSIZEMOVE is generated.
        if (wparam == SIZE_MAXIMIZED) {
          _maximized = true;
          handle_reshape();
    
        } else if (wparam == SIZE_RESTORED && _maximized) {
          // SIZE_RESTORED might mean we restored to its original size
          // before the maximize, but it might also be called while the
          // user is resizing the window by hand.  Checking the _maximized
          // flag that we set above allows us to differentiate the two
          // cases.
          _maximized = false;
          handle_reshape();
        }
        break;
    
      case WM_EXITSIZEMOVE:
        handle_reshape();
        break;
    
      case WM_LBUTTONDOWN:
        button = 0;
        // fall through
      case WM_MBUTTONDOWN:
        if (button < 0) {
          button = 1;
        }
        // fall through
      case WM_RBUTTONDOWN:
        if (_lost_keypresses) {
          resend_lost_keypresses();
        }
        if (button < 0) {
          button = 2;
        }
        SetCapture(hwnd);
        handle_keypress(MouseButton::button(button), 
                        translate_mouse(LOWORD(lparam)), translate_mouse(HIWORD(lparam)),
                        get_message_time());
        break;
    
      case WM_LBUTTONUP:
        button = 0;
        // fall through
      case WM_MBUTTONUP:
        if (button < 0) {
          button = 1;
        }
        // fall through
      case WM_RBUTTONUP:
        if (_lost_keypresses) {
          resend_lost_keypresses();
        }
        if (button < 0) {
          button = 2;
        }
        ReleaseCapture();
        handle_keyrelease(MouseButton::button(button), get_message_time());
        break;

      case WM_MOUSEWHEEL:
        {
          int delta = GET_WHEEL_DELTA_WPARAM(wparam);
          int x = translate_mouse(LOWORD(lparam));
          int y = translate_mouse(HIWORD(lparam));
          double time = get_message_time();

          if (delta >= 0) {
            while (delta > 0) {
              handle_keypress(MouseButton::wheel_up(), x, y, time);
              delta -= WHEEL_DELTA;
            }
          } else {
            while (delta < 0) {
              handle_keypress(MouseButton::wheel_down(), x, y, time);
              delta += WHEEL_DELTA;
            }
          }
          return 0;
        }
    
      case WM_IME_NOTIFY:
        if (wparam == IMN_SETOPENSTATUS) {
          HIMC hIMC = ImmGetContext(hwnd);
          nassertr(hIMC != 0, 0);
          _ime_open = (ImmGetOpenStatus(hIMC) != 0);
          if (!_ime_open) {
            _ime_active = false;  // Sanity enforcement.
          }
          ImmReleaseContext(hwnd, hIMC);
        }
        break;
        
      case WM_IME_STARTCOMPOSITION:
        support_overlay_window(true);
        _ime_active = true;
        break;
        
      case WM_IME_ENDCOMPOSITION:
        support_overlay_window(false);
        _ime_active = false;
        break;
        
      case WM_IME_COMPOSITION:
        if (lparam & GCS_RESULTSTR) {
          HIMC hIMC = ImmGetContext(hwnd);
          nassertr(hIMC != 0, 0);
          
          static const int max_ime_result = 128;
          static char ime_result[max_ime_result];
          
          if (_ime_composition_w) {
            // Since ImmGetCompositionStringA() doesn't seem to work
            // for Win2000 (it always returns question mark
            // characters), we have to use ImmGetCompositionStringW()
            // on this OS.  This is actually the easier of the two
            // functions to use.
            
            DWORD result_size =
              ImmGetCompositionStringW(hIMC, GCS_RESULTSTR,
                                       ime_result, max_ime_result);
            
            // Add this string into the text buffer of the application.
            
            // ImmGetCompositionStringW() returns a string, but it's
            // filled in with wstring data: every two characters defines a
            // 16-bit unicode char.  The docs aren't clear on the
            // endianness of this.  I guess it's safe to assume all Win32
            // machines are little-endian.
            for (DWORD i = 0; i < result_size; i += 2) {
              int result =
                ((int)(unsigned char)ime_result[i + 1] << 8) |
                (unsigned char)ime_result[i];
              _input_devices[0].keystroke(result);
            }
          } else {
            // On the other hand, ImmGetCompositionStringW() doesn't
            // work on Win95 or Win98; for these OS's we must use
            // ImmGetCompositionStringA().
            DWORD result_size =
              ImmGetCompositionStringA(hIMC, GCS_RESULTSTR,
                                       ime_result, max_ime_result);
            
            // ImmGetCompositionStringA() returns an encoded ANSI
            // string, which we now have to map to wide-character
            // Unicode.
            static const int max_wide_result = 128;
            static wchar_t wide_result[max_wide_result];
            
            int wide_size =
              MultiByteToWideChar(CP_ACP, 0,
                                  ime_result, result_size,
                                  wide_result, max_wide_result);
            if (wide_size == 0) {
              show_error_message();
            }
            for (int i = 0; i < wide_size; i++) {
              _input_devices[0].keystroke(wide_result[i]);
            }
          }
          
          ImmReleaseContext(hwnd, hIMC);
          return 0;
        }
        break;
        
      case WM_CHAR:
        // Ignore WM_CHAR messages if we have the IME open, since
        // everything will come in through WM_IME_COMPOSITION.  (It's
        // supposed to come in through WM_CHAR, too, but there seems to
        // be a bug in Win2000 in that it only sends question mark
        // characters through here.)
        if (!_ime_open) {
          _input_devices[0].keystroke(wparam);
        }
        break;
    
      case WM_SYSKEYDOWN: 
        if (_lost_keypresses) {
          resend_lost_keypresses();
        }
        if (windisplay_cat.is_debug()) {
          windisplay_cat.debug()
            << "keydown: " << wparam << " (" << lookup_key(wparam) << ")\n";
        }
        {
          // Alt and F10 are sent as WM_SYSKEYDOWN instead of WM_KEYDOWN
          // want to use defwindproc on Alt syskey so std windows cmd
          // Alt-F4 works, etc
          POINT point;
          GetCursorPos(&point);
          ScreenToClient(hwnd, &point);
          handle_keypress(lookup_key(wparam), point.x, point.y, 
                          get_message_time());
          if (wparam == VK_F10) {
            // bypass default windproc F10 behavior (it activates the main
            // menu, but we have none)
            return 0;
          }
        }
        break;
    
      case WM_SYSCOMMAND:
        if (wparam == SC_KEYMENU) {
          // if Alt is released (alone w/o other keys), defwindproc will
          // send this command, which will 'activate' the title bar menu
          // (we have none) and give focus to it.  we dont want this to
          // happen, so kill this msg.

          // Note that the WM_SYSKEYUP message for Alt has already
          // been sent (if it is going to be), so ignoring this
          // special message does no harm.
          return 0;
        }
        break;
        
      case WM_KEYDOWN: 
        if (_lost_keypresses) {
          resend_lost_keypresses();
        }
        if (windisplay_cat.is_debug()) {
          windisplay_cat.debug()
            << "keydown: " << wparam << " (" << lookup_key(wparam) << ")\n";
        }

        // If this bit is not zero, this is just a keyrepeat echo; we
        // ignore these for handle_keypress (we respect keyrepeat only
        // for handle_keystroke).
        if ((lparam & 0x40000000) == 0) {
          POINT point;
          GetCursorPos(&point);
          ScreenToClient(hwnd, &point);
          handle_keypress(lookup_key(wparam), point.x, point.y,
                          get_message_time());
    
          // Handle Cntrl-V paste from clipboard.  Is there a better way
          // to detect this hotkey?
          if ((wparam=='V') && (GetKeyState(VK_CONTROL) < 0) &&
              !_input_devices.empty()) {
            HGLOBAL hglb;
            char *lptstr;
    
            if (IsClipboardFormatAvailable(CF_TEXT) && OpenClipboard(NULL)) {
              // Maybe we should support CF_UNICODETEXT if it is available
              // too?
              hglb = GetClipboardData(CF_TEXT);
              if (hglb!=NULL) {
                lptstr = (char *) GlobalLock(hglb);
                if (lptstr != NULL)  {
                  char *pChar;
                  for (pChar=lptstr; *pChar!=NULL; pChar++) {
                    _input_devices[0].keystroke((uchar)*pChar);
                  }
                  GlobalUnlock(hglb);
                }
              }
              CloseClipboard();
            }
          }

        } else {
          // Actually, for now we'll respect the repeat anyway, just
          // so we support backspace properly.  Rethink later.
          POINT point;
          GetCursorPos(&point);
          ScreenToClient(hwnd, &point);
          handle_keypress(lookup_key(wparam), point.x, point.y,
                          get_message_time());
        }
        break;
    
      case WM_SYSKEYUP:
      case WM_KEYUP:
        if (_lost_keypresses) {
          resend_lost_keypresses();
        }
        if (windisplay_cat.is_debug()) {
          windisplay_cat.debug()
            << "keyup: " << wparam << " (" << lookup_key(wparam) << ")\n";
        }
        handle_keyrelease(lookup_key(wparam), get_message_time());
        break;
    
      case WM_KILLFOCUS: 
        if (windisplay_cat.is_debug()) {
          windisplay_cat.debug()
            << "killfocus\n";
        }
        if (!_lost_keypresses) {
          // Record the current state of the keyboard when the focus is
          // lost, so we can check it for changes when we regain focus.
          GetKeyboardState(_keyboard_state);
          if (windisplay_cat.is_debug()) {
            // Report the set of keys that are held down at the time of
            // the killfocus event.
            for (int i = 0; i < num_virtual_keys; i++) {
              if (i != VK_SHIFT && i != VK_CONTROL && i != VK_MENU) {
                if ((_keyboard_state[i] & 0x80) != 0) {
                  windisplay_cat.debug()
                    << "on killfocus, key is down: " << i
                    << " (" << lookup_key(i) << ")\n";
                }
              }
            }
          }

          if (!hold_keys_across_windows) {
            // If we don't want to remember the keystate while the
            // window focus is lost, then generate a keyup event
            // right now for each key currently held.
            double message_time = get_message_time();
            for (int i = 0; i < num_virtual_keys; i++) {
              if (i != VK_SHIFT && i != VK_CONTROL && i != VK_MENU) {
                if ((_keyboard_state[i] & 0x80) != 0) {
                  handle_keyrelease(lookup_key(i), message_time);
                  _keyboard_state[i] &= ~0x80;
                }
              }
            }
          }
          
          // Now set the flag indicating that some keypresses from now
          // on may be lost.
          _lost_keypresses = true;
        }
        break;
    
      case WM_SETFOCUS: 
        // You would think that this would be a good time to call
        // resend_lost_keypresses(), but it turns out that we get
        // WM_SETFOCUS slightly before Windows starts resending key
        // up/down events to us.

        // In particular, if the user restored focus using alt-tab,
        // then at this point the keyboard state will indicate that
        // both the alt and tab keys are held down.  However, there is
        // a small window of opportunity for the user to release these
        // keys before Windows starts telling us about keyup events.
        // Thus, if we record the fact that alt and tab are being held
        // down now, we may miss the keyup events for them, and they
        // can get "stuck" down.

        // So we have to defer calling resend_lost_keypresses() until
        // we know Windows is ready to send us key up/down events.  I
        // don't know when we can guarantee that, except when we
        // actually do start to receive key up/down events, so that
        // call is made there.

        if (windisplay_cat.is_debug()) {
          windisplay_cat.debug()
            << "setfocus\n";
        }
        break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}


////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::static_window_proc
//       Access: Private, Static
//  Description: This is attached to the window class for all
//               WinGraphicsWindow windows; it is called to handle
//               window events.
////////////////////////////////////////////////////////////////////
LONG WINAPI WinGraphicsWindow::
static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  // Look up the window in our global map.
  WindowHandles::const_iterator wi;
  wi = _window_handles.find(hwnd);
  if (wi != _window_handles.end()) {
    // We found the window.
    return (*wi).second->window_proc(hwnd, msg, wparam, lparam);
  }

  // The window wasn't in the map; we must be creating it right now.
  if (_creating_window != (WinGraphicsWindow *)NULL) {
    return _creating_window->window_proc(hwnd, msg, wparam, lparam);
  }

  // Oops, we weren't creating a window!  Don't know how to handle the
  // message, so just pass it on to Windows to deal with it.
  return DefWindowProc(hwnd, msg, wparam, lparam);
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::process_1_event
//       Access: Private, Static
//  Description: Handles one event from the message queue.
////////////////////////////////////////////////////////////////////
void WinGraphicsWindow::
process_1_event() {
  MSG msg;

  if (!GetMessage(&msg, NULL, 0, 0)) {
    // WM_QUIT received.  We need a cleaner way to deal with this.
    //    DestroyAllWindows(false);
    exit(msg.wParam);  // this will invoke AtExitFn
  }

  // Translate virtual key messages
  TranslateMessage(&msg);
  // Call window_proc
  DispatchMessage(&msg);
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::resend_lost_keypresses
//       Access: Private, Static
//  Description: Called when the keyboard focus has been restored to
//               the window after it has been lost for a time, this
//               rechecks the keyboard state and generates key up/down
//               messages for keys that have changed state in the
//               meantime.
////////////////////////////////////////////////////////////////////
void WinGraphicsWindow::
resend_lost_keypresses() {
  nassertv(_lost_keypresses);
  if (windisplay_cat.is_debug()) {
    windisplay_cat.debug()
      << "resending lost keypresses\n";
  }

  BYTE new_keyboard_state[num_virtual_keys];
  GetKeyboardState(new_keyboard_state);

  double message_time = get_message_time();
  for (int i = 0; i < num_virtual_keys; i++) {
    // Filter out these particular three.  We don't want to test
    // these, because these are virtual duplicates for
    // VK_LSHIFT/VK_RSHIFT, etc.; and the left/right equivalent is
    // also in the table.  If we respect both VK_LSHIFT as well as
    // VK_SHIFT, we'll generate two keyboard messages when
    // VK_LSHIFT changes state.
    if (i != VK_SHIFT && i != VK_CONTROL && i != VK_MENU) {
      if (((new_keyboard_state[i] ^ _keyboard_state[i]) & 0x80) != 0) {
        // This key has changed state.
        if ((new_keyboard_state[i] & 0x80) != 0) {
          // The key is now held down.
          if (windisplay_cat.is_debug()) {
            windisplay_cat.debug()
              << "key has gone down: " << i << " (" << lookup_key(i) << ")\n";
          }
          
          handle_keyresume(lookup_key(i), message_time);
        } else {
          // The key is now released.
          if (windisplay_cat.is_debug()) {
            windisplay_cat.debug()
              << "key has gone up: " << i << " (" << lookup_key(i) << ")\n";
          }
          handle_keyrelease(lookup_key(i), message_time);
        }
      } else {
        // This key is in the same state.
        if (windisplay_cat.is_debug()) {
          if ((new_keyboard_state[i] & 0x80) != 0) {
            windisplay_cat.debug()
              << "key is still down: " << i << " (" << lookup_key(i) << ")\n";
          }
        }
      }
    }
  }

  // Keypresses are no longer lost.
  _lost_keypresses = false;
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::update_cursor_window
//       Access: Private, Static
//  Description: Changes _cursor_window from its current value to the
//               indicated value.  This also changes the cursor
//               properties appropriately.
////////////////////////////////////////////////////////////////////
void WinGraphicsWindow::
update_cursor_window(WinGraphicsWindow *to_window) {
  bool hide_cursor = false;
  if (to_window == (WinGraphicsWindow *)NULL) {
    // We are leaving a graphics window; we should restore the Win2000
    // effects.
    if (_got_saved_params) {
      SystemParametersInfo(SPI_SETMOUSETRAILS, NULL, 
                           (PVOID)_saved_mouse_trails, NULL);
      SystemParametersInfo(SPI_SETCURSORSHADOW, NULL, 
                           (PVOID)_saved_cursor_shadow, NULL);
      SystemParametersInfo(SPI_SETMOUSEVANISH, NULL,
                           (PVOID)_saved_mouse_vanish, NULL);
      _got_saved_params = false;
    }

  } else {
    const WindowProperties &to_props = to_window->get_properties();
    hide_cursor = to_props.get_cursor_hidden();

    // We are entering a graphics window; we should save and disable
    // the Win2000 effects.  These don't work at all well over a 3-D
    // window.

    // These parameters are only defined for Win2000/XP, but they
    // should just cause a silent error on earlier OS's, which is OK.
    if (!_got_saved_params) {
      SystemParametersInfo(SPI_GETMOUSETRAILS, NULL, 
                           &_saved_mouse_trails, NULL);
      SystemParametersInfo(SPI_GETCURSORSHADOW, NULL, 
                           &_saved_cursor_shadow, NULL);
      SystemParametersInfo(SPI_GETMOUSEVANISH, NULL, 
                           &_saved_mouse_vanish, NULL);
      _got_saved_params = true;

      SystemParametersInfo(SPI_SETMOUSETRAILS, NULL, (PVOID)0, NULL);
      SystemParametersInfo(SPI_SETCURSORSHADOW, NULL, (PVOID)false, NULL);
      SystemParametersInfo(SPI_SETMOUSEVANISH, NULL, (PVOID)false, NULL);
    }
  }

  hide_or_show_cursor(hide_cursor);

  _cursor_window = to_window;
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::hide_or_show_cursor
//       Access: Private, Static
//  Description: Hides or shows the mouse cursor according to the
//               indicated parameter.  This is normally called when
//               the mouse wanders into or out of a window with the
//               cursor_hidden properties.
////////////////////////////////////////////////////////////////////
void WinGraphicsWindow::
hide_or_show_cursor(bool hide_cursor) {
  if (hide_cursor) {
    if (!_cursor_hidden) {
      ShowCursor(false);
      _cursor_hidden = true;
    }
  } else {
    if (_cursor_hidden) {
      ShowCursor(true);
      _cursor_hidden = false;
    }
  }
}
  
////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::register_window_class
//       Access: Private, Static
//  Description: Registers a Window class for all WinGraphicsWindows.
//               This only needs to be done once per session.
////////////////////////////////////////////////////////////////////
void WinGraphicsWindow::
register_window_class() {
  if (_window_class_registered) {
    return;
  }

  WNDCLASS wc;

  HINSTANCE instance = GetModuleHandle(NULL);

  // Clear before filling in window structure!
  ZeroMemory(&wc, sizeof(WNDCLASS));
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = (WNDPROC)static_window_proc;
  wc.hInstance = instance;

  // Might be nice to move these properties into the WindowProperties
  // structure, so they don't have to be global for all windows.
  string windows_icon_filename = get_icon_filename().to_os_specific();
  string windows_mono_cursor_filename = get_mono_cursor_filename().to_os_specific();

  if (!windows_icon_filename.empty()) {
    // Note: LoadImage seems to cause win2k internal heap corruption
    // (outputdbgstr warnings) if icon is more than 8bpp

    // loads a .ico fmt file
    wc.hIcon = (HICON)LoadImage(NULL, windows_icon_filename.c_str(),
                                IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

    if (wc.hIcon == NULL) {
      windisplay_cat.warning()
        << "windows icon filename '" << windows_icon_filename
        << "' not found!!\n";
    }
  } else {
    wc.hIcon = NULL; // use default app icon
  }

  _loaded_custom_cursor = false;
  if (!windows_mono_cursor_filename.empty()) {
    // Note: LoadImage seems to cause win2k internal heap corruption
    // (outputdbgstr warnings) if icon is more than 8bpp (because it
    // was 'mapping' 16bpp colors to the device?)
    
    DWORD load_flags = LR_LOADFROMFILE;

    /*
    if (_props._fullscreen) {
      // I think cursors should use LR_CREATEDIBSECTION since they
      // should not be mapped to the device palette (in the case of
      // 256-color cursors) since they are not going to be used on the
      // desktop
      load_flags |= LR_CREATEDIBSECTION;

      // Of course, we can't make this determination here because one
      // window class is used for all windows, fullscreen as well as
      // desktop windows.
    }
    */

    // loads a .cur fmt file
    _mouse_cursor = (HCURSOR) LoadImage(NULL, windows_mono_cursor_filename.c_str(), IMAGE_CURSOR, 0, 0, load_flags);
    
    if (_mouse_cursor == NULL) {
      windisplay_cat.warning()
        << "windows cursor filename '" << windows_mono_cursor_filename
        << "' not found!!\n";
    } else {
      _loaded_custom_cursor = true;
    }
  }

  if (!_loaded_custom_cursor) {
    _mouse_cursor = LoadCursor(NULL, IDC_ARROW);
  }

  // even if cursor isnt visible, we need to load it so its visible
  // in client-area window border
  wc.hCursor = _mouse_cursor;  
  wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = _window_class_name;
  
  if (!RegisterClass(&wc)) {
    windisplay_cat.error()
      << "could not register window class!" << endl;
    return;
  }
  _window_class_registered = true;
}

// dont pick any video modes < MIN_REFRESH_RATE Hz
#define MIN_REFRESH_RATE 60
// EnumDisplaySettings may indicate 0 or 1 for refresh rate, which means use driver default rate (assume its >min_refresh_rate)
#define ACCEPTABLE_REFRESH_RATE(RATE) ((RATE >= MIN_REFRESH_RATE) || (RATE==0) || (RATE==1))

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::find_acceptable_display_mode
//       Access: Private, Static
//  Description: Looks for a fullscreen mode that meets the specified
//               size and bitdepth requirements.  Returns true if a
//               suitable mode is found, false otherwise.
////////////////////////////////////////////////////////////////////
bool WinGraphicsWindow::
find_acceptable_display_mode(DWORD dwWidth, DWORD dwHeight, DWORD bpp,
                             DEVMODE &dm) {
  int modenum = 0;

  while (1) {
    ZeroMemory(&dm, sizeof(dm));
    dm.dmSize = sizeof(dm);
    
    if (!EnumDisplaySettings(NULL, modenum, &dm)) {
      break;
    }
    
    if ((dm.dmPelsWidth == dwWidth) && (dm.dmPelsHeight == dwHeight) &&
        (dm.dmBitsPerPel == bpp) && 
        ACCEPTABLE_REFRESH_RATE(dm.dmDisplayFrequency)) {
      return true;
    }
    modenum++;
  }
  
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::show_error_message
//       Access: Private, Static
//  Description: Pops up a dialog box with the indicated Windows error
//               message ID (or the last error message generated) for
//               meaningful display to the user.
////////////////////////////////////////////////////////////////////
void WinGraphicsWindow::
show_error_message(DWORD message_id) {
  LPTSTR message_buffer;

  if (message_id == 0) {
    message_id = GetLastError();
  }
  
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL, message_id,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language
                (LPTSTR)&message_buffer,  // the weird ptrptr->ptr cast is intentional, see FORMAT_MESSAGE_ALLOCATE_BUFFER
                1024, NULL);
  MessageBox(GetDesktopWindow(), message_buffer, _T(errorbox_title), MB_OK);
  windisplay_cat.fatal() << "System error msg: " << message_buffer << endl;
  LocalFree(message_buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::lookup_key
//       Access: Private
//  Description: Translates the keycode reported by Windows to an
//               appropriate Panda ButtonHandle.
////////////////////////////////////////////////////////////////////
ButtonHandle WinGraphicsWindow::
lookup_key(WPARAM wparam) const {
  // First, check for a few buttons that we filter out when the IME
  // window is open.
  if (!_ime_active) {
    switch(wparam) {
    case VK_BACK: return KeyboardButton::backspace();
    case VK_DELETE: return KeyboardButton::del();
    case VK_ESCAPE: return KeyboardButton::escape();
    case VK_SPACE: return KeyboardButton::space();
    case VK_UP: return KeyboardButton::up();
    case VK_DOWN: return KeyboardButton::down();
    case VK_LEFT: return KeyboardButton::left();
    case VK_RIGHT: return KeyboardButton::right();
    }
  }

  // Now check for the rest of the buttons, including the ones that
  // we allow through even when the IME window is open.
  switch(wparam) {
  case VK_TAB: return KeyboardButton::tab();
  case VK_PRIOR: return KeyboardButton::page_up();
  case VK_NEXT: return KeyboardButton::page_down();
  case VK_HOME: return KeyboardButton::home();
  case VK_END: return KeyboardButton::end();
  case VK_F1: return KeyboardButton::f1();
  case VK_F2: return KeyboardButton::f2();
  case VK_F3: return KeyboardButton::f3();
  case VK_F4: return KeyboardButton::f4();
  case VK_F5: return KeyboardButton::f5();
  case VK_F6: return KeyboardButton::f6();
  case VK_F7: return KeyboardButton::f7();
  case VK_F8: return KeyboardButton::f8();
  case VK_F9: return KeyboardButton::f9();
  case VK_F10: return KeyboardButton::f10();
  case VK_F11: return KeyboardButton::f11();
  case VK_F12: return KeyboardButton::f12();
  case VK_INSERT: return KeyboardButton::insert();
  case VK_CAPITAL: return KeyboardButton::caps_lock();
  case VK_NUMLOCK: return KeyboardButton::num_lock();
  case VK_SCROLL: return KeyboardButton::scroll_lock();
  case VK_SNAPSHOT: return KeyboardButton::print_screen();

  case VK_SHIFT:
  case VK_LSHIFT:
  case VK_RSHIFT:
    return KeyboardButton::shift();

  case VK_CONTROL:
  case VK_LCONTROL:
  case VK_RCONTROL:
    return KeyboardButton::control();

  case VK_MENU:
  case VK_LMENU:
  case VK_RMENU:
    return KeyboardButton::alt();

  default:
    int key = MapVirtualKey(wparam, 2);
    if (isascii(key) && key != 0) {
      // We used to try to remap lowercase to uppercase keys
      // here based on the state of the shift and/or caps lock
      // keys.  But that's a mistake, and doesn't allow for
      // international or user-defined keyboards; let Windows
      // do that mapping.

      // Nowadays, we make a distinction between a "button"
      // and a "keystroke".  A button corresponds to a
      // physical button on the keyboard and has a down and up
      // event associated.  A keystroke may or may not
      // correspond to a physical button, but will be some
      // Unicode character and will not have a corresponding
      // up event.
      return KeyboardButton::ascii_key(tolower(key));
    }
    break;
  }
  return ButtonHandle::none();
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::handle_mouse_motion
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
bool WinGraphicsWindow::
handle_mouse_motion(int x, int y) {
  _input_devices[0].set_pointer_in_window(x, y);
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::handle_mouse_exit
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void WinGraphicsWindow::
handle_mouse_exit() {
  // note: 'mouse_motion' is considered the 'entry' event
  _input_devices[0].set_pointer_out_of_window();
}

// pops up MsgBox w/system error msg
void PrintErrorMessage(DWORD msgID) {
  LPTSTR pMessageBuffer;

  if (msgID==PRINT_LAST_ERROR)
    msgID=GetLastError();

  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                NULL,msgID,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language
                (LPTSTR) &pMessageBuffer,  // the weird ptrptr->ptr cast is intentional, see FORMAT_MESSAGE_ALLOCATE_BUFFER
                1024, NULL);
  MessageBox(GetDesktopWindow(),pMessageBuffer,_T(errorbox_title),MB_OK);
  windisplay_cat.fatal() << "System error msg: " << pMessageBuffer << endl;
  LocalFree( pMessageBuffer );
}

void
ClearToBlack(HWND hWnd, const WindowProperties &props) {
  if (!props.has_origin()) {
    windisplay_cat.info()
      << "Skipping ClearToBlack, no origin specified yet.\n";
    return;
  }

  if (windisplay_cat.is_debug()) {
    windisplay_cat.debug()
      << "ClearToBlack(" << hWnd << ", " << props << ")\n";
  }
  // clear to black
  HDC hDC=GetDC(hWnd);  // GetDC is not particularly fast.  if this needs to be super-quick, we should cache GetDC's hDC
  RECT clrRect = {
    props.get_x_origin(), props.get_y_origin(),
    props.get_x_origin() + props.get_x_size(),
    props.get_y_origin() + props.get_y_size()
  };
  FillRect(hDC,&clrRect,(HBRUSH)GetStockObject(BLACK_BRUSH));
  ReleaseDC(hWnd,hDC);
  GdiFlush();
}

////////////////////////////////////////////////////////////////////
//     Function: get_client_rect_screen
//  Description: Fills view_rect with the coordinates of the client
//               area of the indicated window, converted to screen
//               coordinates.
////////////////////////////////////////////////////////////////////
void get_client_rect_screen(HWND hwnd, RECT *view_rect) {
  GetClientRect(hwnd, view_rect);

  POINT ul, lr;
  ul.x = view_rect->left;
  ul.y = view_rect->top;
  lr.x = view_rect->right;
  lr.y = view_rect->bottom;

  ClientToScreen(hwnd, &ul);
  ClientToScreen(hwnd, &lr);

  view_rect->left = ul.x;
  view_rect->top = ul.y;
  view_rect->right = lr.x;
  view_rect->bottom = lr.y;
}
