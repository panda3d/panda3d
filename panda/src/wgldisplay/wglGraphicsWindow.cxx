// Filename: wcrGraphicsWindow.cxx
// Created by:
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

#include "wglGraphicsWindow.h"
#include "wglGraphicsPipe.h"
#include "config_wgldisplay.h"
#include <keyboardButton.h>
#include <mouseButton.h>
#include <glGraphicsStateGuardian.h>
#include <errno.h>
#include <time.h>
#include <mmsystem.h>
#include <ddraw.h>
#include <tchar.h>
#include <map>
#include <throw_event.h>
//#include <eventQueue.h>
#include <string.h>
#include "Win32Defs.h"

#ifdef DO_PSTATS
#include <pStatTimer.h>
#endif

#define WGL_WGLEXT_PROTOTYPES
#include "wglext.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle wglGraphicsWindow::_type_handle;

static bool wc_registered = false;

#define MOUSE_ENTERED 0
#define MOUSE_EXITED 1

#define FONT_BITMAP_OGLDISPLAYLISTNUM 1000    // an arbitrary ID #

#define LAST_ERROR 0
#define ERRORBOX_TITLE "Panda3D Error"
#define WGL_WINDOWCLASSNAME "wglDisplay"

#define PAUSED_TIMER_ID  7   // completely arbitrary choice

typedef map<HWND,wglGraphicsWindow *> HWND_PANDAWIN_MAP;

HWND_PANDAWIN_MAP hwnd_pandawin_map;
wglGraphicsWindow *global_wglwinptr=NULL;  // need this for temporary windproc

typedef enum {Software, MCD, ICD} OGLDriverType;
static char *OGLDrvStrings[3] = {"Software","MCD","ICD"};

LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam,LPARAM lparam);
extern char *ConvDDErrorToString(const HRESULT &error);

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
   wgldisplay_cat.fatal() << "System error msg: " << pMessageBuffer << endl;
   LocalFree(pMessageBuffer);
}

// fn exists so AtExitFn can call it without refcntr blowing up since its !=0
void wglGraphicsWindow::DestroyMe(bool bAtExitFnCalled) {

  _exiting_window = true;  // needed before DestroyWindow call

  // several GL drivers (voodoo,ATI, not nvidia) crash if we call these wgl deletion routines from
  // an atexit() fn.  Possible that GL has already unloaded itself.  So we just wont call them for now
  // for that case, we're exiting the app anyway.
  if (!bAtExitFnCalled) {
      // to do gl releases, we need to have the context be current
      if ((_hdc!=NULL)&&(_context!=NULL)) {
          // need to bypass make_current() since it checks _window_inactive which we need to ignore
          HGLRC current_context = wglGetCurrentContext();
          HDC current_dc = wglGetCurrentDC();

          if ((current_context != _context) || (current_dc != _hdc)) {
              if (!wglMakeCurrent(_hdc, _context)) {
                  PrintErrorMessage(LAST_ERROR);
              }
          }
          report_errors();
      }

      if (gl_show_fps_meter)
        glDeleteLists(FONT_BITMAP_OGLDISPLAYLISTNUM, 128);

      report_errors();

      // implicitly calls gsg destructors which release GL objects (textures, display lists, etc)
      release_gsg();

      report_errors();
      // cant report errors after we set cur context to NULL

      HGLRC curcxt=wglGetCurrentContext();
      if (curcxt!=NULL)
        unmake_current();

      if (_context!=NULL) {
          wglDeleteContext(_context);
          _context = NULL;
      }
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

void wglGraphicsWindow::do_close_window() {
  GraphicsWindow::do_close_window();
   DestroyMe(false);
}

////////////////////////////////////////////////////////////////////
//     Function: Destructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsWindow::~wglGraphicsWindow() {
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
    wgldisplay_cat.spam() << "AtExitFn called\n";
#endif

     restore_global_parameters();

     DestroyAllWindows(true);
}

// spare me the trouble of linking with dxguid.lib or defining ALL the dx guids in this .obj by #defining INITGUID
#define MY_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }

MY_DEFINE_GUID(IID_IDirectDraw2, 0xB3A6F3E0,0x2B43,0x11CF,0xA2,0xDE,0x00,0xAA,0x00,0xB9,0x33,0x56);

////////////////////////////////////////////////////////////////////
//  Function: GetAvailVidMem
//  Description: Uses DDraw to get available video memory
////////////////////////////////////////////////////////////////////
static DWORD GetAvailVidMem() {

    LPDIRECTDRAW2 pDD2;
    LPDIRECTDRAW pDD;
    HRESULT hr;

    typedef HRESULT (WINAPI *DIRECTDRAWCREATEPROC)(GUID FAR *lpGUID,LPDIRECTDRAW FAR *lplpDD,IUnknown FAR *pUnkOuter);
    DIRECTDRAWCREATEPROC pfnDDCreate=NULL;

    HINSTANCE DDHinst = LoadLibrary("ddraw.dll");
    if (DDHinst == 0) {
        wgldisplay_cat.fatal() << "LoadLibrary() can't find DDRAW.DLL!" << endl;
        exit(1);
    }

    pfnDDCreate = (DIRECTDRAWCREATEPROC) GetProcAddress(DDHinst, "DirectDrawCreate");

    // just use DX5 DD interface, since that's the minimum ver we need
    if (NULL == pfnDDCreate) {
        wgldisplay_cat.fatal() << "GetProcAddress failed on DirectDrawCreate\n";
        exit(1);
    }

    // Create the Direct Draw Object
    hr = (*pfnDDCreate)((GUID *)DDCREATE_HARDWAREONLY, &pDD, NULL);
    if (hr != DD_OK) {
        wgldisplay_cat.fatal()
        << "DirectDrawCreate failed : result = " << ConvDDErrorToString(hr) << endl;
        exit(1);
    }

    FreeLibrary(DDHinst);    //undo LoadLib above, decrement ddrawl.dll refcnt (after DDrawCreate, since dont want to unload/reload)

    // need DDraw2 interface for GetAvailVidMem
    hr = pDD->QueryInterface(IID_IDirectDraw2, (LPVOID *)&pDD2);
    if (hr != DD_OK) {
        wgldisplay_cat.fatal() << "DDraw QueryInterface failed : result = " << ConvDDErrorToString(hr) << endl;
        exit(1);
     }

    pDD->Release();

    // Now we try to figure out if we can use requested screen resolution and best
    // rendertarget bpp and still have at least 2 meg of texture vidmem

    // Get Current VidMem avail.  Note this is only an estimate, when we switch to fullscreen
    // mode from desktop, more vidmem will be available (typically 1.2 meg).  I dont want
    // to switch to fullscreen more than once due to the annoying monitor flicker, so try
    // to figure out optimal mode using this estimate
    DDSCAPS ddsCaps;
    DWORD dwTotal,dwFree;
    ZeroMemory(&ddsCaps,sizeof(DDSCAPS));
    ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY; //set internally by DX anyway, dont think this any different than 0x0

    if (FAILED(hr = pDD2->GetAvailableVidMem(&ddsCaps,&dwTotal,&dwFree))) {
        if (hr==DDERR_NODIRECTDRAWHW) {
           if (wgldisplay_cat.is_debug())
               wgldisplay_cat.debug() << "GetAvailableVidMem returns no-DDraw HW, assuming we have plenty of vidmem\n";
           dwTotal=dwFree=0x7FFFFFFF;
        } else {
            wgldisplay_cat.fatal() << "GetAvailableVidMem failed : result = " << ConvDDErrorToString(hr) << endl;
            exit(1);
        }
    } else {
       if (wgldisplay_cat.is_debug())
           wgldisplay_cat.debug() << "before FullScreen switch: GetAvailableVidMem returns Total: " << dwTotal/1000000.0 << "  Free: " << dwFree/1000000.0 << endl;
       if (dwTotal==0) {
           if (wgldisplay_cat.is_debug())
               wgldisplay_cat.debug() << "GetAvailVidMem returns bogus total of 0, assuming we have plenty of vidmem\n";
           dwTotal=dwFree=0x7FFFFFFF;
       }
    }

    pDD2->Release();  // bye-bye ddraw

    return dwFree;
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
void wglGraphicsWindow::config() {

    GraphicsWindow::config();

    HINSTANCE hinstance = GetModuleHandle(NULL);
    HWND hDesktopWindow = GetDesktopWindow();

    global_wglwinptr = this;  // need this until we get an HWND from CreateWindow

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
            wgldisplay_cat.warning() << "windows icon filename '" << windows_icon_filename << "' not found!!\n";
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
            wgldisplay_cat.warning() << "windows cursor filename '" << windows_mono_cursor_filename << "' not found!!\n";
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
      wc.lpszClassName  = WGL_WINDOWCLASSNAME;

      if (!RegisterClass(&wc)) {
        wgldisplay_cat.error() << "could not register window class!" << endl;
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

      #define LOWVIDMEMTHRESHOLD 3500000
      if (GetAvailVidMem() < LOWVIDMEMTHRESHOLD) {
          _bIsLowVidMemCard = true;
          wgldisplay_cat.debug() << "small video memory card detect, switching fullscreen to minimum 640x480x16 config\n";
          // we're going to need  640x480 at 16 bit to save enough tex vidmem
          dwFullScreenBitDepth=16;
          dwWidth=640;
          dwHeight=480;
      }

      DEVMODE dm;
      if (!find_acceptable_display_mode(dwWidth,dwHeight,dwFullScreenBitDepth,dm)) {
          wgldisplay_cat.fatal() << "Videocard has no supported display resolutions at specified res (" << dwWidth << " X " << dwHeight << " X " << dwFullScreenBitDepth <<")\n";
          exit(1);
      }

      // I'd prefer to CreateWindow after DisplayChange in case it messes up GL somehow,
      // but I need the window's black background to cover up the desktop during the mode change
      _mwindow = CreateWindow(WGL_WINDOWCLASSNAME, _props._title.c_str(),
                window_style,0,0,dwWidth,dwHeight,hDesktopWindow, NULL, hinstance, 0);

      // move window to top of zorder,
      SetWindowPos(_mwindow, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOSIZE);

      ShowWindow(_mwindow, SW_SHOWNORMAL);
      ShowWindow(_mwindow, SW_SHOWNORMAL);

      int chg_result = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);

      if (chg_result!=DISP_CHANGE_SUCCESSFUL) {
            wgldisplay_cat.fatal() << "ChangeDisplaySettings failed (error code: " << chg_result <<") for specified res (" << dwWidth << " X " << dwHeight << " X " << dwFullScreenBitDepth <<"), " << dm.dmDisplayFrequency  << "Hz\n";
            exit(1);
      }

      _pCurrent_display_settings = new(DEVMODE);
      memcpy(_pCurrent_display_settings,&dm,sizeof(DEVMODE));

      _props._xorg = 0;
      _props._yorg = 0;
      _props._xsize = dwWidth;
      _props._ysize = dwHeight;

       if (wgldisplay_cat.is_debug())
           wgldisplay_cat.debug() << "set fullscreen mode at res (" << dwWidth << " X " << dwHeight << " X " << dwFullScreenBitDepth <<"), " << dm.dmDisplayFrequency  << "Hz\n";
  } else {

        RECT win_rect;
        SetRect(&win_rect, _props._xorg,  _props._yorg, _props._xorg + _props._xsize,
                _props._yorg + _props._ysize);

        if (_props._border) {
            window_style |= WS_OVERLAPPEDWINDOW;
        }

        BOOL bRes = AdjustWindowRect(&win_rect, window_style, FALSE);  //compute window size based on desired client area size

        if (!bRes) {
            wgldisplay_cat.fatal() << "AdjustWindowRect failed!" << endl;
            exit(1);
        }

        // make sure origin is on screen, slide far bounds over if necessary
        if (win_rect.left < 0) {
            win_rect.right += abs(win_rect.left); win_rect.left = 0;
        }
        if (win_rect.top < 0) {
            win_rect.bottom += abs(win_rect.top); win_rect.top = 0;
        }

        _mwindow = CreateWindow(WGL_WINDOWCLASSNAME, _props._title.c_str(),
                                window_style, win_rect.left, win_rect.top, win_rect.right-win_rect.left,
                                win_rect.bottom-win_rect.top,
                                NULL, NULL, hinstance, 0);
  }

  if (!_mwindow) {
        wgldisplay_cat.fatal() << "CreateWindow() failed!" << endl;
        PrintErrorMessage(LAST_ERROR);
        exit(1);
  }

  // Determine the initial open status of the IME.
  _ime_open = false;
  _ime_active = false;
  HIMC hIMC = ImmGetContext(_mwindow);
  if (hIMC != 0) {
    _ime_open = (ImmGetOpenStatus(hIMC) != 0);
    ImmReleaseContext(_mwindow, hIMC);
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

  hwnd_pandawin_map[_mwindow] = this;
  global_wglwinptr = NULL;  // get rid of any reference to this obj

  // move window to top of zorder
  SetWindowPos(_mwindow, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOOWNERZORDER);

  _hdc = GetDC(_mwindow);

  // Configure the framebuffer according to parameters specified in _props
  // Initializes _pixelformat
  int pfnum=choose_visual();

  if (gl_forced_pixfmt!=0) {
    if (wgldisplay_cat.is_debug())
      wgldisplay_cat.debug() << "overriding pixfmt choice algorithm (" << pfnum << ") with gl-force-pixfmt("<<gl_forced_pixfmt<< ")\n";
    pfnum=gl_forced_pixfmt;
  }

  //  int pfnum=ChoosePixelFormat(_hdc, _pixelformat);
  if (wgldisplay_cat.is_debug())
     wgldisplay_cat.debug() << "config() - picking pixfmt #"<< pfnum <<endl;

  if (!SetPixelFormat(_hdc, pfnum, &_pixelformat)) {
    wgldisplay_cat.fatal()
      << "config() - SetPixelFormat("<< pfnum << ") failed after window create" << endl;
    exit(1);
  }

  // Initializes _colormap
  setup_colormap();

  _context = wglCreateContext(_hdc);
  if (!_context) {
    wgldisplay_cat.fatal()
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

//  _glgsg = DCAST(GLGraphicsStateGuardian, _gsg);   dont need this now

  string tmpstr((char*)glGetString(GL_EXTENSIONS));

  _extensions_str = tmpstr;

  PFNWGLGETEXTENSIONSSTRINGEXTPROC wglGetExtensionsStringEXT = NULL;
  PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB = NULL;

  if (!support_wiregl) {
    wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
    wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
  }

  if (wglGetExtensionsStringARB!=NULL) {
       _extensions_str += " ";
       const char *ARBextensions = wglGetExtensionsStringARB(wglGetCurrentDC());
       _extensions_str.append(ARBextensions);
  }

  if (wglGetExtensionsStringEXT!=NULL) {
      // usually this will be the same as ARB extensions, but whatever
      _extensions_str += " ";
      const char *EXTextensions = wglGetExtensionsStringEXT();
      _extensions_str.append(EXTextensions);
  }

  if (wgldisplay_cat.is_spam())
     wgldisplay_cat.spam() << "GL extensions: " << _extensions_str << endl;

  if (gl_sync_video) {
      // set swapbuffers to swap no more than once per monitor refresh
      // note sometimes the ICD advertises this ext, but it still doesn't seem to work
      if (_extensions_str.find("WGL_EXT_swap_control")!=_extensions_str.npos) {
           PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
           wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");
           if (wglSwapIntervalEXT!=NULL)
               wglSwapIntervalEXT(1);

           if (wgldisplay_cat.is_spam())
               wgldisplay_cat.spam() << "setting swapbuffer interval to 1/refresh\n";
      }
  }

  if (gl_show_fps_meter) {

      _start_time = timeGetTime();
      _current_fps = 0.0;
      _start_frame_count = _cur_frame_count = 0;

     // 128 enough to handle all the ascii chars
     // this creates a display list for each char.  displist numbering starts
     // at FONT_BITMAP_OGLDISPLAYLISTNUM.  Might want to optimize just to save
     // mem by just allocing bitmaps for chars we need (0-9 fps,SPC)
     wglUseFontBitmaps(_hdc, 0, 128, FONT_BITMAP_OGLDISPLAYLISTNUM);
  }

  if (wgldisplay_cat.is_info()) {
     const char *vendStr=(const char *) glGetString(GL_VENDOR);
     const char *rendStr=(const char *) glGetString(GL_RENDERER);
     const char *versStr=(const char *) glGetString(GL_VERSION);

     // Note:  glGetString will never return a valid value until you do wglMakeCurrent

     if (vendStr!=NULL) {
         wgldisplay_cat.info()
              << "GL_VENDOR: " <<  vendStr
              << "  GL_RENDERER: " << ((rendStr==NULL) ? "" : rendStr)
              << "  GL_VERSION: " <<  ((versStr==NULL) ? "" : versStr) << endl;
      } else {
         wgldisplay_cat.info() << "glGetString(GL_VENDOR) returns NULL!\n";
      }
  }
}

void wglGraphicsWindow::
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
    const GLubyte *vendorname=glGetString(GL_VENDOR);
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
            wgldisplay_cat.warning() << "windows color cursor filename '" << windows_color_cursor_filename << "' not found!!\n";
            return;
        }

        SetClassLongPtr(_mwindow, GCLP_HCURSOR, (LONG_PTR) hNewMouseCursor);
        SetCursor(hNewMouseCursor);

        if (_bLoadedCustomCursor)
           DestroyCursor(_hMouseCursor);
        _hMouseCursor = hNewMouseCursor;

        if (wgldisplay_cat.is_spam())
            wgldisplay_cat.spam() << "loaded color cursor\n";
    }
}

////////////////////////////////////////////////////////////////////
//     Function: Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsWindow::
wglGraphicsWindow(GraphicsPipe* pipe) : GraphicsWindow(pipe) {
  config();
}

////////////////////////////////////////////////////////////////////
//     Function: Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsWindow::
wglGraphicsWindow(GraphicsPipe* pipe, const
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
  wgldisplay_cat.spam() << "================================\n";

  wgldisplay_cat.spam() << msg << ", " << OGLDrvStrings[drvtype] << " driver\n"
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
int wglGraphicsWindow::choose_visual() {

  int mask = _props._mask;
  int want_depth_bits = _props._want_depth_bits;
  int want_color_bits = _props._want_color_bits;
  OGLDriverType drvtype;

  if (mask & W_MULTISAMPLE) {
    wgldisplay_cat.info()
      << "config() - multisample not supported"<< endl;
    mask &= ~W_MULTISAMPLE;
  }
    wgldisplay_cat.info()
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
  if (wgldisplay_cat.is_debug()) {
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

       // use wglinfo.exe instead
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

      if (wgldisplay_cat.is_debug())
          wgldisplay_cat->debug() << "----------------" << endl;

      if ((pfd.iPixelType == PFD_TYPE_COLORINDEX) && !(mask & W_INDEX))
          continue;

       DWORD dwReqFlags=(PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW);

       if (wgldisplay_cat.is_debug()) {
         if (mask & W_ALPHA)
           wgldisplay_cat->debug() << "want alpha, pfd says '"
                       << (int)(pfd.cAlphaBits) << "'" << endl;
         if (mask & W_DEPTH)
           wgldisplay_cat->debug() << "want depth, pfd says '"
                       << (int)(pfd.cDepthBits) << "'" << endl;
         if (mask & W_STENCIL)
           wgldisplay_cat->debug() << "want stencil, pfd says '"
                       << (int)(pfd.cStencilBits) << "'" << endl;
         wgldisplay_cat->debug() << "final flag check "
                     << (int)(pfd.dwFlags & dwReqFlags) << " =? "
                     << (int)dwReqFlags << endl;
         wgldisplay_cat->debug() << "pfd bits = " << (int)(pfd.cColorBits)
                     << endl;
         wgldisplay_cat->debug() << "cur_bpp = " << cur_bpp << endl;
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
      wgldisplay_cat.error() << "ERROR: couldn't find HW-accelerated OpenGL pixfmt appropriate for this desktop!!\n";
      wgldisplay_cat.error() << "make sure OpenGL driver is installed, and try reducing the screen size\n";
      if (cur_bpp>16)
        wgldisplay_cat.error() << "or reducing the desktop screen pixeldepth\n";
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
void wglGraphicsWindow::setup_colormap() {

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
void wglGraphicsWindow::end_frame() {
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
    // chars (like DX fps meter) because WGL doesnt support GDI in
    // double-buffered mode.  Instead we have to use glBitMap display
    // lists created by wglUseFontBitmaps

    glColor3f(0.0f,1.0f,1.0f);

    GLboolean tex_was_on = glIsEnabled(GL_TEXTURE_2D);

    if (tex_was_on)
      glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0.0f,_props._xsize,
            0.0f,_props._ysize,
            -1.0f,1.0f);

    glRasterPos2f(_props._xsize-70,_props._ysize-20);  // these seem to be good for default font

    // set up for a string-drawing display list call
    glListBase(FONT_BITMAP_OGLDISPLAYLISTNUM);

    // draw a string using font display lists.  chars index their
    // corresponding displist name
    glCallLists(strlen(fps_msg), GL_UNSIGNED_BYTE, fps_msg);

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    if (tex_was_on)
      glEnable(GL_TEXTURE_2D);

    _cur_frame_count++;  // only used by fps meter right now
  }

  {
     DO_PSTATS_STUFF(PStatTimer timer(_swap_pcollector);)
     if (_is_synced)
        glFinish();
     else SwapBuffers(_hdc);
  }
  GraphicsWindow::end_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: swap
//       Access:
//  Description: Swaps the front and back buffers explicitly.
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::swap() {
  if (_is_synced)
    SwapBuffers(_hdc);
}

void wglGraphicsWindow::resize(unsigned int xsize,unsigned int ysize) {
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
          wgldisplay_cat.fatal() << "window resize(" << xsize << "," << ysize << ") failed, no compatible fullscreen display mode found!\n";
          return;
      }

      // this causes WM_SIZE msg to be produced
      SetWindowPos(_mwindow, NULL, 0,0, xsize, ysize, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING);

      int chg_result = ChangeDisplaySettings(&dm, CDS_FULLSCREEN);

      if (chg_result!=DISP_CHANGE_SUCCESSFUL) {
            wgldisplay_cat.fatal() << "resize ChangeDisplaySettings failed (error code: " << chg_result <<") for specified res (" << dwWidth << " X " << dwHeight << " X " << dwFullScreenBitDepth <<"), " << dm.dmDisplayFrequency  << "Hz\n";
            exit(1);
      }

      // this assertion could be violated if we eventually allow dynamic fullscrn/windowed mode switching
      assert(_pCurrent_display_settings!=NULL);
      memcpy(_pCurrent_display_settings,&dm,sizeof(DEVMODE));
    }
}

unsigned int wglGraphicsWindow::
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
void wglGraphicsWindow::handle_reshape() {
      RECT view_rect;
      GetClientRect(_mwindow, &view_rect);
      ClientToScreen(_mwindow, (POINT*)&view_rect.left);   // translates top,left pnt
      ClientToScreen(_mwindow, (POINT*)&view_rect.right);  // translates right,bottom pnt

      // change _props xsize,ysize
      resized((view_rect.right - view_rect.left),(view_rect.bottom - view_rect.top));

      _props._xorg = view_rect.left;  // _props origin should reflect upper left of view rectangle
      _props._yorg = view_rect.top;

      if (wgldisplay_cat.is_spam()) {
          wgldisplay_cat.spam() << "reshape to origin: (" << _props._xorg << "," << _props._yorg << "), size: (" << _props._xsize << "," << _props._ysize << ")\n";
      }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_mouse_motion
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::handle_mouse_motion(int x, int y) {
  if (!_input_devices.empty()) {
    _input_devices[0].set_pointer_in_window(x, y);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_mouse_entry
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::handle_mouse_entry(int state) {
  if (state == MOUSE_EXITED) {
    if (!_input_devices.empty()) {
      _input_devices[0].set_pointer_out_of_window();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_keypress
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::
handle_keypress(ButtonHandle key, int x, int y) {
  if (!_input_devices.empty()) {
    _input_devices[0].set_pointer_in_window(x, y);
    if (key != ButtonHandle::none()) {
      _input_devices[0].button_down(key);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_keyrelease
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::
handle_keyrelease(ButtonHandle key) {
  if (key != ButtonHandle::none()) {
    if (!_input_devices.empty()) {
      _input_devices[0].button_up(key);
    }
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

void INLINE wglGraphicsWindow::process_events() {
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
//     Function: wglGraphicsWindow::supports_update
//       Access: Public, Virtual
//  Description: Returns true if this particular kind of
//               GraphicsWindow supports use of the update() function
//               to update the graphics one frame at a time, so that
//               the window does not need to be the program's main
//               loop.  Returns false if the only way to update the
//               window is to call main_loop().
////////////////////////////////////////////////////////////////////
bool wglGraphicsWindow::
supports_update() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: update
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::update() {
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
void wglGraphicsWindow::enable_mouse_input(bool val) {
  _mouse_input_enabled = val;
}

////////////////////////////////////////////////////////////////////
//     Function: enable_mouse_motion
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::enable_mouse_motion(bool val) {
  _mouse_motion_enabled = val;
}

////////////////////////////////////////////////////////////////////
//     Function: enable_mouse_passive_motion
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::enable_mouse_passive_motion(bool val) {
  _mouse_passive_motion_enabled = val;
}

////////////////////////////////////////////////////////////////////
//     Function: make_current
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::make_current() {
  if ((_hdc==NULL)||(_context==NULL)||(_window_inactive)) {
      return;  // we're only allow unmake_current() to set this to NULL
  }

  DO_PSTATS_STUFF(PStatTimer timer(_make_current_pcollector);)
  HGLRC current_context = wglGetCurrentContext();
  HDC current_dc = wglGetCurrentDC();

  if ((current_context != _context) || (current_dc != _hdc)) {
    if (!wglMakeCurrent(_hdc, _context)) {
        PrintErrorMessage(LAST_ERROR);
    }
  }

  report_errors();
}

////////////////////////////////////////////////////////////////////
//     Function: unmake_current
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::unmake_current() {
  report_errors();

  if (!wglMakeCurrent(NULL, NULL)) {
      PrintErrorMessage(LAST_ERROR);
  }
}

int wglGraphicsWindow::get_depth_bitwidth() {
    return _pixelformat.cDepthBits;
}

////////////////////////////////////////////////////////////////////
//     Function: wglGraphicsWindow::get_gsg_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle of the kind of GSG preferred
//               by this kind of window.
////////////////////////////////////////////////////////////////////
TypeHandle wglGraphicsWindow::
get_gsg_type() const {
  return GLGraphicsStateGuardian::get_class_type();
}

GraphicsWindow *wglGraphicsWindow::
make_wglGraphicsWindow(const FactoryParams &params) {
  GraphicsWindow::WindowPipe *pipe_param;
  if (!get_param_into(pipe_param, params)) {
    wgldisplay_cat.error()
      << "No pipe specified for window creation!" << endl;
    return NULL;
  }

  GraphicsPipe *pipe = pipe_param->get_pipe();

  GraphicsWindow::WindowProps *props_param;
  if (!get_param_into(props_param, params)) {
    return new wglGraphicsWindow(pipe);
  } else {
    return new wglGraphicsWindow(pipe, props_param->get_properties());
  }
}

TypeHandle wglGraphicsWindow::get_class_type() {
  return _type_handle;
}

void wglGraphicsWindow::init_type() {
  GraphicsWindow::init_type();
  register_type(_type_handle, "wglGraphicsWindow",
        GraphicsWindow::get_class_type());
}

TypeHandle wglGraphicsWindow::get_type() const {
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
      wglGraphicsWindow *wglwinptr=(*pwin).second;
      return wglwinptr->window_proc(hwnd, msg, wparam, lparam);
   } else if (global_wglwinptr!=NULL){
       // this stuff should only be used during CreateWindow()
       return global_wglwinptr->window_proc(hwnd, msg, wparam, lparam);
   } else {
       // should never need this??  (maybe at shutdwn?)
       return DefWindowProc(hwnd, msg, wparam, lparam);
   }
}

void wglGraphicsWindow::deactivate_window() {
    // current policy is to suspend minimized or deactivated fullscreen windows, but leave
    // regular windows running normally

   if ((!_props._fullscreen) || _exiting_window || _window_inactive || _active_minimized_fullscreen) {
       #ifdef _DEBUG
          if (wgldisplay_cat.is_spam())
            wgldisplay_cat.spam()  << "deactivate_window called, but ignored in current mode"  << endl;
       #endif
     return;
   }

   throw_event("PandaPaused"); // right now this is used to signal python event handler to disable audio

   if (!bResponsive_minimized_fullscreen_window) {
       if (wgldisplay_cat.is_spam())
           wgldisplay_cat.spam() << "WGL window deactivated, releasing gl context and waiting...\n";

      _window_inactive = true;
      unmake_current();
   } else {
       _active_minimized_fullscreen = true;
       assert(_props._fullscreen);

       if (wgldisplay_cat.is_spam())
           wgldisplay_cat.spam() << "WGL window minimized from fullscreen mode, remaining active...\n";
   }

   // make sure window is minimized

   WINDOWPLACEMENT wndpl;
   wndpl.length=sizeof(WINDOWPLACEMENT);

   if (!GetWindowPlacement(_mwindow,&wndpl)) {
       wgldisplay_cat.error() << "GetWindowPlacement failed!\n";
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
           wgldisplay_cat.error() << "Error in SetTimer!\n";
       }
   }

   if (_props._fullscreen) {
      throw_event("PandaRestarted");  // right now this is used to signal python event handler to re-enable audio
   }
}

void wglGraphicsWindow::reactivate_window() {
    if (_window_inactive) {
        if (wgldisplay_cat.is_spam())
            wgldisplay_cat.spam() << "WGL window re-activated...\n";

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
        if (wgldisplay_cat.is_spam())
            wgldisplay_cat.spam() << "redisplaying minimized fullscrn active WGL window...\n";

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
LONG WINAPI wglGraphicsWindow::
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
              wgldisplay_cat.spam()  << "WM_ACTIVATEAPP(" << (bool)(wparam!=0) <<") received\n";
            #endif

           if (!wparam) {
               deactivate_window();
               return 0;
           }         // dont want to reactivate until window is actually un-minimized (see WM_SIZE)
           break;
        }

    case WM_EXITSIZEMOVE:
            #ifdef _DEBUG
              wgldisplay_cat.spam()  << "WM_EXITSIZEMOVE received"  << endl;
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
                    wgldisplay_cat.spam() << "WM_SIZE received with width:" << width << "  height: " << height << " flags: " <<
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
        if (!_ime_open) {
          _ime_active = false;  // Sanity enforcement.
        }
        ImmReleaseContext(hwnd, hIMC);
      }
      break;

    case WM_IME_STARTCOMPOSITION:
      _ime_active = true;
      break;

    case WM_IME_ENDCOMPOSITION:
      _ime_active = false;
      break;

    case WM_IME_COMPOSITION:
      if (lparam & GCS_RESULTSTR) {
        if (!_input_devices.empty()) {
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
              PrintErrorMessage(LAST_ERROR);
            }
            for (int i = 0; i < wide_size; i++) {
              _input_devices[0].keystroke(wide_result[i]);
            }
          }

          ImmReleaseContext(hwnd, hIMC);
        }
        return 0;
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
      handle_keypress(lookup_key(wparam), point.x, point.y);

      // Handle Cntrl-V paste from clipboard.  Is there a better way
      // to detect this hotkey?
      if ((wparam=='V') && (GetKeyState(VK_CONTROL) < 0) && 
          !_input_devices.empty()) {
        HGLOBAL hglb;
        char *lptstr;

        if (!IsClipboardFormatAvailable(CF_TEXT))
          return 0;

        if (!OpenClipboard(NULL))
          return 0;
        
        // Maybe we should support CF_UNICODETEXT if it is available
        // too?
        hglb = GetClipboardData(CF_TEXT);
        if (hglb!=NULL) {
          lptstr = (char *) GlobalLock(hglb);
          if (lptstr != NULL)  {
            char *pChar;
            for(pChar=lptstr;*pChar!=NULL;pChar++) {
              _input_devices[0].keystroke((uchar)*pChar);
            }
            GlobalUnlock(hglb);
          }
        }
        CloseClipboard();
      }
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
            // wgldisplay_cat.info() << "got WM_SETFOCUS\n";

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
         //wgldisplay_cat.spam() << "returning control to app\n";
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
ButtonHandle wglGraphicsWindow::
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

#if 0
// old fns

////////////////////////////////////////////////////////////////////
//     Function: get_config
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::
get_config(PIXELFORMATDESCRIPTOR *visual, int attrib, int *value) {
  if (visual == NULL)
    return;

  switch (attrib) {
    case GLX_USE_GL:
      if (visual->dwFlags & (PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW)) {
    if (visual->iPixelType == PFD_TYPE_COLORINDEX &&
        visual->cColorBits >= 24) {
      *value = 0;
    } else {
      *value = 1;
    }
      } else {
    *value = 0;
      }
      break;
    case GLX_BUFFER_SIZE:
      if (visual->iPixelType == PFD_TYPE_RGBA)
    *value = visual->cColorBits;
      else
    *value = 8;
      break;
    case GLX_LEVEL:
      *value = visual->bReserved;
      break;
    case GLX_RGBA:
      *value = visual->iPixelType == PFD_TYPE_RGBA;
      break;
    case GLX_DOUBLEBUFFER:
      *value = visual->dwFlags & PFD_DOUBLEBUFFER;
      break;
    case GLX_STEREO:
      *value = visual->dwFlags & PFD_STEREO;
      break;
    case GLX_AUX_BUFFERS:
      *value = visual->cAuxBuffers;
      break;
    case GLX_RED_SIZE:
      *value = visual->cRedBits;
      break;
    case GLX_GREEN_SIZE:
      *value = visual->cGreenBits;
      break;
    case GLX_BLUE_SIZE:
      *value = visual->cBlueBits;
      break;
    case GLX_ALPHA_SIZE:
      *value = visual->cAlphaBits;
      break;
    case GLX_DEPTH_SIZE:
      *value = visual->cDepthBits;
      break;
    case GLX_STENCIL_SIZE:
      *value = visual->cStencilBits;
      break;
    case GLX_ACCUM_RED_SIZE:
      *value = visual->cAccumRedBits;
      break;
    case GLX_ACCUM_GREEN_SIZE:
      *value = visual->cAccumGreenBits;
      break;
    case GLX_ACCUM_BLUE_SIZE:
      *value = visual->cAccumBlueBits;
      break;
    case GLX_ACCUM_ALPHA_SIZE:
      *value = visual->cAccumAlphaBits;
      break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: try_for_visual
//  Description: This is a static function that attempts to get the
//               requested visual, if it is available.  It's just a
//               wrapper around glXChooseVisual().  It returns the
//               visual information if possible, or NULL if it is not.
////////////////////////////////////////////////////////////////////
PIXELFORMATDESCRIPTOR* wglGraphicsWindow::
try_for_visual(wglGraphicsPipe *pipe, int mask,
           int want_depth_bits, int want_color_bits) {
  static const int max_attrib_list = 32;
  int attrib_list[max_attrib_list];
  int n=0;

  wgldisplay_cat.debug()
    << "Trying for visual with: RGB(" << want_color_bits << ")";

  int want_color_component_bits;
  if (mask & W_ALPHA) {
    want_color_component_bits = max(want_color_bits / 4, 1);
  } else {
    want_color_component_bits = max(want_color_bits / 3, 1);
  }

  attrib_list[n++] = GLX_RGBA;
  attrib_list[n++] = GLX_RED_SIZE;
  attrib_list[n++] = want_color_component_bits;
  attrib_list[n++] = GLX_GREEN_SIZE;
  attrib_list[n++] = want_color_component_bits;
  attrib_list[n++] = GLX_BLUE_SIZE;
  attrib_list[n++] = want_color_component_bits;

  if (mask & W_ALPHA) {
    wgldisplay_cat.debug(false) << " ALPHA";
    attrib_list[n++] = GLX_ALPHA_SIZE;
    attrib_list[n++] = want_color_component_bits;
  }
  if (mask & W_DOUBLE) {
    wgldisplay_cat.debug(false) << " DOUBLEBUFFER";
    attrib_list[n++] = GLX_DOUBLEBUFFER;
  }
  if (mask & W_STEREO) {
    wgldisplay_cat.debug(false) << " STEREO";
    attrib_list[n++] = GLX_STEREO;
  }
  if (mask & W_DEPTH) {
    wgldisplay_cat.debug(false) << " DEPTH(" << want_depth_bits << ")";
    attrib_list[n++] = GLX_DEPTH_SIZE;
    attrib_list[n++] = want_depth_bits;
  }
  if (mask & W_STENCIL) {
    wgldisplay_cat.debug(false) << " STENCIL";
    attrib_list[n++] = GLX_STENCIL_SIZE;
    attrib_list[n++] = 1;
  }
  if (mask & W_ACCUM) {
    wgldisplay_cat.debug(false) << " ACCUM";
    attrib_list[n++] = GLX_ACCUM_RED_SIZE;
    attrib_list[n++] = want_color_component_bits;
    attrib_list[n++] = GLX_ACCUM_GREEN_SIZE;
    attrib_list[n++] = want_color_component_bits;
    attrib_list[n++] = GLX_ACCUM_BLUE_SIZE;
    attrib_list[n++] = want_color_component_bits;
    if (mask & W_ALPHA) {
      attrib_list[n++] = GLX_ACCUM_ALPHA_SIZE;
      attrib_list[n++] = want_color_component_bits;
    }
  }

  // Terminate the list
  nassertr(n < max_attrib_list, NULL);
  attrib_list[n] = 0L;

  PIXELFORMATDESCRIPTOR pfd;
  PIXELFORMATDESCRIPTOR *match = NULL;
  bool stereo = false;

  memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
  pfd.nSize = (sizeof(PIXELFORMATDESCRIPTOR));
  pfd.nVersion = 1;

  // Defaults
  pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
  pfd.iPixelType = PFD_TYPE_COLORINDEX;
  pfd.cColorBits = 32;
  pfd.cDepthBits = 0;

  int *p = attrib_list;
  while (*p) {
    switch (*p) {
      case GLX_USE_GL:
    pfd.dwFlags |= PFD_SUPPORT_OPENGL;
    break;
      case GLX_LEVEL:
    pfd.bReserved = *(++p);
    break;
      case GLX_RGBA:
    pfd.iPixelType = PFD_TYPE_RGBA;
    break;
      case GLX_DOUBLEBUFFER:
    pfd.dwFlags |= PFD_DOUBLEBUFFER;
    break;
      case GLX_STEREO:
    stereo = true;
    pfd.dwFlags |= PFD_STEREO;
    break;
      case GLX_AUX_BUFFERS:
    pfd.cAuxBuffers = *(++p);
    break;
      case GLX_RED_SIZE:
    pfd.cRedBits = 8; // Try to get the maximum
    ++p;
    break;
      case GLX_GREEN_SIZE:
    pfd.cGreenBits = 8; // Try to get the maximum
    ++p;
    break;
      case GLX_BLUE_SIZE:
    pfd.cBlueBits = 8; // Try to get the maximum
    ++p;
    break;
      case GLX_ALPHA_SIZE:
    pfd.cAlphaBits = 8; // Try to get the maximum
    ++p;
    break;
      case GLX_DEPTH_SIZE:
    pfd.cDepthBits = 32; // Try to get the maximum
    ++p;
    break;
      case GLX_STENCIL_SIZE:
    pfd.cStencilBits = *(++p);
    break;
      case GLX_ACCUM_RED_SIZE:
      case GLX_ACCUM_GREEN_SIZE:
      case GLX_ACCUM_BLUE_SIZE:
      case GLX_ACCUM_ALPHA_SIZE:
    // Only cAccumBits is used for requesting accum buffer
    pfd.cAccumBits = 1;
    ++p;
    break;
    }
    ++p;
  }

  int pf = ChoosePixelFormat(_hdc, &pfd);
  if (pf > 0) {
    DescribePixelFormat(_hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &_pixelformat);

    // ChoosePixelFormat is dumb about stereo
    if (stereo) {
      if (!(_pixelformat->dwFlags & PFD_STEREO)) {
          wgldisplay_cat.info() << "try_for_visual() - request for stereo failed" << endl;
      }
    }
  }

  return match;
}

#endif

extern char *ConvDDErrorToString(const HRESULT &error) {
    switch(error) {
        case E_FAIL:
            return "Unspecified error E_FAIL";

        case DD_OK:
            return "No error.\0";

        case DDERR_ALREADYINITIALIZED     : // (5)
            return "DDERR_ALREADYINITIALIZED        ";//: // (5)
        case DDERR_CANNOTATTACHSURFACE        : // (10)
            return "DDERR_CANNOTATTACHSURFACE       ";//: // (10)
        case DDERR_CANNOTDETACHSURFACE        : // (20)
            return "DDERR_CANNOTDETACHSURFACE       ";//: // (20)
        case DDERR_CURRENTLYNOTAVAIL          : // (40)
            return "DDERR_CURRENTLYNOTAVAIL         ";//: // (40)
        case DDERR_EXCEPTION              : // (55)
            return "DDERR_EXCEPTION             ";//: // (55)
        case DDERR_HEIGHTALIGN            : // (90)
            return "DDERR_HEIGHTALIGN           ";//: // (90)
        case DDERR_INCOMPATIBLEPRIMARY        : // (95)
            return "DDERR_INCOMPATIBLEPRIMARY       ";//: // (95)
        case DDERR_INVALIDCAPS            : // (100)
            return "DDERR_INVALIDCAPS           ";//: // (100)
        case DDERR_INVALIDCLIPLIST            : // (110)
            return "DDERR_INVALIDCLIPLIST           ";//: // (110)
        case DDERR_INVALIDMODE            : // (120)
            return "DDERR_INVALIDMODE           ";//: // (120)
        case DDERR_INVALIDOBJECT          : // (130)
            return "DDERR_INVALIDOBJECT         ";//: // (130)
        case DDERR_INVALIDPIXELFORMAT     : // (145)
            return "DDERR_INVALIDPIXELFORMAT        ";//: // (145)
        case DDERR_INVALIDRECT            : // (150)
            return "DDERR_INVALIDRECT           ";//: // (150)
        case DDERR_LOCKEDSURFACES         : // (160)
            return "DDERR_LOCKEDSURFACES            ";//: // (160)
        case DDERR_NO3D               : // (170)
            return "DDERR_NO3D              ";//: // (170)
        case DDERR_NOALPHAHW              : // (180)
            return "DDERR_NOALPHAHW             ";//: // (180)
        case DDERR_NOCLIPLIST         : // (205)
            return "DDERR_NOCLIPLIST            ";//: // (205)
        case DDERR_NOCOLORCONVHW          : // (210)
            return "DDERR_NOCOLORCONVHW         ";//: // (210)
        case DDERR_NOCOOPERATIVELEVELSET      : // (212)
            return "DDERR_NOCOOPERATIVELEVELSET     ";//: // (212)
        case DDERR_NOCOLORKEY         : // (215)
            return "DDERR_NOCOLORKEY            ";//: // (215)
        case DDERR_NOCOLORKEYHW           : // (220)
            return "DDERR_NOCOLORKEYHW          ";//: // (220)
        case DDERR_NODIRECTDRAWSUPPORT        : // (222)
            return "DDERR_NODIRECTDRAWSUPPORT       ";//: // (222)
        case DDERR_NOEXCLUSIVEMODE            : // (225)
            return "DDERR_NOEXCLUSIVEMODE           ";//: // (225)
        case DDERR_NOFLIPHW               : // (230)
            return "DDERR_NOFLIPHW              ";//: // (230)
        case DDERR_NOGDI              : // (240)
            return "DDERR_NOGDI             ";//: // (240)
        case DDERR_NOMIRRORHW         : // (250)
            return "DDERR_NOMIRRORHW            ";//: // (250)
        case DDERR_NOTFOUND               : // (255)
            return "DDERR_NOTFOUND              ";//: // (255)
        case DDERR_NOOVERLAYHW            : // (260)
        return "DDERR_NOOVERLAYHW           ";//: // (260)
        case DDERR_NORASTEROPHW           : // (280)
            return "DDERR_NORASTEROPHW          ";//: // (280)
        case DDERR_NOROTATIONHW           : // (290)
            return "DDERR_NOROTATIONHW          ";//: // (290)
        case DDERR_NOSTRETCHHW            : // (310)
            return "DDERR_NOSTRETCHHW           ";//: // (310)
        case DDERR_NOT4BITCOLOR           : // (316)
            return "DDERR_NOT4BITCOLOR          ";//: // (316)
        case DDERR_NOT4BITCOLORINDEX          : // (317)
            return "DDERR_NOT4BITCOLORINDEX         ";//: // (317)
        case DDERR_NOT8BITCOLOR           : // (320)
            return "DDERR_NOT8BITCOLOR          ";//: // (320)
        case DDERR_NOTEXTUREHW            : // (330)
            return "DDERR_NOTEXTUREHW           ";//: // (330)
        case DDERR_NOVSYNCHW              : // (335)
            return "DDERR_NOVSYNCHW             ";//: // (335)
        case DDERR_NOZBUFFERHW            : // (340)
            return "DDERR_NOZBUFFERHW           ";//: // (340)
        case DDERR_NOZOVERLAYHW           : // (350)
            return "DDERR_NOZOVERLAYHW          ";//: // (350)
        case DDERR_OUTOFCAPS              : // (360)
            return "DDERR_OUTOFCAPS             ";//: // (360)
        case DDERR_OUTOFVIDEOMEMORY           : // (380)
            return "DDERR_OUTOFVIDEOMEMORY          ";//: // (380)
        case DDERR_OVERLAYCANTCLIP            : // (382)
            return "DDERR_OVERLAYCANTCLIP           ";//: // (382)
        case DDERR_OVERLAYCOLORKEYONLYONEACTIVE   : // (384)
            return "DDERR_OVERLAYCOLORKEYONLYONEACTIVE  ";//: // (384)
        case DDERR_PALETTEBUSY            : // (387)
            return "DDERR_PALETTEBUSY           ";//: // (387)
        case DDERR_COLORKEYNOTSET         : // (400)
            return "DDERR_COLORKEYNOTSET            ";//: // (400)
        case DDERR_SURFACEALREADYATTACHED     : // (410)
            return "DDERR_SURFACEALREADYATTACHED        ";//: // (410)
        case DDERR_SURFACEALREADYDEPENDENT        : // (420)
            return "DDERR_SURFACEALREADYDEPENDENT       ";//: // (420)
        case DDERR_SURFACEBUSY            : // (430)
            return "DDERR_SURFACEBUSY           ";//: // (430)
        case DDERR_CANTLOCKSURFACE                   : // (435)
            return "DDERR_CANTLOCKSURFACE";//: // (435)
        case DDERR_SURFACEISOBSCURED          : // (440)
            return "DDERR_SURFACEISOBSCURED         ";//: // (440)
        case DDERR_SURFACELOST            : // (450)
            return "DDERR_SURFACELOST           ";//: // (450)
        case DDERR_SURFACENOTATTACHED     : // (460)
            return "DDERR_SURFACENOTATTACHED        ";//: // (460)
        case DDERR_TOOBIGHEIGHT           : // (470)
            return "DDERR_TOOBIGHEIGHT          ";//: // (470)
        case DDERR_TOOBIGSIZE         : // (480)
            return "DDERR_TOOBIGSIZE            ";//: // (480)
        case DDERR_TOOBIGWIDTH            : // (490)
            return "DDERR_TOOBIGWIDTH           ";//: // (490)
        case DDERR_UNSUPPORTEDFORMAT          : // (510)
            return "DDERR_UNSUPPORTEDFORMAT         ";//: // (510)
        case DDERR_UNSUPPORTEDMASK            : // (520)
            return "DDERR_UNSUPPORTEDMASK           ";//: // (520)
        case DDERR_VERTICALBLANKINPROGRESS        : // (537)
            return "DDERR_VERTICALBLANKINPROGRESS       ";//: // (537)
        case DDERR_WASSTILLDRAWING            : // (540)
            return "DDERR_WASSTILLDRAWING           ";//: // (540)
        case DDERR_XALIGN             : // (560)
            return "DDERR_XALIGN                ";//: // (560)
        case DDERR_INVALIDDIRECTDRAWGUID      : // (561)
            return "DDERR_INVALIDDIRECTDRAWGUID     ";//: // (561)
        case DDERR_DIRECTDRAWALREADYCREATED       : // (562)
            return "DDERR_DIRECTDRAWALREADYCREATED      ";//: // (562)
        case DDERR_NODIRECTDRAWHW         : // (563)
            return "DDERR_NODIRECTDRAWHW            ";//: // (563)
        case DDERR_PRIMARYSURFACEALREADYEXISTS    : // (564)
            return "DDERR_PRIMARYSURFACEALREADYEXISTS   ";//: // (564)
        case DDERR_NOEMULATION            : // (565)
            return "DDERR_NOEMULATION           ";//: // (565)
        case DDERR_REGIONTOOSMALL         : // (566)
            return "DDERR_REGIONTOOSMALL            ";//: // (566)
        case DDERR_CLIPPERISUSINGHWND     : // (567)
            return "DDERR_CLIPPERISUSINGHWND        ";//: // (567)
        case DDERR_NOCLIPPERATTACHED          : // (568)
            return "DDERR_NOCLIPPERATTACHED         ";//: // (568)
        case DDERR_NOHWND             : // (569)
            return "DDERR_NOHWND                ";//: // (569)
        case DDERR_HWNDSUBCLASSED         : // (570)
            return "DDERR_HWNDSUBCLASSED            ";//: // (570)
        case DDERR_HWNDALREADYSET         : // (571)
            return "DDERR_HWNDALREADYSET            ";//: // (571)
        case DDERR_NOPALETTEATTACHED          : // (572)
            return "DDERR_NOPALETTEATTACHED         ";//: // (572)
        case DDERR_NOPALETTEHW            : // (573)
            return "DDERR_NOPALETTEHW           ";//: // (573)
        case DDERR_BLTFASTCANTCLIP            : // (574)
            return "DDERR_BLTFASTCANTCLIP           ";//: // (574)
        case DDERR_NOBLTHW                : // (575)
            return "DDERR_NOBLTHW               ";//: // (575)
        case DDERR_NODDROPSHW         : // (576)
            return "DDERR_NODDROPSHW            ";//: // (576)
        case DDERR_OVERLAYNOTVISIBLE          : // (577)
            return "DDERR_OVERLAYNOTVISIBLE         ";//: // (577)
        case DDERR_NOOVERLAYDEST          : // (578)
            return "DDERR_NOOVERLAYDEST         ";//: // (578)
        case DDERR_INVALIDPOSITION            : // (579)
            return "DDERR_INVALIDPOSITION           ";//: // (579)
        case DDERR_NOTAOVERLAYSURFACE     : // (580)
            return "DDERR_NOTAOVERLAYSURFACE        ";//: // (580)
        case DDERR_EXCLUSIVEMODEALREADYSET        : // (581)
            return "DDERR_EXCLUSIVEMODEALREADYSET       ";//: // (581)
        case DDERR_NOTFLIPPABLE           : // (582)
            return "DDERR_NOTFLIPPABLE          ";//: // (582)
        case DDERR_CANTDUPLICATE          : // (583)
            return "DDERR_CANTDUPLICATE         ";//: // (583)
        case DDERR_NOTLOCKED              : // (584)
            return "DDERR_NOTLOCKED             ";//: // (584)
        case DDERR_CANTCREATEDC           : // (585)
            return "DDERR_CANTCREATEDC          ";//: // (585)
        case DDERR_NODC               : // (586)
            return "DDERR_NODC              ";//: // (586)
        case DDERR_WRONGMODE              : // (587)
            return "DDERR_WRONGMODE             ";//: // (587)
        case DDERR_IMPLICITLYCREATED          : // (588)
            return "DDERR_IMPLICITLYCREATED         ";//: // (588)
        case DDERR_NOTPALETTIZED          : // (589)
            return "DDERR_NOTPALETTIZED         ";//: // (589)
        case DDERR_UNSUPPORTEDMODE            : // (590)
            return "DDERR_UNSUPPORTEDMODE           ";//: // (590)
        case DDERR_NOMIPMAPHW         : // (591)
            return "DDERR_NOMIPMAPHW            ";//: // (591)
        case DDERR_INVALIDSURFACETYPE                : // (592)
            return "DDERR_INVALIDSURFACETYPE";//: // (592)
        case DDERR_NOOPTIMIZEHW                      : // (600)
            return "DDERR_NOOPTIMIZEHW";//: // (600)
        case DDERR_NOTLOADED                         : // (601)
            return "DDERR_NOTLOADED";//: // (601)
        case DDERR_NOFOCUSWINDOW                     : // (602)
            return "DDERR_NOFOCUSWINDOW";//: // (602)
        case DDERR_DCALREADYCREATED           : // (620)
            return "DDERR_DCALREADYCREATED          ";//: // (620)
        case DDERR_NONONLOCALVIDMEM                  : // (630)
            return "DDERR_NONONLOCALVIDMEM";//: // (630)
        case DDERR_CANTPAGELOCK           : // (640)
            return "DDERR_CANTPAGELOCK          ";//: // (640)
        case DDERR_CANTPAGEUNLOCK         : // (660)
            return "DDERR_CANTPAGEUNLOCK            ";//: // (660)
        case DDERR_NOTPAGELOCKED          : // (680)
            return "DDERR_NOTPAGELOCKED         ";//: // (680)
        case DDERR_MOREDATA                   : // (690)
            return "DDERR_MOREDATA                  ";//: // (690)
        case DDERR_VIDEONOTACTIVE             : // (695)
            return "DDERR_VIDEONOTACTIVE            ";//: // (695)
        case DDERR_DEVICEDOESNTOWNSURFACE         : // (699)
            return "DDERR_DEVICEDOESNTOWNSURFACE        ";//: // (699)

        case E_UNEXPECTED                     :
            return "E_UNEXPECTED                     ";
        case E_NOTIMPL                        :
            return "E_NOTIMPL                        ";
        case E_OUTOFMEMORY                    :
            return "E_OUTOFMEMORY                    ";
        case E_INVALIDARG                     :
            return "E_INVALIDARG or DDERR_INVALIDPARAMS";
        case E_NOINTERFACE                    :
            return "E_NOINTERFACE                    ";
        case E_POINTER                        :
            return "E_POINTER                        ";
        case E_HANDLE                         :
            return "E_HANDLE                         ";
        case E_ABORT                          :
            return "E_ABORT                          ";
//    case E_FAIL                           :
//    return "E_FAIL                           ";
        case E_ACCESSDENIED                   :
            return "E_ACCESSDENIED                   ";
        case E_PENDING                        :
            return "E_PENDING                        ";
        case CO_E_INIT_TLS                    :
            return "CO_E_INIT_TLS                    ";
        case CO_E_INIT_SHARED_ALLOCATOR       :
            return "CO_E_INIT_SHARED_ALLOCATOR       ";
        case CO_E_INIT_MEMORY_ALLOCATOR       :
            return "CO_E_INIT_MEMORY_ALLOCATOR       ";
        case CO_E_INIT_CLASS_CACHE            :
            return "CO_E_INIT_CLASS_CACHE            ";
        case CO_E_INIT_RPC_CHANNEL            :
            return "CO_E_INIT_RPC_CHANNEL            ";
        case CO_E_INIT_TLS_SET_CHANNEL_CONTROL :
            return "CO_E_INIT_TLS_SET_CHANNEL_CONTROL ";
        case CO_E_INIT_TLS_CHANNEL_CONTROL    :
            return "CO_E_INIT_TLS_CHANNEL_CONTROL    ";
        case CO_E_INIT_UNACCEPTED_USER_ALLOCATOR :
            return "CO_E_INIT_UNACCEPTED_USER_ALLOCATOR ";
        case CO_E_INIT_SCM_MUTEX_EXISTS       :
            return "CO_E_INIT_SCM_MUTEX_EXISTS       ";
        case CO_E_INIT_SCM_FILE_MAPPING_EXISTS :
            return "CO_E_INIT_SCM_FILE_MAPPING_EXISTS ";
        case CO_E_INIT_SCM_MAP_VIEW_OF_FILE   :
            return "CO_E_INIT_SCM_MAP_VIEW_OF_FILE   ";
        case CO_E_INIT_SCM_EXEC_FAILURE       :
            return "CO_E_INIT_SCM_EXEC_FAILURE       ";
        case CO_E_INIT_ONLY_SINGLE_THREADED   :
            return "CO_E_INIT_ONLY_SINGLE_THREADED   ";
        case CO_E_CANT_REMOTE                 :
            return "CO_E_CANT_REMOTE                 ";
        case CO_E_BAD_SERVER_NAME             :
            return "CO_E_BAD_SERVER_NAME             ";
        case CO_E_WRONG_SERVER_IDENTITY       :
            return "CO_E_WRONG_SERVER_IDENTITY       ";
        case CO_E_OLE1DDE_DISABLED            :
            return "CO_E_OLE1DDE_DISABLED            ";
        case CO_E_RUNAS_SYNTAX                :
            return "CO_E_RUNAS_SYNTAX                ";
        case CO_E_CREATEPROCESS_FAILURE       :
            return "CO_E_CREATEPROCESS_FAILURE       ";
        case CO_E_RUNAS_CREATEPROCESS_FAILURE :
            return "CO_E_RUNAS_CREATEPROCESS_FAILURE ";
        case CO_E_RUNAS_LOGON_FAILURE         :
            return "CO_E_RUNAS_LOGON_FAILURE         ";
        case CO_E_LAUNCH_PERMSSION_DENIED     :
            return "CO_E_LAUNCH_PERMSSION_DENIED     ";
        case CO_E_START_SERVICE_FAILURE       :
            return "CO_E_START_SERVICE_FAILURE       ";
        case CO_E_REMOTE_COMMUNICATION_FAILURE :
            return "CO_E_REMOTE_COMMUNICATION_FAILURE ";
        case CO_E_SERVER_START_TIMEOUT        :
            return "CO_E_SERVER_START_TIMEOUT        ";
        case CO_E_CLSREG_INCONSISTENT         :
            return "CO_E_CLSREG_INCONSISTENT         ";
        case CO_E_IIDREG_INCONSISTENT         :
            return "CO_E_IIDREG_INCONSISTENT         ";
        case CO_E_NOT_SUPPORTED               :
            return "CO_E_NOT_SUPPORTED               ";
        case CO_E_RELOAD_DLL                  :
            return "CO_E_RELOAD_DLL                  ";
        case CO_E_MSI_ERROR                   :
            return "CO_E_MSI_ERROR                   ";
        case OLE_E_OLEVERB                    :
            return "OLE_E_OLEVERB                    ";
        case OLE_E_ADVF                       :
            return "OLE_E_ADVF                       ";
        case OLE_E_ENUM_NOMORE                :
            return "OLE_E_ENUM_NOMORE                ";
        case OLE_E_ADVISENOTSUPPORTED         :
            return "OLE_E_ADVISENOTSUPPORTED         ";
        case OLE_E_NOCONNECTION               :
            return "OLE_E_NOCONNECTION               ";
        case OLE_E_NOTRUNNING                 :
            return "OLE_E_NOTRUNNING                 ";
        case OLE_E_NOCACHE                    :
            return "OLE_E_NOCACHE                    ";
        case OLE_E_BLANK                      :
            return "OLE_E_BLANK                      ";
        case OLE_E_CLASSDIFF                  :
            return "OLE_E_CLASSDIFF                  ";
        case OLE_E_CANT_GETMONIKER            :
            return "OLE_E_CANT_GETMONIKER            ";
        case OLE_E_CANT_BINDTOSOURCE          :
            return "OLE_E_CANT_BINDTOSOURCE          ";
        case OLE_E_STATIC                     :
            return "OLE_E_STATIC                     ";
        case OLE_E_PROMPTSAVECANCELLED        :
            return "OLE_E_PROMPTSAVECANCELLED        ";
        case OLE_E_INVALIDRECT                :
            return "OLE_E_INVALIDRECT                ";
        case OLE_E_WRONGCOMPOBJ               :
            return "OLE_E_WRONGCOMPOBJ               ";
        case OLE_E_INVALIDHWND                :
            return "OLE_E_INVALIDHWND                ";
        case OLE_E_NOT_INPLACEACTIVE          :
            return "OLE_E_NOT_INPLACEACTIVE          ";
        case OLE_E_CANTCONVERT                :
            return "OLE_E_CANTCONVERT                ";
        case OLE_E_NOSTORAGE                  :
            return "OLE_E_NOSTORAGE                  ";
        case DV_E_FORMATETC                   :
            return "DV_E_FORMATETC                   ";
        case DV_E_DVTARGETDEVICE              :
            return "DV_E_DVTARGETDEVICE              ";
        case DV_E_STGMEDIUM                   :
            return "DV_E_STGMEDIUM                   ";
        case DV_E_STATDATA                    :
            return "DV_E_STATDATA                    ";
        case DV_E_LINDEX                      :
            return "DV_E_LINDEX                      ";
        case DV_E_TYMED                       :
            return "DV_E_TYMED                       ";
        case DV_E_CLIPFORMAT                  :
            return "DV_E_CLIPFORMAT                  ";
        case DV_E_DVASPECT                    :
            return "DV_E_DVASPECT                    ";
        case DV_E_DVTARGETDEVICE_SIZE         :
            return "DV_E_DVTARGETDEVICE_SIZE         ";
        case DV_E_NOIVIEWOBJECT               :
            return "DV_E_NOIVIEWOBJECT               ";
        case DRAGDROP_E_NOTREGISTERED         :
            return "DRAGDROP_E_NOTREGISTERED         ";
        case DRAGDROP_E_ALREADYREGISTERED     :
            return "DRAGDROP_E_ALREADYREGISTERED     ";
        case DRAGDROP_E_INVALIDHWND           :
            return "DRAGDROP_E_INVALIDHWND           ";
        case CLASS_E_NOAGGREGATION            :
            return "CLASS_E_NOAGGREGATION            ";
        case CLASS_E_CLASSNOTAVAILABLE        :
            return "CLASS_E_CLASSNOTAVAILABLE        ";
        case CLASS_E_NOTLICENSED              :
            return "CLASS_E_NOTLICENSED              ";
        case VIEW_E_DRAW                      :
            return "VIEW_E_DRAW                      ";
        case REGDB_E_READREGDB                :
            return "REGDB_E_READREGDB                ";
        case REGDB_E_WRITEREGDB               :
            return "REGDB_E_WRITEREGDB               ";
        case REGDB_E_KEYMISSING               :
            return "REGDB_E_KEYMISSING               ";
        case REGDB_E_INVALIDVALUE             :
            return "REGDB_E_INVALIDVALUE             ";
        case REGDB_E_CLASSNOTREG              :
            return "REGDB_E_CLASSNOTREG              ";
        case REGDB_E_IIDNOTREG                :
            return "REGDB_E_IIDNOTREG                ";
        case CAT_E_CATIDNOEXIST               :
            return "CAT_E_CATIDNOEXIST               ";
        case CAT_E_NODESCRIPTION              :
            return "CAT_E_NODESCRIPTION              ";
        case CS_E_PACKAGE_NOTFOUND            :
            return "CS_E_PACKAGE_NOTFOUND            ";
        case CS_E_NOT_DELETABLE               :
            return "CS_E_NOT_DELETABLE               ";
        case CS_E_CLASS_NOTFOUND              :
            return "CS_E_CLASS_NOTFOUND              ";
        case CS_E_INVALID_VERSION             :
            return "CS_E_INVALID_VERSION             ";
        case CS_E_NO_CLASSSTORE               :
            return "CS_E_NO_CLASSSTORE               ";
        case CACHE_E_NOCACHE_UPDATED          :
            return "CACHE_E_NOCACHE_UPDATED          ";
        case OLEOBJ_E_NOVERBS                 :
            return "OLEOBJ_E_NOVERBS                 ";
        case OLEOBJ_E_INVALIDVERB             :
            return "OLEOBJ_E_INVALIDVERB             ";
        case INPLACE_E_NOTUNDOABLE            :
            return "INPLACE_E_NOTUNDOABLE            ";
        case INPLACE_E_NOTOOLSPACE            :
            return "INPLACE_E_NOTOOLSPACE            ";
        case CONVERT10_E_OLESTREAM_GET        :
            return "CONVERT10_E_OLESTREAM_GET        ";
        case CONVERT10_E_OLESTREAM_PUT        :
            return "CONVERT10_E_OLESTREAM_PUT        ";
        case CONVERT10_E_OLESTREAM_FMT        :
            return "CONVERT10_E_OLESTREAM_FMT        ";
        case CONVERT10_E_OLESTREAM_BITMAP_TO_DIB :
            return "CONVERT10_E_OLESTREAM_BITMAP_TO_DIB ";
        case CONVERT10_E_STG_FMT              :
            return "CONVERT10_E_STG_FMT              ";
        case CONVERT10_E_STG_NO_STD_STREAM    :
            return "CONVERT10_E_STG_NO_STD_STREAM    ";
        case CONVERT10_E_STG_DIB_TO_BITMAP    :
            return "CONVERT10_E_STG_DIB_TO_BITMAP    ";
        case CLIPBRD_E_CANT_OPEN              :
            return "CLIPBRD_E_CANT_OPEN              ";
        case CLIPBRD_E_CANT_EMPTY             :
            return "CLIPBRD_E_CANT_EMPTY             ";
        case CLIPBRD_E_CANT_SET               :
            return "CLIPBRD_E_CANT_SET               ";
        case CLIPBRD_E_BAD_DATA               :
            return "CLIPBRD_E_BAD_DATA               ";
        case CLIPBRD_E_CANT_CLOSE             :
            return "CLIPBRD_E_CANT_CLOSE             ";
        case MK_E_CONNECTMANUALLY             :
            return "MK_E_CONNECTMANUALLY             ";
        case MK_E_EXCEEDEDDEADLINE            :
            return "MK_E_EXCEEDEDDEADLINE            ";
        case MK_E_NEEDGENERIC                 :
            return "MK_E_NEEDGENERIC                 ";
        case MK_E_UNAVAILABLE                 :
            return "MK_E_UNAVAILABLE                 ";
        case MK_E_SYNTAX                      :
            return "MK_E_SYNTAX                      ";
        case MK_E_NOOBJECT                    :
            return "MK_E_NOOBJECT                    ";
        case MK_E_INVALIDEXTENSION            :
            return "MK_E_INVALIDEXTENSION            ";
        case MK_E_INTERMEDIATEINTERFACENOTSUPPORTED :
            return "MK_E_INTERMEDIATEINTERFACENOTSUPPORTED ";
        case MK_E_NOTBINDABLE                 :
            return "MK_E_NOTBINDABLE                 ";
        case MK_E_NOTBOUND                    :
            return "MK_E_NOTBOUND                    ";
        case MK_E_CANTOPENFILE                :
            return "MK_E_CANTOPENFILE                ";
        case MK_E_MUSTBOTHERUSER              :
            return "MK_E_MUSTBOTHERUSER              ";
        case MK_E_NOINVERSE                   :
            return "MK_E_NOINVERSE                   ";
        case MK_E_NOSTORAGE                   :
            return "MK_E_NOSTORAGE                   ";
        case MK_E_NOPREFIX                    :
            return "MK_E_NOPREFIX                    ";
        case MK_E_ENUMERATION_FAILED          :
            return "MK_E_ENUMERATION_FAILED          ";
        case CO_E_NOTINITIALIZED              :
            return "CO_E_NOTINITIALIZED              ";
        case CO_E_ALREADYINITIALIZED          :
            return "CO_E_ALREADYINITIALIZED          ";
        case CO_E_CANTDETERMINECLASS          :
            return "CO_E_CANTDETERMINECLASS          ";
        case CO_E_CLASSSTRING                 :
            return "CO_E_CLASSSTRING                 ";
        case CO_E_IIDSTRING                   :
            return "CO_E_IIDSTRING                   ";
        case CO_E_APPNOTFOUND                 :
            return "CO_E_APPNOTFOUND                 ";
        case CO_E_APPSINGLEUSE                :
            return "CO_E_APPSINGLEUSE                ";
        case CO_E_ERRORINAPP                  :
            return "CO_E_ERRORINAPP                  ";
        case CO_E_DLLNOTFOUND                 :
            return "CO_E_DLLNOTFOUND                 ";
        case CO_E_ERRORINDLL                  :
            return "CO_E_ERRORINDLL                  ";
        case CO_E_WRONGOSFORAPP               :
            return "CO_E_WRONGOSFORAPP               ";
        case CO_E_OBJNOTREG                   :
            return "CO_E_OBJNOTREG                   ";
        case CO_E_OBJISREG                    :
            return "CO_E_OBJISREG                    ";
        case CO_E_OBJNOTCONNECTED             :
            return "CO_E_OBJNOTCONNECTED             ";
        case CO_E_APPDIDNTREG                 :
            return "CO_E_APPDIDNTREG                 ";
        case CO_E_RELEASED                    :
            return "CO_E_RELEASED                    ";
        case CO_E_FAILEDTOIMPERSONATE         :
            return "CO_E_FAILEDTOIMPERSONATE         ";
        case CO_E_FAILEDTOGETSECCTX           :
            return "CO_E_FAILEDTOGETSECCTX           ";
        case CO_E_FAILEDTOOPENTHREADTOKEN     :
            return "CO_E_FAILEDTOOPENTHREADTOKEN     ";
        case CO_E_FAILEDTOGETTOKENINFO        :
            return "CO_E_FAILEDTOGETTOKENINFO        ";
        case CO_E_TRUSTEEDOESNTMATCHCLIENT    :
            return "CO_E_TRUSTEEDOESNTMATCHCLIENT    ";
        case CO_E_FAILEDTOQUERYCLIENTBLANKET  :
            return "CO_E_FAILEDTOQUERYCLIENTBLANKET  ";
        case CO_E_FAILEDTOSETDACL             :
            return "CO_E_FAILEDTOSETDACL             ";
        case CO_E_ACCESSCHECKFAILED           :
            return "CO_E_ACCESSCHECKFAILED           ";
        case CO_E_NETACCESSAPIFAILED          :
            return "CO_E_NETACCESSAPIFAILED          ";
        case CO_E_WRONGTRUSTEENAMESYNTAX      :
            return "CO_E_WRONGTRUSTEENAMESYNTAX      ";
        case CO_E_INVALIDSID                  :
            return "CO_E_INVALIDSID                  ";
        case CO_E_CONVERSIONFAILED            :
            return "CO_E_CONVERSIONFAILED            ";
        case CO_E_NOMATCHINGSIDFOUND          :
            return "CO_E_NOMATCHINGSIDFOUND          ";
        case CO_E_LOOKUPACCSIDFAILED          :
            return "CO_E_LOOKUPACCSIDFAILED          ";
        case CO_E_NOMATCHINGNAMEFOUND         :
            return "CO_E_NOMATCHINGNAMEFOUND         ";
        case CO_E_LOOKUPACCNAMEFAILED         :
            return "CO_E_LOOKUPACCNAMEFAILED         ";
        case CO_E_SETSERLHNDLFAILED           :
            return "CO_E_SETSERLHNDLFAILED           ";
        case CO_E_FAILEDTOGETWINDIR           :
            return "CO_E_FAILEDTOGETWINDIR           ";
        case CO_E_PATHTOOLONG                 :
            return "CO_E_PATHTOOLONG                 ";
        case CO_E_FAILEDTOGENUUID             :
            return "CO_E_FAILEDTOGENUUID             ";
        case CO_E_FAILEDTOCREATEFILE          :
            return "CO_E_FAILEDTOCREATEFILE          ";
        case CO_E_FAILEDTOCLOSEHANDLE         :
            return "CO_E_FAILEDTOCLOSEHANDLE         ";
        case CO_E_EXCEEDSYSACLLIMIT           :
            return "CO_E_EXCEEDSYSACLLIMIT           ";
        case CO_E_ACESINWRONGORDER            :
            return "CO_E_ACESINWRONGORDER            ";
        case CO_E_INCOMPATIBLESTREAMVERSION   :
            return "CO_E_INCOMPATIBLESTREAMVERSION   ";
        case CO_E_FAILEDTOOPENPROCESSTOKEN    :
            return "CO_E_FAILEDTOOPENPROCESSTOKEN    ";
        case CO_E_DECODEFAILED                :
            return "CO_E_DECODEFAILED                ";
        case CO_E_ACNOTINITIALIZED            :
            return "CO_E_ACNOTINITIALIZED            ";
        case OLE_S_USEREG                     :
            return "OLE_S_USEREG                     ";
        case OLE_S_STATIC                     :
            return "OLE_S_STATIC                     ";
        case OLE_S_MAC_CLIPFORMAT             :
            return "OLE_S_MAC_CLIPFORMAT             ";
        case DRAGDROP_S_DROP                  :
            return "DRAGDROP_S_DROP                  ";
        case DRAGDROP_S_CANCEL                :
            return "DRAGDROP_S_CANCEL                ";
        case DRAGDROP_S_USEDEFAULTCURSORS     :
            return "DRAGDROP_S_USEDEFAULTCURSORS     ";
        case DATA_S_SAMEFORMATETC             :
            return "DATA_S_SAMEFORMATETC             ";
        case VIEW_S_ALREADY_FROZEN            :
            return "VIEW_S_ALREADY_FROZEN            ";
        case CACHE_S_FORMATETC_NOTSUPPORTED   :
            return "CACHE_S_FORMATETC_NOTSUPPORTED   ";
        case CACHE_S_SAMECACHE                :
            return "CACHE_S_SAMECACHE                ";
        case CACHE_S_SOMECACHES_NOTUPDATED    :
            return "CACHE_S_SOMECACHES_NOTUPDATED    ";
        case OLEOBJ_S_INVALIDVERB             :
            return "OLEOBJ_S_INVALIDVERB             ";
        case OLEOBJ_S_CANNOT_DOVERB_NOW       :
            return "OLEOBJ_S_CANNOT_DOVERB_NOW       ";
        case OLEOBJ_S_INVALIDHWND             :
            return "OLEOBJ_S_INVALIDHWND             ";
        case INPLACE_S_TRUNCATED              :
            return "INPLACE_S_TRUNCATED              ";
        case CONVERT10_S_NO_PRESENTATION      :
            return "CONVERT10_S_NO_PRESENTATION      ";
        case MK_S_REDUCED_TO_SELF             :
            return "MK_S_REDUCED_TO_SELF             ";
        case MK_S_ME                          :
            return "MK_S_ME                          ";
        case MK_S_HIM                         :
            return "MK_S_HIM                         ";
        case MK_S_US                          :
            return "MK_S_US                          ";
        case MK_S_MONIKERALREADYREGISTERED    :
            return "MK_S_MONIKERALREADYREGISTERED    ";
        case CO_E_CLASS_CREATE_FAILED         :
            return "CO_E_CLASS_CREATE_FAILED         ";
        case CO_E_SCM_ERROR                   :
            return "CO_E_SCM_ERROR                   ";
        case CO_E_SCM_RPC_FAILURE             :
            return "CO_E_SCM_RPC_FAILURE             ";
        case CO_E_BAD_PATH                    :
            return "CO_E_BAD_PATH                    ";
        case CO_E_SERVER_EXEC_FAILURE         :
            return "CO_E_SERVER_EXEC_FAILURE         ";
        case CO_E_OBJSRV_RPC_FAILURE          :
            return "CO_E_OBJSRV_RPC_FAILURE          ";
        case MK_E_NO_NORMALIZED               :
            return "MK_E_NO_NORMALIZED               ";
        case CO_E_SERVER_STOPPING             :
            return "CO_E_SERVER_STOPPING             ";
        case MEM_E_INVALID_ROOT               :
            return "MEM_E_INVALID_ROOT               ";
        case MEM_E_INVALID_LINK               :
            return "MEM_E_INVALID_LINK               ";
        case MEM_E_INVALID_SIZE               :
            return "MEM_E_INVALID_SIZE               ";
        case CO_S_NOTALLINTERFACES            :
            return "CO_S_NOTALLINTERFACES            ";
        case DISP_E_UNKNOWNINTERFACE          :
            return "DISP_E_UNKNOWNINTERFACE          ";
        case DISP_E_MEMBERNOTFOUND            :
            return "DISP_E_MEMBERNOTFOUND            ";
        case DISP_E_PARAMNOTFOUND             :
            return "DISP_E_PARAMNOTFOUND             ";
        case DISP_E_TYPEMISMATCH              :
            return "DISP_E_TYPEMISMATCH              ";
        case DISP_E_UNKNOWNNAME               :
            return "DISP_E_UNKNOWNNAME               ";
        case DISP_E_NONAMEDARGS               :
            return "DISP_E_NONAMEDARGS               ";
        case DISP_E_BADVARTYPE                :
            return "DISP_E_BADVARTYPE                ";
        case DISP_E_EXCEPTION                 :
            return "DISP_E_EXCEPTION                 ";
        case DISP_E_OVERFLOW                  :
            return "DISP_E_OVERFLOW                  ";
        case DISP_E_BADINDEX                  :
            return "DISP_E_BADINDEX                  ";
        case DISP_E_UNKNOWNLCID               :
            return "DISP_E_UNKNOWNLCID               ";
        case DISP_E_ARRAYISLOCKED             :
            return "DISP_E_ARRAYISLOCKED             ";
        case DISP_E_BADPARAMCOUNT             :
            return "DISP_E_BADPARAMCOUNT             ";
        case DISP_E_PARAMNOTOPTIONAL          :
            return "DISP_E_PARAMNOTOPTIONAL          ";
        case DISP_E_BADCALLEE                 :
            return "DISP_E_BADCALLEE                 ";
        case DISP_E_NOTACOLLECTION            :
            return "DISP_E_NOTACOLLECTION            ";
        case DISP_E_DIVBYZERO                 :
            return "DISP_E_DIVBYZERO                 ";
        case TYPE_E_BUFFERTOOSMALL            :
            return "TYPE_E_BUFFERTOOSMALL            ";
        case TYPE_E_FIELDNOTFOUND             :
            return "TYPE_E_FIELDNOTFOUND             ";
        case TYPE_E_INVDATAREAD               :
            return "TYPE_E_INVDATAREAD               ";
        case TYPE_E_UNSUPFORMAT               :
            return "TYPE_E_UNSUPFORMAT               ";
        case TYPE_E_REGISTRYACCESS            :
            return "TYPE_E_REGISTRYACCESS            ";
        case TYPE_E_LIBNOTREGISTERED          :
            return "TYPE_E_LIBNOTREGISTERED          ";
        case TYPE_E_UNDEFINEDTYPE             :
            return "TYPE_E_UNDEFINEDTYPE             ";
        case TYPE_E_QUALIFIEDNAMEDISALLOWED   :
            return "TYPE_E_QUALIFIEDNAMEDISALLOWED   ";
        case TYPE_E_INVALIDSTATE              :
            return "TYPE_E_INVALIDSTATE              ";
        case TYPE_E_WRONGTYPEKIND             :
            return "TYPE_E_WRONGTYPEKIND             ";
        case TYPE_E_ELEMENTNOTFOUND           :
            return "TYPE_E_ELEMENTNOTFOUND           ";
        case TYPE_E_AMBIGUOUSNAME             :
            return "TYPE_E_AMBIGUOUSNAME             ";
        case TYPE_E_NAMECONFLICT              :
            return "TYPE_E_NAMECONFLICT              ";
        case TYPE_E_UNKNOWNLCID               :
            return "TYPE_E_UNKNOWNLCID               ";
        case TYPE_E_DLLFUNCTIONNOTFOUND       :
            return "TYPE_E_DLLFUNCTIONNOTFOUND       ";
        case TYPE_E_BADMODULEKIND             :
            return "TYPE_E_BADMODULEKIND             ";
        case TYPE_E_SIZETOOBIG                :
            return "TYPE_E_SIZETOOBIG                ";
        case TYPE_E_DUPLICATEID               :
            return "TYPE_E_DUPLICATEID               ";
        case TYPE_E_INVALIDID                 :
            return "TYPE_E_INVALIDID                 ";
        case TYPE_E_TYPEMISMATCH              :
            return "TYPE_E_TYPEMISMATCH              ";
        case TYPE_E_OUTOFBOUNDS               :
            return "TYPE_E_OUTOFBOUNDS               ";
        case TYPE_E_IOERROR                   :
            return "TYPE_E_IOERROR                   ";
        case TYPE_E_CANTCREATETMPFILE         :
            return "TYPE_E_CANTCREATETMPFILE         ";
        case TYPE_E_CANTLOADLIBRARY           :
            return "TYPE_E_CANTLOADLIBRARY           ";
        case TYPE_E_INCONSISTENTPROPFUNCS     :
            return "TYPE_E_INCONSISTENTPROPFUNCS     ";
        case TYPE_E_CIRCULARTYPE              :
            return "TYPE_E_CIRCULARTYPE              ";
        case STG_E_INVALIDFUNCTION            :
            return "STG_E_INVALIDFUNCTION            ";
        case STG_E_FILENOTFOUND               :
            return "STG_E_FILENOTFOUND               ";
        case STG_E_PATHNOTFOUND               :
            return "STG_E_PATHNOTFOUND               ";
        case STG_E_TOOMANYOPENFILES           :
            return "STG_E_TOOMANYOPENFILES           ";
        case STG_E_ACCESSDENIED               :
            return "STG_E_ACCESSDENIED               ";
        case STG_E_INVALIDHANDLE              :
            return "STG_E_INVALIDHANDLE              ";
        case STG_E_INSUFFICIENTMEMORY         :
            return "STG_E_INSUFFICIENTMEMORY         ";
        case STG_E_INVALIDPOINTER             :
            return "STG_E_INVALIDPOINTER             ";
        case STG_E_NOMOREFILES                :
            return "STG_E_NOMOREFILES                ";
        case STG_E_DISKISWRITEPROTECTED       :
            return "STG_E_DISKISWRITEPROTECTED       ";
        case STG_E_SEEKERROR                  :
            return "STG_E_SEEKERROR                  ";
        case STG_E_WRITEFAULT                 :
            return "STG_E_WRITEFAULT                 ";
        case STG_E_READFAULT                  :
            return "STG_E_READFAULT                  ";
        case STG_E_SHAREVIOLATION             :
            return "STG_E_SHAREVIOLATION             ";
        case STG_E_LOCKVIOLATION              :
            return "STG_E_LOCKVIOLATION              ";
        case STG_E_FILEALREADYEXISTS          :
            return "STG_E_FILEALREADYEXISTS          ";
        case STG_E_INVALIDPARAMETER           :
            return "STG_E_INVALIDPARAMETER           ";
        case STG_E_MEDIUMFULL                 :
            return "STG_E_MEDIUMFULL                 ";
        case STG_E_PROPSETMISMATCHED          :
            return "STG_E_PROPSETMISMATCHED          ";
        case STG_E_ABNORMALAPIEXIT            :
            return "STG_E_ABNORMALAPIEXIT            ";
        case STG_E_INVALIDHEADER              :
            return "STG_E_INVALIDHEADER              ";
        case STG_E_INVALIDNAME                :
            return "STG_E_INVALIDNAME                ";
        case STG_E_UNKNOWN                    :
            return "STG_E_UNKNOWN                    ";
        case STG_E_UNIMPLEMENTEDFUNCTION      :
            return "STG_E_UNIMPLEMENTEDFUNCTION      ";
        case STG_E_INVALIDFLAG                :
            return "STG_E_INVALIDFLAG                ";
        case STG_E_INUSE                      :
            return "STG_E_INUSE                      ";
        case STG_E_NOTCURRENT                 :
            return "STG_E_NOTCURRENT                 ";
        case STG_E_REVERTED                   :
            return "STG_E_REVERTED                   ";
        case STG_E_CANTSAVE                   :
            return "STG_E_CANTSAVE                   ";
        case STG_E_OLDFORMAT                  :
            return "STG_E_OLDFORMAT                  ";
        case STG_E_OLDDLL                     :
            return "STG_E_OLDDLL                     ";
        case STG_E_SHAREREQUIRED              :
            return "STG_E_SHAREREQUIRED              ";
        case STG_E_NOTFILEBASEDSTORAGE        :
            return "STG_E_NOTFILEBASEDSTORAGE        ";
        case STG_E_EXTANTMARSHALLINGS         :
            return "STG_E_EXTANTMARSHALLINGS         ";
        case STG_E_DOCFILECORRUPT             :
            return "STG_E_DOCFILECORRUPT             ";
        case STG_E_BADBASEADDRESS             :
            return "STG_E_BADBASEADDRESS             ";
        case STG_E_INCOMPLETE                 :
            return "STG_E_INCOMPLETE                 ";
        case STG_E_TERMINATED                 :
            return "STG_E_TERMINATED                 ";
        case STG_S_CONVERTED                  :
            return "STG_S_CONVERTED                  ";
        case STG_S_BLOCK                      :
            return "STG_S_BLOCK                      ";
        case STG_S_RETRYNOW                   :
            return "STG_S_RETRYNOW                   ";
        case STG_S_MONITORING                 :
            return "STG_S_MONITORING                 ";
        case STG_S_MULTIPLEOPENS              :
            return "STG_S_MULTIPLEOPENS              ";
        case STG_S_CONSOLIDATIONFAILED        :
            return "STG_S_CONSOLIDATIONFAILED        ";
        case STG_S_CANNOTCONSOLIDATE          :
            return "STG_S_CANNOTCONSOLIDATE          ";
        case RPC_E_CALL_REJECTED              :
            return "RPC_E_CALL_REJECTED              ";
        case RPC_E_CALL_CANCELED              :
            return "RPC_E_CALL_CANCELED              ";
        case RPC_E_CANTPOST_INSENDCALL        :
            return "RPC_E_CANTPOST_INSENDCALL        ";
        case RPC_E_CANTCALLOUT_INASYNCCALL    :
            return "RPC_E_CANTCALLOUT_INASYNCCALL    ";
        case RPC_E_CANTCALLOUT_INEXTERNALCALL :
            return "RPC_E_CANTCALLOUT_INEXTERNALCALL ";
        case RPC_E_CONNECTION_TERMINATED      :
            return "RPC_E_CONNECTION_TERMINATED      ";
        case RPC_E_SERVER_DIED                :
            return "RPC_E_SERVER_DIED                ";
        case RPC_E_CLIENT_DIED                :
            return "RPC_E_CLIENT_DIED                ";
        case RPC_E_INVALID_DATAPACKET         :
            return "RPC_E_INVALID_DATAPACKET         ";
        case RPC_E_CANTTRANSMIT_CALL          :
            return "RPC_E_CANTTRANSMIT_CALL          ";
        case RPC_E_CLIENT_CANTMARSHAL_DATA    :
            return "RPC_E_CLIENT_CANTMARSHAL_DATA    ";
        case RPC_E_CLIENT_CANTUNMARSHAL_DATA  :
            return "RPC_E_CLIENT_CANTUNMARSHAL_DATA  ";
        case RPC_E_SERVER_CANTMARSHAL_DATA    :
            return "RPC_E_SERVER_CANTMARSHAL_DATA    ";
        case RPC_E_SERVER_CANTUNMARSHAL_DATA  :
            return "RPC_E_SERVER_CANTUNMARSHAL_DATA  ";
        case RPC_E_INVALID_DATA               :
            return "RPC_E_INVALID_DATA               ";
        case RPC_E_INVALID_PARAMETER          :
            return "RPC_E_INVALID_PARAMETER          ";
        case RPC_E_CANTCALLOUT_AGAIN          :
            return "RPC_E_CANTCALLOUT_AGAIN          ";
        case RPC_E_SERVER_DIED_DNE            :
            return "RPC_E_SERVER_DIED_DNE            ";
        case RPC_E_SYS_CALL_FAILED            :
            return "RPC_E_SYS_CALL_FAILED            ";
        case RPC_E_OUT_OF_RESOURCES           :
            return "RPC_E_OUT_OF_RESOURCES           ";
        case RPC_E_ATTEMPTED_MULTITHREAD      :
            return "RPC_E_ATTEMPTED_MULTITHREAD      ";
        case RPC_E_NOT_REGISTERED             :
            return "RPC_E_NOT_REGISTERED             ";
        case RPC_E_FAULT                      :
            return "RPC_E_FAULT                      ";
        case RPC_E_SERVERFAULT                :
            return "RPC_E_SERVERFAULT                ";
        case RPC_E_CHANGED_MODE               :
            return "RPC_E_CHANGED_MODE               ";
        case RPC_E_INVALIDMETHOD              :
            return "RPC_E_INVALIDMETHOD              ";
        case RPC_E_DISCONNECTED               :
            return "RPC_E_DISCONNECTED               ";
        case RPC_E_RETRY                      :
            return "RPC_E_RETRY                      ";
        case RPC_E_SERVERCALL_RETRYLATER      :
            return "RPC_E_SERVERCALL_RETRYLATER      ";
        case RPC_E_SERVERCALL_REJECTED        :
            return "RPC_E_SERVERCALL_REJECTED        ";
        case RPC_E_INVALID_CALLDATA           :
            return "RPC_E_INVALID_CALLDATA           ";
        case RPC_E_CANTCALLOUT_ININPUTSYNCCALL :
            return "RPC_E_CANTCALLOUT_ININPUTSYNCCALL ";
        case RPC_E_WRONG_THREAD               :
            return "RPC_E_WRONG_THREAD               ";
        case RPC_E_THREAD_NOT_INIT            :
            return "RPC_E_THREAD_NOT_INIT            ";
        case RPC_E_VERSION_MISMATCH           :
            return "RPC_E_VERSION_MISMATCH           ";
        case RPC_E_INVALID_HEADER             :
            return "RPC_E_INVALID_HEADER             ";
        case RPC_E_INVALID_EXTENSION          :
            return "RPC_E_INVALID_EXTENSION          ";
        case RPC_E_INVALID_IPID               :
            return "RPC_E_INVALID_IPID               ";
        case RPC_E_INVALID_OBJECT             :
            return "RPC_E_INVALID_OBJECT             ";
        case RPC_S_CALLPENDING                :
            return "RPC_S_CALLPENDING                ";
        case RPC_S_WAITONTIMER                :
            return "RPC_S_WAITONTIMER                ";
        case RPC_E_CALL_COMPLETE              :
            return "RPC_E_CALL_COMPLETE              ";
        case RPC_E_UNSECURE_CALL              :
            return "RPC_E_UNSECURE_CALL              ";
        case RPC_E_TOO_LATE                   :
            return "RPC_E_TOO_LATE                   ";
        case RPC_E_NO_GOOD_SECURITY_PACKAGES  :
            return "RPC_E_NO_GOOD_SECURITY_PACKAGES  ";
        case RPC_E_ACCESS_DENIED              :
            return "RPC_E_ACCESS_DENIED              ";
        case RPC_E_REMOTE_DISABLED            :
            return "RPC_E_REMOTE_DISABLED            ";
        case RPC_E_INVALID_OBJREF             :
            return "RPC_E_INVALID_OBJREF             ";
        case RPC_E_NO_CONTEXT                 :
            return "RPC_E_NO_CONTEXT                 ";
        case RPC_E_TIMEOUT                    :
            return "RPC_E_TIMEOUT                    ";
        case RPC_E_NO_SYNC                    :
            return "RPC_E_NO_SYNC                    ";
        case RPC_E_UNEXPECTED                 :
            return "RPC_E_UNEXPECTED                 ";
        case NTE_BAD_UID                      :
            return "NTE_BAD_UID                      ";
        case NTE_BAD_HASH                     :
            return "NTE_BAD_HASH                     ";
    //case NTE_BAD_HASH                     :
    //return "NTE_BAD_HASH                     ";
        case NTE_BAD_KEY                      :
            return "NTE_BAD_KEY                      ";
        case NTE_BAD_LEN                      :
            return "NTE_BAD_LEN                      ";
        case NTE_BAD_DATA                     :
            return "NTE_BAD_DATA                     ";
        case NTE_BAD_SIGNATURE                :
            return "NTE_BAD_SIGNATURE                ";
        case NTE_BAD_VER                      :
            return "NTE_BAD_VER                      ";
        case NTE_BAD_ALGID                    :
            return "NTE_BAD_ALGID                    ";
        case NTE_BAD_FLAGS                    :
            return "NTE_BAD_FLAGS                    ";
        case NTE_BAD_TYPE                     :
            return "NTE_BAD_TYPE                     ";
        case NTE_BAD_KEY_STATE                :
            return "NTE_BAD_KEY_STATE                ";
        case NTE_BAD_HASH_STATE               :
            return "NTE_BAD_HASH_STATE               ";
        case NTE_NO_KEY                       :
            return "NTE_NO_KEY                       ";
        case NTE_NO_MEMORY                    :
            return "NTE_NO_MEMORY                    ";
        case NTE_EXISTS                       :
            return "NTE_EXISTS                       ";
        case NTE_PERM                         :
            return "NTE_PERM                         ";
        case NTE_NOT_FOUND                    :
            return "NTE_NOT_FOUND                    ";
        case NTE_DOUBLE_ENCRYPT               :
            return "NTE_DOUBLE_ENCRYPT               ";
        case NTE_BAD_PROVIDER                 :
            return "NTE_BAD_PROVIDER                 ";
        case NTE_BAD_PROV_TYPE                :
            return "NTE_BAD_PROV_TYPE                ";
        case NTE_BAD_PUBLIC_KEY               :
            return "NTE_BAD_PUBLIC_KEY               ";
        case NTE_BAD_KEYSET                   :
            return "NTE_BAD_KEYSET                   ";
        case NTE_PROV_TYPE_NOT_DEF            :
            return "NTE_PROV_TYPE_NOT_DEF            ";
        case NTE_PROV_TYPE_ENTRY_BAD          :
            return "NTE_PROV_TYPE_ENTRY_BAD          ";
        case NTE_KEYSET_NOT_DEF               :
            return "NTE_KEYSET_NOT_DEF               ";
        case NTE_KEYSET_ENTRY_BAD             :
            return "NTE_KEYSET_ENTRY_BAD             ";
        case NTE_PROV_TYPE_NO_MATCH           :
            return "NTE_PROV_TYPE_NO_MATCH           ";
        case NTE_SIGNATURE_FILE_BAD           :
            return "NTE_SIGNATURE_FILE_BAD           ";
        case NTE_PROVIDER_DLL_FAIL            :
            return "NTE_PROVIDER_DLL_FAIL            ";
        case NTE_PROV_DLL_NOT_FOUND           :
            return "NTE_PROV_DLL_NOT_FOUND           ";
        case NTE_BAD_KEYSET_PARAM             :
            return "NTE_BAD_KEYSET_PARAM             ";
        case NTE_FAIL                         :
            return "NTE_FAIL                         ";
        case NTE_SYS_ERR                      :
            return "NTE_SYS_ERR                      ";
        case CRYPT_E_MSG_ERROR                :
            return "CRYPT_E_MSG_ERROR                ";
        case CRYPT_E_UNKNOWN_ALGO             :
            return "CRYPT_E_UNKNOWN_ALGO             ";
        case CRYPT_E_OID_FORMAT               :
            return "CRYPT_E_OID_FORMAT               ";
        case CRYPT_E_INVALID_MSG_TYPE         :
            return "CRYPT_E_INVALID_MSG_TYPE         ";
        case CRYPT_E_UNEXPECTED_ENCODING      :
            return "CRYPT_E_UNEXPECTED_ENCODING      ";
        case CRYPT_E_AUTH_ATTR_MISSING        :
            return "CRYPT_E_AUTH_ATTR_MISSING        ";
        case CRYPT_E_HASH_VALUE               :
            return "CRYPT_E_HASH_VALUE               ";
        case CRYPT_E_INVALID_INDEX            :
            return "CRYPT_E_INVALID_INDEX            ";
        case CRYPT_E_ALREADY_DECRYPTED        :
            return "CRYPT_E_ALREADY_DECRYPTED        ";
        case CRYPT_E_NOT_DECRYPTED            :
            return "CRYPT_E_NOT_DECRYPTED            ";
        case CRYPT_E_RECIPIENT_NOT_FOUND      :
            return "CRYPT_E_RECIPIENT_NOT_FOUND      ";
        case CRYPT_E_CONTROL_TYPE             :
            return "CRYPT_E_CONTROL_TYPE             ";
        case CRYPT_E_ISSUER_SERIALNUMBER      :
            return "CRYPT_E_ISSUER_SERIALNUMBER      ";
        case CRYPT_E_SIGNER_NOT_FOUND         :
            return "CRYPT_E_SIGNER_NOT_FOUND         ";
        case CRYPT_E_ATTRIBUTES_MISSING       :
            return "CRYPT_E_ATTRIBUTES_MISSING       ";
        case CRYPT_E_STREAM_MSG_NOT_READY     :
            return "CRYPT_E_STREAM_MSG_NOT_READY     ";
        case CRYPT_E_STREAM_INSUFFICIENT_DATA :
            return "CRYPT_E_STREAM_INSUFFICIENT_DATA ";
        case CRYPT_E_BAD_LEN                  :
            return "CRYPT_E_BAD_LEN                  ";
        case CRYPT_E_BAD_ENCODE               :
            return "CRYPT_E_BAD_ENCODE               ";
        case CRYPT_E_FILE_ERROR               :
            return "CRYPT_E_FILE_ERROR               ";
        case CRYPT_E_NOT_FOUND                :
            return "CRYPT_E_NOT_FOUND                ";
        case CRYPT_E_EXISTS                   :
            return "CRYPT_E_EXISTS                   ";
        case CRYPT_E_NO_PROVIDER              :
            return "CRYPT_E_NO_PROVIDER              ";
        case CRYPT_E_SELF_SIGNED              :
            return "CRYPT_E_SELF_SIGNED              ";
        case CRYPT_E_DELETED_PREV             :
            return "CRYPT_E_DELETED_PREV             ";
        case CRYPT_E_NO_MATCH                 :
            return "CRYPT_E_NO_MATCH                 ";
        case CRYPT_E_UNEXPECTED_MSG_TYPE      :
            return "CRYPT_E_UNEXPECTED_MSG_TYPE      ";
        case CRYPT_E_NO_KEY_PROPERTY          :
            return "CRYPT_E_NO_KEY_PROPERTY          ";
        case CRYPT_E_NO_DECRYPT_CERT          :
            return "CRYPT_E_NO_DECRYPT_CERT          ";
        case CRYPT_E_BAD_MSG                  :
            return "CRYPT_E_BAD_MSG                  ";
        case CRYPT_E_NO_SIGNER                :
            return "CRYPT_E_NO_SIGNER                ";
        case CRYPT_E_PENDING_CLOSE            :
            return "CRYPT_E_PENDING_CLOSE            ";
        case CRYPT_E_REVOKED                  :
            return "CRYPT_E_REVOKED                  ";
        case CRYPT_E_NO_REVOCATION_DLL        :
            return "CRYPT_E_NO_REVOCATION_DLL        ";
        case CRYPT_E_NO_REVOCATION_CHECK      :
            return "CRYPT_E_NO_REVOCATION_CHECK      ";
        case CRYPT_E_REVOCATION_OFFLINE       :
            return "CRYPT_E_REVOCATION_OFFLINE       ";
        case CRYPT_E_NOT_IN_REVOCATION_DATABASE :
            return "CRYPT_E_NOT_IN_REVOCATION_DATABASE ";
        case CRYPT_E_INVALID_NUMERIC_STRING   :
            return "CRYPT_E_INVALID_NUMERIC_STRING   ";
        case CRYPT_E_INVALID_PRINTABLE_STRING :
            return "CRYPT_E_INVALID_PRINTABLE_STRING ";
        case CRYPT_E_INVALID_IA5_STRING       :
            return "CRYPT_E_INVALID_IA5_STRING       ";
        case CRYPT_E_INVALID_X500_STRING      :
            return "CRYPT_E_INVALID_X500_STRING      ";
        case CRYPT_E_NOT_CHAR_STRING          :
            return "CRYPT_E_NOT_CHAR_STRING          ";
        case CRYPT_E_FILERESIZED              :
            return "CRYPT_E_FILERESIZED              ";
        case CRYPT_E_SECURITY_SETTINGS        :
            return "CRYPT_E_SECURITY_SETTINGS        ";
        case CRYPT_E_NO_VERIFY_USAGE_DLL      :
            return "CRYPT_E_NO_VERIFY_USAGE_DLL      ";
        case CRYPT_E_NO_VERIFY_USAGE_CHECK    :
            return "CRYPT_E_NO_VERIFY_USAGE_CHECK    ";
        case CRYPT_E_VERIFY_USAGE_OFFLINE     :
            return "CRYPT_E_VERIFY_USAGE_OFFLINE     ";
        case CRYPT_E_NOT_IN_CTL               :
            return "CRYPT_E_NOT_IN_CTL               ";
        case CRYPT_E_NO_TRUSTED_SIGNER        :
            return "CRYPT_E_NO_TRUSTED_SIGNER        ";
        case CRYPT_E_OSS_ERROR                :
            return "CRYPT_E_OSS_ERROR                ";
        case CERTSRV_E_BAD_REQUESTSUBJECT     :
            return "CERTSRV_E_BAD_REQUESTSUBJECT     ";
        case CERTSRV_E_NO_REQUEST             :
            return "CERTSRV_E_NO_REQUEST             ";
        case CERTSRV_E_BAD_REQUESTSTATUS      :
            return "CERTSRV_E_BAD_REQUESTSTATUS      ";
        case CERTSRV_E_PROPERTY_EMPTY         :
            return "CERTSRV_E_PROPERTY_EMPTY         ";
    //case CERTDB_E_JET_ERROR               :
    //return "CERTDB_E_JET_ERROR               ";
        case TRUST_E_SYSTEM_ERROR             :
            return "TRUST_E_SYSTEM_ERROR             ";
        case TRUST_E_NO_SIGNER_CERT           :
            return "TRUST_E_NO_SIGNER_CERT           ";
        case TRUST_E_COUNTER_SIGNER           :
            return "TRUST_E_COUNTER_SIGNER           ";
        case TRUST_E_CERT_SIGNATURE           :
            return "TRUST_E_CERT_SIGNATURE           ";
        case TRUST_E_TIME_STAMP               :
            return "TRUST_E_TIME_STAMP               ";
        case TRUST_E_BAD_DIGEST               :
            return "TRUST_E_BAD_DIGEST               ";
        case TRUST_E_BASIC_CONSTRAINTS        :
            return "TRUST_E_BASIC_CONSTRAINTS        ";
        case TRUST_E_FINANCIAL_CRITERIA       :
            return "TRUST_E_FINANCIAL_CRITERIA       ";
        case TRUST_E_PROVIDER_UNKNOWN         :
            return "TRUST_E_PROVIDER_UNKNOWN         ";
        case TRUST_E_ACTION_UNKNOWN           :
            return "TRUST_E_ACTION_UNKNOWN           ";
        case TRUST_E_SUBJECT_FORM_UNKNOWN     :
            return "TRUST_E_SUBJECT_FORM_UNKNOWN     ";
        case TRUST_E_SUBJECT_NOT_TRUSTED      :
            return "TRUST_E_SUBJECT_NOT_TRUSTED      ";
        case DIGSIG_E_ENCODE                  :
            return "DIGSIG_E_ENCODE                  ";
        case DIGSIG_E_DECODE                  :
            return "DIGSIG_E_DECODE                  ";
        case DIGSIG_E_EXTENSIBILITY           :
            return "DIGSIG_E_EXTENSIBILITY           ";
        case DIGSIG_E_CRYPTO                  :
            return "DIGSIG_E_CRYPTO                  ";
        case PERSIST_E_SIZEDEFINITE           :
            return "PERSIST_E_SIZEDEFINITE           ";
        case PERSIST_E_SIZEINDEFINITE         :
            return "PERSIST_E_SIZEINDEFINITE         ";
        case PERSIST_E_NOTSELFSIZING          :
            return "PERSIST_E_NOTSELFSIZING          ";
        case TRUST_E_NOSIGNATURE              :
            return "TRUST_E_NOSIGNATURE              ";
        case CERT_E_EXPIRED                   :
            return "CERT_E_EXPIRED                   ";
        case CERT_E_VALIDITYPERIODNESTING     :
            return "CERT_E_VALIDITYPERIODNESTING     ";
        case CERT_E_ROLE                      :
            return "CERT_E_ROLE                      ";
        case CERT_E_PATHLENCONST              :
            return "CERT_E_PATHLENCONST              ";
        case CERT_E_CRITICAL                  :
            return "CERT_E_CRITICAL                  ";
        case CERT_E_PURPOSE                   :
            return "CERT_E_PURPOSE                   ";
        case CERT_E_ISSUERCHAINING            :
            return "CERT_E_ISSUERCHAINING            ";
        case CERT_E_MALFORMED                 :
            return "CERT_E_MALFORMED                 ";
        case CERT_E_UNTRUSTEDROOT             :
            return "CERT_E_UNTRUSTEDROOT             ";
        case CERT_E_CHAINING                  :
            return "CERT_E_CHAINING                  ";
        case TRUST_E_FAIL                     :
            return "TRUST_E_FAIL                     ";
        case CERT_E_REVOKED                   :
            return "CERT_E_REVOKED                   ";
        case CERT_E_UNTRUSTEDTESTROOT         :
            return "CERT_E_UNTRUSTEDTESTROOT         ";
        case CERT_E_REVOCATION_FAILURE        :
            return "CERT_E_REVOCATION_FAILURE        ";
        case CERT_E_CN_NO_MATCH               :
            return "CERT_E_CN_NO_MATCH               ";
        case CERT_E_WRONG_USAGE               :
            return "CERT_E_WRONG_USAGE               ";
        case SPAPI_E_EXPECTED_SECTION_NAME    :
            return "SPAPI_E_EXPECTED_SECTION_NAME    ";
        case SPAPI_E_BAD_SECTION_NAME_LINE    :
            return "SPAPI_E_BAD_SECTION_NAME_LINE    ";
        case SPAPI_E_SECTION_NAME_TOO_LONG    :
            return "SPAPI_E_SECTION_NAME_TOO_LONG    ";
        case SPAPI_E_GENERAL_SYNTAX           :
            return "SPAPI_E_GENERAL_SYNTAX           ";
        case SPAPI_E_WRONG_INF_STYLE          :
            return "SPAPI_E_WRONG_INF_STYLE          ";
        case SPAPI_E_SECTION_NOT_FOUND        :
            return "SPAPI_E_SECTION_NOT_FOUND        ";
        case SPAPI_E_LINE_NOT_FOUND           :
            return "SPAPI_E_LINE_NOT_FOUND           ";
        case SPAPI_E_NO_ASSOCIATED_CLASS      :
            return "SPAPI_E_NO_ASSOCIATED_CLASS      ";
        case SPAPI_E_CLASS_MISMATCH           :
            return "SPAPI_E_CLASS_MISMATCH           ";
        case SPAPI_E_DUPLICATE_FOUND          :
            return "SPAPI_E_DUPLICATE_FOUND          ";
        case SPAPI_E_NO_DRIVER_SELECTED       :
            return "SPAPI_E_NO_DRIVER_SELECTED       ";
        case SPAPI_E_KEY_DOES_NOT_EXIST       :
            return "SPAPI_E_KEY_DOES_NOT_EXIST       ";
        case SPAPI_E_INVALID_DEVINST_NAME     :
            return "SPAPI_E_INVALID_DEVINST_NAME     ";
        case SPAPI_E_INVALID_CLASS            :
            return "SPAPI_E_INVALID_CLASS            ";
        case SPAPI_E_DEVINST_ALREADY_EXISTS   :
            return "SPAPI_E_DEVINST_ALREADY_EXISTS   ";
        case SPAPI_E_DEVINFO_NOT_REGISTERED   :
            return "SPAPI_E_DEVINFO_NOT_REGISTERED   ";
        case SPAPI_E_INVALID_REG_PROPERTY     :
            return "SPAPI_E_INVALID_REG_PROPERTY     ";
        case SPAPI_E_NO_INF                   :
            return "SPAPI_E_NO_INF                   ";
        case SPAPI_E_NO_SUCH_DEVINST          :
            return "SPAPI_E_NO_SUCH_DEVINST          ";
        case SPAPI_E_CANT_LOAD_CLASS_ICON     :
            return "SPAPI_E_CANT_LOAD_CLASS_ICON     ";
        case SPAPI_E_INVALID_CLASS_INSTALLER  :
            return "SPAPI_E_INVALID_CLASS_INSTALLER  ";
        case SPAPI_E_DI_DO_DEFAULT            :
            return "SPAPI_E_DI_DO_DEFAULT            ";
        case SPAPI_E_DI_NOFILECOPY            :
            return "SPAPI_E_DI_NOFILECOPY            ";
        case SPAPI_E_INVALID_HWPROFILE        :
            return "SPAPI_E_INVALID_HWPROFILE        ";
        case SPAPI_E_NO_DEVICE_SELECTED       :
            return "SPAPI_E_NO_DEVICE_SELECTED       ";
        case SPAPI_E_DEVINFO_LIST_LOCKED      :
            return "SPAPI_E_DEVINFO_LIST_LOCKED      ";
        case SPAPI_E_DEVINFO_DATA_LOCKED      :
            return "SPAPI_E_DEVINFO_DATA_LOCKED      ";
        case SPAPI_E_DI_BAD_PATH              :
            return "SPAPI_E_DI_BAD_PATH              ";
        case SPAPI_E_NO_CLASSINSTALL_PARAMS   :
            return "SPAPI_E_NO_CLASSINSTALL_PARAMS   ";
        case SPAPI_E_FILEQUEUE_LOCKED         :
            return "SPAPI_E_FILEQUEUE_LOCKED         ";
        case SPAPI_E_BAD_SERVICE_INSTALLSECT  :
            return "SPAPI_E_BAD_SERVICE_INSTALLSECT  ";
        case SPAPI_E_NO_CLASS_DRIVER_LIST     :
            return "SPAPI_E_NO_CLASS_DRIVER_LIST     ";
        case SPAPI_E_NO_ASSOCIATED_SERVICE    :
            return "SPAPI_E_NO_ASSOCIATED_SERVICE    ";
        case SPAPI_E_NO_DEFAULT_DEVICE_INTERFACE :
            return "SPAPI_E_NO_DEFAULT_DEVICE_INTERFACE ";
        case SPAPI_E_DEVICE_INTERFACE_ACTIVE  :
            return "SPAPI_E_DEVICE_INTERFACE_ACTIVE  ";
        case SPAPI_E_DEVICE_INTERFACE_REMOVED :
            return "SPAPI_E_DEVICE_INTERFACE_REMOVED ";
        case SPAPI_E_BAD_INTERFACE_INSTALLSECT :
            return "SPAPI_E_BAD_INTERFACE_INSTALLSECT ";
        case SPAPI_E_NO_SUCH_INTERFACE_CLASS  :
            return "SPAPI_E_NO_SUCH_INTERFACE_CLASS  ";
        case SPAPI_E_INVALID_REFERENCE_STRING :
            return "SPAPI_E_INVALID_REFERENCE_STRING ";
        case SPAPI_E_INVALID_MACHINENAME      :
            return "SPAPI_E_INVALID_MACHINENAME      ";
        case SPAPI_E_REMOTE_COMM_FAILURE      :
            return "SPAPI_E_REMOTE_COMM_FAILURE      ";
        case SPAPI_E_MACHINE_UNAVAILABLE      :
            return "SPAPI_E_MACHINE_UNAVAILABLE      ";
        case SPAPI_E_NO_CONFIGMGR_SERVICES    :
            return "SPAPI_E_NO_CONFIGMGR_SERVICES    ";
        case SPAPI_E_INVALID_PROPPAGE_PROVIDER :
            return "SPAPI_E_INVALID_PROPPAGE_PROVIDER ";
        case SPAPI_E_NO_SUCH_DEVICE_INTERFACE :
            return "SPAPI_E_NO_SUCH_DEVICE_INTERFACE ";
        case SPAPI_E_DI_POSTPROCESSING_REQUIRED :
            return "SPAPI_E_DI_POSTPROCESSING_REQUIRED ";
        case SPAPI_E_INVALID_COINSTALLER      :
            return "SPAPI_E_INVALID_COINSTALLER      ";
        case SPAPI_E_NO_COMPAT_DRIVERS        :
            return "SPAPI_E_NO_COMPAT_DRIVERS        ";
        case SPAPI_E_NO_DEVICE_ICON           :
            return "SPAPI_E_NO_DEVICE_ICON           ";
        case SPAPI_E_INVALID_INF_LOGCONFIG    :
            return "SPAPI_E_INVALID_INF_LOGCONFIG    ";
        case SPAPI_E_DI_DONT_INSTALL          :
            return "SPAPI_E_DI_DONT_INSTALL          ";
        case SPAPI_E_INVALID_FILTER_DRIVER    :
            return "SPAPI_E_INVALID_FILTER_DRIVER    ";
        case SPAPI_E_ERROR_NOT_INSTALLED      :
            return "SPAPI_E_ERROR_NOT_INSTALLED      ";

        default:
            static char buff[1000];
            sprintf(buff, "Unrecognized error value: %08X\0", error);

            return buff;
    }
}

// Global system parameters we want to modify during our run
static int iMouseTrails;
static bool bCursorShadowOn,bMouseVanish;

void set_global_parameters() {
  // turn off mousetrails and cursor shadow and mouse cursor vanish
  // cursor shadow causes cursor blink and reduced frame rate due to lack of driver support for
  // cursor alpha blending

  // this is a win2k/xp only param, could use GetVersionEx to do it just for win2k,
  // but since it is just a param it will just cause a silent error on other OS's
  // that should be ok
  SystemParametersInfo(SPI_GETCURSORSHADOW,NULL,&bCursorShadowOn,NULL);
  SystemParametersInfo(SPI_SETCURSORSHADOW,NULL,(PVOID)false,NULL);

  SystemParametersInfo(SPI_GETMOUSETRAILS,NULL,&iMouseTrails,NULL);
  SystemParametersInfo(SPI_SETMOUSETRAILS,NULL,(PVOID)0,NULL);

  // this is ME/XP only feature
  SystemParametersInfo(SPI_GETMOUSEVANISH,NULL,&bMouseVanish,NULL);
  SystemParametersInfo(SPI_SETMOUSEVANISH,NULL,(PVOID)false,NULL);
}

void restore_global_parameters() {
  SystemParametersInfo(SPI_SETCURSORSHADOW,NULL,(PVOID)bCursorShadowOn,NULL);
  SystemParametersInfo(SPI_SETMOUSETRAILS,NULL,(PVOID)iMouseTrails,NULL);
  SystemParametersInfo(SPI_SETMOUSEVANISH,NULL,(PVOID)bMouseVanish,NULL);
}

