/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file x11GraphicsPipe.cxx
 * @author rdb
 * @date 2009-07-07
 */

#include "x11GraphicsPipe.h"
#include "x11GraphicsWindow.h"
#include "config_x11display.h"
#include "frameBufferProperties.h"
#include "displayInformation.h"

#include <dlfcn.h>

TypeHandle x11GraphicsPipe::_type_handle;

bool x11GraphicsPipe::_error_handlers_installed = false;
x11GraphicsPipe::ErrorHandlerFunc *x11GraphicsPipe::_prev_error_handler;
x11GraphicsPipe::IOErrorHandlerFunc *x11GraphicsPipe::_prev_io_error_handler;
bool x11GraphicsPipe::_x_error_messages_enabled = true;
int x11GraphicsPipe::_x_error_count = 0;

LightReMutex x11GraphicsPipe::_x_mutex;

/**
 *
 */
x11GraphicsPipe::
x11GraphicsPipe(const std::string &display) :
  _have_xrandr(false),
  _xcursor_size(-1),
  _XF86DGADirectVideo(nullptr) {

  std::string display_spec = display;
  if (display_spec.empty()) {
    display_spec = display_cfg;
  }
  if (display_spec.empty()) {
    display_spec = ExecutionEnvironment::get_environment_variable("DISPLAY");
  }
  if (display_spec.empty()) {
    display_spec = ":0.0";
  }

  // The X docs say we should do this to get international character support
  // from the keyboard.
  setlocale(LC_ALL, "");

  // But it's important that we use the "C" locale for numeric formatting,
  // since all of the internal Panda code assumes this--we need a decimal
  // point to mean a decimal point.
  setlocale(LC_NUMERIC, "C");

  _is_valid = false;
  _supported_types = OT_window | OT_buffer | OT_texture_buffer;
  _display = nullptr;
  _screen = 0;
  _root = (X11_Window)nullptr;
  _im = (XIM)nullptr;
  _hidden_cursor = None;

  // According to the documentation, we should call this before making any
  // other Xlib calls if we wish to use the Xlib locking system.
  if (x_init_threads) {
    XInitThreads();
  }

  install_error_handlers();

  _display = XOpenDisplay(display_spec.c_str());
  if (!_display) {
    x11display_cat.error()
      << "Could not open display \"" << display_spec << "\".\n";
    _is_valid = false;
    _screen = None;
    _root = None;
    _display_width = 0;
    _display_height = 0;
    return;
  }

  if (!XSupportsLocale()) {
    x11display_cat.warning()
      << "X does not support locale " << setlocale(LC_ALL, nullptr) << "\n";
  }
  XSetLocaleModifiers("");

  _screen = DefaultScreen(_display);
  _root = RootWindow(_display, _screen);
  _display_width = DisplayWidth(_display, _screen);
  _display_height = DisplayHeight(_display, _screen);
  _is_valid = true;

  // Dynamically load the xf86dga extension.
  void *xf86dga = dlopen("libXxf86dga.so.1", RTLD_NOW | RTLD_LOCAL);
  if (xf86dga != nullptr) {
    pfn_XF86DGAQueryVersion _XF86DGAQueryVersion = (pfn_XF86DGAQueryVersion)dlsym(xf86dga, "XF86DGAQueryVersion");
    _XF86DGADirectVideo = (pfn_XF86DGADirectVideo)dlsym(xf86dga, "XF86DGADirectVideo");

    int major_ver, minor_ver;
    if (_XF86DGAQueryVersion == nullptr || _XF86DGADirectVideo == nullptr) {
      x11display_cat.warning()
        << "libXxf86dga.so.1 does not provide required functions; relative mouse mode will not work.\n";

    } else if (!_XF86DGAQueryVersion(_display, &major_ver, &minor_ver)) {
      _XF86DGADirectVideo = nullptr;
    }
  } else {
    _XF86DGADirectVideo = nullptr;
    if (x11display_cat.is_debug()) {
      x11display_cat.debug()
        << "cannot dlopen libXxf86dga.so.1; cursor changing will not work.\n";
    }
  }

  // Dynamically load the XCursor extension.
  void *xcursor = dlopen("libXcursor.so.1", RTLD_NOW | RTLD_LOCAL);
  if (xcursor != nullptr) {
    pfn_XcursorGetDefaultSize _XcursorGetDefaultSize = (pfn_XcursorGetDefaultSize)dlsym(xcursor, "XcursorGetDefaultSize");
    _XcursorXcFileLoadImages = (pfn_XcursorXcFileLoadImages)dlsym(xcursor, "XcursorXcFileLoadImages");
    _XcursorImagesLoadCursor = (pfn_XcursorImagesLoadCursor)dlsym(xcursor, "XcursorImagesLoadCursor");
    _XcursorImagesDestroy = (pfn_XcursorImagesDestroy)dlsym(xcursor, "XcursorImagesDestroy");
    _XcursorImageCreate = (pfn_XcursorImageCreate)dlsym(xcursor, "XcursorImageCreate");
    _XcursorImageLoadCursor = (pfn_XcursorImageLoadCursor)dlsym(xcursor, "XcursorImageLoadCursor");
    _XcursorImageDestroy = (pfn_XcursorImageDestroy)dlsym(xcursor, "XcursorImageDestroy");

    if (_XcursorGetDefaultSize == nullptr || _XcursorXcFileLoadImages == nullptr ||
        _XcursorImagesLoadCursor == nullptr || _XcursorImagesDestroy == nullptr ||
        _XcursorImageCreate == nullptr || _XcursorImageLoadCursor == nullptr ||
        _XcursorImageDestroy == nullptr) {
      _xcursor_size = -1;
      x11display_cat.warning()
        << "libXcursor.so.1 does not provide required functions; cursor changing will not work.\n";

    } else if (x_cursor_size.get_value() >= 0) {
      _xcursor_size = x_cursor_size;
    } else {
      _xcursor_size = _XcursorGetDefaultSize(_display);
    }
  } else {
    _xcursor_size = -1;
    if (x11display_cat.is_debug()) {
      x11display_cat.debug()
        << "cannot dlopen libXcursor.so.1; cursor changing will not work.\n";
    }
  }

  // Dynamically load the XRandr extension.
  void *xrandr = dlopen("libXrandr.so.2", RTLD_NOW | RTLD_LOCAL);
  if (xrandr != nullptr) {
    pfn_XRRQueryExtension _XRRQueryExtension = (pfn_XRRQueryExtension)dlsym(xrandr, "XRRQueryExtension");
    pfn_XRRQueryVersion _XRRQueryVersion = (pfn_XRRQueryVersion)dlsym(xrandr, "XRRQueryVersion");

    _XRRSizes = (pfn_XRRSizes)dlsym(xrandr, "XRRSizes");
    _XRRRates = (pfn_XRRRates)dlsym(xrandr, "XRRRates");
    _XRRGetScreenInfo = (pfn_XRRGetScreenInfo)dlsym(xrandr, "XRRGetScreenInfo");
    _XRRConfigCurrentConfiguration = (pfn_XRRConfigCurrentConfiguration)dlsym(xrandr, "XRRConfigCurrentConfiguration");
    _XRRSetScreenConfig = (pfn_XRRSetScreenConfig)dlsym(xrandr, "XRRSetScreenConfig");

    int event, error, major, minor;
    if (_XRRQueryExtension == nullptr || _XRRSizes == nullptr || _XRRRates == nullptr ||
        _XRRGetScreenInfo == nullptr || _XRRConfigCurrentConfiguration == nullptr ||
        _XRRSetScreenConfig == nullptr || _XRRQueryVersion == nullptr) {
      _have_xrandr = false;
      x11display_cat.warning()
        << "libXrandr.so.2 does not provide required functions; resolution setting will not work.\n";
    }
    else if (_XRRQueryExtension(_display, &event, &error) &&
             _XRRQueryVersion(_display, &major, &minor)) {
      _have_xrandr = true;
      if (x11display_cat.is_debug()) {
        x11display_cat.debug()
          << "Found RandR extension " << major << "." << minor << "\n";
      }

      if (major > 1 || (major == 1 && minor >= 2)) {
        if (major > 1 || (major == 1 && minor >= 3)) {
          _XRRGetScreenResourcesCurrent = (pfn_XRRGetScreenResources)
            dlsym(xrandr, "XRRGetScreenResourcesCurrent");
        } else {
          // Fall back to this slower version.
          _XRRGetScreenResourcesCurrent = (pfn_XRRGetScreenResources)
            dlsym(xrandr, "XRRGetScreenResources");
        }

        _XRRFreeScreenResources = (pfn_XRRFreeScreenResources)dlsym(xrandr, "XRRFreeScreenResources");
        _XRRGetCrtcInfo = (pfn_XRRGetCrtcInfo)dlsym(xrandr, "XRRGetCrtcInfo");
        _XRRFreeCrtcInfo = (pfn_XRRFreeCrtcInfo)dlsym(xrandr, "XRRFreeCrtcInfo");
      } else {
        _XRRGetScreenResourcesCurrent = nullptr;
        _XRRFreeScreenResources = nullptr;
        _XRRGetCrtcInfo = nullptr;
        _XRRFreeCrtcInfo = nullptr;
      }
    } else {
      _have_xrandr = false;
      if (x11display_cat.is_debug()) {
        x11display_cat.debug()
          << "RandR extension not supported; resolution setting will not work.\n";
      }
    }
  } else {
    _have_xrandr = false;
    if (x11display_cat.is_debug()) {
      x11display_cat.debug()
        << "cannot dlopen libXrandr.so.2; resolution setting will not work.\n";
    }
  }

  // Use Xrandr to fill in the supported resolution list.
  if (_have_xrandr) {
    // If we have XRRGetScreenResources, we prefer that.  It seems to be more
    // reliable than XRRSizes in multi-monitor set-ups.
    if (auto res = get_screen_resources()) {
      if (x11display_cat.is_debug()) {
        x11display_cat.debug()
          << "Using XRRScreenResources to obtain display modes\n";
      }
      _display_information->_total_display_modes = res->nmode;
      _display_information->_display_mode_array = new DisplayMode[res->nmode];
      for (int i = 0; i < res->nmode; ++i) {
        XRRModeInfo &mode = res->modes[i];

        DisplayMode *dm = _display_information->_display_mode_array + i;
        dm->width = mode.width;
        dm->height = mode.height;
        dm->bits_per_pixel = -1;
        dm->fullscreen_only = false;

        if (mode.hTotal && mode.vTotal) {
          dm->refresh_rate = (double)mode.dotClock /
            ((double)mode.hTotal * (double)mode.vTotal);
        } else {
          dm->refresh_rate = 0;
        }
      }
    } else {
      if (x11display_cat.is_debug()) {
        x11display_cat.debug()
          << "Using XRRSizes and XRRRates to obtain display modes\n";
      }

      int num_sizes, num_rates;
      XRRScreenSize *xrrs;
      xrrs = _XRRSizes(_display, 0, &num_sizes);
      _display_information->_total_display_modes = 0;
      for (int i = 0; i < num_sizes; ++i) {
        _XRRRates(_display, 0, i, &num_rates);
        _display_information->_total_display_modes += num_rates;
      }

      short *rates;
      short counter = 0;
      _display_information->_display_mode_array = new DisplayMode[_display_information->_total_display_modes];
      for (int i = 0; i < num_sizes; ++i) {
        int num_rates;
        rates = _XRRRates(_display, 0, i, &num_rates);
        for (int j = 0; j < num_rates; ++j) {
          DisplayMode* dm = _display_information->_display_mode_array + counter;
          dm->width = xrrs[i].width;
          dm->height = xrrs[i].height;
          dm->refresh_rate = rates[j];
          dm->bits_per_pixel = -1;
          dm->fullscreen_only = false;
          ++counter;
        }
      }
    }
  }

  // Connect to an input method for supporting international text entry.
  _im = XOpenIM(_display, nullptr, nullptr, nullptr);
  if (_im == (XIM)nullptr) {
    x11display_cat.warning()
      << "Couldn't open input method.\n";
  }

  // What styles does the current input method support?
  /*
  XIMStyles *im_supported_styles;
  XGetIMValues(_im, XNQueryInputStyle, &im_supported_styles, NULL);

  for (int i = 0; i < im_supported_styles->count_styles; i++) {
    XIMStyle style = im_supported_styles->supported_styles[i];
    cerr << "style " << i << ". " << hex << style << dec << "\n";
  }

  XFree(im_supported_styles);
  */

  // Get some X atom numbers.
  _wm_delete_window = XInternAtom(_display, "WM_DELETE_WINDOW", false);
  _net_wm_pid = XInternAtom(_display, "_NET_WM_PID", false);
  _net_wm_window_type = XInternAtom(_display, "_NET_WM_WINDOW_TYPE", false);
  _net_wm_window_type_splash = XInternAtom(_display, "_NET_WM_WINDOW_TYPE_SPLASH", false);
  _net_wm_window_type_fullscreen = XInternAtom(_display, "_NET_WM_WINDOW_TYPE_FULLSCREEN", false);
  _net_wm_state = XInternAtom(_display, "_NET_WM_STATE", false);
  _net_wm_state_fullscreen = XInternAtom(_display, "_NET_WM_STATE_FULLSCREEN", false);
  _net_wm_state_above = XInternAtom(_display, "_NET_WM_STATE_ABOVE", false);
  _net_wm_state_below = XInternAtom(_display, "_NET_WM_STATE_BELOW", false);
  _net_wm_state_add = XInternAtom(_display, "_NET_WM_STATE_ADD", false);
  _net_wm_state_remove = XInternAtom(_display, "_NET_WM_STATE_REMOVE", false);
  _net_wm_bypass_compositor = XInternAtom(_display, "_NET_WM_BYPASS_COMPOSITOR", false);
}

/**
 *
 */
x11GraphicsPipe::
~x11GraphicsPipe() {
  release_hidden_cursor();
  if (_im) {
    XCloseIM(_im);
  }
  if (_display) {
    XCloseDisplay(_display);
  }
}

/**
 * Returns an XRRScreenResources object, or null if RandR 1.2 is not supported.
 */
std::unique_ptr<XRRScreenResources, pfn_XRRFreeScreenResources> x11GraphicsPipe::
get_screen_resources() const {
  XRRScreenResources *res = nullptr;

  if (_have_xrandr && _XRRGetScreenResourcesCurrent != nullptr) {
    res = _XRRGetScreenResourcesCurrent(_display, _root);
  }

  return std::unique_ptr<XRRScreenResources, pfn_XRRFreeScreenResources>(res, _XRRFreeScreenResources);
}

/**
 * Returns an XRRCrtcInfo object, or null if RandR 1.2 is not supported.
 */
std::unique_ptr<XRRCrtcInfo, pfn_XRRFreeCrtcInfo> x11GraphicsPipe::
get_crtc_info(XRRScreenResources *res, RRCrtc crtc) const {
  XRRCrtcInfo *info = nullptr;

  if (_have_xrandr && _XRRGetCrtcInfo != nullptr) {
    info = _XRRGetCrtcInfo(_display, res, crtc);
  }

  return std::unique_ptr<XRRCrtcInfo, pfn_XRRFreeCrtcInfo>(info, _XRRFreeCrtcInfo);
}

/**
 * Finds a CRTC for going fullscreen to, at the given origin.  The new CRTC
 * is returned, along with its x, y, width and height.
 *
 * If the required RandR extension is not supported, a value of None will be
 * returned, but x, y, width and height will still be populated.
 */
RRCrtc x11GraphicsPipe::
find_fullscreen_crtc(const LPoint2i &point,
                     int &x, int &y, int &width, int &height) {
  x = 0;
  y = 0;
  width = DisplayWidth(_display, _screen);
  height = DisplayHeight(_display, _screen);

  if (auto res = get_screen_resources()) {
    for (int i = 0; i < res->ncrtc; ++i) {
      RRCrtc crtc = res->crtcs[i];
      if (auto info = get_crtc_info(res.get(), crtc)) {
        if (point[0] >= info->x && point[0] < info->x + info->width &&
            point[1] >= info->y && point[1] < info->y + info->height) {

          x = info->x;
          y = info->y;
          width = info->width;
          height = info->height;
          return crtc;
        }
      }
    }
  }

  return None;
}

/**
 * Returns an indication of the thread in which this GraphicsPipe requires its
 * window processing to be performed: typically either the app thread (e.g.
 * X) or the draw thread (Windows).
 */
GraphicsPipe::PreferredWindowThread
x11GraphicsPipe::get_preferred_window_thread() const {
  // Actually, since we're creating the graphics context in open_window() now,
  // it appears we need to ensure the open_window() call is performed in the
  // draw thread for now, even though X wants all of its calls to be single-
  // threaded.

  // This means that all X windows may have to be handled by the same draw
  // thread, which we didn't intend (though the global _x_mutex may allow them
  // to be technically served by different threads, even though the actual X
  // calls will be serialized).  There might be a better way.

  return PWT_draw;
}

/**
 * Called once to make an invisible Cursor for return from
 * get_hidden_cursor().
 */
void x11GraphicsPipe::
make_hidden_cursor() {
  nassertv(_hidden_cursor == None);

  unsigned int x_size, y_size;
  XQueryBestCursor(_display, _root, 1, 1, &x_size, &y_size);

  Pixmap empty = XCreatePixmap(_display, _root, x_size, y_size, 1);

  XColor black;
  memset(&black, 0, sizeof(black));

  _hidden_cursor = XCreatePixmapCursor(_display, empty, empty,
                                       &black, &black, x_size, y_size);
  XFreePixmap(_display, empty);
}

/**
 * Called once to release the invisible cursor created by
 * make_hidden_cursor().
 */
void x11GraphicsPipe::
release_hidden_cursor() {
  if (_hidden_cursor != None) {
    XFreeCursor(_display, _hidden_cursor);
    _hidden_cursor = None;
  }
}

/**
 * Installs new Xlib error handler functions if this is the first time this
 * function has been called.  These error handler functions will attempt to
 * reduce Xlib's annoying tendency to shut down the client at the first error.
 * Unfortunately, it is difficult to play nice with the client if it has
 * already installed its own error handlers.
 */
void x11GraphicsPipe::
install_error_handlers() {
  if (_error_handlers_installed) {
    return;
  }

  _prev_error_handler = (ErrorHandlerFunc *)XSetErrorHandler(error_handler);
  _prev_io_error_handler = (IOErrorHandlerFunc *)XSetIOErrorHandler(io_error_handler);
  _error_handlers_installed = true;
}

/**
 * This function is installed as the error handler for a non-fatal Xlib error.
 */
int x11GraphicsPipe::
error_handler(X11_Display *display, XErrorEvent *error) {
  ++_x_error_count;

  static const int msg_len = 80;
  char msg[msg_len];
  XGetErrorText(display, error->error_code, msg, msg_len);

  if (!_x_error_messages_enabled) {
    if (x11display_cat.is_debug()) {
      x11display_cat.debug()
        << msg << "\n";
    }
    return 0;
  }

  x11display_cat.error()
    << msg << "\n";

  if (x_error_abort) {
    abort();
  }

  // We return to allow the application to continue running, unlike the
  // default X error handler which exits.
  return 0;
}

/**
 * This function is installed as the error handler for a fatal Xlib error.
 */
int x11GraphicsPipe::
io_error_handler(X11_Display *display) {
  x11display_cat.fatal()
    << "X fatal error on display " << (void *)display << "\n";

  // Unfortunately, we can't continue from this function, even if we promise
  // never to use X again.  We're supposed to terminate without returning, and
  // if we do return, the caller will exit anyway.  Sigh.  Very poor design on
  // X's part.
  return 0;
}
