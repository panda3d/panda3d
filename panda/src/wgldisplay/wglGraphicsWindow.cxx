// Filename: wglGraphicsWindow.cxx
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
//#include <throw_event.h>
//#include <eventQueue.h>
#include <glGraphicsStateGuardian.h>
#include <errno.h>
#include <time.h>
#include <mmsystem.h>
#include <pStatTimer.h>
#include <ddraw.h>
#include <tchar.h>
#include <map>

#define WGL_WGLEXT_PROTOTYPES
#include "wglext.h"

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle wglGraphicsWindow::_type_handle;

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

LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam,LPARAM lparam);

void PrintErrorMessage(DWORD msgID) {
   LPTSTR pMessageBuffer;

   if(msgID==LAST_ERROR)
     msgID=GetLastError();

   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                 NULL,msgID,  
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language
                 (LPTSTR) &pMessageBuffer,  // the weird ptrptr->ptr cast is intentional, see FORMAT_MESSAGE_ALLOCATE_BUFFER
                 1024, NULL);
   MessageBox(GetDesktopWindow(),pMessageBuffer,_T(ERRORBOX_TITLE),MB_OK);
   wgldisplay_cat.fatal() << "System error msg: " << pMessageBuffer << endl;
   LocalFree( pMessageBuffer ); 
}

// fn exists so AtExitFn can call it without refcntr blowing up since its !=0
void wglGraphicsWindow::DestroyMe(bool bAtExitFnCalled) {

  _exiting_window = true;  // needed before DestroyWindow call

  if(_visual!=NULL) {
      free(_visual);
      _visual = NULL;
  }

  // several GL drivers (voodoo,ATI, not nvidia) crash if we call these wgl deletion routines from
  // an atexit() fn.  Possible that GL has already unloaded itself.  So we just wont call them for now
  // for that case, we're exiting the app anyway.
  if(!bAtExitFnCalled) {
      // to do gl releases, we need to have the context be current
      if((_hdc!=NULL)&&(_context!=NULL)) {
          // need to bypass make_current() since it checks _window_inactive which we need to ignore
          HGLRC current_context = wglGetCurrentContext();
          HDC current_dc = wglGetCurrentDC();

          if ((current_context != _context) || (current_dc != _hdc)) {
              if(!wglMakeCurrent(_hdc, _context)) {
                  PrintErrorMessage(LAST_ERROR);
              }
          }
          report_errors();
      }

      if(gl_show_fps_meter)
        glDeleteLists(FONT_BITMAP_OGLDISPLAYLISTNUM, 128);

      report_errors();

      // implicitly calls gsg destructors which release GL objects (textures, display lists, etc)
      release_gsg();

      report_errors();
      // cant report errors after we set cur context to NULL

      HGLRC curcxt=wglGetCurrentContext();
      if(curcxt!=NULL) 
        unmake_current();

      if(_context!=NULL) {
          wglDeleteContext(_context);
          _context = NULL; 
      }
  }

  if(_hdc!=NULL) {
    ReleaseDC(_mwindow,_hdc);
    _hdc = NULL;
  }

  if((_hOldForegroundWindow!=NULL) && (_mwindow==GetForegroundWindow())) {
      SetForegroundWindow(_hOldForegroundWindow);
  }

  if(_mwindow!=NULL) {
    DestroyWindow(_mwindow);
    hwnd_pandawin_map.erase(_mwindow);
    _mwindow = NULL;
  }

  if(_pCurrent_display_settings!=NULL) {
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
wglGraphicsWindow::~wglGraphicsWindow(void) {
   close_window();
}

void DestroyAllWindows(bool bAtExitFnCalled) {
   // need to go through all windows in map var and delete them
   while(!hwnd_pandawin_map.empty()) {
     // cant use a for loop cause DestroyMe erases things out from under us, so iterator is invalid
     HWND_PANDAWIN_MAP::iterator pwin = hwnd_pandawin_map.begin();
     if((*pwin).second != NULL) 
         (*pwin).second->DestroyMe(bAtExitFnCalled);
   }
}

void AtExitFn() {
#ifdef _DEBUG
    wgldisplay_cat.spam() << "AtExitFn called\n";
#endif
  
     DestroyAllWindows(true);
}

// spare me the trouble of linking with dxguid.lib or defining ALL the dx guids in this .obj by #defining INITGUID
#define MY_DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
        EXTERN_C const GUID DECLSPEC_SELECTANY name \
                = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
MY_DEFINE_GUID( IID_IDirectDraw2, 0xB3A6F3E0,0x2B43,0x11CF,0xA2,0xDE,0x00,0xAA,0x00,0xB9,0x33,0x56 );

////////////////////////////////////////////////////////////////////
//  Function: GetAvailVidMem
//  Description: Uses DDraw to get available video memory
////////////////////////////////////////////////////////////////////
static DWORD GetAvailVidMem(void) {

    LPDIRECTDRAW2 pDD2;
    LPDIRECTDRAW pDD;
    HRESULT hr;

    typedef HRESULT (WINAPI *DIRECTDRAWCREATEPROC)(GUID FAR *lpGUID,LPDIRECTDRAW FAR *lplpDD,IUnknown FAR *pUnkOuter); 
    DIRECTDRAWCREATEPROC pfnDDCreate=NULL;

    HINSTANCE DDHinst = LoadLibrary( "ddraw.dll" );
    if(DDHinst == 0) {
        wgldisplay_cat.fatal() << "LoadLibrary() can't find DDRAW.DLL!" << endl;
        exit(1);
    }

    pfnDDCreate = (DIRECTDRAWCREATEPROC) GetProcAddress( DDHinst, "DirectDrawCreate" );

    // just use DX5 DD interface, since that's the minimum ver we need
    if(NULL == pfnDDCreate) {
        wgldisplay_cat.fatal() << "GetProcAddress failed on DirectDrawCreate\n";
        exit(1);
    }

    // Create the Direct Draw Object
    hr = (*pfnDDCreate)((GUID *)DDCREATE_HARDWAREONLY, &pDD, NULL);
    if(hr != DD_OK) {
        wgldisplay_cat.fatal()
        << "DirectDrawCreate failed : result = " << (void*)hr << endl;
        exit(1);
    }

    FreeLibrary(DDHinst);    //undo LoadLib above, decrement ddrawl.dll refcnt (after DDrawCreate, since dont want to unload/reload)

    // need DDraw2 interface for GetAvailVidMem
    hr = pDD->QueryInterface(IID_IDirectDraw2, (LPVOID *)&pDD2); 
    if(hr != DD_OK) {
        wgldisplay_cat.fatal() << "DDraw QueryInterface failed : result = " << (void*)hr << endl;
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
    ZeroMemory(&ddsCaps,sizeof(DDSCAPS2));
    ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY; //set internally by DX anyway, dont think this any different than 0x0

    if(FAILED(  hr = pDD2->GetAvailableVidMem(&ddsCaps,&dwTotal,&dwFree))) {
        wgldisplay_cat.fatal() << "GetAvailableVidMem failed : result = " << (void*)hr << endl;
        exit(1);
    }

#ifdef _DEBUG
    wgldisplay_cat.debug() << "before FullScreen switch: GetAvailableVidMem returns Total: " << dwTotal/1000000.0 << "  Free: " << dwFree/1000000.0 << endl;
#endif

    pDD2->Release();  // bye-bye ddraw

    return dwFree;
}

////////////////////////////////////////////////////////////////////
//     Function: config
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::config(void) {
    
    GraphicsWindow::config();

    HINSTANCE hinstance = GetModuleHandle(NULL);
    HWND hDesktopWindow = GetDesktopWindow();

    global_wglwinptr = this;  // need this until we get an HWND from CreateWindow

    _exiting_window = false;
    _return_control_to_app = false;
    _PandaPausedTimer = NULL;
    _mouse_input_enabled = false;
    _mouse_motion_enabled = false;
    _mouse_passive_motion_enabled = false;
    _mouse_entry_enabled = false;
    _entry_state = -1;
    _visual = NULL;
    _context = NULL;
    _hdc = NULL;
    _window_inactive = false;
    _pCurrent_display_settings = NULL;
    _mwindow = NULL;
    _gsg = NULL;
    _hOldForegroundWindow=GetForegroundWindow();
    
    WNDCLASS wc;
    
    // Clear before filling in window structure!
    ZeroMemory(&wc, sizeof(WNDCLASS));
    wc.style      = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc = (WNDPROC) static_window_proc;
    wc.hInstance   = hinstance;
    
    string windows_icon_filename = get_icon_filename_().to_os_specific();
    
    if(!windows_icon_filename.empty()) {
        // Note: LoadImage seems to cause win2k internal heap corruption (outputdbgstr warnings)
        // if icon is more than 8bpp
        wc.hIcon = (HICON) LoadImage(NULL, windows_icon_filename.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    } else {
        wc.hIcon = NULL; // use default app icon
    }
    
    wc.hCursor        = _hMouseCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName   = NULL;
    wc.lpszClassName  = WGL_WINDOWCLASSNAME;
    
    if(!RegisterClass(&wc)) {
        wgldisplay_cat.fatal() << "could not register window class!" << endl;
        exit(1);
    }

    DWORD window_style = WS_POPUP | WS_SYSMENU;  // for CreateWindow

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

      // dont pick any video modes < MIN_REFRESH_RATE Hz
      #define MIN_REFRESH_RATE 60
      
      // EnumDisplaySettings may indicate 0 or 1 for refresh rate, which means use driver default rate
      #define ACCEPTABLE_REFRESH_RATE(RATE) ((RATE >= MIN_REFRESH_RATE) || (RATE==0) || (RATE==1))
      
      #define LOWVIDMEMTHRESHOLD 3500000
      if(GetAvailVidMem() < LOWVIDMEMTHRESHOLD) {
          wgldisplay_cat.debug() << "small video memory card detect, switching fullscreen to minimum 640x480x16 config\n";
          // we're going to need  640x480 at 16 bit to save enough tex vidmem
          dwFullScreenBitDepth=16;
          dwWidth=640;
          dwHeight=480;
      }

      DEVMODE dm;
      bool bGoodModeFound=false;
      BOOL bGotNewMode;
      int j=0;

      while(1) {
          memset( &dm, 0, sizeof( dm ) );
          dm.dmSize = sizeof( dm );

          bGotNewMode=EnumDisplaySettings(NULL,j,&dm);
          if(!bGotNewMode)
            break;

          if((dm.dmPelsWidth==dwWidth) && (dm.dmPelsHeight==dwHeight) &&
             (dm.dmBitsPerPel==dwFullScreenBitDepth) &&
             ACCEPTABLE_REFRESH_RATE(dm.dmDisplayFrequency)) {
              bGoodModeFound=true;
              break;
          }
          j++;
      }

      if(!bGoodModeFound) {
          wgldisplay_cat.fatal() << "Videocard has no supported display resolutions at specified res ( " << dwWidth << " X " << dwHeight << " X " << dwFullScreenBitDepth <<" )\n";
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
    
      if(chg_result!=DISP_CHANGE_SUCCESSFUL) {
            wgldisplay_cat.fatal() << "ChangeDisplaySettings failed (error code: " << chg_result <<") for specified res ( " << dwWidth << " X " << dwHeight << " X " << dwFullScreenBitDepth <<" ), " << dm.dmDisplayFrequency  << "Hz\n";
            exit(1);
      }

      _pCurrent_display_settings = new(DEVMODE);
      memcpy(_pCurrent_display_settings,&dm,sizeof(DEVMODE));

      _props._xorg = 0;
      _props._yorg = 0;
      _props._xsize = dwWidth;
      _props._ysize = dwHeight;

       if(wgldisplay_cat.is_debug())
           wgldisplay_cat.debug() << "set fullscreen mode at res ( " << dwWidth << " X " << dwHeight << " X " << dwFullScreenBitDepth <<" ), " << dm.dmDisplayFrequency  << "Hz\n";
  } else {
        
        RECT win_rect;
        SetRect(&win_rect, _props._xorg,  _props._yorg, _props._xorg + _props._xsize,
                _props._yorg + _props._ysize);

        if(_props._border) {
            window_style |= WS_OVERLAPPEDWINDOW;
        }

        BOOL bRes = AdjustWindowRect(&win_rect, window_style, FALSE);  //compute window size based on desired client area size

        if(!bRes) {
            wgldisplay_cat.fatal() << "AdjustWindowRect failed!" << endl;
            exit(1);
        }

        // make sure origin is on screen, slide far bounds over if necessary
        if(win_rect.left < 0) {
            win_rect.right += abs(win_rect.left); win_rect.left = 0;
        }
        if(win_rect.top < 0) {
            win_rect.bottom += abs(win_rect.top); win_rect.top = 0;
        }

        _mwindow = CreateWindow(WGL_WINDOWCLASSNAME, _props._title.c_str(),
                                window_style, win_rect.left, win_rect.top, win_rect.right-win_rect.left,
                                win_rect.bottom-win_rect.top,
                                NULL, NULL, hinstance, 0);
  }
    
  if(!_mwindow) {
        wgldisplay_cat.fatal() << "CreateWindow() failed!" << endl;
        PrintErrorMessage(LAST_ERROR);
        exit(1);
  }
    
  hwnd_pandawin_map[_mwindow] = this;
  global_wglwinptr = NULL;  // get rid of any reference to this obj
    
  // move window to top of zorder
  SetWindowPos(_mwindow, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOOWNERZORDER);
    
  _hdc = GetDC(_mwindow);

  // Configure the framebuffer according to parameters specified in _props
  // Initializes _visual
  int pfnum=choose_visual();

  //  int pfnum=ChoosePixelFormat(_hdc, _visual);
  if(wgldisplay_cat.is_debug())
     wgldisplay_cat.debug() << "config() - picking pixfmt #"<< pfnum <<endl;

  if (!SetPixelFormat(_hdc, pfnum, _visual)) {
    wgldisplay_cat.fatal()
      << "config() - SetPixelFormat failed after window create" << endl;
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

//  _glgsg = DCAST(GLGraphicsStateGuardian, _gsg);   dont need this now

  string tmpstr((char*)glGetString(GL_EXTENSIONS));

  _extensions_str = tmpstr;

  PFNWGLGETEXTENSIONSSTRINGEXTPROC wglGetExtensionsStringEXT;
  PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB;
  wglGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
  wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");

  if(wglGetExtensionsStringARB!=NULL) {
       _extensions_str += " ";
       const char *ARBextensions = wglGetExtensionsStringARB(wglGetCurrentDC());
       _extensions_str.append(ARBextensions);
  }

  if(wglGetExtensionsStringEXT!=NULL) {
      // usually this will be the same as ARB extensions, but whatever
      _extensions_str += " ";
      const char *EXTextensions = wglGetExtensionsStringEXT();
      _extensions_str.append(EXTextensions);
  }

  if(wgldisplay_cat.is_spam())
     wgldisplay_cat.spam() << "GL extensions: " << _extensions_str << endl;

  if(gl_sync_video) {
      // set swapbuffers to swap no more than once per monitor refresh
      // note sometimes the ICD advertises this ext, but it still doesn't seem to work
      if(_extensions_str.find("WGL_EXT_swap_control")!=_extensions_str.npos) {
           PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
           wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");
           if(wglSwapIntervalEXT!=NULL)
               wglSwapIntervalEXT(1);   

           if(wgldisplay_cat.is_spam())
               wgldisplay_cat.spam() << "setting swapbuffer interval to 1/refresh\n";
      }
  }

  if(gl_show_fps_meter) {

      _start_time = timeGetTime();
      _current_fps = 0.0;
      _start_frame_count = _cur_frame_count = 0;

     // 128 enough to handle all the ascii chars
     // this creates a display list for each char.  displist numbering starts
     // at FONT_BITMAP_OGLDISPLAYLISTNUM.  Might want to optimize just to save
     // mem by just allocing bitmaps for chars we need (0-9 fps,SPC)
     wglUseFontBitmaps(_hdc, 0, 128, FONT_BITMAP_OGLDISPLAYLISTNUM);
  }

  if(wgldisplay_cat.is_debug()) {
      const GLubyte *vendorname=glGetString(GL_VENDOR);
      if(vendorname!=NULL) {
          if(strncmp((const char *)vendorname,"Microsoft",9)==0) {
              wgldisplay_cat.debug() << " GL VendorID: " <<   glGetString(GL_VENDOR) << " (Software Rendering)" << endl;
          } else {
              wgldisplay_cat.debug() << " GL VendorID: " <<   glGetString(GL_VENDOR) << endl;
          }
      } else {
         wgldisplay_cat.info() << " glGetString(GL_VENDOR) returns NULL!!!\n";
      }
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
    match = (PIXELFORMATDESCRIPTOR *)malloc(sizeof(PIXELFORMATDESCRIPTOR));
    DescribePixelFormat(_hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), match);

    // ChoosePixelFormat is dumb about stereo
    if (stereo) {
      if (!(match->dwFlags & PFD_STEREO)) {
    wgldisplay_cat.info()
      << "try_for_visual() - request for stereo failed" << endl;
      }
    }
  }

  return match;
}

#ifdef _DEBUG
void PrintPFD(PIXELFORMATDESCRIPTOR *pfd,char *msg) {

  wgldisplay_cat.spam() << msg << endl
                         << "PFD flags: 0x" << (void*)pfd->dwFlags << " (" <<
            ((pfd->dwFlags &  PFD_GENERIC_ACCELERATED) ? " PFD_GENERIC_ACCELERATED |" : "") <<
            ((pfd->dwFlags &  PFD_GENERIC_FORMAT) ? " PFD_GENERIC_FORMAT |" : "") <<
            ((pfd->dwFlags &  PFD_DOUBLEBUFFER) ? " PFD_DOUBLEBUFFER |" : "") <<
            ((pfd->dwFlags &  PFD_DRAW_TO_WINDOW) ? " PFD_DRAW_TO_WINDOW |" : "") <<
            ((pfd->dwFlags &  PFD_SUPPORT_OPENGL) ? " PFD_SUPPORT_OPENGL |" : "") <<
            ((pfd->dwFlags & PFD_SWAP_EXCHANGE) ? " PFD_SWAP_EXCHANGE |" : "") <<
            ((pfd->dwFlags & PFD_SWAP_COPY) ? " PFD_SWAP_COPY |" : "") << ")\n"
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
int wglGraphicsWindow::choose_visual(void) {

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
  int i;

  for(i=1;i<=MaxPixFmtNum;i++) {
      DescribePixelFormat(_hdc, i, sizeof(PIXELFORMATDESCRIPTOR), &pfd);


      if((pfd.dwFlags & PFD_GENERIC_ACCELERATED) && (pfd.dwFlags & PFD_GENERIC_FORMAT))
          drvtype=MCD;
       else if(!(pfd.dwFlags & PFD_GENERIC_ACCELERATED) && !(pfd.dwFlags & PFD_GENERIC_FORMAT))
          drvtype=ICD;
       else {
         drvtype=Software;
         continue;  // skipping all SW fmts
       }

      if(wgldisplay_cat.is_debug())
          wgldisplay_cat->debug() << "----------------" << endl;

      if((pfd.iPixelType == PFD_TYPE_COLORINDEX) && !(mask & W_INDEX))
          continue;

#if 0
    // use wglinfo.exe instead
    char msg[200];
    sprintf(msg,"\nGL PixelFormat[%d]",i);
    PrintPFD(&pfd,msg);
#endif

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

       if(mask & W_DOUBLE)
           dwReqFlags|= PFD_DOUBLEBUFFER;
       if((mask & W_ALPHA) && (pfd.cAlphaBits==0))
           continue;
       if((mask & W_DEPTH) && (pfd.cDepthBits==0))
           continue;
       if((mask & W_STENCIL) && (pfd.cStencilBits==0))
           continue;
       if((pfd.dwFlags & dwReqFlags)!=dwReqFlags)
           continue;

       // now we ignore the specified want_color_bits for windowed mode
       // instead we use the current screen depth

       if((pfd.cColorBits!=cur_bpp) && (!((cur_bpp==16) && (pfd.cColorBits==15)))
                                    && (!((cur_bpp==32) && (pfd.cColorBits==24))))
           continue;
       // we've passed all the tests, go ahead and pick this fmt
       // note: could go continue looping looking for more alpha bits or more depth bits
       // so this would pick 16bpp depth buffer, probably not 24bpp

       break;
  }

  if(i>MaxPixFmtNum) {
      wgldisplay_cat.error() << "ERROR: couldn't find HW-accelerated OpenGL pixfmt appropriate for this desktop!!\n";
      wgldisplay_cat.error() << "make sure OpenGL driver is installed, and try reducing the screen size\n";
      if(cur_bpp>16)
        wgldisplay_cat.error() << "or reducing the screen pixeldepth\n";
      exit(1);
  }

  _visual = (PIXELFORMATDESCRIPTOR*)malloc(sizeof(PIXELFORMATDESCRIPTOR));
  if(_visual==NULL) {
      wgldisplay_cat.error() << "couldnt alloc mem for PIXELFORMATDESCRIPTOR\n";
      exit(1);
  }

  #ifdef _DEBUG
    char msg[200];
    sprintf(msg,"Selected GL PixelFormat is #%d",i);
    PrintPFD(&pfd,msg);
  #endif

  *_visual = pfd;
  return i;
}

////////////////////////////////////////////////////////////////////
//     Function: setup_colormap
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::setup_colormap(void) {

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
void wglGraphicsWindow::end_frame(void) {
  if (gl_show_fps_meter) {
    PStatTimer timer(_show_fps_pcollector);
    DWORD now = timeGetTime();  // this is win32 fn

    float time_delta = (now - _start_time) * 0.001f;

    if(time_delta > gl_fps_meter_update_interval) {
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

    glColor3f(0.0,1.0,1.0);

    GLboolean tex_was_on = glIsEnabled(GL_TEXTURE_2D);

    if(tex_was_on)
      glDisable(GL_TEXTURE_2D);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0,_props._xsize,
            0,_props._ysize,
            -1.0,1.0);

    glRasterPos2f(_props._xsize-70,_props._ysize-20);  // these seem to be good for default font

    // set up for a string-drawing display list call
    glListBase(FONT_BITMAP_OGLDISPLAYLISTNUM);

    // draw a string using font display lists.  chars index their
    // corresponding displist name
    glCallLists(strlen(fps_msg), GL_UNSIGNED_BYTE, fps_msg);

    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    if(tex_was_on)
      glEnable(GL_TEXTURE_2D);

    _cur_frame_count++;  // only used by fps meter right now
  }

  {
    PStatTimer timer(_swap_pcollector);
    SwapBuffers(_hdc);
  }
  GraphicsWindow::end_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: handle_reshape
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::handle_reshape() {
      RECT view_rect;
      GetClientRect( _mwindow, &view_rect );
      ClientToScreen( _mwindow, (POINT*)&view_rect.left );   // translates top,left pnt
      ClientToScreen( _mwindow, (POINT*)&view_rect.right );  // translates right,bottom pnt

      // change _props xsize,ysize
      resized((view_rect.right - view_rect.left),(view_rect.bottom - view_rect.top));

      _props._xorg = view_rect.left;  // _props origin should reflect upper left of view rectangle
      _props._yorg = view_rect.top;

      if(wgldisplay_cat.is_spam()) {
          wgldisplay_cat.spam() << "reshape to origin: (" << _props._xorg << "," << _props._yorg << "), size: (" << _props._xsize << "," << _props._ysize << ")\n";
      }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_mouse_motion
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::handle_mouse_motion(int x, int y) {
  _input_devices[0].set_pointer_in_window(x, y);
}

////////////////////////////////////////////////////////////////////
//     Function: handle_mouse_entry
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::handle_mouse_entry(int state) {
  if (state == MOUSE_EXITED) {
    _input_devices[0].set_pointer_out_of_window();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_keypress
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::
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
void wglGraphicsWindow::
handle_keyrelease(ButtonHandle key) {
  if (key != ButtonHandle::none()) {
    _input_devices[0].button_up(key);
  }
}

void INLINE process_1_event(void) {
  MSG msg;

  if(!GetMessage(&msg, NULL, 0, 0)) {
      // WM_QUIT received
      DestroyAllWindows(false);
      exit(msg.wParam);  // this will invoke AtExitFn
  }

  // Translate virtual key messages
  TranslateMessage(&msg);
  // Call window_proc
  DispatchMessage(&msg);
}

void INLINE wglGraphicsWindow::process_events(void) {
  if(_window_inactive) {
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
void wglGraphicsWindow::update(void) {
#ifdef DO_PSTATS
  _show_code_pcollector.stop();

  if(!_window_inactive) {
      PStatClient::main_tick();
  }
#endif

  process_events();

  if(_window_inactive) {
      // note _window_inactive must be checked after process_events is called, to avoid draw_callback being called
      if(_idle_callback)
          call_idle_callback();
      return;
  }

  call_draw_callback(true);

  if(_idle_callback)
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
void wglGraphicsWindow::make_current(void) {
  if((_hdc==NULL)||(_context==NULL)||(_window_inactive)) {
      return;  // we're only allow unmake_current() to set this to NULL
  }

  PStatTimer timer(_make_current_pcollector);
  HGLRC current_context = wglGetCurrentContext();
  HDC current_dc = wglGetCurrentDC();

  if ((current_context != _context) || (current_dc != _hdc)) {
    if(!wglMakeCurrent(_hdc, _context)) {
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
void wglGraphicsWindow::unmake_current(void) {
  report_errors();

  if(!wglMakeCurrent(NULL, NULL)) {
      PrintErrorMessage(LAST_ERROR);
  }
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

TypeHandle wglGraphicsWindow::get_class_type(void) {
  return _type_handle;
}

void wglGraphicsWindow::init_type(void) {
  GraphicsWindow::init_type();
  register_type(_type_handle, "wglGraphicsWindow",
        GraphicsWindow::get_class_type());
}

TypeHandle wglGraphicsWindow::get_type(void) const {
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

   if(pwin!=hwnd_pandawin_map.end()) {
      wglGraphicsWindow *wglwinptr=(*pwin).second;
      return wglwinptr->window_proc(hwnd, msg, wparam, lparam);
   } else if(global_wglwinptr!=NULL){
       // this stuff should only be used during CreateWindow()
       return global_wglwinptr->window_proc(hwnd, msg, wparam, lparam);
   } else {
       // should never need this??  (maybe at shutdwn?)
       return DefWindowProc(hwnd, msg, wparam, lparam);
   }
}

void wglGraphicsWindow::deactivate_window(void) {
    // current policy is to suspend minimized or deactivated fullscreen windows, but leave
    // regular windows running normally
#ifdef _DEBUG
   if(wgldisplay_cat.is_spam())
       wgldisplay_cat.spam()  << "deactivate_window called"  << endl;
#endif

   if((!_props._fullscreen) || _exiting_window || _window_inactive) 
     return;

   if(wgldisplay_cat.is_spam())
       wgldisplay_cat.spam() << "WGL window deactivated, releasing gl context and waiting...\n";
   _window_inactive = true;
   unmake_current();

   // make sure window is minimized

   WINDOWPLACEMENT wndpl;
   wndpl.length=sizeof(WINDOWPLACEMENT);
   
   if(!GetWindowPlacement(_mwindow,&wndpl)) {
       wgldisplay_cat.error() << "GetWindowPlacement failed!\n";
       return;
   }
   if((wndpl.showCmd!=SW_MINIMIZE)&&(wndpl.showCmd!=SW_SHOWMINIMIZED)) {
       ShowWindow(_mwindow, SW_MINIMIZE);
   }

   // revert to default display mode
   ChangeDisplaySettings(NULL,0x0);

   _PandaPausedTimer = SetTimer(_mwindow,PAUSED_TIMER_ID,1500,NULL);
   if(_PandaPausedTimer!=PAUSED_TIMER_ID) {
       wgldisplay_cat.error() << "Error in SetTimer!\n";
   }
}

void wglGraphicsWindow::reactivate_window(void) {
    if(_window_inactive) {
        if(wgldisplay_cat.is_spam())
            wgldisplay_cat.spam() << "WGL window re-activated...\n";

        _window_inactive = false;

        if(_PandaPausedTimer!=NULL) {
            KillTimer(_mwindow,_PandaPausedTimer);
            _PandaPausedTimer = NULL;
        }

        // move window to top of zorder,
        SetWindowPos(_mwindow, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOOWNERZORDER);

        ChangeDisplaySettings(_pCurrent_display_settings,CDS_FULLSCREEN);
        
        GdiFlush();
        make_current();
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
    
          if(hwnd_pandawin_map.size()==0) {
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
            
           if(!wparam) {
               deactivate_window();
               return 0;
           }         // dont want to reactivate until window is actually un-minimized (see WM_SIZE)
           break;
        }

    case WM_EXITSIZEMOVE:
            #ifdef _DEBUG
              wgldisplay_cat.spam()  << "WM_EXITSIZEMOVE received"  << endl;
            #endif
            
            if(_window_inactive)
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
                if(_mwindow==NULL)
                    break;

                if((wparam==SIZE_MAXIMIZED) || (wparam==SIZE_RESTORED)) { // old comment -- added SIZE_RESTORED to handle 3dfx case  (what does this mean?)
                    if(_window_inactive)
                        reactivate_window();

//                  if((_props._xsize != width) || (_props._ysize != height))
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

    case WM_SYSKEYUP:
    case WM_SYSKEYDOWN:
    case WM_SYSCHAR:
    case WM_CHAR:
        break;

    case WM_KEYDOWN: {
        POINT point;

        GetCursorPos(&point);
        ScreenToClient(hwnd, &point);

        // handle Cntrl-V paste from clipboard
        if(!((wparam=='V') && (GetKeyState(VK_CONTROL) < 0))) {
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
                if(lptstr != NULL)  {
                    char *pChar;
                    for(pChar=lptstr;*pChar!=NULL;pChar++) {
                       handle_keypress(KeyboardButton::ascii_key((uchar)*pChar), point.x, point.y);
                    }
                    GlobalUnlock(hglb); 
                } 
            }
            CloseClipboard(); 
        }
        return 0;
    }

    case WM_KEYUP: {
        // dont need x,y for this
        handle_keyrelease(lookup_key(wparam));
        break;
    }
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
      if (x & 1 << 15) 
        x -= (1 << 16);
      if (y & 1 << 15) 
        y -= (1 << 16);
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
          x = LOWORD(lparam);
          y = HIWORD(lparam);
          if (x & 1 << 15) 
              x -= (1 << 16);
          if (y & 1 << 15)
              y -= (1 << 16);
          // make_current();  what does OGL have to do with mouse input??
      #endif
      handle_keyrelease(MouseButton::button(button));
      break;

    case WM_MOUSEMOVE:
        x = LOWORD(lparam);
        y = HIWORD(lparam);
        if (x & 1 << 15) 
          x -= (1 << 16);
        if (y & 1 << 15) 
          y -= (1 << 16);
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

    case WM_SETFOCUS:
          SetCursor(_hMouseCursor);
          if (mouse_entry_enabled()) {
               make_current();
               handle_mouse_entry(MOUSE_ENTERED);
          }
          break;

    case WM_KILLFOCUS:
          if (mouse_entry_enabled()) {
               //make_current();  this doesnt make any sense, we're leaving our window
               handle_mouse_entry(MOUSE_EXITED);
          }
          break;

    case WM_TIMER:
      if((wparam==_PandaPausedTimer) && _window_inactive) {
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
ButtonHandle
wglGraphicsWindow::lookup_key(WPARAM wparam) const {
  // probably would be faster with a map var
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


