// Filename: wcrGraphicsWindow.cxx
// Created by:  skyler, based on wgl* file.
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

#include "wcrGraphicsWindow.h"
#include "wcrGraphicsPipe.h"
#include "config_wcrdisplay.h"
#include "keyboardButton.h"
#include "mouseButton.h"
#include "crGraphicsStateGuardian.h"
#include <errno.h>
#include <time.h>
#include <mmsystem.h>
#include <tchar.h>
#include <map>
#include "throw_event.h"
//#include "eventQueue.h"
#include <string.h>
#include "../wgldisplay/Win32Defs.h"

//#include "ChromiumOpenGL.h"

#ifdef DO_PSTATS
#include "pStatTimer.h"
#endif

#define WCR_WCREXT_PROTOTYPES
#include "wcrext.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle wcrGraphicsWindow::_type_handle;

static bool wc_registered = false;

#define MOUSE_ENTERED 0
#define MOUSE_EXITED 1

#define FONT_BITMAP_OGLDISPLAYLISTNUM 1000    // an arbitrary ID #

#define LAST_ERROR 0
#define ERRORBOX_TITLE "Panda3D Error"
#define WCR_WINDOWCLASSNAME "wcrDisplay"

#define PAUSED_TIMER_ID  7   // completely arbitrary choice

typedef map<HWND,wcrGraphicsWindow *> HWND_PANDAWIN_MAP;

HWND_PANDAWIN_MAP hwnd_pandawin_map;
wcrGraphicsWindow *global_wcrwinptr=NULL;  // need this for temporary windproc

typedef enum {Software, MCD, ICD} OGLDriverType;
static char *OGLDrvStrings[3] = {"Software","MCD","ICD"};

LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam,LPARAM lparam);

// because we dont have access to ModifierButtons, as a hack just synchronize state of these
// keys on get/lose keybd focus
#define NUM_MODIFIER_KEYS 16
unsigned int hardcoded_modifier_buttons[NUM_MODIFIER_KEYS]={VK_SHIFT,VK_MENU,VK_CONTROL,VK_SPACE,VK_TAB,
                                         VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,VK_PRIOR,VK_NEXT,VK_HOME,VK_END,
                                         VK_INSERT,VK_DELETE,VK_ESCAPE};

// dont pick any video modes < MIN_REFRESH_RATE Hz
#define MIN_REFRESH_RATE 60
// EnumDisplaySettings may indicate 0 or 1 for refresh rate, which means use driver default rate (assume its >min_refresh_rate)
#define ACCEPTABLE_REFRESH_RATE(RATE) ((RATE >= MIN_REFRESH_RATE) || (RATE==0) || (RATE==1))

void PrintErrorMessage(DWORD msgID) {
   LPTSTR pMessageBuffer;

   if (msgID==LAST_ERROR)
     msgID=GetLastError();

   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                 NULL,msgID,
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language
                 (LPTSTR) &pMessageBuffer,  // the weird ptrptr->ptr cast is intentional, see FORMAT_MESSAGE_ALLOCATE_BUFFER
                 1024, NULL);
   MessageBox(GetDesktopWindow(),pMessageBuffer,_T(ERRORBOX_TITLE),MB_OK);
   wcrdisplay_cat.fatal() << "System error msg: " << pMessageBuffer << endl;
   LocalFree(pMessageBuffer);
}

// fn exists so AtExitFn can call it without refcntr blowing up since its !=0
void wcrGraphicsWindow::DestroyMe(bool bAtExitFnCalled) {

  _exiting_window = true;  // needed before DestroyWindow call

  // several GL drivers (voodoo,ATI, not nvidia) crash if we call these wcr deletion routines from
  // an atexit() fn.  Possible that GL has already unloaded itself.  So we just wont call them for now
  // for that case, we're exiting the app anyway.
  if (!bAtExitFnCalled) {
      // to do gl releases, we need to have the context be current
      #if 0 //[ TODO:skyler
      if ((_hdc!=NULL)&&(_context!=NULL)) {
          // need to bypass make_current() since it checks _window_inactive 
          // which we need to ignore
          HGLRC current_context = GetCurrentContext();
          //TODO: HDC current_dc = chromium.GetCurrentDC();
          HDC current_dc = NULL;

          if ((current_context != _context) || (current_dc != _hdc)) {
              if (!chromium.MakeCurrent(_hdc, 0, _context)) {
                  PrintErrorMessage(LAST_ERROR);
              }
          }
          report_errors();
      }
      #endif //]

      if (gl_show_fps_meter)
        chromium.DeleteLists(FONT_BITMAP_OGLDISPLAYLISTNUM, 128);

      report_errors();

      // implicitly calls gsg destructors which release GL objects (textures, display lists, etc)
      release_gsg();

      report_errors();
      // cant report errors after we set cur context to NULL

      #if 0 //[ TODO:skyler
      HGLRC curcxt=GetCurrentContext();
      if (curcxt!=NULL)
        unmake_current();

      if (_context!=NULL) {
          chromium.DeleteContext(_context);
          _context = NULL;
      }
      #endif //]
  }

  if (_hdc!=NULL) {
    ReleaseDC(_mwindow,_hdc);
    _hdc = NULL;
  }

  if ((_hOldForegroundWindow!=NULL) && (_mwindow==GetForegroundWindow())) {
      SetForegroundWindow(_hOldForegroundWindow);
  }

  if (_mwindow!=NULL) {
    if (_bLoadedCustomCursor && _hMouseCursor!=NULL)
      DestroyCursor(_hMouseCursor);

    DestroyWindow(_mwindow);
    hwnd_pandawin_map.erase(_mwindow);
    _mwindow = NULL;
  }

  if (_pCurrent_display_settings!=NULL) {
      delete _pCurrent_display_settings;
      _pCurrent_display_settings = NULL;
  }

  if (_props._fullscreen) {
      // revert to default display mode
      ChangeDisplaySettings(NULL,0x0);
  }
}

void wcrGraphicsWindow::do_close_window() {
  GraphicsWindow::do_close_window();
   DestroyMe(false);
}

////////////////////////////////////////////////////////////////////
//     Function: Destructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
wcrGraphicsWindow::~wcrGraphicsWindow() {
   close_window();
}

void DestroyAllWindows(bool bAtExitFnCalled) {
   // need to go through all windows in map var and delete them
   while(!hwnd_pandawin_map.empty()) {
     // cant use a for loop cause DestroyMe erases things out from under us, so iterator is invalid
     HWND_PANDAWIN_MAP::iterator pwin = hwnd_pandawin_map.begin();
     if ((*pwin).second != NULL)
         (*pwin).second->DestroyMe(bAtExitFnCalled);
   }
}

void AtExitFn() {
  #ifdef _DEBUG
    wcrdisplay_cat.spam() << "AtExitFn called\n";
  #endif
  DestroyAllWindows(true);
}

bool find_acceptable_display_mode(DWORD dwWidth,DWORD dwHeight,DWORD bpp,DEVMODE &dm) {
    int modenum=0;

    // look for acceptable mode
    while(1) {
        ZeroMemory(&dm, sizeof(dm));
        dm.dmSize = sizeof(dm);

        if (!EnumDisplaySettings(NULL,modenum,&dm))
          break;

        if ((dm.dmPelsWidth==dwWidth) && (dm.dmPelsHeight==dwHeight) &&
           (dm.dmBitsPerPel==bpp) && ACCEPTABLE_REFRESH_RATE(dm.dmDisplayFrequency)) {
           return true;
        }
        modenum++;
    }

    return false;
}

////////////////////////////////////////////////////////////////////
//     Function: config
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wcrGraphicsWindow::config() {
#if 1 //[
  int spu_ids[] = { 0, 1 };
  char *spu_names[] = { "readback", "pack" };
  //char *spu_names[] = { "print", "pack" };
  SPU *spu;
  spu = crSPULoadChain(sizeof(spu_names)/sizeof(spu_names[0]), spu_ids, 
      spu_names, NULL, NULL);
  if (!spu) {
    cerr << "!spu" << endl;
    exit(0);
  }
  chromium=spu->dispatch_table;
  /***/ cerr << "CRGraphicsStateGuardian chromium:" << (void*)&chromium << 
      " spu:" << (void*)spu << " dispatch:" << (void*)&(spu->dispatch_table) << "\n\n\n\n\n\n" << endl;
  ///***/ __asm int 3
#endif //]

    GraphicsWindow::config();

    HINSTANCE hinstance = GetModuleHandle(NULL);
    HWND hDesktopWindow = GetDesktopWindow();

    global_wcrwinptr = this;  // need this until we get an HWND from CreateWindow

    _exiting_window = false;
    _return_control_to_app = false;
    _bIsLowVidMemCard = false;
    _active_minimized_fullscreen = false;
    _PandaPausedTimer = NULL;
    _mouse_input_enabled = false;
    _mouse_motion_enabled = false;
    _mouse_passive_motion_enabled = false;
    _mouse_entry_enabled = false;
    _context = NULL;
    _hdc = NULL;
    _window_inactive = false;
    _pCurrent_display_settings = NULL;
    _mwindow = NULL;
    _gsg = NULL;
    ZeroMemory(&_pixelformat,sizeof(_pixelformat));
    _hOldForegroundWindow=GetForegroundWindow();

    WNDCLASS wc;

    // Clear before filling in window structure!
    ZeroMemory(&wc, sizeof(WNDCLASS));
    wc.style      = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC) static_window_proc;
    wc.hInstance   = hinstance;

    string windows_icon_filename = get_icon_filename_2().to_os_specific();
    string windows_mono_cursor_filename = get_mono_cursor_filename_2().to_os_specific();

    if (!windows_icon_filename.empty()) {
        // Note: LoadImage seems to cause win2k internal heap corruption (outputdbgstr warnings)
        // if icon is more than 8bpp

        // loads a .ico fmt file
        wc.hIcon = (HICON) LoadImage(NULL, windows_icon_filename.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

        if (wc.hIcon==NULL) {
            wcrdisplay_cat.warning() << "windows icon filename '" << windows_icon_filename << "' not found!!\n";
        }
    } else {
        wc.hIcon = NULL; // use default app icon
    }

    _bLoadedCustomCursor = false;
    if (!windows_mono_cursor_filename.empty()) {
        // Note: LoadImage seems to cause win2k internal heap corruption (outputdbgstr warnings)
        // if icon is more than 8bpp (because it was 'mapping' 16bpp colors to the device?)

        DWORD load_flags = LR_LOADFROMFILE;

        if (_props._fullscreen) {
          // I think cursors should use LR_CREATEDIBSECTION since they should not be mapped to the device palette (in the case of 256-color cursors)
          // since they are not going to be used on the desktop
          load_flags |= LR_CREATEDIBSECTION;
        }

        // loads a .cur fmt file
        _hMouseCursor = (HCURSOR) LoadImage(NULL, windows_mono_cursor_filename.c_str(), IMAGE_CURSOR, 0, 0, load_flags);

        if (_hMouseCursor==NULL) {
            wcrdisplay_cat.warning() << "windows cursor filename '" << windows_mono_cursor_filename << "' not found!!\n";
        }
        _bLoadedCustomCursor = true;
    } else {
        _hMouseCursor = LoadCursor(NULL, IDC_ARROW);
    }

    if (!wc_registered) {
      // We only need to register the window class once per session.
      wc.hCursor = _hMouseCursor;
      wc.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
      wc.lpszMenuName   = NULL;
      wc.lpszClassName  = WCR_WINDOWCLASSNAME;

      if (!RegisterClass(&wc)) {
        wcrdisplay_cat.error() << "could not register window class!" << endl;
      }
      wc_registered = true;
    }

//  from MSDN:
//  An OpenGL window has its own pixel format. Because of this, only device contexts retrieved
//  for the client area of an OpenGL window are allowed to draw into the window. As a result, an
//  OpenGL window should be created with the WS_CLIPCHILDREN and WS_CLIPSIBLINGS styles. Additionally,
//  the window class attribute should not include the CS_PARENTDC style.

    DWORD window_style = WS_POPUP | WS_SYSMENU | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;  // for CreateWindow

    // rect now contains the coords for the entire window, not the client
    if (_props._fullscreen) {
      DWORD dwWidth =  _props._xsize;
      DWORD dwHeight = _props._ysize;

      HDC scrnDC=GetDC(hDesktopWindow);
      DWORD drvr_ver=GetDeviceCaps(scrnDC,DRIVERVERSION);
      DWORD cur_bitdepth=GetDeviceCaps(scrnDC,BITSPIXEL);
      DWORD cur_scrnwidth=GetDeviceCaps(scrnDC,HORZRES);
      DWORD cur_scrnheight=GetDeviceCaps(scrnDC,VERTRES);
      ReleaseDC(hDesktopWindow,scrnDC);

      DWORD dwFullScreenBitDepth=cur_bitdepth;

      DEVMODE dm;
      if (!find_acceptable_display_mode(dwWidth,dwHeight,dwFullScreenBitDepth,dm)) {
          wcrdisplay_cat.fatal() << "Videocard has no supported display resolutions at specified res (" << dwWidth << " X " << dwHeight << " X " << dwFullScreenBitDepth <<")\n";
          exit(1);
      }

      // I'd prefer to CreateWindow after DisplayChange in case it messes up GL somehow,
      // but I need the window's black background to cover up the desktop during the mode change
      _mwindow = CreateWindow(WCR_WINDOWCLASSNAME, _props._title.c_str(),
                window_style,0,0,dwWidth,dwHeight,hDesktopWindow, NULL, hinstance, 0);

      // move window to top of zorder,
      SetWindowPos(_mwindow, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOSIZE);

      ShowWindow(_mwindow, SW_SHOWNORMAL);
      ShowWindow(_mwindow, SW_SHOWNORMAL);

      int chg_result = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);

      if (chg_result!=DISP_CHANGE_SUCCESSFUL) {
            wcrdisplay_cat.fatal() << "ChangeDisplaySettings failed (error code: " << chg_result <<") for specified res (" << dwWidth << " X " << dwHeight << " X " << dwFullScreenBitDepth <<"), " << dm.dmDisplayFrequency  << "Hz\n";
            exit(1);
      }

      _pCurrent_display_settings = new(DEVMODE);
      memcpy(_pCurrent_display_settings,&dm,sizeof(DEVMODE));

      _props._xorg = 0;
      _props._yorg = 0;
      _props._xsize = dwWidth;
      _props._ysize = dwHeight;

       if (wcrdisplay_cat.is_debug())
           wcrdisplay_cat.debug() << "set fullscreen mode at res (" << dwWidth << " X " << dwHeight << " X " << dwFullScreenBitDepth <<"), " << dm.dmDisplayFrequency  << "Hz\n";
  } else {

        RECT win_rect;
        SetRect(&win_rect, _props._xorg,  _props._yorg, _props._xorg + _props._xsize,
                _props._yorg + _props._ysize);

        if (_props._border) {
            window_style |= WS_OVERLAPPEDWINDOW;
        }

        BOOL bRes = AdjustWindowRect(&win_rect, window_style, FALSE);  //compute window size based on desired client area size

        if (!bRes) {
            wcrdisplay_cat.fatal() << "AdjustWindowRect failed!" << endl;
            exit(1);
        }

        // make sure origin is on screen, slide far bounds over if necessary
        if (win_rect.left < 0) {
            win_rect.right += abs(win_rect.left); win_rect.left = 0;
        }
        if (win_rect.top < 0) {
            win_rect.bottom += abs(win_rect.top); win_rect.top = 0;
        }

        _mwindow = CreateWindow(WCR_WINDOWCLASSNAME, _props._title.c_str(),
                                window_style, win_rect.left, win_rect.top, win_rect.right-win_rect.left,
                                win_rect.bottom-win_rect.top,
                                NULL, NULL, hinstance, 0);
  }

  if (!_mwindow) {
        wcrdisplay_cat.fatal() << "CreateWindow() failed!" << endl;
        PrintErrorMessage(LAST_ERROR);
        exit(1);
  }

  // Determine the initial open status of the IME.
  _ime_open = false;
  HIMC hIMC = ImmGetContext(_mwindow);
  if (hIMC != 0) {
    _ime_open = (ImmGetOpenStatus(hIMC) != 0);
    ImmReleaseContext(_mwindow, hIMC);
  }

  hwnd_pandawin_map[_mwindow] = this;
  global_wcrwinptr = NULL;  // get rid of any reference to this obj

  // move window to top of zorder
  SetWindowPos(_mwindow, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOOWNERZORDER);

  _hdc = GetDC(_mwindow);

  // Configure the framebuffer according to parameters specified in _props
  // Initializes _pixelformat
  int pfnum=choose_visual();

  if (gl_forced_pixfmt!=0) {
    if (wcrdisplay_cat.is_debug())
      wcrdisplay_cat.debug() << "overriding pixfmt choice algorithm (" << pfnum << ") with gl-force-pixfmt("<<gl_forced_pixfmt<< ")\n";
    pfnum=gl_forced_pixfmt;
  }

  //  int pfnum=ChoosePixelFormat(_hdc, _pixelformat);
  if (wcrdisplay_cat.is_debug())
     wcrdisplay_cat.debug() << "config() - picking pixfmt #"<< pfnum <<endl;

  if (!SetPixelFormat(_hdc, pfnum, &_pixelformat)) {
    wcrdisplay_cat.fatal()
      << "config() - SetPixelFormat("<< pfnum << ") failed after window create" << endl;
    exit(1);
  }

  // Initializes _colormap
  setup_colormap();

  _context = chromium.CreateContext(_hdc, CR_RGB_BIT | CR_DOUBLE_BIT);
  if (!_context) {
    wcrdisplay_cat.fatal()
      << "config() - failed to create Win32 rendering context" << endl;
    exit(1);
  }

  // need to do twice to override any minimized flags in StartProcessInfo
  ShowWindow(_mwindow, SW_SHOWNORMAL);
  ShowWindow(_mwindow, SW_SHOWNORMAL);

  // Enable detection of mouse input
  enable_mouse_input(true);
  enable_mouse_motion(true);
  enable_mouse_passive_motion(true);

  // Now indicate that we have our keyboard/mouse device ready.
  GraphicsWindowInputDevice device =
    GraphicsWindowInputDevice::pointer_and_keyboard("keyboard/mouse");
  _input_devices.push_back(device);

  // Create a GSG to manage the graphics
  // First make the new context and window the current one so GL knows how
  // to configure itself in the gsg
  make_current();
  make_gsg();

  check_for_color_cursor_support();

  //PT(crGraphicsStateGuardian) crgsg = DCAST(CRGraphicsStateGuardian, _gsg);

  string tmpstr((char*)chromium.GetString(GL_EXTENSIONS));

  _extensions_str = tmpstr;

  PFNWCRGETEXTENSIONSSTRINGEXTPROC wcrGetExtensionsStringEXT = NULL;
  PFNWCRGETEXTENSIONSSTRINGARBPROC wcrGetExtensionsStringARB = NULL;

  #if 0 //[
  if (!support_wiregl) {
    wcrGetExtensionsStringARB = (PFNWCRGETEXTENSIONSSTRINGARBPROC)chromium.GetProcAddress("wcrGetExtensionsStringARB");
    wcrGetExtensionsStringEXT = (PFNWCRGETEXTENSIONSSTRINGEXTPROC)chromium.GetProcAddress("wcrGetExtensionsStringEXT");
  }
  #endif //]

  if (wcrGetExtensionsStringARB!=NULL) {
       _extensions_str += " ";
       //TODO: const char *ARBextensions = wcrGetExtensionsStringARB(chromium.GetCurrentDC());
       const char *ARBextensions = wcrGetExtensionsStringARB(NULL);
       _extensions_str.append(ARBextensions);
  }

  if (wcrGetExtensionsStringEXT!=NULL) {
      // usually this will be the same as ARB extensions, but whatever
      _extensions_str += " ";
      const char *EXTextensions = wcrGetExtensionsStringEXT();
      _extensions_str.append(EXTextensions);
  }

  if (wcrdisplay_cat.is_spam())
     wcrdisplay_cat.spam() << "GL extensions: " << _extensions_str << endl;

  #if 0 //[
  if (gl_sync_video) {
      // set swapbuffers to swap no more than once per monitor refresh
      // note sometimes the ICD advertises this ext, but it still doesn't seem to work
      if (_extensions_str.find("WCR_EXT_swap_control")!=_extensions_str.npos) {
           PFNWCRSWAPINTERVALEXTPROC wcrSwapIntervalEXT;
           wcrSwapIntervalEXT = (PFNWCRSWAPINTERVALEXTPROC) chromium.GetProcAddress("wcrSwapIntervalEXT");
           if (wcrSwapIntervalEXT!=NULL)
               wcrSwapIntervalEXT(1);

           if (wcrdisplay_cat.is_spam())
               wcrdisplay_cat.spam() << "setting swapbuffer interval to 1/refresh\n";
      }
  }
  #endif //]

  if (gl_show_fps_meter) {

      _start_time = timeGetTime();
      _current_fps = 0.0;
      _start_frame_count = _cur_frame_count = 0;

     // 128 enough to handle all the ascii chars
     // this creates a display list for each char.  displist numbering starts
     // at FONT_BITMAP_OGLDISPLAYLISTNUM.  Might want to optimize just to save
     // mem by just allocing bitmaps for chars we need (0-9 fps,SPC)
     
     //TODO: chromium.UseFontBitmaps(_hdc, 0, 128, FONT_BITMAP_OGLDISPLAYLISTNUM);
  }

  if (wcrdisplay_cat.is_info()) {
     const char *vendStr=(const char *) chromium.GetString(GL_VENDOR);
     const char *rendStr=(const char *) chromium.GetString(GL_RENDERER);
     const char *versStr=(const char *) chromium.GetString(GL_VERSION);

     // Note:  glGetString will never return a valid value until you do chromium.MakeCurrent

     if (vendStr!=NULL) {
         wcrdisplay_cat.info()
              << "GL_VENDOR: " <<  vendStr
              << "  GL_RENDERER: " << ((rendStr==NULL) ? "" : rendStr)
              << "  GL_VERSION: " <<  ((versStr==NULL) ? "" : versStr) << endl;
      } else {
         wcrdisplay_cat.info() << "chromium.GetString(GL_VENDOR) returns NULL!\n";
      }
  }
}

void wcrGraphicsWindow::
check_for_color_cursor_support() {
    // card support for non-black/white GDI cursors varies greatly.  if the cursor is not supported,
    // it is rendered in software by GDI, which causes a flickering effect (because it's not synced
    // with flip?).  GDI transparently masks what's happening so there is no easy way for app to detect
    // if HW cursor support exists.  alternatives are to tie cursor motion to frame rate using DDraw blts
    // or overlays, or to have separate thread draw cursor (sync issues?).  instead we do mono cursor
    // unless card is known to support 256 color cursors

    string windows_color_cursor_filename = get_color_cursor_filename_2().to_os_specific();
    if (windows_color_cursor_filename.empty())
       return;

    bool bSupportsColorCursor=false;
    const GLubyte *vendorname=chromium.GetString(GL_VENDOR);
    if (vendorname==NULL) {
        return;
    }
    char vendorstr[500];
    strncpy(vendorstr,(const char *)vendorname,sizeof(vendorstr));
    _strlwr(vendorstr);
    if (strstr(vendorstr,"nvidia")!=NULL)
        bSupportsColorCursor=true;

    // for now, just assume only nvidia supports color. need to add more checks for other cards
    // like in DX l8r.

    if (bSupportsColorCursor) {
        // Note: LoadImage seems to cause win2k internal heap corruption (outputdbgstr warnings)
        // if icon is more than 8bpp (because it was 'mapping' 16bpp colors to the device?)

        DWORD load_flags = LR_LOADFROMFILE;

        if (_props._fullscreen) {
          // I think cursors should use LR_CREATEDIBSECTION since they should not be mapped to the device palette (in the case of 256-color cursors)
          // since they are not going to be used on the desktop
          load_flags |= LR_CREATEDIBSECTION;

          // note: this is still doing weird stuff when it loads 8bpp colors, even with LR_CREATEDIBSECTION
          // there is still a bug here  BUGBUG
        }

        // loads a .cur fmt file
        HCURSOR hNewMouseCursor = (HCURSOR) LoadImage(NULL, windows_color_cursor_filename.c_str(), IMAGE_CURSOR, 0, 0, load_flags);

        if (hNewMouseCursor==NULL) {
            wcrdisplay_cat.warning() << "windows color cursor filename '" << windows_color_cursor_filename << "' not found!!\n";
            return;
        }

        SetClassLongPtr(_mwindow, GCLP_HCURSOR, (LONG_PTR) hNewMouseCursor);
        SetCursor(hNewMouseCursor);

        if (_bLoadedCustomCursor)
           DestroyCursor(_hMouseCursor);
        _hMouseCursor = hNewMouseCursor;

        if (wcrdisplay_cat.is_spam())
            wcrdisplay_cat.spam() << "loaded color cursor\n";
    }
}

////////////////////////////////////////////////////////////////////
//     Function: Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
wcrGraphicsWindow::
wcrGraphicsWindow(GraphicsPipe* pipe) : GraphicsWindow(pipe) {
  config();
}

////////////////////////////////////////////////////////////////////
//     Function: Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
wcrGraphicsWindow::
wcrGraphicsWindow(GraphicsPipe* pipe, const
    GraphicsWindow::Properties& props) : GraphicsWindow(pipe, props) {
  config();
}

#ifdef _DEBUG
void PrintPFD(PIXELFORMATDESCRIPTOR *pfd,char *msg) {

  OGLDriverType drvtype;
  if ((pfd->dwFlags & PFD_GENERIC_ACCELERATED) && (pfd->dwFlags & PFD_GENERIC_FORMAT))
      drvtype=MCD;
   else if (!(pfd->dwFlags & PFD_GENERIC_ACCELERATED) && !(pfd->dwFlags & PFD_GENERIC_FORMAT))
      drvtype=ICD;
   else {
     drvtype=Software;
   }

#define PrintFlag(FLG) ((pfd->dwFlags &  PFD_##FLG) ? (" PFD_" #FLG "|") : "")
  wcrdisplay_cat.spam() << "================================\n";

  wcrdisplay_cat.spam() << msg << ", " << OGLDrvStrings[drvtype] << " driver\n"
                         << "PFD flags: 0x" << (void*)pfd->dwFlags << " (" <<
                        PrintFlag(GENERIC_ACCELERATED) <<
                        PrintFlag(GENERIC_FORMAT) <<
                        PrintFlag(DOUBLEBUFFER) <<
                        PrintFlag(SUPPORT_OPENGL) <<
                        PrintFlag(SUPPORT_GDI) <<
                        PrintFlag(STEREO) <<
                        PrintFlag(DRAW_TO_WINDOW) <<
                        PrintFlag(DRAW_TO_BITMAP) <<
                        PrintFlag(SWAP_EXCHANGE) <<
                        PrintFlag(SWAP_COPY) <<
                        PrintFlag(SWAP_LAYER_BUFFERS) <<
                        PrintFlag(NEED_PALETTE) <<
                        PrintFlag(NEED_SYSTEM_PALETTE) <<
                        PrintFlag(SUPPORT_DIRECTDRAW) << ")\n"
                         << "PFD iPixelType: " << ((pfd->iPixelType==PFD_TYPE_RGBA) ? "PFD_TYPE_RGBA":"PFD_TYPE_COLORINDEX") << endl
                         << "PFD cColorBits: " << (DWORD)pfd->cColorBits << "  R: " << (DWORD)pfd->cRedBits <<" G: " << (DWORD)pfd->cGreenBits <<" B: " << (DWORD)pfd->cBlueBits << endl
                         << "PFD cAlphaBits: " << (DWORD)pfd->cAlphaBits << "  DepthBits: " << (DWORD)pfd->cDepthBits <<" StencilBits: " << (DWORD)pfd->cStencilBits <<" AccumBits: " << (DWORD)pfd->cAccumBits << endl;
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: choose visual
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
int wcrGraphicsWindow::choose_visual() {

  int mask = _props._mask;
  int want_depth_bits = _props._want_depth_bits;
  int want_color_bits = _props._want_color_bits;
  OGLDriverType drvtype;

  if (mask & W_MULTISAMPLE) {
    wcrdisplay_cat.info()
      << "config() - multisample not supported"<< endl;
    mask &= ~W_MULTISAMPLE;
  }
    wcrdisplay_cat.info()
      << "mask =0x" << (void*) mask
    << endl;

  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory(&pfd,sizeof(PIXELFORMATDESCRIPTOR));
  pfd.nSize=sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion=1;

//  if (_props._fullscreen) {
//  do anything different for fullscrn?

  // just use the pixfmt of the current desktop

  int MaxPixFmtNum=DescribePixelFormat(_hdc, 1, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
  int cur_bpp=GetDeviceCaps(_hdc,BITSPIXEL);
  int pfnum;

#ifdef _DEBUG
  if (wcrdisplay_cat.is_debug()) {
    for(pfnum=1;pfnum<=MaxPixFmtNum;pfnum++) {
      DescribePixelFormat(_hdc, pfnum, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

      if ((pfd.dwFlags & PFD_GENERIC_ACCELERATED) && (pfd.dwFlags & PFD_GENERIC_FORMAT))
          drvtype=MCD;
       else if (!(pfd.dwFlags & PFD_GENERIC_ACCELERATED) && !(pfd.dwFlags & PFD_GENERIC_FORMAT))
          drvtype=ICD;
       else {
         drvtype=Software;
         continue;  // skipping all SW fmts
       }

       // use wcrinfo.exe instead
       char msg[200];
       sprintf(msg,"GL PixelFormat[%d]",pfnum);
       PrintPFD(&pfd,msg);
    }
  }
#endif

  for(pfnum=1;pfnum<=MaxPixFmtNum;pfnum++) {
      DescribePixelFormat(_hdc, pfnum, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    // official, nvidia sanctioned way.  should be equiv to my algorithm
    if ((pfd.dwFlags & PFD_GENERIC_FORMAT) != 0) {
        drvtype = Software;
        continue;
    }
    else if (pfd.dwFlags & PFD_GENERIC_ACCELERATED)
        drvtype = MCD;
    else
        drvtype = ICD;

#if MY_OLD_ALGORITHM
      if ((pfd.dwFlags & PFD_GENERIC_ACCELERATED) && (pfd.dwFlags & PFD_GENERIC_FORMAT))
          drvtype=MCD;
       else if (!(pfd.dwFlags & PFD_GENERIC_ACCELERATED) && !(pfd.dwFlags & PFD_GENERIC_FORMAT))
          drvtype=ICD;
       else {
         drvtype=Software;
         continue;  // skipping all SW fmts
       }
#endif

      if (wcrdisplay_cat.is_debug())
          wcrdisplay_cat->debug() << "----------------" << endl;

      if ((pfd.iPixelType == PFD_TYPE_COLORINDEX) && !(mask & W_INDEX))
          continue;

       DWORD dwReqFlags=(PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW);

       if (wcrdisplay_cat.is_debug()) {
         if (mask & W_ALPHA)
           wcrdisplay_cat->debug() << "want alpha, pfd says '"
                       << (int)(pfd.cAlphaBits) << "'" << endl;
         if (mask & W_DEPTH)
           wcrdisplay_cat->debug() << "want depth, pfd says '"
                       << (int)(pfd.cDepthBits) << "'" << endl;
         if (mask & W_STENCIL)
           wcrdisplay_cat->debug() << "want stencil, pfd says '"
                       << (int)(pfd.cStencilBits) << "'" << endl;
         wcrdisplay_cat->debug() << "final flag check "
                     << (int)(pfd.dwFlags & dwReqFlags) << " =? "
                     << (int)dwReqFlags << endl;
         wcrdisplay_cat->debug() << "pfd bits = " << (int)(pfd.cColorBits)
                     << endl;
         wcrdisplay_cat->debug() << "cur_bpp = " << cur_bpp << endl;
       }

       if (mask & W_DOUBLE)
           dwReqFlags|= PFD_DOUBLEBUFFER;
       if ((mask & W_ALPHA) && (pfd.cAlphaBits==0))
           continue;
       if ((mask & W_DEPTH) && (pfd.cDepthBits==0))
           continue;
       if ((mask & W_STENCIL) && (pfd.cStencilBits==0))
           continue;

       if ((pfd.dwFlags & dwReqFlags)!=dwReqFlags)
           continue;

       // now we ignore the specified want_color_bits for windowed mode
       // instead we use the current screen depth

       if ((pfd.cColorBits!=cur_bpp) && (!((cur_bpp==16) && (pfd.cColorBits==15)))
                                    && (!((cur_bpp==32) && (pfd.cColorBits==24))))
           continue;
       // we've passed all the tests, go ahead and pick this fmt
       // note: could go continue looping looking for more alpha bits or more depth bits
       // so this would pick 16bpp depth buffer, probably not 24bpp

       break;
  }

  if (pfnum>MaxPixFmtNum) {
      wcrdisplay_cat.error() << "ERROR: couldn't find HW-accelerated OpenGL pixfmt appropriate for this desktop!!\n";
      wcrdisplay_cat.error() << "make sure OpenGL driver is installed, and try reducing the screen size\n";
      if (cur_bpp>16)
        wcrdisplay_cat.error() << "or reducing the desktop screen pixeldepth\n";
      exit(1);
  }

  #ifdef _DEBUG
    char msg[200];
    sprintf(msg,"Selected GL PixelFormat is #%d",pfnum);
    PrintPFD(&pfd,msg);
  #endif

  memcpy(&_pixelformat,&pfd,sizeof(PIXELFORMATDESCRIPTOR));

  return pfnum;
}

////////////////////////////////////////////////////////////////////
//     Function: setup_colormap
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wcrGraphicsWindow::setup_colormap() {

  PIXELFORMATDESCRIPTOR pfd;
  LOGPALETTE *logical;
  int n;

  /* grab the pixel format */
  memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
  DescribePixelFormat(_hdc, GetPixelFormat(_hdc),
                      sizeof(PIXELFORMATDESCRIPTOR), &pfd);

  if (!(pfd.dwFlags & PFD_NEED_PALETTE ||
      pfd.iPixelType == PFD_TYPE_COLORINDEX))
    return;

  n = 1 << pfd.cColorBits;

  /* allocate a bunch of memory for the logical palette (assume 256
     colors in a Win32 palette */
  logical = (LOGPALETTE*)malloc(sizeof(LOGPALETTE) +
                                sizeof(PALETTEENTRY) * n);
  memset(logical, 0, sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * n);

  /* set the entries in the logical palette */
  logical->palVersion = 0x300;
  logical->palNumEntries = n;

  /* start with a copy of the current system palette */
  GetSystemPaletteEntries(_hdc, 0, 256, &logical->palPalEntry[0]);

  if (pfd.iPixelType == PFD_TYPE_RGBA) {
    int redMask = (1 << pfd.cRedBits) - 1;
    int greenMask = (1 << pfd.cGreenBits) - 1;
    int blueMask = (1 << pfd.cBlueBits) - 1;
    int i;

    /* fill in an RGBA color palette */
    for (i = 0; i < n; ++i) {
      logical->palPalEntry[i].peRed =
        (((i >> pfd.cRedShift)   & redMask)   * 255) / redMask;
      logical->palPalEntry[i].peGreen =
        (((i >> pfd.cGreenShift) & greenMask) * 255) / greenMask;
        logical->palPalEntry[i].peBlue =
        (((i >> pfd.cBlueShift)  & blueMask)  * 255) / blueMask;
      logical->palPalEntry[i].peFlags = 0;
    }
  }

  _colormap = CreatePalette(logical);
  free(logical);

  SelectPalette(_hdc, _colormap, FALSE);
  RealizePalette(_hdc);
}

////////////////////////////////////////////////////////////////////
//     Function: end_frame
//       Access:
//  Description: Swaps the front and back buffers.
////////////////////////////////////////////////////////////////////
void wcrGraphicsWindow::end_frame() {
  if (gl_show_fps_meter) {
    DO_PSTATS_STUFF(PStatTimer timer(_show_fps_pcollector);)
    DWORD now = timeGetTime();  // this is win32 fn

    float time_delta = (now - _start_time) * 0.001f;

    if (time_delta > gl_fps_meter_update_interval) {
      // didnt use global clock object, it wasnt working properly when
      // I tried, its probably slower due to cache faults, and I can
      // easily track all the info I need in dxgsg
      DWORD num_frames = _cur_frame_count - _start_frame_count;

      _current_fps = num_frames / time_delta;
      _start_time = now;
      _start_frame_count = _cur_frame_count;
    }

    char fps_msg[15];
    sprintf(fps_msg, "%.02f fps", _current_fps);

    // Note: we cant use simple GDI TextOut calls to draw FPS meter
    // chars (like DX fps meter) because WCR doesnt support GDI in
    // double-buffered mode.  Instead we have to use glBitMap display
    // lists created by chromium.UseFontBitmaps

    chromium.Color3f(0.0f,1.0f,1.0f);

    GLboolean tex_was_on = chromium.IsEnabled(GL_TEXTURE_2D);

    if (tex_was_on)
      chromium.Disable(GL_TEXTURE_2D);

    chromium.MatrixMode(GL_MODELVIEW);
    chromium.PushMatrix();
    chromium.LoadIdentity();
    chromium.MatrixMode(GL_PROJECTION);
    chromium.PushMatrix();
    chromium.LoadIdentity();

    chromium.Ortho(0.0f,_props._xsize,
            0.0f,_props._ysize,
            -1.0f,1.0f);

    chromium.RasterPos2f(_props._xsize-70,_props._ysize-20);  // these seem to be good for default font

    // set up for a string-drawing display list call
    chromium.ListBase(FONT_BITMAP_OGLDISPLAYLISTNUM);

    // draw a string using font display lists.  chars index their
    // corresponding displist name
    chromium.CallLists(strlen(fps_msg), GL_UNSIGNED_BYTE, fps_msg);

    chromium.PopMatrix();
    chromium.MatrixMode(GL_MODELVIEW);
    chromium.PopMatrix();

    if (tex_was_on)
      chromium.Enable(GL_TEXTURE_2D);

    _cur_frame_count++;  // only used by fps meter right now
  }

  {
    DO_PSTATS_STUFF(PStatTimer timer(_swap_pcollector);)
    if (_is_synced)
      chromium.Finish();
    else {
      chromium.SwapBuffers();
    }
  }
  GraphicsWindow::end_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: swap
//       Access:
//  Description: Swaps the front and back buffers explicitly.
////////////////////////////////////////////////////////////////////
void wcrGraphicsWindow::swap() {
  if (_is_synced)
    chromium.SwapBuffers();
}

bool wcrGraphicsWindow::resize(unsigned int xsize,unsigned int ysize) {
    if (!_props._fullscreen) {
        // resizing windowed mode is easy
        SetWindowPos(_mwindow, NULL, 0,0, xsize,ysize, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING);
    } else {
      DWORD dwWidth =  xsize;
      DWORD dwHeight = ysize;

      HWND hDesktopWindow = GetDesktopWindow();
      HDC scrnDC=GetDC(hDesktopWindow);
      DWORD dwFullScreenBitDepth=GetDeviceCaps(scrnDC,BITSPIXEL);
      ReleaseDC(hDesktopWindow,scrnDC);

      // resize will always leave screen bitdepth unchanged

      // allowing resizing of lowvidmem cards to > 640x480.  why?  I'll assume
      // check was already done by caller, so he knows what he wants

      DEVMODE dm;
      if (!find_acceptable_display_mode(dwWidth,dwHeight,dwFullScreenBitDepth,dm)) {
          wcrdisplay_cat.fatal() << "window resize(" << xsize << "," << ysize << ") failed, no compatible fullscreen display mode found!\n";
          return false;
      }

      // this causes WM_SIZE msg to be produced
      SetWindowPos(_mwindow, NULL, 0,0, xsize, ysize, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING);

      int chg_result = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);

      if (chg_result!=DISP_CHANGE_SUCCESSFUL) {
            wcrdisplay_cat.fatal() << "resize ChangeDisplaySettings failed (error code: " << chg_result <<") for specified res (" << dwWidth << " X " << dwHeight << " X " << dwFullScreenBitDepth <<"), " << dm.dmDisplayFrequency  << "Hz\n";
            exit(1);
      }

      // this assertion could be violated if we eventually allow dynamic fullscrn/windowed mode switching
      assert(_pCurrent_display_settings!=NULL);
      memcpy(_pCurrent_display_settings,&dm,sizeof(DEVMODE));
    }
    return true;
}

unsigned int wcrGraphicsWindow::
verify_window_sizes(unsigned int numsizes,unsigned int *dimen) {
  // see if window sizes are supported (i.e. in fullscrn mode)
  // dimen is an array containing contiguous x,y pairs specifying
  // possible display sizes, it is numsizes*2 long.  fn will zero
  // out any invalid x,y size pairs.  return value is number of valid
  // sizes that were found.
  //
  // note: it might be better to implement some sort of query
  //       interface that returns an array of supported sizes,
  //       but this way is somewhat simpler and will do the job
  //       on most cards, assuming they handle the std sizes the app
  //       knows about.

  if (!_props._fullscreen || (numsizes==0)) {
      return numsizes;
  }

  assert(dimen!=NULL);

  HWND hDesktopWindow = GetDesktopWindow();
  HDC scrnDC=GetDC(hDesktopWindow);
  DWORD dwFullScreenBitDepth=GetDeviceCaps(scrnDC,BITSPIXEL);
  ReleaseDC(hDesktopWindow,scrnDC);

  // gonna do an n^2 loop alg for simplicity.  if speed is necessary,
  // could do linear time with some kind of STL hash container I guess

  DEVMODE dm;
  uint modenum=0;
  uint goodmodes=0;
  unsigned int *cur_dim_pair=dimen;
  for(;modenum<numsizes;modenum++,cur_dim_pair+=2) {
      bool bIsGoodmode;
      DWORD dwWidth=cur_dim_pair[0];
      DWORD dwHeight=cur_dim_pair[1];

      if ((dwWidth==0)||(dwHeight==0)) {
          bIsGoodmode=false;
      } else {
          if (_bIsLowVidMemCard) {
              bIsGoodmode=((float)(dwWidth*(float)dwHeight)<=(float)(640*480));
          } else {
              bIsGoodmode = find_acceptable_display_mode(dwWidth,dwHeight,dwFullScreenBitDepth,dm);
          }
      }

      if (bIsGoodmode) {
         goodmodes++;
      } else {
         // zero out the bad mode
         cur_dim_pair[0]=0;
         cur_dim_pair[1]=0;
      }
  }

  return goodmodes;
}

////////////////////////////////////////////////////////////////////
//     Function: handle_reshape
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wcrGraphicsWindow::handle_reshape() {
      RECT view_rect;
      GetClientRect(_mwindow, &view_rect);
      ClientToScreen(_mwindow, (POINT*)&view_rect.left);   // translates top,left pnt
      ClientToScreen(_mwindow, (POINT*)&view_rect.right);  // translates right,bottom pnt

      // change _props xsize,ysize
      resized((view_rect.right - view_rect.left),(view_rect.bottom - view_rect.top));

      _props._xorg = view_rect.left;  // _props origin should reflect upper left of view rectangle
      _props._yorg = view_rect.top;

      if (wcrdisplay_cat.is_spam()) {
          wcrdisplay_cat.spam() << "reshape to origin: (" << _props._xorg << "," << _props._yorg << "), size: (" << _props._xsize << "," << _props._ysize << ")\n";
      }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_mouse_motion
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wcrGraphicsWindow::handle_mouse_motion(int x, int y) {
  _input_devices[0].set_pointer_in_window(x, y);
}

////////////////////////////////////////////////////////////////////
//     Function: handle_mouse_entry
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wcrGraphicsWindow::handle_mouse_entry(int state) {
  if (state == MOUSE_EXITED) {
    _input_devices[0].set_pointer_out_of_window();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_keypress
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wcrGraphicsWindow::
handle_keypress(ButtonHandle key, int x, int y) {
  _input_devices[0].set_pointer_in_window(x, y);
  if (key != ButtonHandle::none()) {
    _input_devices[0].button_down(key);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_keyrelease
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wcrGraphicsWindow::
handle_keyrelease(ButtonHandle key) {
  if (key != ButtonHandle::none()) {
    _input_devices[0].button_up(key);
  }
}

void INLINE process_1_event() {
  MSG msg;

  if (!GetMessage(&msg, NULL, 0, 0)) {
      // WM_QUIT received
      DestroyAllWindows(false);
      exit(msg.wParam);  // this will invoke AtExitFn
  }

  // Translate virtual key messages
  TranslateMessage(&msg);
  // Call window_proc
  DispatchMessage(&msg);
}

void INLINE wcrGraphicsWindow::process_events() {
  if (_window_inactive) {
      // Get 1 msg at a time until no more are left and we block and sleep,
      // or message changes _return_control_to_app or _window_inactive status

      while(_window_inactive && (!_return_control_to_app)) {
          process_1_event();
      }
      _return_control_to_app = false;

  } else {
      MSG msg;

      // handle all msgs on queue in a row
      while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
          process_1_event();
      }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wcrGraphicsWindow::supports_update
//       Access: Public, Virtual
//  Description: Returns true if this particular kind of
//               GraphicsWindow supports use of the update() function
//               to update the graphics one frame at a time, so that
//               the window does not need to be the program's main
//               loop.  Returns false if the only way to update the
//               window is to call main_loop().
////////////////////////////////////////////////////////////////////
bool wcrGraphicsWindow::
supports_update() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: update
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wcrGraphicsWindow::update() {
#ifdef DO_PSTATS
  _show_code_pcollector.stop();

  if (!_window_inactive) {
      PStatClient::main_tick();
  }
#endif

  process_events();

  if (_window_inactive) {
      // note _window_inactive must be checked after process_events is called, to avoid draw_callback being called
      if (_idle_callback)
          call_idle_callback();
      return;
  }

  call_draw_callback(true);

  if (_idle_callback)
    call_idle_callback();

#ifdef DO_PSTATS
  _show_code_pcollector.start();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: enable_mouse_input
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wcrGraphicsWindow::enable_mouse_input(bool val) {
  _mouse_input_enabled = val;
}

////////////////////////////////////////////////////////////////////
//     Function: enable_mouse_motion
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wcrGraphicsWindow::enable_mouse_motion(bool val) {
  _mouse_motion_enabled = val;
}

////////////////////////////////////////////////////////////////////
//     Function: enable_mouse_passive_motion
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wcrGraphicsWindow::enable_mouse_passive_motion(bool val) {
  _mouse_passive_motion_enabled = val;
}

////////////////////////////////////////////////////////////////////
//     Function: make_current
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void wcrGraphicsWindow::make_current() {
  //chromium.MakeCurrent(_hdc, 0, _context);
  chromium.MakeCurrent(0, 0, _context);
  #if 0 //[
  if ((_hdc==NULL)||(_context==NULL)||(_window_inactive)) {
      return;  // we're only allow unmake_current() to set this to NULL
  }

  DO_PSTATS_STUFF(PStatTimer timer(_make_current_pcollector);)
  //HGLRC current_context = GetCurrentContext();
  ////GLint current_context = (int)GetCurrentContext();
  GLint current_context = 0;
  //TODO: HDC current_dc = chromium.GetCurrentDC();
  HDC current_dc = NULL;

  if ((current_context != (int)_context) || (current_dc != _hdc)) {
    #if 1 //[
    chromium.MakeCurrent(_hdc, 0, _context);
    #else //][
    if (!chromium.MakeCurrent(_hdc, 0, _context)) {
        PrintErrorMessage(LAST_ERROR);
    }
    #endif //]
  }
  #endif //]

  report_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: unmake_current
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void wcrGraphicsWindow::unmake_current() {
  report_errors();

  #if 1 //[
  chromium.MakeCurrent(0, 0, 0);
  #else //][
  if (!chromium.MakeCurrent(NULL, NULL, 0)) {
      PrintErrorMessage(LAST_ERROR);
  }
  #endif //]
}

int wcrGraphicsWindow::get_depth_bitwidth() {
    return _pixelformat.cDepthBits;
}

////////////////////////////////////////////////////////////////////
//     Function: wcrGraphicsWindow::get_gsg_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle of the kind of GSG preferred
//               by this kind of window.
////////////////////////////////////////////////////////////////////
TypeHandle wcrGraphicsWindow::
get_gsg_type() const {
  return CRGraphicsStateGuardian::get_class_type();
}

GraphicsWindow *wcrGraphicsWindow::
make_wcrGraphicsWindow(const FactoryParams &params) {
  GraphicsWindow::WindowPipe *pipe_param;
  if (!get_param_into(pipe_param, params)) {
    wcrdisplay_cat.error()
      << "No pipe specified for window creation!" << endl;
    return NULL;
  }

  GraphicsPipe *pipe = pipe_param->get_pipe();

  GraphicsWindow::WindowProps *props_param;
  if (!get_param_into(props_param, params)) {
    return new wcrGraphicsWindow(pipe);
  } else {
    return new wcrGraphicsWindow(pipe, props_param->get_properties());
  }
}

TypeHandle wcrGraphicsWindow::get_class_type() {
  return _type_handle;
}

void wcrGraphicsWindow::init_type() {
  GraphicsWindow::init_type();
  register_type(_type_handle, "wcrGraphicsWindow",
        GraphicsWindow::get_class_type());
}

TypeHandle wcrGraphicsWindow::get_type() const {
  return get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: static_window_proc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
   HWND_PANDAWIN_MAP::iterator pwin;
   pwin=hwnd_pandawin_map.find(hwnd);

   if (pwin!=hwnd_pandawin_map.end()) {
      wcrGraphicsWindow *wcrwinptr=(*pwin).second;
      return wcrwinptr->window_proc(hwnd, msg, wparam, lparam);
   } else if (global_wcrwinptr!=NULL){
       // this stuff should only be used during CreateWindow()
       return global_wcrwinptr->window_proc(hwnd, msg, wparam, lparam);
   } else {
       // should never need this??  (maybe at shutdwn?)
       return DefWindowProc(hwnd, msg, wparam, lparam);
   }
}

void wcrGraphicsWindow::deactivate_window() {
    // current policy is to suspend minimized or deactivated fullscreen windows, but leave
    // regular windows running normally

   if ((!_props._fullscreen) || _exiting_window || _window_inactive || _active_minimized_fullscreen) {
       #ifdef _DEBUG
          if (wcrdisplay_cat.is_spam())
            wcrdisplay_cat.spam()  << "deactivate_window called, but ignored in current mode"  << endl;
       #endif
     return;
   }

   throw_event("PandaPaused"); // right now this is used to signal python event handler to disable audio

   if (!bResponsive_minimized_fullscreen_window) {
       if (wcrdisplay_cat.is_spam())
           wcrdisplay_cat.spam() << "WCR window deactivated, releasing gl context and waiting...\n";

      _window_inactive = true;
      unmake_current();
   } else {
       _active_minimized_fullscreen = true;
       assert(_props._fullscreen);

       if (wcrdisplay_cat.is_spam())
           wcrdisplay_cat.spam() << "WCR window minimized from fullscreen mode, remaining active...\n";
   }

   // make sure window is minimized

   WINDOWPLACEMENT wndpl;
   wndpl.length=sizeof(WINDOWPLACEMENT);

   if (!GetWindowPlacement(_mwindow,&wndpl)) {
       wcrdisplay_cat.error() << "GetWindowPlacement failed!\n";
       return;
   }

   if ((wndpl.showCmd!=SW_MINIMIZE)&&(wndpl.showCmd!=SW_SHOWMINIMIZED)) {
       ShowWindow(_mwindow, SW_MINIMIZE);
   }

   // revert to default display mode
   ChangeDisplaySettings(NULL,0x0);

   if (!bResponsive_minimized_fullscreen_window) {
       _PandaPausedTimer = SetTimer(_mwindow,PAUSED_TIMER_ID,1500,NULL);
       if (_PandaPausedTimer!=PAUSED_TIMER_ID) {
           wcrdisplay_cat.error() << "Error in SetTimer!\n";
       }
   }

   if (_props._fullscreen) {
      throw_event("PandaRestarted");  // right now this is used to signal python event handler to re-enable audio
   }
}

void wcrGraphicsWindow::reactivate_window() {
    if (_window_inactive) {
        if (wcrdisplay_cat.is_spam())
            wcrdisplay_cat.spam() << "WCR window re-activated...\n";

        _window_inactive = false;

        if (_PandaPausedTimer!=NULL) {
            KillTimer(_mwindow,_PandaPausedTimer);
            _PandaPausedTimer = NULL;
        }

        // move window to top of zorder,
        SetWindowPos(_mwindow, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOOWNERZORDER);

        ChangeDisplaySettings(_pCurrent_display_settings,CDS_FULLSCREEN);

        GdiFlush();
        make_current();
    } else if (_active_minimized_fullscreen) {
        if (wcrdisplay_cat.is_spam())
            wcrdisplay_cat.spam() << "redisplaying minimized fullscrn active WCR window...\n";

        // move window to top of zorder,
        SetWindowPos(_mwindow, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOOWNERZORDER);

        ChangeDisplaySettings(_pCurrent_display_settings,CDS_FULLSCREEN);

        GdiFlush();
        make_current();
        _active_minimized_fullscreen = false;
    }
}

////////////////////////////////////////////////////////////////////
//     Function: window_proc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
LONG WINAPI wcrGraphicsWindow::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  int button = -1;
  int x, y;

  switch (msg) {
    case WM_CREATE:
      break;

    case WM_CLOSE:
          close_window();

          // BUGBUG:  right now there is no way to tell the panda app the graphics window is invalid or
          //          has been closed by the user, to prevent further methods from being called on the window.
          //          this needs to be added to panda for multiple windows to work.  in the meantime, just
          //          trigger an exit here if numwindows==0, since that is the expected behavior when all
          //          windows are closed (should be done by the app though, and it assumes you only make this
          //          type of panda gfx window)

          if (hwnd_pandawin_map.size()==0) {
              exit(0);
          }
          break;

    case WM_MOVE:
          // handle all this stuff in EXITSIZEMOVE.  will rendering work during moving?  do we care?
          //_props._xorg = LOWORD(lparam);
          //_props._yorg = HIWORD(lparam);
          break;


    case WM_ACTIVATEAPP: {
            #ifdef _DEBUG
              wcrdisplay_cat.spam()  << "WM_ACTIVATEAPP(" << (bool)(wparam!=0) <<") received\n";
            #endif

           if (!wparam) {
               deactivate_window();
               return 0;
           }         // dont want to reactivate until window is actually un-minimized (see WM_SIZE)
           break;
        }

    case WM_EXITSIZEMOVE:
            #ifdef _DEBUG
              wcrdisplay_cat.spam()  << "WM_EXITSIZEMOVE received"  << endl;
            #endif

            reactivate_window();
            handle_reshape();
            break;

    case WM_ENTERSIZEMOVE:
            break;

    case WM_SIZE: {
                DWORD width,height;

                width = LOWORD(lparam);  height = HIWORD(lparam);
            #ifdef _DEBUG
                {
                    wcrdisplay_cat.spam() << "WM_SIZE received with width:" << width << "  height: " << height << " flags: " <<
                    ((wparam == SIZE_MAXHIDE)? "SIZE_MAXHIDE " : "") << ((wparam == SIZE_MAXSHOW)? "SIZE_MAXSHOW " : "") <<
                    ((wparam == SIZE_MINIMIZED)? "SIZE_MINIMIZED " : "") << ((wparam == SIZE_RESTORED)? "SIZE_RESTORED " : "") <<
                    ((wparam == SIZE_MAXIMIZED)? "SIZE_MAXIMIZED " : "") << endl;
                }
            #endif
                if (_mwindow==NULL)
                    break;

                if ((wparam==SIZE_MAXIMIZED) || (wparam==SIZE_RESTORED)) { // old comment -- added SIZE_RESTORED to handle 3dfx case  (what does this mean?)
                     reactivate_window();

//                  if ((_props._xsize != width) || (_props._ysize != height))
                     handle_reshape();
                }
                break;
    }

    case WM_PAINT: {
          PAINTSTRUCT ps;
          BeginPaint(hwnd, &ps);
            // glReadBuffer(GL_BACK);  need to copy current rendering to front buffer, a la wdxdisplay?
          EndPaint(hwnd, &ps);
          return 0;
    }

    case WM_IME_NOTIFY:
      if (wparam == IMN_SETOPENSTATUS) {
        HIMC hIMC = ImmGetContext(hwnd);
        nassertr(hIMC != 0, 0);
        _ime_open = (ImmGetOpenStatus(hIMC) != 0);
        ImmReleaseContext(hwnd, hIMC);
      }
      break;

    case WM_IME_COMPOSITION:
      if (lparam & GCS_RESULTSTR) {
        HIMC hIMC = ImmGetContext(hwnd);
        nassertr(hIMC != 0, 0);

        static const int max_ime_result = 128;
        static char ime_result[max_ime_result];

        DWORD result_size =
          ImmGetCompositionStringW(hIMC, GCS_RESULTSTR,
                                   ime_result, max_ime_result);
        ImmReleaseContext(hwnd, hIMC);

        // Add this string into the text buffer of the application.
        wchar_t *ime_wchar_result = (wchar_t *)ime_result;
        for (DWORD i = 0; i < result_size / 2; i++) {
          _input_devices[0].keystroke(ime_wchar_result[i]);
        }
      }
      break;

    case WM_CHAR:
      // Ignore WM_CHAR messages if we have the IME open, since
      // everything will come in through WM_IME_COMPOSITION.  (It's
      // supposed to come in through WM_CHAR, too, but there seems to
      // be a bug in Win2000 in that it only sends question mark
      // characters through here.)
      if (!_ime_open && !_input_devices.empty()) {
        _input_devices[0].keystroke(wparam);
      }
      break;

    case WM_SYSKEYDOWN:
      // want to use defwindproc on Alt syskey so Alt-F4 works, etc
      // but do want to bypass defwindproc F10 behavior (it activates
      // the main menu, but we have none)
      if (wparam == VK_F10) {
        return 0;
      }
      break;

    case WM_KEYDOWN: {
            POINT point;

            GetCursorPos(&point);
            ScreenToClient(hwnd, &point);

          #ifdef NDEBUG
               handle_keypress(lookup_key(wparam), point.x, point.y);
          #else
            // handle Cntrl-V paste from clipboard
            if (!((wparam=='V') && (GetKeyState(VK_CONTROL) < 0))) {
               handle_keypress(lookup_key(wparam), point.x, point.y);
            } else {
                HGLOBAL   hglb;
                char    *lptstr;

                if (!IsClipboardFormatAvailable(CF_TEXT))
                   return 0;

                if (!OpenClipboard(NULL))
                   return 0;

                hglb = GetClipboardData(CF_TEXT);
                if (hglb!=NULL) {
                    lptstr = (char *) GlobalLock(hglb);
                    if (lptstr != NULL)  {
                        char *pChar;
                        for(pChar=lptstr;*pChar!=NULL;pChar++) {
                           handle_keypress(KeyboardButton::ascii_key((uchar)*pChar), point.x, point.y);
                        }
                        GlobalUnlock(hglb);
                    }
                }
                CloseClipboard();
            }
          #endif
              break;
    }

    case WM_SYSKEYUP:
    case WM_KEYUP:
      handle_keyrelease(lookup_key(wparam));
      break;

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
      #define SET_MOUSE_COORD(iVal,VAL) { \
            iVal = VAL;                   \
            if (iVal & 0x8000)             \
              iVal -= 0x10000;            \
      }

      SET_MOUSE_COORD(x,LOWORD(lparam));
      SET_MOUSE_COORD(y,HIWORD(lparam));

      // make_current();  what does OGL have to do with mouse input??
      handle_keypress(MouseButton::button(button), x, y);
      break;
    case WM_LBUTTONUP:
      button = 0;
    case WM_MBUTTONUP:
      if (button < 0)
          button = 1;
    case WM_RBUTTONUP:
      if (button < 0)
          button = 2;
      ReleaseCapture();
      #if 0
          SET_MOUSE_COORD(x,LOWORD(lparam));
          SET_MOUSE_COORD(y,HIWORD(lparam));
          // make_current();  what does OGL have to do with mouse input??
      #endif
      handle_keyrelease(MouseButton::button(button));
      break;

    case WM_MOUSEMOVE:
        SET_MOUSE_COORD(x,LOWORD(lparam));
        SET_MOUSE_COORD(y,HIWORD(lparam));

        if (mouse_motion_enabled() &&
            (wparam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON))) {
            // make_current();  what does OGL have to do with mouse input??
            handle_mouse_motion(x, y);
        } else if (mouse_passive_motion_enabled() &&
                   ((wparam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)) == 0)) {
                    // make_current();  what does OGL have to do with mouse input??
                    handle_mouse_motion(x, y);
        }
        break;

    case WM_SETFOCUS: {
            // wcrdisplay_cat.info() << "got WM_SETFOCUS\n";

            if (_mouse_entry_enabled) {
                make_current();
                handle_mouse_entry(MOUSE_ENTERED);
            }

            POINT point;
            GetCursorPos(&point);
            ScreenToClient(hwnd, &point);

            // this is a hack to make sure common modifier keys have proper state
            // since at focus loss, app may never receive key-up event corresponding to
            // a key-down. it would be better to know the exact set of ModifierButtons the
            // user is using, since we may miss some here

            int i;
            for(i=0;i<NUM_MODIFIER_KEYS;i++) {
              if (GetKeyState(hardcoded_modifier_buttons[i]) < 0)
                handle_keypress(lookup_key(hardcoded_modifier_buttons[i]),point.x,point.y);
            }
            return 0;
        }

    case WM_KILLFOCUS: {
            if (_mouse_entry_enabled)
              handle_mouse_entry(MOUSE_EXITED);

            int i;
            for(i=0;i<NUM_MODIFIER_KEYS;i++) {
              if (GetKeyState(hardcoded_modifier_buttons[i]) < 0)
                handle_keyrelease(lookup_key(hardcoded_modifier_buttons[i]));
            }

            return 0;
    }
    break;

    case WM_TIMER:
      if ((wparam==_PandaPausedTimer) && _window_inactive) {
         //wcrdisplay_cat.spam() << "returning control to app\n";
          _return_control_to_app = true;
         // throw_event("PandaPaused");
         // do we still need to do this since I return control to app periodically using timer msgs?
         // does app need to know to avoid major computation?
      }

      break;
  }

  return DefWindowProc(hwnd, msg, wparam, lparam);
}

////////////////////////////////////////////////////////////////////
//     Function: lookup_key
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
ButtonHandle wcrGraphicsWindow::
lookup_key(WPARAM wparam) const {
    switch(wparam) {
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
                bool bCapsLockDown=((GetKeyState(VK_CAPITAL) & 0x1)!=0);
                bool bShiftUp = (GetKeyState(VK_SHIFT) >= 0);
                if (bShiftUp) {
                    if (bCapsLockDown)
                        key = toupper(key);
                    else key = tolower(key);
                } else {
                    switch(key) {
                        // these keys are unaffected by capslock
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
                        default:
                            if (bCapsLockDown)
                                key = tolower(key);
                            else key = toupper(key);
                    }
                }
                return KeyboardButton::ascii_key((uchar)key);
            }
            break;
    }
    return ButtonHandle::none();
}


