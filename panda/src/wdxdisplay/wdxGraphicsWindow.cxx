// Filename: wdxGraphicsWindow.cxx
// Created by:  mike (09Jan00)
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

#include <errno.h>
#include <time.h>
#include <math.h>
#include <tchar.h>
#include "wdxGraphicsWindow.h"
#include "wdxGraphicsPipe.h"
#include "config_wdxdisplay.h"

#include <keyboardButton.h>
#include <mouseButton.h>
#include <throw_event.h>

#ifdef DO_PSTATS
#include <pStatTimer.h>
#endif

#include <map>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle wdxGraphicsWindow::_type_handle;

#define LAST_ERROR 0
#define ERRORBOX_TITLE "Panda3D Error"
#define WDX_WINDOWCLASSNAME "wdxDisplay"
#define DEFAULT_CURSOR IDC_ARROW

typedef map<HWND,wdxGraphicsWindow *> HWND_PANDAWIN_MAP;

HWND_PANDAWIN_MAP hwnd_pandawin_map;
wdxGraphicsWindow* global_wdxwinptr = NULL;  // need this for temporary windproc

#define MAX_DISPLAYS 20

extern bool dx_full_screen_antialiasing;  // defined in dxgsg_config.cxx

#define PAUSED_TIMER_ID  7   // completely arbitrary choice
#define DXREADY ((_dxgsg!=NULL)&&(_dxgsg->GetDXReady()))

LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam,LPARAM lparam);

// imperfect method to ID NVid? could also scan desc str, but that isnt fullproof either
#define IS_NVIDIA(DDDEVICEID) ((DDDEVICEID.dwVendorId==0x10DE) || (DDDEVICEID.dwVendorId==0x12D2))
#define IS_ATI(DDDEVICEID) (DDDEVICEID.dwVendorId==0x1002) 
#define IS_MATROX(DDDEVICEID) (DDDEVICEID.dwVendorId==0x102B)

// because we dont have access to ModifierButtons, as a hack just synchronize state of these
// keys on get/lose keybd focus
#define NUM_MODIFIER_KEYS 16
unsigned int hardcoded_modifier_buttons[NUM_MODIFIER_KEYS]={VK_SHIFT,VK_MENU,VK_CONTROL,VK_SPACE,VK_TAB,
                                         VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,VK_PRIOR,VK_NEXT,VK_HOME,VK_END,
                                         VK_INSERT,VK_DELETE,VK_ESCAPE};

static DWORD BitDepth_2_DDBDMask(DWORD iBitDepth) {
    switch(iBitDepth) {
        case 16: return DDBD_16;
        case 32: return DDBD_32;
        case 24: return DDBD_24;
        case 8: return DDBD_8;
        case 1: return DDBD_1;
        case 2: return DDBD_2;
        case 4: return DDBD_4;
    }
    return 0x0;
}
/*
typedef enum {DBGLEV_FATAL,DBGLEV_ERROR,DBGLEV_WARNING,DBGLEV_INFO,DBGLEV_DEBUG,DBGLEV_SPAM
    } DebugLevels;
 
void PrintDBGStr(DebugLevels level,HRESULT hr,const char *msgstr) {
    ostream *pstrm;
    static ostream dbg_strms[DBGLEV_SPAM+1]={wdxdisplay_cat.fatal,wdxdisplay_cat.error,
        wdxdisplay_cat.warning,wdxdisplay_cat.info,wdxdisplay_cat.debug,wdxdisplay_cat.spam};
    assert(level<=DBGLEV_SPAM);

    pstrm=dbg_strms[level];
    (*pstrm)->fatal() << "GetDisplayMode failed, result = " << ConvD3DErrorToString(hr) << endl;
}
*/
#ifdef _DEBUG
static void DebugPrintPixFmt(DDPIXELFORMAT* pddpf) {
    static int iddpfnum=0;
    ostream *dbgout = &dxgsg_cat.debug();

    *dbgout << "ZBuf DDPF[" << iddpfnum << "]: RGBBitCount:" << pddpf->dwRGBBitCount
    << " Flags:"  << (void *)pddpf->dwFlags ;

    if(pddpf->dwFlags & DDPF_STENCILBUFFER) {
        *dbgout << " StencilBits:" << (void *) pddpf->dwStencilBitDepth;
    }

    *dbgout << endl;

    iddpfnum++;
}
#endif

// pops up MsgBox w/system error msg
#define LAST_ERROR 0
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
   wdxdisplay_cat.fatal() << "System error msg: " << pMessageBuffer << endl;
   LocalFree( pMessageBuffer ); 
}

//#if defined(NOTIFY_DEBUG) || defined(DO_PSTATS)
#ifdef _DEBUG
extern void dbgPrintVidMem(LPDIRECTDRAW7 pDD, LPDDSCAPS2 lpddsCaps,const char *pMsg) {
    DWORD dwTotal,dwFree;
    HRESULT hr;

 //  These Caps bits arent allowed to be specified when calling GetAvailVidMem.
 //  They don't affect surface allocation in a vram heap.

#define AVAILVIDMEM_BADCAPS  (DDSCAPS_BACKBUFFER   | \
                              DDSCAPS_FRONTBUFFER  | \
                              DDSCAPS_COMPLEX      | \
                              DDSCAPS_FLIP         | \
                              DDSCAPS_OWNDC        | \
                              DDSCAPS_PALETTE      | \
                              DDSCAPS_SYSTEMMEMORY | \
                              DDSCAPS_VISIBLE      | \
                              DDSCAPS_WRITEONLY)

    DDSCAPS2 ddsCaps = *lpddsCaps;
    ddsCaps.dwCaps &= ~(AVAILVIDMEM_BADCAPS);  // turn off the bad caps
//  ddsCaps.dwCaps |= DDSCAPS_VIDEOMEMORY; done internally by DX anyway

    if(FAILED(  hr = pDD->GetAvailableVidMem(&ddsCaps,&dwTotal,&dwFree))) {
        wdxdisplay_cat.debug() << "GetAvailableVidMem failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

  // Write a debug message to the console reporting the texture memory.
    char tmpstr[100],tmpstr2[100];
    sprintf(tmpstr,"%.4g",dwTotal/1000000.0);
    sprintf(tmpstr2,"%.4g",dwFree/1000000.0);
    if(wdxdisplay_cat.is_debug())
       wdxdisplay_cat.debug() << "AvailableVidMem before creating "<< pMsg << ",(megs) total: " << tmpstr << "  free:" << tmpstr2 <<endl;
}
#endif

#define MAX_DX_ZBUF_FMTS 20
static int cNumZBufFmts;

HRESULT CALLBACK EnumZBufFmtsCallback( LPDDPIXELFORMAT pddpf, VOID* param )  {
    DDPIXELFORMAT *ZBufFmtsArr = (DDPIXELFORMAT *) param;
    assert(cNumZBufFmts < MAX_DX_ZBUF_FMTS);
    memcpy( &(ZBufFmtsArr[cNumZBufFmts]), pddpf, sizeof(DDPIXELFORMAT) );
    cNumZBufFmts++;
    return DDENUMRET_OK;
}

// fn exists so AtExitFn can call it without refcntr blowing up since its !=0
void wdxGraphicsWindow::DestroyMe(bool bAtExitFnCalled) {
  if(wdxdisplay_cat.is_spam())
      wdxdisplay_cat.spam() << "DestroyMe called, AtExitFnCalled=" << bAtExitFnCalled << endl;

  _exiting_window = true;  // may be needed for DestroyWindow call
  DXScreenData scrn;
  memcpy(&scrn,&_dxgsg->scrn,sizeof(DXScreenData));

  if(_dxgsg!=NULL) {
      _dxgsg->dx_cleanup(_props._fullscreen, bAtExitFnCalled);
      _dxgsg=NULL;
  }

/*
  if(scrn._hdc!=NULL) {
    ReleaseDC(scrn.hWnd,scrn._hdc);
//    _hdc = NULL;
  }
*/  
/*
  if((scrn._hOldForegroundWindow!=NULL) && (scrn.hWnd==GetForegroundWindow())) {
      SetForegroundWindow(scrn._hOldForegroundWindow);
  }
"*/
  if(scrn.hWnd!=NULL) {
/*
    if(_bLoadedCustomCursor && (_hMouseCursor!=NULL))
        DestroyCursor(_hMouseCursor);
*/      
      DestroyWindow(scrn.hWnd);
      hwnd_pandawin_map.erase(scrn.hWnd);
  }
}

void wdxGraphicsWindow::do_close_window() {
  GraphicsWindow::do_close_window();
   DestroyMe(false);
}

////////////////////////////////////////////////////////////////////
//     Function: Destructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
wdxGraphicsWindow::~wdxGraphicsWindow(void) {
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
    wdxdisplay_cat.spam() << "AtExitFn called\n";
#endif

     restore_global_parameters();

     DestroyAllWindows(true);
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow::set_window_handle
//       Access: Public
//  Description: This is called on the object once the actual Window
//               has been created.  It gives the window handle of the
//               created window so the wdxGraphicsWindow object can
//               perform any additional initialization based on this.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::
set_window_handle(HWND hwnd) {
  // Tell the associated dxGSG about the window handle.
  _dxgsg->scrn.hWnd = hwnd;

  // Determine the initial open status of the IME.
  _ime_open = false;
  _ime_active = false;
  HIMC hIMC = ImmGetContext(hwnd);
  if (hIMC != 0) {
    _ime_open = (ImmGetOpenStatus(hIMC) != 0);
    ImmReleaseContext(hwnd, hIMC);
  }
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
      wdxGraphicsWindow *wdxwinptr=(*pwin).second;
      return wdxwinptr->window_proc(hwnd, msg, wparam, lparam);
   } else if(global_wdxwinptr!=NULL){
       // this stuff should only be used during CreateWindow()
       return global_wdxwinptr->window_proc(hwnd, msg, wparam, lparam);
   } else {
       // should never need this??  (maybe at shutdwn?)
       return DefWindowProc(hwnd, msg, wparam, lparam);
   }
}

// Note: could use _TrackMouseEvent in comctrl32.dll (part of IE 3.0+) which emulates
// TrackMouseEvent on w95, but that requires another 500K of memory to hold that DLL,
// which is lame just to support w95, which probably has other issues anyway
INLINE void wdxGraphicsWindow::
track_mouse_leaving(HWND hwnd) {
  if(_pParentWindowGroup->_pfnTrackMouseEvent==NULL)
      return;

  TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT),TME_LEAVE,hwnd,0};
  BOOL bSucceeded = _pParentWindowGroup->_pfnTrackMouseEvent(&tme);  // tell win32 to post WM_MOUSELEAVE msgs

  if((!bSucceeded) && wdxdisplay_cat.is_debug())
     wdxdisplay_cat.debug() << "TrackMouseEvent failed!, LastError=" << GetLastError() << endl;

  _tracking_mouse_leaving=true;
}

////////////////////////////////////////////////////////////////////
//     Function: window_proc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
LONG wdxGraphicsWindow::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  int button = -1;
  int x, y, width, height;

  switch(msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        if(DXREADY)
            show_frame();
        EndPaint(hwnd, &ps);
        return 0;
    }
    
    case WM_MOUSEMOVE:
        if(!DXREADY)
            break;
    
        // Win32 doesn't return the same numbers as X does when the mouse
        // goes beyond the upper or left side of the window
        #define SET_MOUSE_COORD(iVal,VAL) { \
                iVal = VAL;                   \
                if(iVal & 0x8000)             \
                  iVal -= 0x10000;            \
        }
        
        if(!_tracking_mouse_leaving) {
            // need to re-call TrackMouseEvent every time mouse re-enters window
            track_mouse_leaving(hwnd);
        }
    
        SET_MOUSE_COORD(x,LOWORD(lparam));
        SET_MOUSE_COORD(y,HIWORD(lparam));
    
        handle_mouse_motion(x, y);
        return 0;
    
    // if cursor is invisible, make it visible when moving in the window bars,etc
    case WM_NCMOUSEMOVE: {
        if(!_props._bCursorIsVisible) {
            if(!_cursor_in_windowclientarea) {
                ShowCursor(true);
                _cursor_in_windowclientarea=true;
            }
        }
        break;
    }

    case WM_NCMOUSELEAVE: {
        if(!_props._bCursorIsVisible) {
            ShowCursor(false);
            _cursor_in_windowclientarea=false;
        }
        break;
    }
    
    case WM_MOUSELEAVE: {
       // wdxdisplay_cat.fatal() << "XXXXX WM_MOUSELEAVE received\n";

       _tracking_mouse_leaving=false;  
       handle_mouse_entry(false,0,0);         
       break;
    }
    
    case WM_CREATE: {
      track_mouse_leaving(hwnd);
    
      _cursor_in_windowclientarea=false;
      if(!_props._bCursorIsVisible)
          ShowCursor(false);
      break;
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
      // In case we're running fullscreen mode, we have to turn on
      // explicit DX support for overlay windows now, so we'll be able
      // to see the IME window.
      _dxgsg->support_overlay_window(true);
      _ime_active = true;
      break;

    case WM_IME_ENDCOMPOSITION:
      // Turn off the support for overlay windows, since we're done
      // with the IME window for now and it just slows things down.
      _dxgsg->support_overlay_window(false);
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

    case WM_SYSKEYDOWN: {
      // Alt and F10 are sent as WM_SYSKEYDOWN instead of WM_KEYDOWN
      // want to use defwindproc on Alt syskey so std windows cmd Alt-F4 works, etc
      POINT point;
      GetCursorPos(&point);
      ScreenToClient(hwnd, &point);
      handle_keypress(lookup_key(wparam), point.x, point.y);
      if (wparam == VK_F10) {
        //bypass default windproc F10 behavior (it activates the main menu, but we have none)
        return 0;
      }
    }
    break;

    case WM_SYSCOMMAND:
        if(wparam==SC_KEYMENU) {
            // if Alt is released (alone w/o other keys), defwindproc will send 
            // this command, which will 'activate' the title bar menu (we have none)
            // and give focus to it.  we dont want this to happen, so kill this msg
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
        if(button < 0)
            button = 1;
    case WM_RBUTTONDOWN:
        if(!DXREADY) 
          break;
    
        if(button < 0)
            button = 2;
        SetCapture(hwnd);
        SET_MOUSE_COORD(x,LOWORD(lparam));
        SET_MOUSE_COORD(y,HIWORD(lparam));
        handle_keypress(MouseButton::button(button), x, y);
        return 0;
    
    case WM_LBUTTONUP:
        button = 0;
    case WM_MBUTTONUP:
        if(button < 0)
            button = 1;
    case WM_RBUTTONUP:
        if(!DXREADY) 
          break;
    
        if(button < 0)
            button = 2;
        ReleaseCapture();
        #if 0
           SET_MOUSE_COORD(x,LOWORD(lparam));
           SET_MOUSE_COORD(y,HIWORD(lparam));           
        #endif
        handle_keyrelease(MouseButton::button(button));
        return 0;
    
    case WM_MOVE:
        if(!DXREADY)
            break;
        handle_window_move(LOWORD(lparam), HIWORD(lparam) );
        return 0;

    case WM_EXITSIZEMOVE:
        #ifdef _DEBUG
          wdxdisplay_cat.spam()  << "WM_EXITSIZEMOVE received"  << endl;
        #endif
        
        if(_WindowAdjustingType==Resizing) {
            handle_reshape(true);
        }
    
        _WindowAdjustingType = NotAdjusting;
        return 0;
    
    case WM_ENTERSIZEMOVE: {
            if(_dxgsg!=NULL)
                _dxgsg->SetDXReady(true);   // dont disable here because I want to see pic as I resize
            _WindowAdjustingType = MovingOrResizing;
        }
        break;

    case WM_DISPLAYCHANGE: {
        #ifdef _DEBUG
            width = LOWORD(lparam);  height = HIWORD(lparam);
            DWORD newbitdepth=wparam;
            wdxdisplay_cat.spam() <<"WM_DISPLAYCHANGE received with width:" << width << "  height: " << height << " bpp: " << wparam<< endl;
        #endif

            //    unfortunately this doesnt seem to work because RestoreAllSurfaces doesn't
            //    seem to think we're back in the original displaymode even after I've received
            //    the WM_DISPLAYCHANGE msg, and returns WRONGMODE error.  So the only way I can
            //    think of to make this work is to have the timer periodically check for restored
            //    coop level

            //    if(_props._fullscreen && !_window_active) {
            //          if(_dxgsg!=NULL)
            //              _dxgsg->CheckCooperativeLevel(DO_REACTIVATE_WINDOW);
            //           else reactivate_window();
            //    }

            // does the windowed case handle displaychange properly?  no. need to recreate all devices
          }
          break;

      case WM_SIZE: {
                #ifdef _DEBUG
                {
                    width = LOWORD(lparam);  height = HIWORD(lparam);
                    wdxdisplay_cat.spam() << "WM_SIZE received with width:" << width << "  height: " << height << " flags: " <<
                    ((wparam == SIZE_MAXHIDE)? "SIZE_MAXHIDE " : "") << ((wparam == SIZE_MAXSHOW)? "SIZE_MAXSHOW " : "") <<
                    ((wparam == SIZE_MINIMIZED)? "SIZE_MINIMIZED " : "") << ((wparam == SIZE_RESTORED)? "SIZE_RESTORED " : "") <<
                    ((wparam == SIZE_MAXIMIZED)? "SIZE_MAXIMIZED " : "") << endl;
                }
                #endif
                // old comment -- added SIZE_RESTORED to handle 3dfx case
                if(dx_full_screen || ((_dxgsg==NULL) || (_dxgsg->scrn.hWnd==NULL)) || ((wparam != SIZE_RESTORED) && (wparam != SIZE_MAXIMIZED)))
                    break;

                width = LOWORD(lparam);  height = HIWORD(lparam);

                if((_props._xsize != width) || (_props._ysize != height)) {
                    _WindowAdjustingType = Resizing;

                 // for maximized,unmaximize, need to call resize code artificially
                 // since no WM_EXITSIZEMOVE is generated.
                 if(wparam==SIZE_MAXIMIZED) {
                       _bSizeIsMaximized=TRUE;
                       window_proc(hwnd, WM_EXITSIZEMOVE, 0x0,0x0);
                 } else if((wparam==SIZE_RESTORED) && _bSizeIsMaximized) {
                       _bSizeIsMaximized=FALSE;  // only want to reinit dx if restoring from maximized state
                       window_proc(hwnd, WM_EXITSIZEMOVE, 0x0,0x0);
                 }
                }

                break;
            }

        case WM_SETFOCUS: {
            if(!DXREADY) {
              break;
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
              if(GetKeyState(hardcoded_modifier_buttons[i]) < 0) 
                handle_keypress(lookup_key(hardcoded_modifier_buttons[i]),point.x,point.y);
            }
            return 0;
        }

        case WM_KILLFOCUS: {
            if(!DXREADY) {
              break;
            }

            int i;
            for(i=0;i<NUM_MODIFIER_KEYS;i++) {
              if(GetKeyState(hardcoded_modifier_buttons[i]) < 0)
                handle_keyrelease(lookup_key(hardcoded_modifier_buttons[i]));
            }

            return 0;
        }

#if 0
        case WM_WINDOWPOSCHANGING: {
                LPWINDOWPOS pWindPos = (LPWINDOWPOS) lparam;
                wdxdisplay_cat.spam() << "WM_WINDOWPOSCHANGING received, flags 0x" << pWindPos->flags <<  endl;
                break;
            }

        case WM_GETMINMAXINFO:
            wdxdisplay_cat.spam() << "WM_GETMINMAXINFO received\n" <<  endl;
            break;
#endif

        case WM_ERASEBKGND:
        // WM_ERASEBKGND will be ignored during resizing, because 
        // we dont want to redraw as user is manually resizing window
            if(_WindowAdjustingType)
                break;
            return 0;  // dont let GDI waste time redrawing the deflt background

        case WM_TIMER:
            // 2 cases of app deactivation:
            //
            // 1) user has switched out of fullscreen mode
            //    this is first signalled when ACTIVATEAPP returns false
            //    for this case, we dont wake up until WM_SIZE returns restore or maximize
            //    and WM_TIMER just periodically reawakens app for idle processing

            //    unfortunately this doesnt seem to work because RestoreAllSurfaces doesn't
            //    seem to think we're back in the original displaymode even after I've received
            //    the WM_DISPLAYCHANGE msg, and returns WRONGMODE error.  So the only way I can
            //    think of to make this work is to have the timer periodically check for restored
            //    coop level, as it does in case 2)

            //
            // 2) windowed app has lost access to dx because another app has taken dx exclusive mode
            //    here we rely on WM_TIMER to periodically check if it is ok to reawaken app.
            //    windowed apps currently run regardless of if its window is in the foreground
            //    so we cannot rely on window messages to reawaken app

            if((wparam==_PandaPausedTimer) && (!_window_active||_active_minimized_fullscreen)) {
                assert(_dxgsg!=NULL);
                _dxgsg->CheckCooperativeLevel(DO_REACTIVATE_WINDOW);

                // wdxdisplay_cat.spam() << "periodic return of control to app\n";
                _return_control_to_app = true;
                // throw_event("PandaPaused");   
                // do we still need to do this since I return control to app periodically using timer msgs?
                // does app need to know to avoid major computation?
            }
            return 0;

        case WM_CLOSE:
         // close_window();
          delete _pParentWindowGroup;

          // BUGBUG:  right now there is no way to tell the panda app the graphics window is invalid or
          //          has been closed by the user, to prevent further methods from being called on the window.
          //          this needs to be added to panda for multiple windows to work.  in the meantime, just
          //          trigger an exit here if # windows==0, since that is the expected behavior when all 
          //          windows are closed (should be done by the app though, and it assumes you only make this
          //          type of panda gfx window)
    
          if(hwnd_pandawin_map.size()==0) {
              exit(0);
          }
          return 0;


        case WM_ACTIVATEAPP: {
            #ifdef _DEBUG
              wdxdisplay_cat.spam()  << "WM_ACTIVATEAPP(" << (bool)(wparam!=0) <<") received\n";
            #endif
            
           if((!wparam) && _props._fullscreen) {
               deactivate_window();
               return 0;
           }         // dont want to reactivate until window is actually un-minimized (see WM_SIZE)
           break;
        }
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

  
////////////////////////////////////////////////////////////////////
//     Function: handle_reshape
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::handle_reshape(bool bDoDxReset) {
    
    GdiFlush();

    if(bDoDxReset && _dxgsg!=NULL) {
        HRESULT hr;

        if(_dxgsg->scrn.pddsBack==NULL) {
            //assume this is initial creation reshape and ignore this call
            return;
        }

        // Clear the back/primary surface to black
        DX_DECLARE_CLEAN(DDBLTFX, bltfx)
        bltfx.dwDDFX |= DDBLTFX_NOTEARING;
        hr = _dxgsg->scrn.pddsPrimary->Blt(NULL,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bltfx);
        if(FAILED( hr )) {
            wdxdisplay_cat.fatal() << "Blt to Black of Primary Surf failed! : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        if(FAILED(hr = _dxgsg->scrn.pDD->TestCooperativeLevel())) {
             wdxdisplay_cat.error() << "TestCooperativeLevel failed : result = " << ConvD3DErrorToString(hr) << endl;
             return;
        }

        _dxgsg->SetDXReady(false);  // disable rendering whilst we mess with surfs

        _dxgsg->RestoreAllVideoSurfaces();

        // Want to change rendertarget size without destroying d3d device.  To save vid memory
        // (and make resizing work on memory-starved 4MB cards), we need to construct
        // a temporary mini-sized render target for the d3d device (it cannot point to a
        // NULL rendertarget) before creating the fully resized buffers.  The old
        // rendertargets will be freed when these temp targets are set, and that will give
        // us the memory to create the resized target
    
        LPDIRECTDRAWSURFACE7 pddsDummy = NULL, pddsDummyZ = NULL;
        ULONG refcnt;
    
        DX_DECLARE_CLEAN( DDSURFACEDESC2, ddsd );

        _dxgsg->scrn.pddsBack->GetSurfaceDesc(&ddsd);
        LPDIRECTDRAW7 pDD = _dxgsg->scrn.pDD;
    
        ddsd.dwFlags &= ~DDSD_PITCH;
        ddsd.dwWidth  = 1; ddsd.dwHeight = 1;
        ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER | DDSCAPS_BACKBUFFER);
    
        PRINTVIDMEM(pDD,&ddsd.ddsCaps,"dummy backbuf");
    
        if(FAILED( hr = pDD->CreateSurface( &ddsd, &pddsDummy, NULL ) )) {
            wdxdisplay_cat.fatal() << "Resize CreateSurface for temp backbuf failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }
    
        if(_dxgsg->scrn.pddsZBuf!=NULL) {
            DX_DECLARE_CLEAN( DDSURFACEDESC2, ddsdZ );
            _dxgsg->scrn.pddsZBuf->GetSurfaceDesc(&ddsdZ);
            ddsdZ.dwFlags &= ~DDSD_PITCH;
            ddsdZ.dwWidth  = 1;   ddsdZ.dwHeight = 1;
        
            PRINTVIDMEM(pDD,&ddsdZ.ddsCaps,"dummy zbuf");
        
            if(FAILED( hr = pDD->CreateSurface( &ddsdZ, &pddsDummyZ, NULL ) )) {
                wdxdisplay_cat.fatal() << "Resize CreateSurface for temp zbuf failed : result = " << ConvD3DErrorToString(hr) << endl;
                exit(1);
            }
        
            if(FAILED( hr = pddsDummy->AddAttachedSurface( pddsDummyZ ) )) {
                wdxdisplay_cat.fatal() << "Resize AddAttachedSurf for temp zbuf failed : result = " << ConvD3DErrorToString(hr) << endl;
                exit(1);
            }
        }
    
        if(FAILED( hr = _dxgsg->scrn.pD3DDevice->SetRenderTarget( pddsDummy, 0x0 ))) {
            wdxdisplay_cat.fatal()
            << "Resize failed to set render target to temporary surface, result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }
        RELEASE(pddsDummyZ,wdxdisplay,"dummy resize zbuffer",false);
        RELEASE(pddsDummy,wdxdisplay,"dummy resize rendertarget buffer",false);
    }
    
    RECT view_rect;

    assert(!dx_full_screen);

    HWND hWnd=_dxgsg->scrn.hWnd;
    GetClientRect( hWnd, &view_rect );
    ClientToScreen( hWnd, (POINT*)&view_rect.left );   // translates top,left pnt
    ClientToScreen( hWnd, (POINT*)&view_rect.right );  // translates right,bottom pnt
    
    // change _props xsize,ysize
    resized((view_rect.right - view_rect.left),(view_rect.bottom - view_rect.top));
    
    _props._xorg = view_rect.left;  // _props origin should reflect upper left of view rectangle
    _props._yorg = view_rect.top;
    
    if(wdxdisplay_cat.is_spam()) {
      wdxdisplay_cat.spam() << "reshape to origin: (" << _props._xorg << "," << _props._yorg << "), size: (" << _props._xsize << "," << _props._ysize << ")\n";
    }
    
    if(_dxgsg!=NULL) {
      if(bDoDxReset)
          _dxgsg->dx_setup_after_resize(view_rect,hWnd);  // create the new resized rendertargets
      _dxgsg->SetDXReady(true);
    }
}

void wdxGraphicsWindow::deactivate_window(void) {
    // current policy is to suspend minimized or deactivated fullscreen windows, but leave
    // regular windows running normally

   if(!_window_active || _exiting_window || _active_minimized_fullscreen) {
       #ifdef _DEBUG
          if(wdxdisplay_cat.is_spam())
            wdxdisplay_cat.spam()  << "deactivate_window called, but ignored in current mode\n";
       #endif
     return;
   }

   if(bResponsive_minimized_fullscreen_window) {
       if(wdxdisplay_cat.is_spam())
           wdxdisplay_cat.spam() << "WDX fullscreen window switching to active minimized mode...\n";
       _active_minimized_fullscreen = true;
   } else {

       if(wdxdisplay_cat.is_spam())
           wdxdisplay_cat.spam() << "WDX window deactivated, waiting...\n";

       _window_active = false;
   }

   if(_props._fullscreen) {
       // make sure window is minimized

       WINDOWPLACEMENT wndpl;
       wndpl.length=sizeof(WINDOWPLACEMENT);
       if(!GetWindowPlacement(_dxgsg->scrn.hWnd,&wndpl)) {
          wdxdisplay_cat.error() << "GetWindowPlacement failed!\n";
          return;
       }

       if((wndpl.showCmd!=SW_MINIMIZE)&&(wndpl.showCmd!=SW_SHOWMINIMIZED)) {
          ShowWindow(_dxgsg->scrn.hWnd, SW_MINIMIZE);
       }

       throw_event("PandaPaused"); // right now this is used to signal python event handler to disable audio
   }

//   if(!bResponsive_minimized_fullscreen_window) {
   // need this even in responsive-mode to trigger the dxgsg check of cooplvl, i think?

       _PandaPausedTimer = SetTimer(_dxgsg->scrn.hWnd,PAUSED_TIMER_ID,500,NULL);
       if(_PandaPausedTimer!=PAUSED_TIMER_ID) {
           wdxdisplay_cat.error() << "Error in SetTimer!\n";
       }
//   }
}

void wdxGraphicsWindow::reactivate_window(void) {
    if(!_window_active) {
    
        // first see if dx cooperative level is OK for reactivation
    //    if(!_dxgsg->CheckCooperativeLevel())
    //        return;
    
        if(wdxdisplay_cat.is_spam())
            wdxdisplay_cat.spam() << "WDX window re-activated...\n";
    
        _window_active = true;
    
        if(_PandaPausedTimer!=NULL) {
            KillTimer(_dxgsg->scrn.hWnd,_PandaPausedTimer);
            _PandaPausedTimer = NULL;
        }
    
        // move window to top of zorder
    //  if(_props._fullscreen)
    //      SetWindowPos(_DisplayDataArray[0].hWnd, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOOWNERZORDER);
        GdiFlush();

    } else if(_active_minimized_fullscreen) {
        if(wdxdisplay_cat.is_spam())
            wdxdisplay_cat.spam() << "WDX window unminimized from active-minimized state...\n";
    
        if(_PandaPausedTimer!=NULL) {
            KillTimer(_dxgsg->scrn.hWnd,_PandaPausedTimer);
            _PandaPausedTimer = NULL;
        }

        _active_minimized_fullscreen = false;
    
        // move window to top of zorder
    //  if(_props._fullscreen)
    //      SetWindowPos(_DisplayDataArray[0].hWnd, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOOWNERZORDER);
        GdiFlush();
    }

    if(_props._fullscreen) {
        throw_event("PandaRestarted");  // right now this is used to signal python event handler to re-enable audio
    }
}

////////////////////////////////////////////////////////////////////
//     Function: Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
wdxGraphicsWindow::wdxGraphicsWindow(GraphicsPipe* pipe) : GraphicsWindow(pipe) {
   _ime_active = false;
   _pParentWindowGroup=NULL;
   _pParentWindowGroup=new wdxGraphicsWindowGroup(this);
}

////////////////////////////////////////////////////////////////////
//     Function: Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
wdxGraphicsWindow::wdxGraphicsWindow(GraphicsPipe* pipe, const GraphicsWindow::Properties& props) 
                  : GraphicsWindow(pipe, props) {
   _ime_open = false;
   _pParentWindowGroup=NULL;
   _pParentWindowGroup=new wdxGraphicsWindowGroup(this);
}

bool supports_color_cursors(DDDEVICEIDENTIFIER2 &DevID) {
    // TODO: add more cards as more testing is done
    if(IS_NVIDIA(DevID)) {    
        // all nvidia seem to support 256 color
        return true;
    } else if(IS_ATI(DevID)) {
        // radeons seem to be in the 5100 range and support color, assume anything in 6000 or above 
        // is newer than radeon and supports 256 color
        if(((DevID.dwDeviceId>=0x5100) && (DevID.dwDeviceId<=0x5200)) ||
           (DevID.dwDeviceId>=0x6000))
            return true;
    } else if IS_MATROX(DevID) {
        if(DevID.dwDeviceId==0x0525)   // G400 seems to support color cursors, havent tested other matrox
            return true;
    }

    return false;
}

void wdxGraphicsWindowGroup::CreateWindows(void) {
    HINSTANCE hProgramInstance = GetModuleHandle(NULL);
    WNDCLASS wc;
    static bool wc_registered = false;
    _hParentWindow = NULL;        

    // these fns arent defined on win95, so get dynamic ptrs to them to avoid
    // ugly DLL loader failures on w95
    HINSTANCE hUser32 = (HINSTANCE) LoadLibrary("user32.dll");
    assert(hUser32);

    _pfnGetMonitorInfo = (PFN_GETMONITORINFO) GetProcAddress(hUser32, "GetMonitorInfoA");
    _pfnTrackMouseEvent = (PFN_TRACKMOUSEEVENT) GetProcAddress(hUser32, "TrackMouseEvent");
    FreeLibrary(hUser32);

    // Clear before filling in window structure!
    ZeroMemory(&wc, sizeof(WNDCLASS));
    wc.style      = CS_HREDRAW | CS_VREDRAW; //CS_OWNDC;
    wc.lpfnWndProc    = (WNDPROC) static_window_proc;
    wc.hInstance      = hProgramInstance;

    string windows_icon_filename = get_icon_filename().to_os_specific();
    string windows_mono_cursor_filename = get_mono_cursor_filename().to_os_specific();
    string windows_color_cursor_filename = get_color_cursor_filename().to_os_specific();

    if(!windows_icon_filename.empty()) {
        // Note: LoadImage seems to cause win2k internal heap corruption (outputdbgstr warnings)
        // if icon is more than 8bpp

        // loads a .ico fmt file
        wc.hIcon = (HICON) LoadImage(NULL, windows_icon_filename.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE );

        if(wc.hIcon==NULL) {
            wdxdisplay_cat.warning() << "windows icon filename '" << windows_icon_filename << "' not found!!\n";
        }
    } else {
        wc.hIcon = NULL; // use default app icon
    }

    _bLoadedCustomCursor=false;
    _hMouseCursor=NULL;

    if(!windows_color_cursor_filename.empty()) {
        // card support for full color non-black/white GDI cursors varies greatly.  if the cursor is not supported,
        // it is rendered in software by GDI, which causes a flickering effect (because it's not synced 
        // with flip?).  GDI transparently masks the lack of HW support so there is no easy way for app to detect
        // if HW cursor support exists.  alternatives are to tie cursor motion to frame rate using DDraw blts
        // or overlays (this is done automatically by DX8 runtime mouse cursor support), or to have separate thread draw cursor 
        // (sync issues?).  instead we do mono cursor  unless card is known to support 256 color cursors
        bool bSupportsColorCursor=true;
    
        /* if any card doesnt support color, dont load it*/
        for(DWORD w=0;w<_windows.size();w++)
            bSupportsColorCursor &= supports_color_cursors(_windows[w]->_dxgsg->scrn.DXDeviceID);
    
        if(bSupportsColorCursor) {
            DWORD load_flags = LR_LOADFROMFILE;
    
            if(dx_full_screen) {
                // I think cursors should use LR_CREATEDIBSECTION since they should not be mapped to the device palette (in the case of 256-color cursors)
                // since they are not going to be used on the desktop
                load_flags |= LR_CREATEDIBSECTION;
            }
    
            // Note: LoadImage seems to cause win2k internal heap corruption (outputdbgstr warnings)
            // if icon is more than 8bpp

            // loads a .cur fmt file. 
            _hMouseCursor = (HCURSOR) LoadImage(NULL, windows_color_cursor_filename.c_str(), IMAGE_CURSOR, 0, 0, load_flags );
    
            if(_hMouseCursor==NULL) {
                wdxdisplay_cat.warning() << "windows color cursor filename '" << windows_color_cursor_filename << "' not found!!\n";
                goto try_mono_cursor;

            }

/*          dont need these anymore since we are do mousestuff before window creation    
            SetClassLongPtr(_mwindow, GCLP_HCURSOR, (LONG_PTR) hNewMouseCursor);
            SetCursor(hNewMouseCursor);
    
            if(_bLoadedCustomCursor)
               DestroyCursor(_hMouseCursor);
*/             
            _bLoadedCustomCursor=true;
        }
    }

    try_mono_cursor:

    if((!_bLoadedCustomCursor) && (!windows_mono_cursor_filename.empty())) {
        // Note: LoadImage seems to cause win2k internal heap corruption (outputdbgstr warnings)
        // if icon is more than 8bpp

        DWORD load_flags = LR_LOADFROMFILE;

        if(dx_full_screen) {
            // I think cursors should use LR_CREATEDIBSECTION since they should not be mapped to the device palette (in the case of 256-color cursors)
            // since they are not going to be used on the desktop
            load_flags |= LR_CREATEDIBSECTION;
        }
        // loads a .cur fmt file. 
        _hMouseCursor = (HCURSOR) LoadImage(NULL, windows_mono_cursor_filename.c_str(), IMAGE_CURSOR, 0, 0, load_flags);

        if(_hMouseCursor==NULL) {
            wdxdisplay_cat.warning() << "windows mono cursor filename '" << windows_mono_cursor_filename << "' not found!!\n";
        } else _bLoadedCustomCursor=true;
    }

    if(!_bLoadedCustomCursor) 
      _hMouseCursor = LoadCursor(NULL, DEFAULT_CURSOR);

    if (!wc_registered) {
      // We only need to register the window class once per session.
      wc.hCursor = _hMouseCursor;
      wc.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
      wc.lpszMenuName   = NULL;
      wc.lpszClassName  = WDX_WINDOWCLASSNAME;
      
      if(!RegisterClass(&wc)) {
        wdxdisplay_cat.error() << "could not register window class!" << endl;
      }
      wc_registered = true;
    }

    DWORD window_style = WS_POPUP | WS_SYSMENU;  // for CreateWindow

    global_wdxwinptr = _windows[0];  // for use during createwin()  bugbug look at this again

    // rect now contains the coords for the entire window, not the client
    if(dx_full_screen) {

        // extra windows must be parented to the first so app doesnt minimize when user selects them
        for(DWORD devnum=0;devnum<_windows.size();devnum++) {
            MONITORINFO minfo;
            ZeroMemory(&minfo, sizeof(MONITORINFO));
            minfo.cbSize = sizeof(MONITORINFO);
            if(_pfnGetMonitorInfo!=NULL)
                // get upper-left corner coords using GetMonitorInfo
                (*_pfnGetMonitorInfo)(_windows[devnum]->_dxgsg->scrn.hMon, &minfo);
             else {
                 minfo.rcMonitor.left = minfo.rcMonitor.top = 0;
             }

            GraphicsWindow::Properties *props = &_windows[devnum]->_props;

            wdxdisplay_cat.info() << "opening " << props->_xsize << "x" << props->_ysize << " fullscreen window\n";

            HWND hWin = CreateWindow(WDX_WINDOWCLASSNAME, props->_title.c_str(),
                                      window_style, minfo.rcMonitor.left, minfo.rcMonitor.top,
                                      props->_xsize,props->_ysize,
                                      _hParentWindow, NULL, hProgramInstance, 0);

            if(!hWin) {
                wdxdisplay_cat.fatal() << "CreateWindow failed for monitor " << devnum << "!, LastError=" << GetLastError() << endl;
                #ifdef _DEBUG
                   PrintErrorMessage(LAST_ERROR);
                #endif
                exit(1);
            }

            _windows[devnum]->set_window_handle(hWin);
            if(devnum==0) {
                _hParentWindow=hWin;
            }
        }
    } else {
        assert(_windows.size()==1);

        GraphicsWindow::Properties *props = &_windows[0]->_props;

        RECT win_rect;
        SetRect(&win_rect, props->_xorg,  props->_yorg, props->_xorg + props->_xsize,
                props->_yorg + props->_ysize);

        if(props->_border)
            window_style |= WS_OVERLAPPEDWINDOW;  // should we just use WS_THICKFRAME instead?

        AdjustWindowRect(&win_rect, window_style, FALSE);  //compute window size based on desired client area size

        // make sure origin is on screen
        if(win_rect.left < 0) {
            win_rect.right -= win_rect.left; win_rect.left = 0;
        }
        if(win_rect.top < 0) {
            win_rect.bottom -= win_rect.top; win_rect.top = 0;
        }

        UINT xsize = win_rect.right-win_rect.left;
        UINT ysize = win_rect.bottom-win_rect.top;

        wdxdisplay_cat.info() << "opening " << props->_xsize << "x" << props->_ysize << " regular window\n";

        _hParentWindow =
            CreateWindow(WDX_WINDOWCLASSNAME, props->_title.c_str(),
                         window_style, win_rect.left, win_rect.top, xsize,ysize,
                         NULL, NULL, hProgramInstance, 0);
        _windows[0]->set_window_handle(_hParentWindow);
    }

    if(_hParentWindow==NULL) {
        wdxdisplay_cat.fatal() << "CreateWindow failed!\n";
        exit(1);
    }

    for(DWORD devnum=0;devnum<_windows.size();devnum++) {
        wdxGraphicsWindow *pWDXWin = _windows[devnum];
        // for use by the window_proc
        hwnd_pandawin_map[pWDXWin->_dxgsg->scrn.hWnd] = pWDXWin;

/*
        // DC is mostly used for writing fps meter (but also old palette code, which needs updating)
        // probably only need to do this for devnum 0
        HDC hdc = GetDC(pWDXWin->_dxgsg_>scrn.hWnd);
        pWDXWin->_dxgsg->Set_HDC(hdc);
*/      
    }

    // now we can stop using the global_wdxwinptr crutch since windows are created
    global_wdxwinptr = NULL;  // get rid of any reference to this obj
}

////////////////////////////////////////////////////////////////////
//     Function: config
//       Access:
//  Description:  Set up win32 window.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::config_window(wdxGraphicsWindowGroup *pParentGroup) {
//    assert(_pParentWindowGroup==NULL);
    _pParentWindowGroup=pParentGroup;

    GraphicsWindow::config();

    _hdc = NULL;
    _gsg = _dxgsg = NULL;
    _exiting_window = false;
    _window_active = true;
    _return_control_to_app = false;
    _active_minimized_fullscreen = false;

    if(dx_full_screen || _props._fullscreen) {
        _props._fullscreen = dx_full_screen = true;
    }

    _WindowAdjustingType = NotAdjusting;
    _bSizeIsMaximized=FALSE;

    _gsg = _dxgsg = NULL;
    // Create a GSG to manage the graphics
    if(_gsg==NULL) {
        make_gsg();
        if(_gsg==NULL) {
            wdxdisplay_cat.error() << "make_gsg() failed!\n";
            exit(1);
        }
    }
    _dxgsg = DCAST(DXGraphicsStateGuardian, _gsg);

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
}

void wdxGraphicsWindow::finish_window_setup(void) {
    // Now indicate that we have our keyboard/mouse device ready.
    GraphicsWindowInputDevice device = GraphicsWindowInputDevice::pointer_and_keyboard("keyboard/mouse");
    _input_devices.push_back(device);

    // move windows to top of zorder
    HWND hWin = _dxgsg->scrn.hWnd;

    SetWindowPos(hWin, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOSIZE);
    // call twice to override STARTUPINFO value, which may be set to hidden initially (by emacs for instance)
    ShowWindow(hWin, SW_SHOWNORMAL);
    ShowWindow(hWin, SW_SHOWNORMAL);  
    //  UpdateWindow( _mwindow );
}



HRESULT CALLBACK EnumDevicesCallback(LPSTR pDeviceDescription, LPSTR pDeviceName,
                                     LPD3DDEVICEDESC7 pD3DDeviceDesc,LPVOID pContext) {
    D3DDEVICEDESC7 *pd3ddevs = (D3DDEVICEDESC7 *)pContext;
#ifdef _DEBUG
    wdxdisplay_cat.spam() << "Enumerating Device " << pDeviceName << " : " << pDeviceDescription << endl;
#endif


#define REGHALIDX 0
#define TNLHALIDX 1
#define SWRASTIDX 2

    if(IsEqualGUID(pD3DDeviceDesc->deviceGUID,IID_IDirect3DHALDevice)) {
        CopyMemory(&pd3ddevs[REGHALIDX],pD3DDeviceDesc,sizeof(D3DDEVICEDESC7));
    } else if(IsEqualGUID(pD3DDeviceDesc->deviceGUID,IID_IDirect3DTnLHalDevice)) {
        CopyMemory(&pd3ddevs[TNLHALIDX],pD3DDeviceDesc,sizeof(D3DDEVICEDESC7));
    } else if(IsEqualGUID(pD3DDeviceDesc->deviceGUID,IID_IDirect3DRGBDevice)) {
        CopyMemory(&pd3ddevs[SWRASTIDX],pD3DDeviceDesc,sizeof(D3DDEVICEDESC7));
    }
    return DDENUMRET_OK;
}

#define MAX_DISPLAY_MODES 100  // probably dont need this much, since i already screen for width&hgt
typedef struct {
  DWORD maxWidth,maxHeight;
  DWORD supportedBitDepths;    // uses DDBD_* flags
  LPDDSURFACEDESC2 pDDSD_Arr;
  DWORD cNumSurfDescs;
} DisplayModeInfo;

HRESULT WINAPI EnumDisplayModesCallBack(LPDDSURFACEDESC2 lpDDSurfaceDesc,LPVOID lpContext) {
    DisplayModeInfo *pDMI = (DisplayModeInfo *) lpContext;

    // ddsd_search should assure this is true
    assert((lpDDSurfaceDesc->dwWidth == pDMI->maxWidth) && (lpDDSurfaceDesc->dwHeight == pDMI->maxHeight));

    // ignore refresh rates under 60Hz (and special values of 0 & 1)
    if((lpDDSurfaceDesc->dwRefreshRate>1) && (lpDDSurfaceDesc->dwRefreshRate<60))
      return DDENUMRET_OK;

    assert(pDMI->cNumSurfDescs < MAX_DISPLAY_MODES);
    memcpy( &(pDMI->pDDSD_Arr[pDMI->cNumSurfDescs]), lpDDSurfaceDesc, sizeof(DDSURFACEDESC2) );
    pDMI->cNumSurfDescs++;
    pDMI->supportedBitDepths |= BitDepth_2_DDBDMask(lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount);

    return DDENUMRET_OK;
}

/*
BOOL WINAPI DriverEnumCallback_Voodoo1( GUID* pGUID, TCHAR* strDesc,TCHAR* strName,
                                VOID *argptr, HMONITOR hm) {

    if(hm!=NULL)  // skip over non-primary and non-voodoo-type display devices
        return DDENUMRET_OK;

    GUID null_guid;
    ZeroMemory(&null_guid,sizeof(GUID));

    // primary display driver will have NULL guid
    // ignore that and save any non-null value, which
    // indicates a secondary driver, which is usually voodoo1/2
    if((pGUID!=NULL) && !IsEqualGUID(null_guid,*pGUID)) {
        memcpy(argptr,pGUID,sizeof(GUID));
    }

    return DDENUMRET_OK;
}
*/

BOOL WINAPI save_devinfo( GUID* pGUID, TCHAR* strDesc,TCHAR* strName,VOID *argptr, HMONITOR hm) {

    DXDeviceInfoVec *pDevInfoArr = (DXDeviceInfoVec *) argptr;
   
    DXDeviceInfo devinfo;
    ZeroMemory(&devinfo,sizeof(devinfo));

    // primary display driver will have NULL guid
    if(pGUID!=NULL) {
        memcpy(&devinfo.guidDeviceIdentifier,pGUID,sizeof(GUID));
    }
    if(strDesc!=NULL) {
        _tcsncpy(devinfo.szDescription,
            strDesc,
            MAX_DDDEVICEID_STRING);
    }
    if(strName!=NULL) {
        _tcsncpy(devinfo.szDriver,strName,MAX_DDDEVICEID_STRING);
    }
    devinfo.hMon=hm;

    pDevInfoArr->push_back(devinfo);
    return DDENUMRET_OK;
}

BOOL WINAPI DriverEnumCallback_MultiMon( GUID* pGUID, TCHAR* strDesc,TCHAR* strName,VOID *argptr, HMONITOR hm) {
    if(hm==NULL) {
         // skip over the 'primary' since it will duplicated later as an explicit device
        return DDENUMRET_OK;
    }

    return save_devinfo(pGUID,strDesc,strName,argptr,hm);
}

bool wdxGraphicsWindow::resize(unsigned int xsize,unsigned int ysize) {

   if (!_props._fullscreen) {
       if(wdxdisplay_cat.is_debug())
          wdxdisplay_cat.debug() << "resize("<<xsize<<","<<ysize<<") called\n";

        // is this enough?
        SetWindowPos(_dxgsg->scrn.hWnd, NULL, 0,0, xsize, ysize, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING);
        // WM_ERASEBKGND will be ignored, because _WindowAdjustingType!=NotAdjusting because 
        // we dont want to redraw as user is manually resizing window, so need to force explicit
        // background clear for the programmatic resize fn call
         _WindowAdjustingType=NotAdjusting;
        
         // this doesnt seem to be working in toontown resize, so I put ddraw blackblt in handle_reshape instead
         //window_proc(_mwindow, WM_ERASEBKGND,(WPARAM)_hdc,0x0);  
        handle_reshape(true);
        return true;
   }

   if(wdxdisplay_cat.is_info())
      wdxdisplay_cat.info() << "resize("<<xsize<<","<<ysize<<") called\n";

   _dxgsg->SetDXReady(false);

   HRESULT hr;

   DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd_curmode);

   if(FAILED(hr = _dxgsg->scrn.pDD->GetDisplayMode(&ddsd_curmode))) {
       wdxdisplay_cat.fatal() << "resize() - GetDisplayMode failed, result = " << ConvD3DErrorToString(hr) << endl;
       exit(1);
   }

   DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd_search);

   ddsd_search.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
   ddsd_search.dwWidth=xsize;  ddsd_search.dwHeight=ysize;

   // not requesting same refresh rate since changing res might not support same refresh rate

   DDSURFACEDESC2 DDSD_Arr[MAX_DISPLAY_MODES];
   DisplayModeInfo DMI;
   ZeroMemory(&DDSD_Arr,sizeof(DDSD_Arr));
   ZeroMemory(&DMI,sizeof(DMI));
   DMI.maxWidth=xsize;  DMI.maxHeight=ysize;
   DMI.pDDSD_Arr=DDSD_Arr;

   if(FAILED(hr = _dxgsg->scrn.pDD->EnumDisplayModes(DDEDM_REFRESHRATES,&ddsd_search,&DMI,EnumDisplayModesCallBack))) {
       wdxdisplay_cat.fatal() << "resize() - EnumDisplayModes failed, result = " << ConvD3DErrorToString(hr) << endl;
       return false;
   }

   DMI.supportedBitDepths &= _dxgsg->scrn.D3DDevDesc.dwDeviceRenderBitDepth;

   DWORD dwFullScreenBitDepth;
   DWORD requested_bpp=ddsd_curmode.ddpfPixelFormat.dwRGBBitCount;

   // would like to match current bpp first.  if that is not possible, try 16bpp, then 32
   DWORD requested_bpp_DDBD = BitDepth_2_DDBDMask(requested_bpp);

   if(DMI.supportedBitDepths & requested_bpp_DDBD) {
       dwFullScreenBitDepth=requested_bpp;
   } else if(DMI.supportedBitDepths & DDBD_16) {
       dwFullScreenBitDepth=16;
   } else if(DMI.supportedBitDepths & DDBD_32) {
       dwFullScreenBitDepth=32;
   } else {
       wdxdisplay_cat.error()
          << "resize failed, no fullScreen resolutions at " << xsize << "x" << ysize << endl;
       return false;
   }

   if(FAILED(hr = _dxgsg->scrn.pDD->TestCooperativeLevel())) {
        wdxdisplay_cat.error() << "TestCooperativeLevel failed : result = " << ConvD3DErrorToString(hr) << endl;
        wdxdisplay_cat.error() << "Full screen app failed to get exclusive mode on resize, exiting..\n";
        return false;
   }

   _dxgsg->free_dxgsg_objects();

   // let driver choose default refresh rate (hopefully its >=60Hz)   
   if(FAILED( hr = _dxgsg->scrn.pDD->SetDisplayMode( xsize,ysize,dwFullScreenBitDepth, 0L, 0L ))) {
        wdxdisplay_cat.error() << "resize failed to reset display mode to (" << xsize <<"x"<<ysize<<"x"<<dwFullScreenBitDepth<<"): result = " << ConvD3DErrorToString(hr) << endl;
   }

   if(wdxdisplay_cat.is_debug()) {
      DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd34); 
      _dxgsg->scrn.pDD->GetDisplayMode(&ddsd34);
      wdxdisplay_cat.debug() << "set displaymode to " << ddsd34.dwWidth << "x" << ddsd34.dwHeight << " at "<< ddsd34.ddpfPixelFormat.dwRGBBitCount << "bpp, " << ddsd34.dwRefreshRate<< "Hz\n";
   }

   _dxgsg->scrn.dwRenderWidth=xsize;
   _dxgsg->scrn.dwRenderHeight=ysize;

   CreateScreenBuffersAndDevice(_dxgsg->scrn);
   _dxgsg->RecreateAllVideoSurfaces();
   _dxgsg->SetDXReady(true);
   return true;
}


// overrides of the general estimator for known working cases
bool wdxGraphicsWindow::
special_check_fullscreen_resolution(UINT xsize,UINT ysize) {
    assert(IS_VALID_PTR(_dxgsg));

    DWORD VendorId=_dxgsg->scrn.DXDeviceID.dwVendorId;
    DWORD DeviceId=_dxgsg->scrn.DXDeviceID.dwDeviceId;
    switch(VendorId) {
        case 0x8086:  // Intel
            // Intel i810,i815
            if((DeviceId==0x7121)||(DeviceId==0x7121)||(DeviceId==0x1132)) {
                if((xsize==800)&&(ysize==600))
                    return true;
                if((xsize==1024)&&(ysize==768))
                    return true;
            }
            break;
    }

    return false;
}

unsigned int wdxGraphicsWindow::
verify_window_sizes(unsigned int numsizes,unsigned int *dimen) {
   DWORD num_valid_modes=0;
   HRESULT hr;

   // not requesting same refresh rate since changing res might not support same refresh rate at new size

   DDSURFACEDESC2 DDSD_Arr[MAX_DISPLAY_MODES];
   DisplayModeInfo DMI;
   DWORD i,*pCurDim=(DWORD *)dimen;

   for(i=0;i<numsizes;i++,pCurDim+=2) {
      DWORD xsize=pCurDim[0];
      DWORD ysize=pCurDim[1];

       DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd_search);
       ddsd_search.dwFlags = DDSD_HEIGHT | DDSD_WIDTH; //| DDSD_PIXELFORMAT;
       ddsd_search.dwWidth=xsize; ddsd_search.dwHeight=ysize;
    
       ZeroMemory(&DDSD_Arr,sizeof(DDSD_Arr));
       ZeroMemory(&DMI,sizeof(DMI));
       DMI.maxWidth=xsize;  DMI.maxHeight=ysize;
       DMI.pDDSD_Arr=DDSD_Arr;
    
       if(FAILED(hr = _dxgsg->scrn.pDD->EnumDisplayModes(DDEDM_REFRESHRATES,&ddsd_search,&DMI,EnumDisplayModesCallBack))) {
           wdxdisplay_cat.fatal() << "resize() - EnumDisplayModes failed, result = " << ConvD3DErrorToString(hr) << endl;
           return 0;
       }
    
       // get rid of bpp's we cant render at
       DMI.supportedBitDepths &= _dxgsg->scrn.D3DDevDesc.dwDeviceRenderBitDepth;

       bool bIsGoodMode=false;

       if(special_check_fullscreen_resolution(xsize,ysize)) {
           // bypass the test below for certain cards we know have valid modes
           bIsGoodMode=true;
       } else {
           if(_dxgsg->scrn.bIsLowVidMemCard) 
               bIsGoodMode=(((float)xsize*(float)ysize)<=(float)(640*480));
             else if(DMI.supportedBitDepths & (DDBD_16 | DDBD_24 | DDBD_32)) {
                 // assume user is testing fullscreen, not windowed, so use the dwTotal value
                 // see if 3 scrnbufs (front/back/z)at 16bpp at xsize*ysize will fit with a few 
                 // extra megs for texmem
    
                 // 8MB Rage Pro says it has 6.8 megs Total free and will run at 1024x768, so
                 // formula makes it so that is OK
    
                 #define REQD_TEXMEM 1800000.0f  
    
                 if(_dxgsg->scrn.MaxAvailVidMem==0) {
                     //assume buggy drivers return bad val of 0 and everything will be OK
                     bIsGoodMode=true;
                 } else {
                     bIsGoodMode = ((((float)xsize*(float)ysize)*6+REQD_TEXMEM) < (float)_dxgsg->scrn.MaxAvailVidMem);
                 }
             }
       }

       if(bIsGoodMode)
         num_valid_modes++;
        else {
            pCurDim[0] = 0;
            pCurDim[1] = 0;
        }
   }

   return num_valid_modes;
}

/*
void wdxGraphicsWindow::
check_for_color_cursor_support(void) {
    // card support for non-black/white GDI cursors varies greatly.  if the cursor is not supported,
    // it is rendered in software by GDI, which causes a flickering effect (because it's not synced 
    // with flip?).  GDI transparently masks what's happening so there is no easy way for app to detect
    // if HW cursor support exists.  alternatives are to tie cursor motion to frame rate using DDraw blts
    // or overlays, or to have separate thread draw cursor (sync issues?).  instead we do mono cursor 
    // unless card is known to support 256 color cursors

    string windows_color_cursor_filename = get_color_cursor_filename().to_os_specific();
    if(windows_color_cursor_filename.empty())
       return;

    bool bSupportsColorCursor=false;

    if(IS_NVIDIA(_DXDeviceID)) {    
        // all nvidia seem to support 256 color
        bSupportsColorCursor=true;
    } else if(IS_ATI(_DXDeviceID)) {
        // radeons seem to be in the 5100 range and support color, assume anything in 6000 or above 
        // is newer than radeon and supports 256 color
        if(((_DXDeviceID.dwDeviceId>=0x5100) && (_DXDeviceID.dwDeviceId<=0x5200)) ||
           (_DXDeviceID.dwDeviceId>=0x6000))
            bSupportsColorCursor=true;
    } else if IS_MATROX(_DXDeviceID) {
        if(_DXDeviceID.dwDeviceId==0x0525)   // G400 seems to support color cursors, havent tested other matrox
            bSupportsColorCursor=true;
    }

    // TODO: add more cards as more testing is done

    if(bSupportsColorCursor) {
        // Note: LoadImage seems to cause win2k internal heap corruption (outputdbgstr warnings)
        // if icon is more than 8bpp

        DWORD load_flags = LR_LOADFROMFILE;

        if(dx_full_screen) {
            // I think cursors should use LR_CREATEDIBSECTION since they should not be mapped to the device palette (in the case of 256-color cursors)
            // since they are not going to be used on the desktop
            load_flags |= LR_CREATEDIBSECTION;
        }

        // loads a .cur fmt file. 
        HCURSOR hNewMouseCursor = (HCURSOR) LoadImage(NULL, windows_color_cursor_filename.c_str(), IMAGE_CURSOR, 0, 0, load_flags );

        if(hNewMouseCursor==NULL) {
            wdxdisplay_cat.warning() << "windows color cursor filename '" << windows_color_cursor_filename << "' not found!!\n";
            return;
        }

        SetClassLongPtr(_mwindow, GCLP_HCURSOR, (LONG_PTR) hNewMouseCursor);
        SetCursor(hNewMouseCursor);

        if(_bLoadedCustomCursor)
           DestroyCursor(_hMouseCursor);
        _hMouseCursor = hNewMouseCursor;
    }
}
*/

// returns true if successful
bool wdxGraphicsWindow::search_for_device(int devnum,DXDeviceInfo *pDevinfo) {
    DWORD dwRenderWidth  = _props._xsize;
    DWORD dwRenderHeight = _props._ysize;
    LPDIRECTDRAW7 pDD=NULL;
    HRESULT hr;

    assert(_dxgsg!=NULL);

    GUID *pDDDeviceGUID;
    if(pDevinfo==NULL)
       pDDDeviceGUID=NULL;
     else pDDDeviceGUID=&pDevinfo->guidDeviceIdentifier;

    assert(_pParentWindowGroup->_pDDCreateEx!=NULL);

    // Create the Direct Draw Objects
    hr = (*(_pParentWindowGroup->_pDDCreateEx))(pDDDeviceGUID,(void **)&pDD, IID_IDirectDraw7, NULL);
    if((hr != DD_OK)||(pDD==NULL)) {
          wdxdisplay_cat.fatal() << "DirectDrawCreateEx failed for monitor("<<devnum<< "): result = " << ConvD3DErrorToString(hr) << endl;
          return false;
    }

    _dxgsg->scrn.pDD=pDD;

    //GetDeviceID bug writes an extra 4 bytes, so need xtra space
    BYTE id_arr[sizeof(DDDEVICEIDENTIFIER2)+4];
    pDD->GetDeviceIdentifier((DDDEVICEIDENTIFIER2 *)&id_arr,0x0);

    memcpy(&_dxgsg->scrn.DXDeviceID,id_arr,sizeof(DDDEVICEIDENTIFIER2));

    if(wdxdisplay_cat.is_info()) {
       DDDEVICEIDENTIFIER2 *pDevID=&_dxgsg->scrn.DXDeviceID;
       wdxdisplay_cat.info() << "GfxCard: " << pDevID->szDescription <<  "; DriverFile: '" << pDevID->szDriver  
                             << "'; VendorID: " <<  pDevID->dwVendorId << "; DriverVer: " 
                             << HIWORD(pDevID->liDriverVersion.HighPart) << "." 
                             << LOWORD(pDevID->liDriverVersion.HighPart) << "."
                             << HIWORD(pDevID->liDriverVersion.LowPart) << "." 
                             << LOWORD(pDevID->liDriverVersion.LowPart) << endl;
    }

    // Query DirectDraw for access to Direct3D
    hr = pDD->QueryInterface( IID_IDirect3D7, (VOID**)&_dxgsg->scrn.pD3D);
    if(hr != DD_OK) {
        wdxdisplay_cat.fatal() << "QI for D3D failed : result = " << ConvD3DErrorToString(hr) << endl;
        goto error_exit;
    }

    D3DDEVICEDESC7 d3ddevs[3];  // put HAL in 0, TnLHAL in 1, SW rast in 2

    // just look for HAL and TnL devices right now.  I dont think
    // we have any interest in the sw rasts at this point

    ZeroMemory(d3ddevs,3*sizeof(D3DDEVICEDESC7));

    hr = _dxgsg->scrn.pD3D->EnumDevices(EnumDevicesCallback,d3ddevs);
    if(hr != DD_OK) {
       wdxdisplay_cat.fatal() << "EnumDevices failed : result = " << ConvD3DErrorToString(hr) << endl;
        goto error_exit;
    }
    
    WORD DeviceIdx;

    // select TNL if present
    if(d3ddevs[TNLHALIDX].dwDevCaps & D3DDEVCAPS_HWRASTERIZATION) {
       DeviceIdx=TNLHALIDX;
    } else if(d3ddevs[REGHALIDX].dwDevCaps & D3DDEVCAPS_HWRASTERIZATION) {
       DeviceIdx=REGHALIDX;
    } else if(dx_allow_software_renderer || dx_force_software_renderer) {
       DeviceIdx=SWRASTIDX;      
    } else {
       wdxdisplay_cat.error() << "No 3D HW present on device #"<<devnum<<", skipping it... (" << _dxgsg->scrn.DXDeviceID.szDescription<<")\n";
       goto error_exit;
    }

    if(dx_force_software_renderer) {
       DeviceIdx=SWRASTIDX; 
    }
    
    memcpy(&_dxgsg->scrn.D3DDevDesc,&d3ddevs[DeviceIdx],sizeof(D3DDEVICEDESC7));

    _dxgsg->scrn.bIsTNLDevice=(DeviceIdx==TNLHALIDX);

    // Get Current VidMem avail.  Note this is only an estimate, when we switch to fullscreen
    // mode from desktop, more vidmem will be available (typically 1.2 meg).  I dont want
    // to switch to fullscreen more than once due to the annoying monitor flicker, so try
    // to figure out optimal mode using this estimate
    DDSCAPS2 ddsGAVMCaps;
    DWORD dwVidMemTotal,dwVidMemFree;
    dwVidMemTotal=dwVidMemFree=0;
    ZeroMemory(&ddsGAVMCaps,sizeof(DDSCAPS2));
    ddsGAVMCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;  // dont count AGP mem!
    if(FAILED(hr = pDD->GetAvailableVidMem(&ddsGAVMCaps,&dwVidMemTotal,&dwVidMemFree))) {
       wdxdisplay_cat.error() << "GetAvailableVidMem failed for device #"<<devnum<<": result = " << ConvD3DErrorToString(hr) << endl;
       // goto skip_device;
       exit(1);  // probably want to exit, since it may be my fault
    }
    
    // after SetDisplayMode, GetAvailVidMem totalmem seems to go down by 1.2 meg (contradicting above
    // comment and what I think would be correct behavior (shouldnt FS mode release the desktop vidmem?),
    // so this is the true value
    _dxgsg->scrn.MaxAvailVidMem = dwVidMemTotal;
    
    #define LOWVIDMEMTHRESHOLD 3500000
    #define CRAPPY_DRIVER_IS_LYING_VIDMEMTHRESHOLD 1000000     // every vidcard we deal with should have at least 1MB
    
    // assume buggy drivers (this means you, FireGL2) may return zero for dwVidMemTotal, so ignore value if its < CRAPPY_DRIVER_IS_LYING_VIDMEMTHRESHOLD
    _dxgsg->scrn.bIsLowVidMemCard = ((dwVidMemTotal>CRAPPY_DRIVER_IS_LYING_VIDMEMTHRESHOLD) && (dwVidMemTotal< LOWVIDMEMTHRESHOLD));   

    if(!dx_do_vidmemsize_check) {
       _dxgsg->scrn.MaxAvailVidMem = 0xFFFFFFFF;
       _dxgsg->scrn.bIsLowVidMemCard = false; 
    }

    if(DeviceIdx==SWRASTIDX) {
        // this will force 640x480x16, is this what we want for all sw rast?
        _dxgsg->scrn.bIsLowVidMemCard = true; 
        _dxgsg->scrn.bIsSWRast = true; 
        dx_force_16bpp_zbuffer = true;
    }

    if(dx_full_screen) {
        _props._xorg = _props._yorg = 0;
        
        DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd_search);
        ddsd_search.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
        ddsd_search.dwWidth=dwRenderWidth;  ddsd_search.dwHeight=dwRenderHeight;
        
        DDSURFACEDESC2 DDSD_Arr[MAX_DISPLAY_MODES];
        DisplayModeInfo DMI;
        ZeroMemory(&DDSD_Arr,sizeof(DDSD_Arr));
        ZeroMemory(&DMI,sizeof(DMI));
        DMI.maxWidth=dwRenderWidth;  DMI.maxHeight=dwRenderHeight;
        DMI.pDDSD_Arr=DDSD_Arr;
        
        if(FAILED(hr= pDD->EnumDisplayModes(DDEDM_REFRESHRATES,&ddsd_search,&DMI,EnumDisplayModesCallBack))) {
           wdxdisplay_cat.fatal() << "EnumDisplayModes failed for device #" << devnum << " (" << _dxgsg->scrn.DXDeviceID.szDescription<<"), result = " << ConvD3DErrorToString(hr) << endl;
           // goto skip_device;
           exit(1);  // probably want to exit, since it may be my fault
        }
        
        if(wdxdisplay_cat.is_info())
           wdxdisplay_cat.info() << "Before fullscreen switch: GetAvailableVidMem for device #"<<devnum<<" returns Total: " << dwVidMemTotal/1000000.0 << "  Free: " << dwVidMemFree/1000000.0 << endl;
        
        // Now we try to figure out if we can use requested screen resolution and best
        // rendertarget bpp and still have at least 2 meg of texture vidmem
        
        DMI.supportedBitDepths &= _dxgsg->scrn.D3DDevDesc.dwDeviceRenderBitDepth;

        DWORD dwFullScreenBitDepth;

        // note: this chooses 32bpp, which may not be preferred over 16 for memory & speed reasons
        if(DMI.supportedBitDepths & DDBD_32) {
           dwFullScreenBitDepth=32;              // go for 32bpp if its avail
        } else if(DMI.supportedBitDepths & DDBD_24) {
           dwFullScreenBitDepth=24;              // go for 24bpp if its avail
        } else if(DMI.supportedBitDepths & DDBD_16) {
           dwFullScreenBitDepth=16;              // do 16bpp
        } else {
           wdxdisplay_cat.fatal() << "No Supported FullScreen resolutions at " << dwRenderWidth << "x" << dwRenderHeight 
               << " for device #" << devnum << " (" << _dxgsg->scrn.DXDeviceID.szDescription<<"), skipping device...\n";
          goto error_exit;
        }
        
        if(_dxgsg->scrn.bIsLowVidMemCard) {
           {
               // hack: figuring out exactly what res to use is tricky, instead I will
               // just use 640x480 if we have < 3 meg avail
    
               dwFullScreenBitDepth=16; 
               dwRenderWidth=640;
               dwRenderHeight=480;
               dx_force_16bpptextures = true;
        
               if(wdxdisplay_cat.is_info())
                   wdxdisplay_cat.info() << "Available VidMem (" << dwVidMemFree<<") is under " << LOWVIDMEMTHRESHOLD <<", using 640x480 16bpp rendertargets to save tex vidmem.\n";
           }
        
               #if 0
                 /*
                   // cant use this method without more accurate way to estimate mem used before actually switching
                   // to that fullscrn mode.  simply computing memsize based on GetDisplayMode doesnt seem
                   // to be accurate within more than 1 meg
            
                           // we think we need to reserve at least 2 megs of vidmem for textures.
                           // to do this, reduce buffer bitdepth if possible
                   #define RESERVEDTEXVIDMEM 2000000
        
                   int rendertargetmem=dwRenderWidth*dwRenderHeight*(dwFullScreenBitDepth>>3);
                   int memleft = dwFree-rendertargetmem*2;   //*2 to handle backbuf/zbuf
        
                   if(memleft < RESERVEDTEXVIDMEM) {
                       dwFullScreenBitDepth=16;
                       wdxdisplay_cat.debug() << "using 16bpp rendertargets to save tex vidmem\n";
                       assert((DMI.supportedBitDepths & DDBD_16) && (pD3DDevDesc->dwDeviceRenderBitDepth & DDBD_16));   // probably a safe assumption
                       rendertargetmem=dwRenderWidth*dwRenderHeight*(dwFullScreenBitDepth>>3);
                       memleft = dwFree-rendertargetmem*2;
        
                        // BUGBUG:  if we still cant reserve 2 megs of vidmem, need to auto-reduce the scrn res
                       if(memleft < RESERVEDTEXVIDMEM)
                           wdxdisplay_cat.debug() << "XXXX WARNING: cant reserve 2MB of tex vidmem. only " << memleft << " bytes available. Need to rewrite wdxdisplay to try lower resolutions  XXXXXXXX\n";
                   }
                 */
               #endif
        }

        _dxgsg->scrn.dwFullScreenBitDepth=dwFullScreenBitDepth;
    }
    
    _dxgsg->scrn.dwRenderWidth=dwRenderWidth;
    _dxgsg->scrn.dwRenderHeight=dwRenderHeight;
    if(pDevinfo)
        _dxgsg->scrn.hMon=pDevinfo->hMon;
    _dxgsg->scrn.CardIDNum=devnum;  // add ID tag for dbgprint purposes

    return true;
    
    // handle errors within this for device loop
    
    error_exit:
       if(_dxgsg->scrn.pD3D!=NULL)
         _dxgsg->scrn.pD3D->Release();
       if(_dxgsg->scrn.pDD!=NULL)
         _dxgsg->scrn.pDD->Release();

       _dxgsg->scrn.pDD=NULL;
       _dxgsg->scrn.pD3D=NULL;
       return false;
}

void wdxGraphicsWindowGroup::
SetCoopLevelsAndDisplayModes(void) {
    HRESULT hr;
    DWORD SCL_FPUFlag;

    if(dx_preserve_fpu_state)
      SCL_FPUFlag = DDSCL_FPUPRESERVE;  // tell d3d to preserve the fpu state across calls.  this hurts perf, but is good for dbgging
    else SCL_FPUFlag = DDSCL_FPUSETUP;

    DXScreenData *pScrn=&_windows[0]->_dxgsg->scrn;
    // All SetCoopLevels must use the parent window

    if(!dx_full_screen) {
        if(FAILED(hr = pScrn->pDD->SetCooperativeLevel(_hParentWindow, SCL_FPUFlag | DDSCL_NORMAL))) {
            wdxdisplay_cat.fatal() << "SetCooperativeLevel failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }
        return; 
    }
    
    DWORD SCL_FLAGS = SCL_FPUFlag | DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT;
    
    if(_windows.size()>1) {
       SCL_FLAGS |= DDSCL_SETDEVICEWINDOW;
    }

    for(DWORD devnum=0;devnum<_windows.size();devnum++) {
       DXScreenData *pScrn=&_windows[devnum]->_dxgsg->scrn;

       // need to set focus/device windows for multimon
       // focus window is primary monitor that will receive keybd input
       // all ddraw objs need to have same focus window
       if(_windows.size()>1) {    
           if(FAILED(hr = pScrn->pDD->SetCooperativeLevel(_hParentWindow, DDSCL_SETFOCUSWINDOW))) {
               wdxdisplay_cat.fatal() << "SetCooperativeLevel SetFocusWindow failed on device 0: result = " << ConvD3DErrorToString(hr) << endl;
               exit(1);
           }
       }

       // s3 savage2000 on w95 seems to set EXCLUSIVE_MODE only if you call SetCoopLevel twice.
       // so we do it, it really shouldnt be necessary if drivers werent buggy
       for(int jj=0;jj<2;jj++) {
           if(FAILED(hr = pScrn->pDD->SetCooperativeLevel(pScrn->hWnd, SCL_FLAGS))) {
               wdxdisplay_cat.fatal() << "SetCooperativeLevel failed for device #"<< devnum<<": result = " << ConvD3DErrorToString(hr) << endl;
               exit(1);
           }
       }
    
       if(FAILED(hr = pScrn->pDD->TestCooperativeLevel())) {
           wdxdisplay_cat.fatal() << "TestCooperativeLevel failed for device #"<< devnum<<": result = " << ConvD3DErrorToString(hr) << endl;
           wdxdisplay_cat.fatal() << "Full screen app failed to get exclusive mode on init, exiting..\n";
           exit(1);
       }
    
       // note: its important we call SetDisplayMode on all cards before creating surfaces on any of them
       // let driver choose default refresh rate (hopefully its >=60Hz)
       if(FAILED( hr = pScrn->pDD->SetDisplayMode( pScrn->dwRenderWidth, pScrn->dwRenderHeight,
                                            pScrn->dwFullScreenBitDepth, 0, 0 ))) {
           wdxdisplay_cat.fatal() << "SetDisplayMode failed to set ("<<pScrn->dwRenderWidth<<"x"<<pScrn->dwRenderHeight<<"x"<<pScrn->dwFullScreenBitDepth<<") on device #"<< pScrn->CardIDNum<<": result = " << ConvD3DErrorToString(hr) << endl;
           exit(1);
       }
    
       if(wdxdisplay_cat.is_debug()) {
          DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd34); 
          pScrn->pDD->GetDisplayMode(&ddsd34);
          wdxdisplay_cat.debug() << "set displaymode to " << ddsd34.dwWidth << "x" << ddsd34.dwHeight << " at "<< ddsd34.ddpfPixelFormat.dwRGBBitCount << "bpp, " << ddsd34.dwRefreshRate<< "Hz\n";
    
         /*
         #ifdef _DEBUG
          if(FAILED(hr = (*Disply).pDD->GetAvailableVidMem(&ddsGAVMCaps,&dwVidMemTotal,&dwVidMemFree))) {
              wdxdisplay_cat.debug() << "GetAvailableVidMem failed : result = " << ConvD3DErrorToString(hr) << endl;
              exit(1);
          }
          wdxdisplay_cat.debug() << "after fullscreen switch: GetAvailableVidMem returns Total: " << dwVidMemTotal/1000000.0 << "  Free: " << dwVidMemFree/1000000.0 << endl;
         #endif
         */
       }
    }
}

//return true if successful
void wdxGraphicsWindow::
CreateScreenBuffersAndDevice(DXScreenData &Display) {

    DWORD dwRenderWidth=Display.dwRenderWidth;
    DWORD dwRenderHeight=Display.dwRenderHeight;
    LPDIRECT3D7 pD3DI=Display.pD3D;
    LPDIRECTDRAW7 pDD=Display.pDD;
    D3DDEVICEDESC7 *pD3DDevDesc=&Display.D3DDevDesc;

    LPDIRECTDRAWSURFACE7 pPrimaryDDSurf,pBackDDSurf,pZDDSurf;
    LPDIRECT3DDEVICE7 pD3DDevice;
    RECT view_rect;
    int i;
    HRESULT hr;
    DX_DECLARE_CLEAN( DDSURFACEDESC2, SurfaceDesc );

    assert(pDD!=NULL);
    assert(pD3DI!=NULL);

/*
    // select the best device if the caller does not provide one
    D3DDEVICEDESC7 d3ddevs[2];  // put HAL in 0, TnLHAL in 1    

    if(pD3DDevDesc==NULL) {
        // just look for HAL and TnL devices right now.  I dont think
        // we have any interest in the sw rasts at this point

        ZeroMemory(d3ddevs,2*sizeof(D3DDEVICEDESC7));
    
        hr = pD3DI->EnumDevices(EnumDevicesCallback,d3ddevs);
        if(hr != DD_OK) {
            wdxdisplay_cat.fatal() << "EnumDevices failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        WORD DeviceIdx=REGHALIDX;

        if(!(d3ddevs[DeviceIdx].dwDevCaps & D3DDEVCAPS_HWRASTERIZATION )) {
            wdxdisplay_cat.fatal() << "No 3D HW present, exiting..." << endl;
            exit(1);
        }

        // select TNL if present
        if(d3ddevs[TNLHALIDX].dwDevCaps & D3DDEVCAPS_HWRASTERIZATION) {
            DeviceIdx=TNLHALIDX;
        }

        pD3DDevDesc=&d3ddevs[DeviceIdx];
    }
*/  

   DX_DECLARE_CLEAN(DDCAPS,DDCaps);
   pDD->GetCaps(&DDCaps,NULL);

   // if window is not foreground in exclusive mode, ddraw thinks you are 'not active', so
   // it changes your WM_ACTIVATEAPP from true to false, causing us
   // to go into a 'wait-for WM_ACTIVATEAPP true' loop, and the event never comes so we hang
   // in fullscreen wait.

   SetForegroundWindow(Display.hWnd);

   if(dx_full_screen) {
        // Setup to create the primary surface w/backbuffer
        DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd)
        ddsd.dwFlags           = DDSD_CAPS|DDSD_BACKBUFFERCOUNT;
        ddsd.ddsCaps.dwCaps    = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE |
                                 DDSCAPS_FLIP | DDSCAPS_COMPLEX;
        ddsd.dwBackBufferCount = 1;

        if(dx_full_screen_antialiasing) {
            // cant check that d3ddevice has this capability yet, so got to set it anyway.
            // hope this is OK.
            ddsd.ddsCaps.dwCaps2 |= DDSCAPS2_HINTANTIALIASING; 
        }

        PRINTVIDMEM(pDD,&ddsd.ddsCaps,"initial primary & backbuf");

        // Create the primary surface
        if(FAILED( hr = pDD->CreateSurface( &ddsd, &pPrimaryDDSurf, NULL ) )) {
            wdxdisplay_cat.fatal() << "CreateSurface failed for primary surface: result = " << ConvD3DErrorToString(hr) << endl;

            if(((hr==DDERR_OUTOFVIDEOMEMORY)||(hr==DDERR_OUTOFMEMORY)) &&
               (Display.dwFullScreenBitDepth>16)) {
                // emergency fallback to 16bpp (shouldnt have to do this unless GetAvailVidMem lied)
                // will this work for multimon?  what if surfs are already created on 1st mon?
                Display.dwFullScreenBitDepth=16;

                if(wdxdisplay_cat.info())
                    wdxdisplay_cat.info() << "GetAvailVidMem lied, not enough VidMem for 32bpp, so trying 16bpp on device #"<< Display.CardIDNum<< endl;

                if(FAILED( hr = pDD->SetDisplayMode( Display.dwRenderWidth, Display.dwRenderHeight,Display.dwFullScreenBitDepth, 0, 0 ))) {
                    wdxdisplay_cat.fatal() << "SetDisplayMode failed to set ("<<Display.dwRenderWidth<<"x"<<Display.dwRenderHeight<<"x"<<Display.dwFullScreenBitDepth<<") on device #"<< Display.CardIDNum<<": result = " << ConvD3DErrorToString(hr) << endl;
                    exit(1);
                }
                bool saved_value=dx_force_16bpp_zbuffer;
                dx_force_16bpp_zbuffer=true;
                CreateScreenBuffersAndDevice(Display);
                dx_force_16bpp_zbuffer=saved_value;
                return;
            } else exit(1);
        }

        // Clear the primary surface to black

        DX_DECLARE_CLEAN(DDBLTFX, bltfx)
        bltfx.dwDDFX |= DDBLTFX_NOTEARING;
        hr = pPrimaryDDSurf->Blt(NULL,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bltfx);

        if(FAILED(hr)) {
            wdxdisplay_cat.fatal() << "Blt to Black of Primary Surf failed! : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        // Get the backbuffer, which was created along with the primary.
        DDSCAPS2 ddscaps = { DDSCAPS_BACKBUFFER, 0, 0, 0};
        if(FAILED( hr = pPrimaryDDSurf->GetAttachedSurface( &ddscaps, &pBackDDSurf ) )) {
            wdxdisplay_cat.fatal() << "Can't get the backbuffer: result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        if(FAILED( hr = pBackDDSurf->Blt(NULL,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bltfx))) {
            wdxdisplay_cat.fatal() << "Blt to Black of Back Surf failed! : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        SetRect(&view_rect, 0, 0, dwRenderWidth, dwRenderHeight);
    }   // end create full screen buffers

    else {          // CREATE WINDOWED BUFFERS

        if(!(DDCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED)) {
            wdxdisplay_cat.fatal() << "the 3D HW cannot render windowed, exiting..." << endl;
            exit(1);
        }

        if(FAILED(hr = pDD->GetDisplayMode( &SurfaceDesc ))) {
            wdxdisplay_cat.fatal() << "GetDisplayMode failed result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }
        if(SurfaceDesc.ddpfPixelFormat.dwRGBBitCount <= 8) {
            wdxdisplay_cat.fatal() << "Can't run windowed in an 8-bit or less display mode" << endl;
            exit(1);
        }

        if(!(BitDepth_2_DDBDMask(SurfaceDesc.ddpfPixelFormat.dwRGBBitCount) & pD3DDevDesc->dwDeviceRenderBitDepth)) {
            wdxdisplay_cat.fatal() << "3D Device doesnt support rendering at " << SurfaceDesc.ddpfPixelFormat.dwRGBBitCount << "bpp (current desktop bitdepth)" << endl;
            exit(1);
        }

     
        // Get the dimensions of the viewport and screen bounds

        GetClientRect( Display.hWnd, &view_rect );
        POINT ul,lr;
        ul.x=view_rect.left;  ul.y=view_rect.top;
        lr.x=view_rect.right;  lr.y=view_rect.bottom;
        ClientToScreen(Display.hWnd, &ul );
        ClientToScreen(Display.hWnd, &lr );
        view_rect.left=ul.x; view_rect.top=ul.y;
        view_rect.right=lr.x; view_rect.bottom=lr.y;

        dwRenderWidth  = view_rect.right - view_rect.left;
        dwRenderHeight = view_rect.bottom - view_rect.top;
        _props._xorg = view_rect.left;  // _props should reflect view rectangle
        _props._yorg = view_rect.top;
        _props._xsize = dwRenderWidth;
        _props._ysize = dwRenderHeight;

        // Initialize the description of the primary surface
        ZeroMemory( &SurfaceDesc, sizeof(DDSURFACEDESC2) );
        SurfaceDesc.dwSize         = sizeof(DDSURFACEDESC2);
        SurfaceDesc.dwFlags        = DDSD_CAPS ;
        SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;

        PRINTVIDMEM(pDD,&SurfaceDesc.ddsCaps,"initial primary surface");

        // Create the primary surface.  This includes all of the visible
        // window, so no need to specify height/width
        if(FAILED(hr = pDD->CreateSurface( &SurfaceDesc, &pPrimaryDDSurf, NULL ))) {
            wdxdisplay_cat.fatal()
            << "CreateSurface failed for primary surface: result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        // Create a clipper object which handles all our clipping for cases when
        // our window is partially obscured by other windows.
        LPDIRECTDRAWCLIPPER Clipper;
        if(FAILED(hr = pDD->CreateClipper( 0, &Clipper, NULL ))) {
            wdxdisplay_cat.fatal() << "CreateClipper failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        // Associate the clipper with our window. Note that, afterwards, the
        // clipper is internally referenced by the primary surface, so it is safe
        // to release our local reference to it.
        Clipper->SetHWnd( 0, Display.hWnd );
        pPrimaryDDSurf->SetClipper( Clipper );
        Clipper->Release();
   
        // Clear the primary surface to black
        DX_DECLARE_CLEAN(DDBLTFX, bltfx)

        if(FAILED( hr = pPrimaryDDSurf->Blt(NULL,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bltfx))) {
            wdxdisplay_cat.fatal()
            << "Blt to Black of Primary Surf failed! : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        // Setup a surface description to create a backbuffer.
        SurfaceDesc.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
        SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
        SurfaceDesc.dwWidth  = dwRenderWidth;
        SurfaceDesc.dwHeight = dwRenderHeight;

        if(dx_full_screen_antialiasing) {
            // cant check that d3ddevice has this capability yet, so got to set it anyway.
            // hope this is OK.
            SurfaceDesc.ddsCaps.dwCaps2 |= DDSCAPS2_HINTANTIALIASING; 
        }

        PRINTVIDMEM(pDD,&SurfaceDesc.ddsCaps,"initial backbuf");

        // Create the backbuffer. (might want to handle failure due to running out of video memory)
        if(FAILED(hr = pDD->CreateSurface( &SurfaceDesc, &pBackDDSurf, NULL ))) {
            wdxdisplay_cat.fatal()
            << "CreateSurface failed for backbuffer : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        if(FAILED( hr = pBackDDSurf->Blt(NULL,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bltfx))) {
            wdxdisplay_cat.fatal()
            << "Blt to Black of Back Surf failed! : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

    }  // end create windowed buffers

//  ========================================================

    resized(dwRenderWidth,dwRenderHeight);  // update panda channel/display rgn info

    #ifndef NDEBUG
      if(!(_props._mask & W_DEPTH)) {
        wdxdisplay_cat.info() << "no zbuffer requested, skipping zbuffer creation\n";
      }
    #endif

    // Check if the device supports z-bufferless hidden surface removal. If so,
    // we don't really need a z-buffer
    if((!(pD3DDevDesc->dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR )) &&
       (_props._mask & W_DEPTH)) {

        // Get z-buffer dimensions from the render target
        DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd);
        pBackDDSurf->GetSurfaceDesc( &ddsd );

        // Setup the surface desc for the z-buffer.
        ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
        ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | ((_dxgsg->scrn.bIsSWRast) ?  DDSCAPS_SYSTEMMEMORY : DDSCAPS_VIDEOMEMORY);

        DDPIXELFORMAT ZBufPixFmts[MAX_DX_ZBUF_FMTS];
        cNumZBufFmts=0;

        // Get an appropiate pixel format from enumeration of the formats. On the
        // first pass, we look for a zbuffer dpeth which is equal to the frame
        // buffer depth (as some cards unfornately require this).
        if(FAILED(pD3DI->EnumZBufferFormats((Display.bIsTNLDevice ? IID_IDirect3DHALDevice : IID_IDirect3DTnLHalDevice),
                                            EnumZBufFmtsCallback,
                                            (VOID*)&ZBufPixFmts ))) {
            wdxdisplay_cat.fatal() << "EnumZBufferFormats failed" << endl;
            exit(1);
        }

#ifdef _DEBUG
        { static BOOL bPrinted=FALSE;
            if(!bPrinted) {
                for(i=0;i<cNumZBufFmts;i++) {
                    DebugPrintPixFmt(&ZBufPixFmts[i]);
                }
                bPrinted=TRUE;
            }
        }
#endif

        // int want_depth_bits = _props._want_depth_bits;  should we pay attn to these at some point?
        // int want_color_bits = _props._want_color_bits;
        bool bWantStencil = ((_props._mask & W_STENCIL)!=0);

        LPDDPIXELFORMAT pCurPixFmt,pz16=NULL,pz24=NULL,pz32=NULL;
        for(i=0,pCurPixFmt=ZBufPixFmts;i<cNumZBufFmts;i++,pCurPixFmt++) {
          if(bWantStencil==((pCurPixFmt->dwFlags & DDPF_STENCILBUFFER)!=0)) {
            switch(pCurPixFmt->dwRGBBitCount) {
                case 16:
                    pz16=pCurPixFmt;
                    break;
                case 24:
                    pz24=pCurPixFmt;
                    break;
                case 32:
                    pz32=pCurPixFmt;
                    break;
            }
          }
        }

        if((pz16==NULL)&&(pz24==NULL)&&(pz32==NULL)) {
            if(bWantStencil) 
                wdxdisplay_cat.fatal() << "stencil buffer requested, device has no stencil capability\n";
              else wdxdisplay_cat.fatal() << "failed to find adequate zbuffer capability\n";
            exit(1);
        }

        #define SET_ZBUF_DEPTH(DEPTH) { assert(pz##DEPTH != NULL); Display.depth_buffer_bitdepth=DEPTH; ddsd.ddpfPixelFormat = *pz##DEPTH;}
        
        if(_dxgsg->scrn.bIsSWRast) {
            SET_ZBUF_DEPTH(16);    // need this for fast path rasterizers
        } else
        if(IS_NVIDIA(Display.DXDeviceID)) {
           DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd_pri)
            pPrimaryDDSurf->GetSurfaceDesc(&ddsd_pri);

            // must pick zbuf depth to match primary surface depth for nvidia
            if(ddsd_pri.ddpfPixelFormat.dwRGBBitCount==16) {
                SET_ZBUF_DEPTH(16);
            } else {
                if(dx_force_16bpp_zbuffer) {
                    wdxdisplay_cat.fatal() << "'dx-force-16bpp-zbuffer #t' requires a 16bpp desktop on nvidia cards\n";
                    exit(1);
                }
                // take the smaller of 24 or 32.  (already assured to match stencil capability)
                if(pz24!=NULL) {
                    SET_ZBUF_DEPTH(24);
                } else SET_ZBUF_DEPTH(32);
            }
        } else {
            if(dx_force_16bpp_zbuffer) {
                if(pz16==NULL) {
                    wdxdisplay_cat.fatal() << "'dx-force-16bpp-zbuffer #t', but no 16bpp zbuf fmts available on this card\n";
                    exit(1);
                }

                if(wdxdisplay_cat.is_debug())
                   wdxdisplay_cat.debug() << "forcing use of 16bpp Z-Buffer\n";
                SET_ZBUF_DEPTH(16);
                ddsd.ddpfPixelFormat = *pz16;
            } else {
                // pick the highest res zbuffer format avail.  Note: this is choosing to waste vid-memory
                // and possibly perf for more accuracy, less z-fighting at long distance (std 16bpp would 
                // be smaller// maybe faster)
                // order of preference 24: (should be enough), 32: probably means 24 of Z, then 16

                if(bWantStencil && (pz32!=NULL)) {
                    // dont want to select 16/8 z/stencil over 24/8 z/stenc
                    SET_ZBUF_DEPTH(32);
                } else {
                    if(pz24!=NULL) {
                        SET_ZBUF_DEPTH(24);
                    } else if(pz32!=NULL) {
                        SET_ZBUF_DEPTH(32);
                    } else {
                        SET_ZBUF_DEPTH(16);
                    }
                }
            }
        }

        PRINTVIDMEM(pDD,&ddsd.ddsCaps,"initial zbuf");

#ifdef _DEBUG
        wdxdisplay_cat.info() << "Creating " << ddsd.ddpfPixelFormat.dwRGBBitCount << "bpp zbuffer\n";
#endif

        // Create and attach a z-buffer
        if(FAILED( hr = pDD->CreateSurface( &ddsd, &pZDDSurf, NULL ) )) {
            wdxdisplay_cat.fatal() << "CreateSurface failed for Z buffer: result = " <<  ConvD3DErrorToString(hr) << endl;

            if(((hr==DDERR_OUTOFVIDEOMEMORY)||(hr==DDERR_OUTOFMEMORY)) &&
               ((Display.dwFullScreenBitDepth>16)||(ddsd.ddpfPixelFormat.dwRGBBitCount>16))) {
                Display.dwFullScreenBitDepth=16;
                // emergency fallback to 16bpp (shouldnt have to do this unless GetAvailVidMem lied)
                // will this work for multimon?  what if surfs are already created on 1st mon?

                if(wdxdisplay_cat.info())
                    wdxdisplay_cat.info() << "GetAvailVidMem lied, not enough VidMem for 32bpp, so trying 16bpp on device #"<< Display.CardIDNum<< endl;

                ULONG refcnt;

                // free pri and back (maybe should just free pri since created as complex chain?)
                RELEASE(pBackDDSurf,wdxdisplay,"backbuffer",false);
                RELEASE(pPrimaryDDSurf,wdxdisplay,"primary surface",false);

                if(FAILED( hr = pDD->SetDisplayMode( Display.dwRenderWidth, Display.dwRenderHeight,Display.dwFullScreenBitDepth, 0, 0 ))) {
                    wdxdisplay_cat.fatal() << "SetDisplayMode failed to set ("<<Display.dwRenderWidth<<"x"<<Display.dwRenderHeight<<"x"<<Display.dwFullScreenBitDepth<<") on device #"<< Display.CardIDNum<<": result = " << ConvD3DErrorToString(hr) << endl;
                    exit(1);
                }
                bool saved_value=dx_force_16bpp_zbuffer;
                dx_force_16bpp_zbuffer=true;
                CreateScreenBuffersAndDevice(Display);
                dx_force_16bpp_zbuffer=saved_value;
                return;
            } else {
               exit(1);
            }
        }

        if(FAILED( hr = pBackDDSurf->AddAttachedSurface( pZDDSurf ) )) {
            wdxdisplay_cat.fatal() << "AddAttachedSurface failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }
    }

    // Create the device. The device is created off of our back buffer, which
    // becomes the render target for the newly created device.
    hr = pD3DI->CreateDevice(pD3DDevDesc->deviceGUID, pBackDDSurf, &pD3DDevice );
    if(hr != DD_OK) {
        wdxdisplay_cat.fatal() << "CreateDevice failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    // Create the viewport
    D3DVIEWPORT7 vp = { 0, 0, _props._xsize, _props._ysize, 0.0f, 1.0f};
    hr = pD3DDevice->SetViewport( &vp );
    if(hr != DD_OK) {
        wdxdisplay_cat.fatal() << "SetViewport failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    Display.pD3DDevice=pD3DDevice;
    Display.pddsPrimary=pPrimaryDDSurf;
    Display.pddsBack=pBackDDSurf;
    Display.pddsZBuf=pZDDSurf;
    Display.view_rect = view_rect;

//pDD, pPrimaryDDSurf, pBackDDSurf, pZDDSurf, pD3DI, pD3DDevice, view_rect);
    _dxgsg->dx_init();
    // do not SetDXReady() yet since caller may want to do more work before letting rendering proceed
}

////////////////////////////////////////////////////////////////////
//     Function: setup_colormap
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::setup_colormap(void) {
    PIXELFORMATDESCRIPTOR pfd;
    LOGPALETTE *logical;
    int n;

  // should probably do this some other way than through opengl PixelFormat calls

  /* grab the pixel format */
    memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
    DescribePixelFormat(_hdc, GetPixelFormat(_hdc),
                        sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    if(!(pfd.dwFlags & PFD_NEED_PALETTE ||
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

    if(pfd.iPixelType == PFD_TYPE_RGBA) {
        int redMask = (1 << pfd.cRedBits) - 1;
        int greenMask = (1 << pfd.cGreenBits) - 1;
        int blueMask = (1 << pfd.cBlueBits) - 1;
        int i;

    /* fill in an RGBA color palette */
        for(i = 0; i < n; ++i) {
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
//     Function: begin_frame
//       Access:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::begin_frame(void) {
    GraphicsWindow::begin_frame();
}

void wdxGraphicsWindow::show_frame(void) {
    _dxgsg->show_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: end_frame
//       Access:
//  Description:  timer info, incs frame #
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::end_frame(void) {
    GraphicsWindow::end_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: handle_window_move
//       Access:
//  Description: we receive the new x and y position of the client
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::handle_window_move(int x, int y) {
    _dxgsg->adjust_view_rect(x,y);
    _props._xorg = x;
    _props._yorg = y;
}

////////////////////////////////////////////////////////////////////
//     Function: handle_mouse_motion
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::handle_mouse_motion(int x, int y) {
    _input_devices[0].set_pointer_in_window(x, y);
}

////////////////////////////////////////////////////////////////////
//     Function: handle_mouse_exit
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////

void wdxGraphicsWindow::handle_mouse_entry(bool bEntering, int x, int y) {
    // usually 'motion' event is equivalent to entering, so
    // this will never be called w/bEntering true
    if(bEntering) {
        _input_devices[0].set_pointer_in_window(x, y);
    } else {
        _input_devices[0].set_pointer_out_of_window();
    }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_keypress
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::
handle_keypress(ButtonHandle key, int x, int y) {
/*
    if(key.has_ascii_equivalent()) {
        wdxdisplay_cat.spam() << key.get_ascii_equivalent() << endl;

        short d1 = GetKeyState(VK_CONTROL);
        short d2 = GetKeyState(VK_SHIFT);
        wdxdisplay_cat.spam().flags(ios::hex | ios::uppercase );
        wdxdisplay_cat.spam()  << " Control: " << ((d1 < 0)? "down" : "up") << "  Shift: " << ((d2 < 0)? "down" : "up") << endl;
    }
*/
    _input_devices[0].set_pointer_in_window(x, y);
    if(key != ButtonHandle::none()) {
        _input_devices[0].button_down(key);
    }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_keyrelease
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::
handle_keyrelease(ButtonHandle key) {
    if(key != ButtonHandle::none()) {
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

void INLINE wdxGraphicsWindow::process_events(void) {
  if(!_window_active) {
      // Get 1 msg at a time until no more are left and we block and sleep,
      // or message changes _return_control_to_app or _window_active status

      while(!_window_active && (!_return_control_to_app)) {
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
//     Function: wdxGraphicsWindow::get_gsg_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle of the kind of GSG preferred
//               by this kind of window.
////////////////////////////////////////////////////////////////////
TypeHandle wdxGraphicsWindow::
get_gsg_type() const {
    return DXGraphicsStateGuardian::get_class_type();
}

GraphicsWindow *wdxGraphicsWindow::
make_wdxGraphicsWindow(const FactoryParams &params) {
    GraphicsWindow::WindowPipe *pipe_param;
    if(!get_param_into(pipe_param, params)) {
        wdxdisplay_cat.error()
        << "No pipe specified for window creation!" << endl;
        return NULL;
    }

    GraphicsPipe *pipe = pipe_param->get_pipe();

    GraphicsWindow::WindowProps *props_param;
    if(!get_param_into(props_param, params)) {
        return new wdxGraphicsWindow(pipe);
    } else {
        return new wdxGraphicsWindow(pipe, props_param->get_properties());
    }
}

TypeHandle wdxGraphicsWindow::get_class_type(void) {
    return _type_handle;
}

void wdxGraphicsWindow::init_type(void) {
    GraphicsWindow::init_type();
    register_type(_type_handle, "wdxGraphicsWindow",
                  GraphicsWindow::get_class_type());
}

TypeHandle wdxGraphicsWindow::get_type(void) const {
    return get_class_type();
}

////////////////////////////////////////////////////////////////////
//     Function: lookup_key
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
ButtonHandle wdxGraphicsWindow::
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
        case VK_RMENU: {
            return KeyboardButton::alt();
        }

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

int wdxGraphicsWindow::
get_depth_bitwidth(void) {
    assert(_dxgsg!=NULL);
    if(_dxgsg->scrn.pddsZBuf!=NULL)
       return _dxgsg->scrn.depth_buffer_bitdepth;
     else return 0;

// GetSurfaceDesc is not reliable, on GF2, GetSurfDesc returns 32bpp when you created a 24bpp zbuf
// instead store the depth used at creation time

//    DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd);
//    _dxgsg->_zbuf->GetSurfaceDesc(&ddsd); 
//  return ddsd.ddpfPixelFormat.dwRGBBitCount;
}

// Global system parameters we want to modify during our run
static int iMouseTrails;
static bool bCursorShadowOn,bMouseVanish;

#ifndef SPI_GETMOUSEVANISH
// get rid of this when we upgrade to winxp winuser.h in newest platform sdk
#define SPI_GETMOUSEVANISH                  0x1020
#define SPI_SETMOUSEVANISH                  0x1021
#endif

#ifndef SPI_GETCURSORSHADOW
#error You have old windows headers, need to get latest MS Platform SDK
#endif

void set_global_parameters(void) {
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

void restore_global_parameters(void) {
  SystemParametersInfo(SPI_SETCURSORSHADOW,NULL,(PVOID)bCursorShadowOn,NULL);
  SystemParametersInfo(SPI_SETMOUSETRAILS,NULL,(PVOID)iMouseTrails,NULL);
  SystemParametersInfo(SPI_SETMOUSEVANISH,NULL,(PVOID)bMouseVanish,NULL);
}


// fns for exporting to python, which cant pass variable length arrays
wdxGraphicsWindowGroup::wdxGraphicsWindowGroup(GraphicsPipe *pipe,const GraphicsWindow::Properties &Win1Prop) {
    GraphicsWindow::Properties WinPropArr[1];
    WinPropArr[0]=Win1Prop;
    make_windows(pipe,1,WinPropArr);
}

wdxGraphicsWindowGroup::wdxGraphicsWindowGroup(GraphicsPipe *pipe,
                                               const GraphicsWindow::Properties &Win1Prop,
                                               const GraphicsWindow::Properties &Win2Prop) {
    GraphicsWindow::Properties WinPropArr[2];

    WinPropArr[0]=Win1Prop;
    WinPropArr[1]=Win2Prop;
    make_windows(pipe,2,WinPropArr);
}

wdxGraphicsWindowGroup::wdxGraphicsWindowGroup(GraphicsPipe *pipe,
                                               const GraphicsWindow::Properties &Win1Prop,
                                               const GraphicsWindow::Properties &Win2Prop,
                                               const GraphicsWindow::Properties &Win3Prop) {
    GraphicsWindow::Properties WinPropArr[3];

    WinPropArr[0]=Win1Prop;
    WinPropArr[1]=Win2Prop;
    WinPropArr[2]=Win3Prop;
    make_windows(pipe,3,WinPropArr);
}

wdxGraphicsWindowGroup::wdxGraphicsWindowGroup(wdxGraphicsWindow *OneWindow) {
    // init a 1 window group
    // called from config_window

    _windows.reserve(1);
    _windows.push_back(OneWindow);
    initWindowGroup();
}

void wdxGraphicsWindowGroup::initWindowGroup(void) {
    HRESULT hr;
    unsigned int i;

    assert(_windows.size()>0);
    _hOldForegroundWindow=GetForegroundWindow();
    _bClosingAllWindows= false;

    unsigned int num_windows=_windows.size();

    #define DDRAW_NAME "ddraw.dll"

    _hDDrawDLL = LoadLibrary(DDRAW_NAME);
    if(_hDDrawDLL == 0) {
        wdxdisplay_cat.fatal() << "can't locate " << DDRAW_NAME <<"!\n";
        exit(1);
    }

    _hMouseCursor = NULL;
    _bLoadedCustomCursor = false;

    // can only get multimon HW acceleration in fullscrn on DX7

    unsigned int numMonitors = GetSystemMetrics(SM_CMONITORS);

    if(numMonitors < num_windows) {
        if(numMonitors==0) {
             numMonitors=1;   //win95 system will fail this call
          } else {
              wdxdisplay_cat.fatal() << "system has only "<< numMonitors << " monitors attached, couldn't find enough devices to meet multi window reqmt of " << num_windows << endl;
              exit(1);
          }
    }     

    _pDDCreateEx = (LPDIRECTDRAWCREATEEX) GetProcAddress(_hDDrawDLL,"DirectDrawCreateEx");
    if(_pDDCreateEx == NULL) {
        wdxdisplay_cat.fatal() << "Panda currently requires at least DirectX 7.0!\n";
        exit(1);
    }

    if(_windows.size()>1) {
        // just do Enumeration for multimon case.
        // enumeration was reqd to select voodoo1-class devices, but I'm no longer
        // going to support them on DX7 due to lack of mouse cursor

        LPDIRECTDRAWENUMERATEEX pDDEnumEx = (LPDIRECTDRAWENUMERATEEX) GetProcAddress(_hDDrawDLL,"DirectDrawEnumerateExA");
        if(pDDEnumEx == NULL) {
            wdxdisplay_cat.fatal() << "GetProcAddr failed for DirectDrawEnumerateEx!\n";
            exit(1);
        }
    
        hr = (*pDDEnumEx)(DriverEnumCallback_MultiMon, &_DeviceInfoVec, DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES);
        if(FAILED(hr)) {
            wdxdisplay_cat.fatal()   << "DirectDrawEnumerateEx failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }
    
        if(_DeviceInfoVec.size() < num_windows) {
            wdxdisplay_cat.fatal() << "couldn't find enough devices attached to meet multi window reqmt of " << num_windows << endl;
            exit(1);
        }
    }

    assert(_windows[0] != NULL);

    for(i=0;i<num_windows;i++) {
        _windows[i]->config_window(this);
    }

    assert(_windows[0] != NULL);

    DWORD good_device_count=0;

    if(num_windows==1) {
        assert(_windows[0] != NULL);
        if(_windows[0]->search_for_device(0,NULL))
            good_device_count=1;
    } else {
        for(DWORD devnum=0;devnum<_DeviceInfoVec.size() && (good_device_count < num_windows);devnum++) {
            if(_windows[devnum]->search_for_device(devnum,&(_DeviceInfoVec[devnum])))
                good_device_count++;
        }
    }

    if(good_device_count < num_windows) {
      if(good_device_count==0)
         wdxdisplay_cat.fatal() << "no usable display devices, exiting...\n";
       else wdxdisplay_cat.fatal() << "multi-device request for " << num_windows << "devices, found only "<< good_device_count << endl;
      exit(1);
    }

    _DeviceInfoVec.clear();  // dont need this anymore

    CreateWindows();  // creates win32 windows  (need to do this before Setting coopLvls and display modes, 
                      // but after we have all the monitor handles needed by CreateWindow()

    SetCoopLevelsAndDisplayModes();

    if(dx_show_fps_meter)
       _windows[0]->_dxgsg->_bShowFPSMeter = true;  // just show fps on 1st mon

    for(i=0;i<num_windows;i++) {
        _windows[i]->CreateScreenBuffersAndDevice(_windows[i]->_dxgsg->scrn);
    }

    for(i=0;i<num_windows;i++) {
        _windows[i]->finish_window_setup();
    }

    for(i=0;i<num_windows;i++) {
        _windows[i]->_dxgsg->SetDXReady(true);
    }
}

wdxGraphicsWindowGroup::wdxGraphicsWindowGroup(GraphicsPipe *pipe,int num_windows,GraphicsWindow::Properties *WinPropArray) {
    make_windows(pipe,num_windows,WinPropArray);
}

void wdxGraphicsWindowGroup::
make_windows(GraphicsPipe *pipe,int num_windows,GraphicsWindow::Properties *WinPropArray) {
    _hParentWindow=NULL;
    _windows.reserve(num_windows);
    int i;

    // first make all the objs without running the dx config() stuff
    for(i=0;i<num_windows;i++) {
        wdxGraphicsWindow *pWdxWinptr= new wdxGraphicsWindow(pipe,WinPropArray[i],this);
        assert(pWdxWinptr!=NULL);
        _windows.push_back(pWdxWinptr);
    }

    initWindowGroup();  // needs _windows.size() to be valid
}

wdxGraphicsWindow::wdxGraphicsWindow(GraphicsPipe* pipe, const GraphicsWindow::Properties &props, wdxGraphicsWindowGroup *pParentGroup)
                   : GraphicsWindow(pipe, props) {
    _pParentWindowGroup=pParentGroup;
    _ime_open = false;
   // just call the GraphicsWindow constructor, have to hold off rest of init
}

wdxGraphicsWindowGroup::~wdxGraphicsWindowGroup() {
    // this fn must be called before windows are actually closed
    _bClosingAllWindows= true;

    for(DWORD i=0;i<_windows.size();i++) {
        _windows[i]->close_window();
    }

    if((_hOldForegroundWindow!=NULL) /*&& (scrn.hWnd==GetForegroundWindow())*/) {
        SetForegroundWindow(_hOldForegroundWindow);
    }

    if(_bLoadedCustomCursor && (_hMouseCursor!=NULL))
      DestroyCursor(_hMouseCursor);


    if(_hDDrawDLL != NULL) {
       FreeLibrary(_hDDrawDLL);
       _hDDrawDLL = NULL;
    }
}

