// Filename: wglGraphicsPipe.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "wglGraphicsPipe.h"
#include "config_wgldisplay.h"
#include <mouseButton.h>
#include <keyboardButton.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle wglGraphicsPipe::_type_handle;

wglGraphicsPipe *global_pipe;

#define MOUSE_ENTERED 0
#define MOUSE_EXITED 1

wglGraphicsPipe::wglGraphicsPipe(const PipeSpecifier& spec)
  : InteractiveGraphicsPipe(spec)
{
  // Register a standard window class
  WNDCLASS stdwc;
  HINSTANCE hinstance = GetModuleHandle(NULL);

  // Clear before filling in window structure!
  memset(&stdwc, 0, sizeof(WNDCLASS));
  stdwc.style		= CS_OWNDC;
  stdwc.lpfnWndProc	= (WNDPROC)static_window_proc;
  stdwc.hInstance	= hinstance;
  stdwc.hCursor		= LoadCursor(NULL, IDC_CROSS);
  stdwc.hbrBackground	= NULL;
  stdwc.lpszMenuName	= NULL;
  stdwc.lpszClassName	= "wglStandard";

  if(!IconFileName.empty()) {
	  stdwc.hIcon = (HICON) LoadImage(NULL, IconFileName.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
  } else {
	  stdwc.hIcon = NULL; // use default app icon
  }

  if (!RegisterClass(&stdwc)) {
    wgldisplay_cat.fatal()
      << "wglGraphicsPipe::construct(): could not register standard window "
	<< "class" << endl;
    exit(0);
  }

  // Register a fullscreen window class
  WNDCLASS fswc;

  // Clear before filling in window structure!
  memset(&fswc, 0, sizeof(WNDCLASS));
  fswc.style              = CS_HREDRAW | CS_VREDRAW;
  fswc.lpfnWndProc        = (WNDPROC)static_window_proc;
  fswc.hInstance          = hinstance;
  fswc.hIcon              = NULL;
  fswc.hCursor            = LoadCursor(hinstance, IDC_CROSS);
  fswc.hbrBackground      = NULL;
  fswc.lpszMenuName       = NULL;
  fswc.lpszClassName      = "wglFullscreen";

  if (!RegisterClass(&fswc)) {
    wgldisplay_cat.fatal()
      << "wglGraphicsPipe::construct(): could not register fullscreen window "
        << "class" << endl;
    exit(0);
  }

  _width = GetSystemMetrics(SM_CXSCREEN);
  _height = GetSystemMetrics(SM_CYSCREEN);
  _shift = false;
  global_pipe = this;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsPipe::get_window_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle of the kind of window
//               preferred by this kind of pipe.
////////////////////////////////////////////////////////////////////
TypeHandle wglGraphicsPipe::
get_window_type() const {
  return wglGraphicsWindow::get_class_type();
}

GraphicsPipe *wglGraphicsPipe::
make_wglGraphicsPipe(const FactoryParams &params) {
  GraphicsPipe::PipeSpec *pipe_param;
  if (!get_param_into(pipe_param, params)) {
    return new wglGraphicsPipe(PipeSpecifier());
  } else {
    return new wglGraphicsPipe(pipe_param->get_specifier());
  }
}


TypeHandle wglGraphicsPipe::get_class_type(void) {
  return _type_handle;
}

void wglGraphicsPipe::init_type(void) {
  InteractiveGraphicsPipe::init_type();
  register_type(_type_handle, "wglGraphicsPipe",
		InteractiveGraphicsPipe::get_class_type());
}

TypeHandle wglGraphicsPipe::get_type(void) const {
  return get_class_type();
}

wglGraphicsPipe::wglGraphicsPipe(void) {
  wgldisplay_cat.error()
    << "wglGraphicsPipes should not be created with the default constructor"
    << endl;
}

wglGraphicsPipe::wglGraphicsPipe(const wglGraphicsPipe&) {
  wgldisplay_cat.error()
    << "wglGraphicsPipes should not be copied" << endl;
}

wglGraphicsPipe& wglGraphicsPipe::operator=(const wglGraphicsPipe&) {
  wgldisplay_cat.error()
    << "wglGraphicsPipes should not be assigned" << endl;
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: find_window
//       Access:
//  Description: Find the window that has the xwindow "win" in the
//		 window list for the pipe (if it exists)
////////////////////////////////////////////////////////////////////
wglGraphicsWindow *wglGraphicsPipe::
find_window(HWND win) {
  int num_windows = get_num_windows();
  for (int w = 0; w < num_windows; w++) {
    wglGraphicsWindow *window = DCAST(wglGraphicsWindow, get_window(w));
    if (window->_mwindow == win)
      return window;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: static_window_proc 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
LONG WINAPI wglGraphicsPipe::
static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  return global_pipe->window_proc(hwnd, msg, wparam, lparam);
}

////////////////////////////////////////////////////////////////////
//     Function: window_proc 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
LONG wglGraphicsPipe::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  PAINTSTRUCT ps;
  wglGraphicsWindow *window;
  int button = -1;
  int x, y, width, height;
  static HCURSOR hMouseCrossIcon = NULL;

  switch (msg) {
    case WM_CREATE:
      hMouseCrossIcon = LoadCursor(NULL, IDC_CROSS);
      SetCursor(hMouseCrossIcon);
      return 0;
    case WM_CLOSE:
      PostQuitMessage(0);
      return 0;
    case WM_PAINT:
      window = find_window(hwnd);
      if (window) {
        BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
      }
      return 0;
    case WM_SYSCHAR:
    case WM_CHAR:
      return 0;
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
      window = find_window(hwnd);
      if (window) {
	POINT point;
	window->make_current();
	GetCursorPos(&point);
	ScreenToClient(hwnd, &point);
	window->handle_keypress(lookup_key(wparam), point.x, point.y);
      }
      return 0;
    case WM_SYSKEYUP:
    case WM_KEYUP:
      window = find_window(hwnd);
      if (window) {
 	POINT point;
	window->make_current();
	GetCursorPos(&point);
	ScreenToClient(hwnd, &point);
	window->handle_keyrelease(lookup_key(wparam), point.x, point.y);
      }
      return 0;
    case WM_LBUTTONDOWN:
      button = 0;
    case WM_MBUTTONDOWN:
      if (button < 0)
        button = 1;
    case WM_RBUTTONDOWN:
      if (button < 0)
	button = 2;
      SetCapture(hwnd);
      // Win32 doesn't return the same numbers as X does when the mouse
      // goes beyond the upper or left side of the window
      x = LOWORD(lparam);
      y = HIWORD(lparam);
      if (x & 1 << 15) x -= (1 << 16);
      if (y & 1 << 15) y -= (1 << 16);
      window = find_window(hwnd);
      if (window) {
	window->make_current();
	window->handle_keypress(MouseButton::button(button), x, y);
      }
      return 0;
    case WM_LBUTTONUP:
      button = 0;
    case WM_MBUTTONUP:
      if (button < 0)
	button = 1;
    case WM_RBUTTONUP:
      if (button < 0)
	button = 2;
      ReleaseCapture();
      window = find_window(hwnd);
      if (window) {
	x = LOWORD(lparam);
     	y = HIWORD(lparam);
	if (x & 1 << 15) x -= (1 << 16);
	if (y & 1 << 15) y -= (1 << 16);
	window->make_current();
	window->handle_keyrelease(MouseButton::button(button), x, y);
      }
      return 0;
    case WM_MOUSEMOVE:
      window = find_window(hwnd);
      if (window) {
	x = LOWORD(lparam);
    	y = HIWORD(lparam);
	if (x & 1 << 15) x -= (1 << 16);
	if (y & 1 << 15) y -= (1 << 16);
	if (window->mouse_motion_enabled() 
		&& wparam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)) {
	  window->make_current();
	  window->handle_mouse_motion(x, y);
	} else if (window->mouse_passive_motion_enabled() && 
		((wparam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)) == 0)) {
	  window->make_current();
	  window->handle_mouse_motion(x, y);
	}
      }
      return 0;
    case WM_SIZE:
      window = find_window(hwnd);
      if (window) {
	width = LOWORD(lparam);
 	height = HIWORD(lparam);
	window->handle_reshape(width, height);
      }
      return 0;

#if 0
//this is unnecessary, just handle mouse entry
    case WM_SETCURSOR:
      // We need to set the cursor every time the mouse moves on Win32!
      if (LOWORD(lparam) != HTCLIENT)
          return DefWindowProc(hwnd, msg, wparam, lparam);
      window = find_window(hwnd);
      if (window) {
          SetCursor(LoadCursor(NULL, IDC_CROSS));
      }
      return 1;
#endif
    case WM_SETFOCUS:
      SetCursor(hMouseCrossIcon);  
      window = find_window(hwnd);
      if (window) {
	if (window->mouse_entry_enabled()) {
	  window->make_current();
	  window->handle_mouse_entry(MOUSE_ENTERED);
	}
      }
      return 0;
    case WM_KILLFOCUS:
      window = find_window(hwnd);
      if (window) {
	if (window->mouse_entry_enabled()) {
	  window->make_current();
	  window->handle_mouse_entry(MOUSE_EXITED);
	}
      }
      return 0;
    default:
      return DefWindowProc(hwnd, msg, wparam, lparam);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: lookup_key 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
ButtonHandle
wglGraphicsPipe::lookup_key(WPARAM wparam) const {
  switch (wparam) {
  case VK_BACK: return KeyboardButton::backspace();
  case VK_TAB: return KeyboardButton::tab();
  case VK_ESCAPE: return KeyboardButton::escape();
  case VK_SPACE: return KeyboardButton::space();
  case VK_UP: return KeyboardButton::up();
  case VK_DOWN: return KeyboardButton::down();
  case VK_LEFT: return KeyboardButton::left();
  case VK_RIGHT: return KeyboardButton::right();
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
  case VK_DELETE: return KeyboardButton::del();

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
      if (GetKeyState(VK_SHIFT) >= 0)
	key = tolower(key); 
      else {
	switch (key) {
	case '1': key = '!'; break;
	case '2': key = '@'; break;
	case '3': key = '#'; break;
	case '4': key = '$'; break;
	case '5': key = '%'; break;
	case '6': key = '^'; break;
	case '7': key = '&'; break;
	case '8': key = '*'; break;
	case '9': key = '('; break;
	case '0': key = ')'; break;
	case '-': key = '_'; break;
	case '=': key = '+'; break;
	case ',': key = '<'; break;
	case '.': key = '>'; break;
	case '/': key = '?'; break;
	case ';': key = ':'; break;
	case '\'': key = '"'; break;
	case '[': key = '{'; break;
	case ']': key = '}'; break;
	case '\\': key = '|'; break;
	case '`': key = '~'; break;
	}
      }
      return KeyboardButton::ascii_key((uchar)key);
    }
    break;
  }
  return ButtonHandle::none();
}
