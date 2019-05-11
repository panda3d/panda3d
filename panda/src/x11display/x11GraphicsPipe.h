/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file x11GraphicsPipe.h
 * @author rdb
 * @date 2009-07-07
 */

#ifndef X11GRAPHICSPIPE_H
#define X11GRAPHICSPIPE_H

#include "pandabase.h"
#include "graphicsWindow.h"
#include "graphicsPipe.h"
#include "lightMutex.h"
#include "lightReMutex.h"
#include "windowHandle.h"
#include "get_x11.h"
#include "config_x11display.h"

// Excerpt the few definitions we need for the extensions.
#define XF86DGADirectMouse 0x0004

typedef struct _XcursorFile XcursorFile;
typedef struct _XcursorImage XcursorImage;
typedef struct _XcursorImages XcursorImages;

typedef unsigned short Rotation;
typedef unsigned short SizeID;
typedef unsigned long XRRModeFlags;
typedef XID RROutput;
typedef XID RRCrtc;
typedef XID RRMode;

typedef struct _XRRScreenConfiguration XRRScreenConfiguration;
typedef struct {
  int width, height;
  int mwidth, mheight;
} XRRScreenSize;

typedef struct _XRRModeInfo {
  RRMode id;
  unsigned int width;
  unsigned int height;
  unsigned long dotClock;
  unsigned int hSyncStart;
  unsigned int hSyncEnd;
  unsigned int hTotal;
  unsigned int hSkew;
  unsigned int vSyncStart;
  unsigned int vSyncEnd;
  unsigned int vTotal;
  char *name;
  unsigned int nameLength;
  XRRModeFlags modeFlags;
} XRRModeInfo;

typedef struct _XRRScreenResources {
  Time timestamp;
  Time configTimestamp;
  int ncrtc;
  RRCrtc *crtcs;
  int noutput;
  RROutput *outputs;
  int nmode;
  XRRModeInfo *modes;
} XRRScreenResources;

typedef struct _XRRCrtcInfo {
  Time timestamp;
  int x, y;
  unsigned int width, height;
  RRMode mode;
  Rotation rotation;
  int noutput;
  RROutput *outputs;
  Rotation rotations;
  int npossible;
  RROutput *possible;
} XRRCrtcInfo;

typedef void (*pfn_XRRFreeScreenResources)(XRRScreenResources *resources);
typedef void (*pfn_XRRFreeCrtcInfo)(XRRCrtcInfo *crtcInfo);

class FrameBufferProperties;

/**
 * This graphics pipe represents the interface for creating graphics windows
 * on an X-based client.
 */
class x11GraphicsPipe : public GraphicsPipe {
public:
  x11GraphicsPipe(const std::string &display = std::string());
  virtual ~x11GraphicsPipe();

  INLINE X11_Display *get_display() const;
  INLINE int get_screen() const;
  INLINE X11_Window get_root() const;
  INLINE XIM get_im() const;

  INLINE X11_Cursor get_hidden_cursor();

  INLINE bool supports_relative_mouse() const;
  INLINE bool enable_relative_mouse();
  INLINE void disable_relative_mouse();

  static INLINE int disable_x_error_messages();
  static INLINE int enable_x_error_messages();
  static INLINE int get_x_error_count();

  std::unique_ptr<XRRScreenResources, pfn_XRRFreeScreenResources> get_screen_resources() const;
  std::unique_ptr<XRRCrtcInfo, pfn_XRRFreeCrtcInfo> get_crtc_info(XRRScreenResources *res, RRCrtc crtc) const;

  RRCrtc find_fullscreen_crtc(const LPoint2i &point,
                              int &x, int &y, int &width, int &height);

public:
  virtual PreferredWindowThread get_preferred_window_thread() const;

public:
  // Atom specifications.
  Atom _wm_delete_window;
  Atom _net_wm_pid;
  Atom _net_wm_window_type;
  Atom _net_wm_window_type_splash;
  Atom _net_wm_window_type_fullscreen;
  Atom _net_wm_state;
  Atom _net_wm_state_fullscreen;
  Atom _net_wm_state_above;
  Atom _net_wm_state_below;
  Atom _net_wm_state_add;
  Atom _net_wm_state_remove;
  Atom _net_wm_bypass_compositor;

  // Extension functions.
  typedef int (*pfn_XcursorGetDefaultSize)(X11_Display *);
  typedef XcursorImages *(*pfn_XcursorXcFileLoadImages)(XcursorFile *, int);
  typedef X11_Cursor (*pfn_XcursorImagesLoadCursor)(X11_Display *, const XcursorImages *);
  typedef void (*pfn_XcursorImagesDestroy)(XcursorImages *);
  typedef XcursorImage *(*pfn_XcursorImageCreate)(int, int);
  typedef X11_Cursor (*pfn_XcursorImageLoadCursor)(X11_Display *, const XcursorImage *);
  typedef void (*pfn_XcursorImageDestroy)(XcursorImage *);

  int _xcursor_size;
  pfn_XcursorXcFileLoadImages _XcursorXcFileLoadImages;
  pfn_XcursorImagesLoadCursor _XcursorImagesLoadCursor;
  pfn_XcursorImagesDestroy _XcursorImagesDestroy;
  pfn_XcursorImageCreate _XcursorImageCreate;
  pfn_XcursorImageLoadCursor _XcursorImageLoadCursor;
  pfn_XcursorImageDestroy _XcursorImageDestroy;

  typedef Bool (*pfn_XRRQueryExtension)(X11_Display *, int*, int*);
  typedef Status (*pfn_XRRQueryVersion)(X11_Display *, int*, int*);
  typedef XRRScreenSize *(*pfn_XRRSizes)(X11_Display*, int, int*);
  typedef short *(*pfn_XRRRates)(X11_Display*, int, int, int*);
  typedef XRRScreenConfiguration *(*pfn_XRRGetScreenInfo)(X11_Display*, X11_Window);
  typedef SizeID (*pfn_XRRConfigCurrentConfiguration)(XRRScreenConfiguration*, Rotation*);
  typedef Status (*pfn_XRRSetScreenConfig)(X11_Display*, XRRScreenConfiguration *,
                                        Drawable, int, Rotation, Time);

  bool _have_xrandr;
  pfn_XRRSizes _XRRSizes;
  pfn_XRRRates _XRRRates;
  pfn_XRRGetScreenInfo _XRRGetScreenInfo;
  pfn_XRRConfigCurrentConfiguration _XRRConfigCurrentConfiguration;
  pfn_XRRSetScreenConfig _XRRSetScreenConfig;

protected:
  X11_Display *_display;
  int _screen;
  X11_Window _root;
  XIM _im;

  X11_Cursor _hidden_cursor;

  typedef Bool (*pfn_XF86DGAQueryVersion)(X11_Display *, int*, int*);
  typedef Status (*pfn_XF86DGADirectVideo)(X11_Display *, int, int);
  pfn_XF86DGADirectVideo _XF86DGADirectVideo;

  typedef XRRScreenResources *(*pfn_XRRGetScreenResources)(X11_Display*, X11_Window);
  typedef XRRCrtcInfo *(*pfn_XRRGetCrtcInfo)(X11_Display *dpy, XRRScreenResources *resources, RRCrtc crtc);

  pfn_XRRGetScreenResources _XRRGetScreenResourcesCurrent;
  pfn_XRRFreeScreenResources _XRRFreeScreenResources;
  pfn_XRRGetCrtcInfo _XRRGetCrtcInfo;
  pfn_XRRFreeCrtcInfo _XRRFreeCrtcInfo;

private:
  void make_hidden_cursor();
  void release_hidden_cursor();

  static void install_error_handlers();
  static int error_handler(X11_Display *display, XErrorEvent *error);
  static int io_error_handler(X11_Display *display);

  typedef int ErrorHandlerFunc(X11_Display *, XErrorEvent *);
  typedef int IOErrorHandlerFunc(X11_Display *);
  static bool _error_handlers_installed;
  static ErrorHandlerFunc *_prev_error_handler;
  static IOErrorHandlerFunc *_prev_io_error_handler;

  static bool _x_error_messages_enabled;
  static int _x_error_count;

public:
  // This Mutex protects any X library calls, which all have to be single-
  // threaded.  In particular, it protects glXMakeCurrent().
  static LightReMutex _x_mutex;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GraphicsPipe::init_type();
    register_type(_type_handle, "x11GraphicsPipe",
                  GraphicsPipe::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "x11GraphicsPipe.I"

#endif
