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
#include "wdxGraphicsWindow8.h"
#include "wdxGraphicsPipe8.h"
#include "config_wdxdisplay8.h"

#include <keyboardButton.h>
#include <mouseButton.h>
#include <throw_event.h>

#ifdef DO_PSTATS
#include <pStatTimer.h>
#endif

#include <ddraw.h>
#include <map>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle wdxGraphicsWindow::_type_handle;

#define LAST_ERROR 0
#define ERRORBOX_TITLE "Panda3D Error"
#define WDX_WINDOWCLASSNAME "wdxDisplay"
#define WDX_WINDOWCLASSNAME_NOCURSOR WDX_WINDOWCLASSNAME "_NoCursor"
#define DEFAULT_CURSOR IDC_ARROW

// define this to enable debug testing of dinput joystick
//#define DINPUT_DEBUG_POLL

typedef map<HWND,wdxGraphicsWindow *> HWND_PANDAWIN_MAP;

// CardIDVec is used in DX7 lowmem card-classification pass so DX8 can
// establish correspondence b/w DX7 mem info & DX8 device
typedef struct {
   HMONITOR hMon;
   DWORD MaxAvailVidMem;
   bool  bIsLowVidMemCard;
   GUID  DX7_DeviceGUID;
   DWORD VendorID,DeviceID;
//   char  szDriver[MAX_DEVICE_IDENTIFIER_STRING];
} CardID;

typedef vector<CardID> CardIDVec;
static CardIDVec *g_pCardIDVec=NULL;

HWND_PANDAWIN_MAP hwnd_pandawin_map;
wdxGraphicsWindow* global_wdxwinptr = NULL;  // need this for temporary windproc

#define MAX_DISPLAYS 20

#define PAUSED_TIMER_ID        7   // completely arbitrary choice
#define JOYSTICK_POLL_TIMER_ID 8
#define DX_IS_READY ((_dxgsg!=NULL)&&(_dxgsg->GetDXReady()))

LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam,LPARAM lparam);

// imperfect method to ID NVid? could also scan desc str, but that isnt fullproof either
#define IS_NVIDIA(DDDEVICEID) ((DDDEVICEID.VendorId==0x10DE) || (DDDEVICEID.VendorId==0x12D2))
#define IS_ATI(DDDEVICEID) (DDDEVICEID.VendorId==0x1002)
#define IS_MATROX(DDDEVICEID) (DDDEVICEID.VendorId==0x102B)

// because we dont have access to ModifierButtons, as a hack just synchronize state of these
// keys on get/lose keybd focus
#define NUM_MODIFIER_KEYS 16
unsigned int hardcoded_modifier_buttons[NUM_MODIFIER_KEYS]={VK_SHIFT,VK_MENU,VK_CONTROL,VK_SPACE,VK_TAB,
                                         VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,VK_PRIOR,VK_NEXT,VK_HOME,VK_END,
                                         VK_INSERT,VK_DELETE,VK_ESCAPE};

#define UNKNOWN_VIDMEM_SIZE 0xFFFFFFFF
/*
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

typedef enum {DBGLEV_FATAL,DBGLEV_ERROR,DBGLEV_WARNING,DBGLEV_INFO,DBGLEV_DEBUG,DBGLEV_SPAM
    } DebugLevels;

void PrintDBGStr(DebugLevels level,HRESULT hr,const char *msgstr) {
    ostream *pstrm;
    static ostream dbg_strms[DBGLEV_SPAM+1]={wdxdisplay_cat.fatal,wdxdisplay_cat.error,
        wdxdisplay_cat.warning,wdxdisplay_cat.info,wdxdisplay_cat.debug,wdxdisplay_cat.spam};
    assert(level<=DBGLEV_SPAM);

    pstrm=dbg_strms[level];
    (*pstrm)->fatal() << "GetDisplayMode failed, hr = " << ConvD3DErrorToString(hr) << endl;
}
*/

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
//#ifdef _DEBUG
#if 0
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
        wdxdisplay_cat.debug() << "GetAvailableVidMem failed : hr = " << ConvD3DErrorToString(hr) << endl;
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

void ClearToBlack(HWND hWnd,GraphicsWindow::Properties &props) {
    // clear to black
    HDC hDC=GetDC(hWnd);  // GetDC is not particularly fast.  if this needs to be super-quick, we should cache GetDC's hDC
    RECT clrRect={props._xorg,props._yorg,props._xorg+props._xsize,props._yorg+props._ysize};
    FillRect(hDC,&clrRect,(HBRUSH)GetStockObject(BLACK_BRUSH));
//          PatBlt(hDC,_props._xorg,_props._yorg,_props._xsize,_props._ysize,BLACKNESS);
    ReleaseDC(hWnd,hDC);
    GdiFlush();
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

  if(scrn.hWnd!=NULL) {
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
  TRACKMOUSEEVENT tme = {sizeof(TRACKMOUSEEVENT),TME_LEAVE,hwnd,0};
  BOOL bSucceeded = TrackMouseEvent(&tme);  // tell win32 to post WM_MOUSELEAVE msgs

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
            if((_WindowAdjustingType != NotAdjusting) || (!DX_IS_READY)) {
              // let DefWndProc do WM_ERASEBKGND & just draw black,
              // rather than forcing Present to stretchblt the old window contents
              // into the new size
              break;
            }

            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            if(DX_IS_READY) {
               _dxgsg->show_frame(true);  // 'true' since just want to show the last rendered backbuf, if any
            }
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_MOUSEMOVE: {
            if(!DX_IS_READY)
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

            WORD newX=LOWORD(lparam);
            WORD newY=HIWORD(lparam);

            SET_MOUSE_COORD(x,newX);
            SET_MOUSE_COORD(y,newY);

            handle_mouse_motion(x, y);
            if(_props._fullscreen && (_dxgsg!=NULL) && (_dxgsg->scrn.pD3DDevice!=NULL))
                _dxgsg->scrn.pD3DDevice->SetCursorPosition(newX,newY,D3DCURSOR_IMMEDIATE_UPDATE);
            return 0;
        }

        // if cursor is invisible, make it visible when moving in the window bars,etc
        case WM_NCMOUSEMOVE: {
            if(!_props._bCursorIsVisible) {
                if(!_cursor_in_windowclientarea) {
//                  SetCursor(_pParentWindowGroup->_hMouseCursor);
                    ShowCursor(true);
                    _cursor_in_windowclientarea=true;
                }
            }
            break;
        }

        case WM_NCMOUSELEAVE: {
            if(!_props._bCursorIsVisible) {
                ShowCursor(false);
//              SetCursor(NULL);
                _cursor_in_windowclientarea=false;
            }
            break;
        }

        case WM_MOUSELEAVE: {
           // wdxdisplay_cat.fatal() << "XXXXX WM_MOUSELEAVE received\n";

           _tracking_mouse_leaving=false;
           handle_mouse_exit();
           break;
        }

        case WM_CREATE: {
          track_mouse_leaving(hwnd);
          _cursor_in_windowclientarea=false;
          ClearToBlack(hwnd,_props);
          set_cursor_visibility(true);
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
            if(!DX_IS_READY)
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
            if(!DX_IS_READY)
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

        case WM_SETCURSOR:
            // Turn off any GDI window cursor
            //  dx8 cursor not working yet

            if(_use_dx8_cursor && _props._fullscreen) {
                //            SetCursor( NULL );
            //                _dxgsg->scrn.pD3DDevice->ShowCursor(true);

                set_cursor_visibility(true);
                return TRUE; // prevent Windows from setting cursor to window class cursor (see docs on WM_SETCURSOR)
            }
            break;

        case WM_MOVE:
            if(!DX_IS_READY)
                break;
            handle_window_move(LOWORD(lparam), HIWORD(lparam) );
            return 0;

#if 0
            doesnt work yet

        case WM_GETMINMAXINFO: {

            // make sure window size doesnt go to zero
            LPMINMAXINFO pInfo=(LPMINMAXINFO) lparam;
            wdxdisplay_cat.spam() << "initial WM_GETMINMAXINFO:  MinX:" << pInfo->ptMinTrackSize.x << " MinY:" << pInfo->ptMinTrackSize.y << endl;

            RECT client_rect;
            GetClientRect( hwnd, &client_rect );

            wdxdisplay_cat.spam() << "WM_GETMINMAXINFO: ClientRect: left: " << client_rect.left << " right: " << client_rect.right
                                  << " top: " << client_rect.top << " bottom: " << client_rect.bottom << endl;


            UINT client_ysize=RECT_YSIZE(client_rect);
            UINT client_xsize=RECT_XSIZE(client_rect);

            if(client_ysize==0) {
                RECT wnd_rect;
                GetWindowRect( hwnd, &wnd_rect );
                wdxdisplay_cat.spam() << "WM_GETMINMAXINFO: WndRect: left: " << wnd_rect.left << " right: " << wnd_rect.right
                                  << " top: " << wnd_rect.top << " bottom: " << wnd_rect.bottom << endl;

                pInfo->ptMinTrackSize.y=RECT_YSIZE(wnd_rect)-client_ysize+2;
            }

            if(client_xsize==0) {
                RECT wnd_rect;
                GetWindowRect( hwnd, &wnd_rect );
                pInfo->ptMinTrackSize.x=RECT_XSIZE(wnd_rect)-client_xsize+2;
            }

            if((client_ysize==0) || (client_xsize==0)) {
               wdxdisplay_cat.spam() << "final WM_GETMINMAXINFO:  MinX:" << pInfo->ptMinTrackSize.x << " MinY:" << pInfo->ptMinTrackSize.y << endl;
                return 0;
            }
            break;
        }
#endif

        case WM_EXITSIZEMOVE:
            #ifdef _DEBUG
              wdxdisplay_cat.spam()  << "WM_EXITSIZEMOVE received"  << endl;
            #endif

            if(_WindowAdjustingType==Resizing) {
                bool bSucceeded=handle_windowed_resize(hwnd,true);

                if(!bSucceeded) {
#if 0
 bugbug need to fix this stuff
                     SetWindowPos(hwnd,NULL,0,0,lastxsize,lastysize,
                                  SWP_NOMOVE |
#endif
                }
            }

            _WindowAdjustingType = NotAdjusting;
            _dxgsg->SetDXReady(true);
            return 0;

        case WM_ENTERSIZEMOVE: {
                if(_dxgsg!=NULL)
                    _dxgsg->SetDXReady(false);   // dont see pic during resize
                _WindowAdjustingType = MovingOrResizing;
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
                if(_props._fullscreen || ((_dxgsg==NULL) || (_dxgsg->scrn.hWnd==NULL)) || ((wparam != SIZE_RESTORED) && (wparam != SIZE_MAXIMIZED)))
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

        case WM_SETFOCUS: {
            // wdxdisplay_cat.info() << "got WM_SETFOCUS\n";
            if(!DX_IS_READY) {
              break;
            }

            POINT point;
            GetCursorPos(&point);
            ScreenToClient(hwnd, &point);

            // this is a hack to make sure common modifier keys have proper state
            // since at focus loss, app may never receive key-up event corresponding to
            // a key-down. it would be better to know the exact set of ModifierButtons the
            // user is using, since we may miss some here

            for(int i=0;i<NUM_MODIFIER_KEYS;i++) {
              if(GetKeyState(hardcoded_modifier_buttons[i]) < 0)
                handle_keypress(lookup_key(hardcoded_modifier_buttons[i]),point.x,point.y);
            }
            return 0;
        }

        case WM_KILLFOCUS: {
            // wdxdisplay_cat.info() << "got WM_KILLFOCUS\n";
            if(!DX_IS_READY) {
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

            if((wparam==_PandaPausedTimer) && ((!_window_active)||_active_minimized_fullscreen)) {
                assert(_dxgsg!=NULL);
                _dxgsg->CheckCooperativeLevel(DO_REACTIVATE_WINDOW);

                // wdxdisplay_cat.spam() << "periodic return of control to app\n";
                _return_control_to_app = true;
                // throw_event("PandaPaused");
                // do we still need to do this since I return control to app periodically using timer msgs?
                // does app need to know to avoid major computation?
            }

         #ifdef DINPUT_DEBUG_POLL
            // probably want to get rid of this in favor of event-based input
            if(dx_use_joystick && (wparam==_pParentWindowGroup->_pDInputInfo->_JoystickPollTimer)) {
                DIJOYSTATE2 js;
                ZeroMemory(&js,sizeof(js));
                if(_pParentWindowGroup->_pDInputInfo->ReadJoystick(0,js)) {
                    // for now just print stuff out to make sure it works
                    wdxdisplay_cat.debug() << "joyPos (X: " << js.lX << ",Y: " << js.lY << ",Z: " << js.lZ << ")\n";
                    for(int i=0;i<128;i++) {
                        if(js.rgbButtons[i]!=0)
                            wdxdisplay_cat.debug() << "joyButton "<< i << " pressed\n";
                    }
                } else {
                    wdxdisplay_cat.error() << "read of Joystick failed!\n";
                    exit(1);
                }
            }
          #endif
            return 0;

        case WM_CLOSE:
            #ifdef _DEBUG
              wdxdisplay_cat.spam()  << "WM_CLOSE received\n";
            #endif
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

// should be used by both fullscrn and windowed resize
bool wdxGraphicsWindow::reset_device_resize_window(UINT new_xsize, UINT new_ysize) {
    DXScreenData *pScrn=&_dxgsg->scrn;
    assert((new_xsize>0)&&(new_ysize>0));
    bool bRetval=true;

    D3DPRESENT_PARAMETERS d3dpp;
    memcpy(&d3dpp,&pScrn->PresParams,sizeof(D3DPRESENT_PARAMETERS));
    d3dpp.BackBufferWidth = new_xsize;
    d3dpp.BackBufferHeight = new_ysize;
    HRESULT hr=_dxgsg->reset_d3d_device(&d3dpp);

    if(FAILED(hr)) {
        bRetval=false;
        wdxdisplay_cat.error() << "reset_device_resize_window Reset() failed" << D3DERRORSTRING(hr);
        if(hr==D3DERR_OUTOFVIDEOMEMORY) {
            hr=_dxgsg->reset_d3d_device(&pScrn->PresParams);
            if(FAILED(hr)) {
                wdxdisplay_cat.error() << "reset_device_resize_window Reset() failed OutOfVidmem, then failed again doing Reset w/original params:" << D3DERRORSTRING(hr);
                exit(1);
            } else {
                if(wdxdisplay_cat.is_info())
                    wdxdisplay_cat.info() << "reset of original size (" <<pScrn->PresParams.BackBufferWidth << ","
                    << pScrn->PresParams.BackBufferHeight << ") succeeded\n";
            }
        } else {
            exit(1);
        }
    }

    init_resized_window();
    return bRetval;
}

////////////////////////////////////////////////////////////////////
//     Function: handle_reshape
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow::handle_windowed_resize(HWND hWnd,bool bDoDxReset) {
  // handles windowed, non-fullscreen resizing
    GdiFlush();

    assert(!_props._fullscreen);

    if(bDoDxReset && (_dxgsg!=NULL)) {
        if(_dxgsg->scrn.pD3DDevice==NULL) {
            //assume this is initial creation reshape, so ignore this call
            return true;
        }
    }

    if(_dxgsg!=NULL)
       _dxgsg->SetDXReady(false);

    RECT view_rect;

    GetClientRect( hWnd, &view_rect );
    ClientToScreen( hWnd, (POINT*)&view_rect.left );   // translates top,left pnt
    ClientToScreen( hWnd, (POINT*)&view_rect.right );  // translates right,bottom pnt

    _props._xorg = view_rect.left;  // _props origin should reflect upper left of view rectangle
    _props._yorg = view_rect.top;

    DWORD xsize= RECT_XSIZE(view_rect);
    DWORD ysize= RECT_YSIZE(view_rect);

    if((xsize==0)||(ysize==0)) {
      return false;
    }

    bool bResizeSucceeded=true;

/*
   fail if resize fails, dont adjust size
   do {
        // change _props xsize,ysize  (need to do this here in case _dxgsg==NULL)
        resized(xsize,ysize);

        if((_dxgsg!=NULL)&& bDoDxReset) {
          bResizeSucceeded=reset_device_resize_window(xsize,ysize);  // create the new resized rendertargets
          if(!bResizeSucceeded) {
              // size was too large.  try a smaller size
              if(wdxdisplay_cat.is_debug()) {
                  wdxdisplay_cat.debug() << "windowed_resize to size: (" << xsize << "," << ysize << ") failed due to out-of-memory, retrying w/reduced size\n";
              }

              xsize *= 0.8f;
              ysize *= 0.8f;

              view_rect.right=view_rect.left+xsize;
              view_rect.bottom=view_rect.top+ysize;
          }
        }
    } while(!bResizeSucceeded);
*/

    // change _props xsize,ysize  (need to do this here in case _dxgsg==NULL)
    // reset_device_resize will call it again, this is OK
    resized(xsize,ysize);

    if((_dxgsg!=NULL)&& bDoDxReset) {
      bResizeSucceeded=reset_device_resize_window(xsize,ysize);  // create the new resized rendertargets
      if(!bResizeSucceeded) {
          if(wdxdisplay_cat.is_debug())
              wdxdisplay_cat.debug() << "windowed_resize to size: (" << xsize << "," << ysize << ") failed due to out-of-memory\n";
      } else {
          if(wdxdisplay_cat.is_debug())
              wdxdisplay_cat.debug() << "windowed_resize to origin: (" << _props._xorg << "," << _props._yorg << "), size: (" << _props._xsize << "," << _props._ysize << ")\n";
      }
    }

    if(_dxgsg!=NULL)
       _dxgsg->SetDXReady(true);

    return bResizeSucceeded;
}

void wdxGraphicsWindow::deactivate_window(void) {
    // current policy is to suspend minimized or deactivated fullscreen windows, but leave
    // regular windows running normally

   if((!_window_active) || _exiting_window || _active_minimized_fullscreen) {
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


     // bugbug: need to handle dinput devices
}

// currently this should only be called from CheckCoopLvl to return from Alt-tab
void wdxGraphicsWindow::reactivate_window(void) {
    if((!_window_active)||(_active_minimized_fullscreen)) {

        // first see if dx cooperative level is OK for reactivation
    //    if(!_dxgsg->CheckCooperativeLevel())
    //        return;

        if(_PandaPausedTimer!=NULL) {
            KillTimer(_dxgsg->scrn.hWnd,_PandaPausedTimer);
            _PandaPausedTimer = NULL;
        }

        if(!_window_active) {
            _window_active = true;
            if(wdxdisplay_cat.is_spam())
                wdxdisplay_cat.spam() << "WDX window re-activated...\n";
        } else {
            _active_minimized_fullscreen = false;
            if(wdxdisplay_cat.is_spam())
                wdxdisplay_cat.spam() << "WDX window unminimized from active-minimized state...\n";
        }

        // need to call dx_init and ResourceManagerDiscardBytes since D3D Reset() was called
        init_resized_window();

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

bool supports_color_cursors(D3DADAPTER_IDENTIFIER8 &DevID) {
    // TODO: add more cards as more testing is done
    if(IS_NVIDIA(DevID)) {
        // all nvidia seem to support 256 color
        return true;
    } else if(IS_ATI(DevID)) {
        // radeons seem to be in the 5100 range and support color, assume anything in 6000 or above
        // is newer than radeon and supports 256 color
        if(((DevID.DeviceId>=0x5100) && (DevID.DeviceId<=0x5200)) ||
           (DevID.DeviceId>=0x6000))
            return true;
    } else if IS_MATROX(DevID) {
        if(DevID.DeviceId==0x0525)   // G400 seems to support color cursors, havent tested other matrox
            return true;
    }

    return false;
}

HCURSOR CreateNullCursor(HINSTANCE hInst) {
  #define CURSORBYTESIZE (32*4)

  // 1-bit 32x32
  BYTE ANDPlane[CURSORBYTESIZE],XORPlane[CURSORBYTESIZE];

  ZeroMemory(XORPlane,CURSORBYTESIZE);
  memset(ANDPlane,0xFF,CURSORBYTESIZE);

  return CreateCursor(hInst,0,0,32,32,ANDPlane,XORPlane);
}

void wdxGraphicsWindowGroup::CreateWindows(void) {
    HINSTANCE hProgramInstance = GetModuleHandle(NULL);
    WNDCLASS wc;
    static bool wc_registered = false;
    _hParentWindow = NULL;
    _bLoadedCustomCursor=false;

    // Clear before filling in window structure!
    ZeroMemory(&wc, sizeof(WNDCLASS));
    wc.style      = CS_HREDRAW | CS_VREDRAW; //CS_OWNDC;
    wc.lpfnWndProc    = (WNDPROC) static_window_proc;
    wc.hInstance      = hProgramInstance;

  // all this must be moved to dx_init, since we need to create DX surface
    string windows_icon_filename = get_icon_filename().to_os_specific();


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

#if 0
    // Note: dx_init() uses the cursor handle to create the dx cursor surface
    string windows_color_cursor_filename = get_color_cursor_filename().to_os_specific();

    if(!windows_color_cursor_filename.empty()) {
        // card support for full color non-black/white GDI cursors varies greatly.  if the cursor is not supported,
        // it is rendered in software by GDI, which causes a flickering effect (because it's not synced
        // with flip?).  GDI transparently masks the lack of HW support so there is no easy way for app to detect
        // if HW cursor support exists.  alternatives are to tie cursor motion to frame rate using DDraw blts
        // or overlays (this is done automatically by DX8 runtime mouse cursor support), or to have separate thread draw cursor
        // (sync issues?).  instead we do mono cursor  unless card is known to support 256 color cursors
        bool bSupportsColorCursor=true;

        #if 0
          // remove this check for now, since dx8 should emulate color cursors
        /* if any card doesnt support color, dont load it*/
         for(int w=0;w<_windows.size();w++)
               bSupportsColorCursor &= supports_color_cursors(_windows[w]->_dxgsg->scrn.DXDeviceID);
        #endif

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

            _bLoadedCustomCursor=true;
        }
    }

    try_mono_cursor:
#endif

    if(!_bLoadedCustomCursor) {
        string windows_mono_cursor_filename = get_mono_cursor_filename().to_os_specific();

        if(!windows_mono_cursor_filename.empty()) {
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
    }

    if(!_bLoadedCustomCursor)
      _hMouseCursor = LoadCursor(NULL, DEFAULT_CURSOR);

    // need both a mouse and no-mouse class in case we have mixed fullscrn/windowed
    // windowed must use GDI mouse, fullscrn usually wont
    if (!wc_registered) {
      // We only need to register the window class once per session.

      wc.hCursor = _hMouseCursor;  // for windowed mode use the GDI cursor.
      // bugbug:  for fullscreen do we need to do a SetWindowLongNULL
      wc.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
      wc.lpszMenuName   = NULL;
      wc.lpszClassName  = WDX_WINDOWCLASSNAME;

      if(!RegisterClass(&wc)) {
        wdxdisplay_cat.error() << "could not register window class " << WDX_WINDOWCLASSNAME << endl;
      }

      if(dx_use_dx_cursor) {
          wc.hCursor = CreateNullCursor(hProgramInstance);
          if(wc.hCursor==NULL)
            wdxdisplay_cat.error() << "failed to create NULL cursor, error=" << GetLastError() << endl;

          wc.lpszClassName = WDX_WINDOWCLASSNAME_NOCURSOR;

          if(!RegisterClass(&wc)) {
            wdxdisplay_cat.error() << "could not register window class " << WDX_WINDOWCLASSNAME_NOCURSOR << endl;
          }
      }

      wc_registered = true;
    }

    DWORD base_window_style = WS_POPUP | WS_SYSMENU;  // for CreateWindow

    global_wdxwinptr = _windows[0];  // for use during createwin()  bugbug look at this again

    // rect now contains the coords for the entire window, not the client
    // extra windows must be parented to the first so app doesnt minimize when user selects them
    for(DWORD devnum=0;devnum<_windows.size();devnum++) {
        DWORD xleft,ytop,xsize,ysize;
        GraphicsWindow::Properties *props = &_windows[devnum]->_props;
        DWORD final_window_style;

        final_window_style=base_window_style;
        char *pWindowClassName;

        if(_windows[devnum]->_props._fullscreen) {
            MONITORINFO minfo;
            ZeroMemory(&minfo, sizeof(MONITORINFO));
            minfo.cbSize = sizeof(MONITORINFO);
            // since DX8 cant be installed on w95, ok to statically link to GetMonInfo
            // get upper-left corner coords using GetMonitorInfo
            GetMonitorInfo(_windows[devnum]->_dxgsg->scrn.hMon, &minfo);
            xleft=minfo.rcMonitor.left;
            ytop=minfo.rcMonitor.top;
            xsize=props->_xsize;
            ysize=props->_ysize;

            pWindowClassName= (dx_use_dx_cursor ? WDX_WINDOWCLASSNAME_NOCURSOR : WDX_WINDOWCLASSNAME);
        } else {
            // specify client area, use adjustwindowrect to figure out size of final window to
            // pass to CreateWin

            RECT win_rect;
            SetRect(&win_rect, props->_xorg,  props->_yorg, props->_xorg + props->_xsize,
                    props->_yorg + props->_ysize);

            if(props->_border)
                final_window_style |= WS_OVERLAPPEDWINDOW;  // should we just use WS_THICKFRAME instead?

            AdjustWindowRect(&win_rect, final_window_style, false);  //compute window size based on desired client area size

            // make sure origin is on screen
            if(win_rect.left < 0) {
                win_rect.right -= win_rect.left; win_rect.left = 0;
            }
            if(win_rect.top < 0) {
                win_rect.bottom -= win_rect.top; win_rect.top = 0;
            }

            xleft=win_rect.left;
            ytop= win_rect.top;
            xsize=RECT_XSIZE(win_rect);
            ysize=RECT_YSIZE(win_rect);

            pWindowClassName=WDX_WINDOWCLASSNAME;
        }

        wdxdisplay_cat.info() << "opening " << props->_xsize << "x" << props->_ysize
                              << ((_windows[devnum]->_props._fullscreen) ? " fullscreen" : " regular") << " window\n";

        if((xsize==0) || (ysize==0)) {
            wdxdisplay_cat.fatal() << "can't create window with zero area for device " << devnum << "!\n";
            exit(1);
        }

        // BUGBUG: this sets window posns based on desktop arrangement of monitors (that is what GetMonInfo is for).
        // need to move to chancfg stuff instead (i.e. use the properties x/yorg's) when that is ready
        HWND hWin = CreateWindow(pWindowClassName, props->_title.c_str(),
                                  final_window_style, xleft,ytop,xsize,ysize,
                                  _hParentWindow, NULL, hProgramInstance, 0);

        if(!hWin) {
            wdxdisplay_cat.fatal() << "CreateWindow failed for device " << devnum << "!, LastError=" << GetLastError() << endl;
            #ifdef _DEBUG
               PrintErrorMessage(LAST_ERROR);
            #endif
            exit(1);
        }

        _windows[devnum]->set_window_handle(hWin);
        _windows[devnum]->_dxgsg->scrn.pProps = &_windows[devnum]->_props;

        if(devnum==0)
            _hParentWindow=hWin;
    }

/*
    } else {
//        assert(_windows.size()==1);

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

        _hParentWindow =
            CreateWindow(WDX_WINDOWCLASSNAME, props->_title.c_str(),
                         window_style,
                         win_rect.left, win_rect.top, win_rect.right-win_rect.left,win_rect.bottom-win_rect.top,
                         NULL, NULL, hProgramInstance, 0);
        _windows[0]->set_window_handle(_hParentWindow);
    }

    if(_hParentWindow==NULL) {
        wdxdisplay_cat.fatal() << "CreateWindow failed!\n";
        exit(1);
    }
*/
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

    if(_props._fullscreen || dx_full_screen) {
        _props._fullscreen = dx_full_screen= true;
    }

    _use_dx8_cursor = dx_use_dx_cursor;
    if(!_props._fullscreen)
       _use_dx8_cursor = false;

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
    _dxgsg->scrn.bIsDX81 = pParentGroup->_bIsDX81;

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
}

void wdxGraphicsWindow::finish_window_setup(void) {
    // Now indicate that we have our keyboard/mouse device ready.
    GraphicsWindowInputDevice device = GraphicsWindowInputDevice::pointer_and_keyboard("keyboard/mouse");
    _input_devices.push_back(device);

    // move windows to top of zorder
    HWND hWin = _dxgsg->scrn.hWnd;

    // call twice to override STARTUPINFO value, which may be set to hidden initially (by emacs for instance)
    ShowWindow(hWin, SW_SHOWNORMAL);
    ShowWindow(hWin, SW_SHOWNORMAL);
    //  UpdateWindow( _mwindow );
    SetWindowPos(hWin, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOSIZE);
}

// this handles external programmatic requests for resizing (usually fullscrn resize)
bool wdxGraphicsWindow::resize(unsigned int xsize,unsigned int ysize) {
    bool bResizeSucceeded=false;

    if((xsize==_props._xsize)&&(ysize==_props._ysize)) {
        if(wdxdisplay_cat.is_debug())
          wdxdisplay_cat.debug() << "redundant resize() called, returning\n";
        return true;
    }

    if(!_props._fullscreen) {
       if(wdxdisplay_cat.is_debug())
          wdxdisplay_cat.debug() << "resize("<<xsize<<","<<ysize<<") called\n";

       // need to figure out actual window size based on props client area rect size
       RECT win_rect;
       SetRect(&win_rect, _props._xorg, _props._yorg, _props._xorg+xsize, _props._yorg+ysize);

       WINDOWINFO wi;
       GetWindowInfo(_dxgsg->scrn.hWnd,&wi);
       AdjustWindowRectEx(&win_rect, wi.dwStyle, false, wi.dwExStyle);  //compute window size based on desired client area size

       // make sure origin is on screen
       if(win_rect.left < 0) {
           win_rect.right -= win_rect.left; win_rect.left = 0;
           _props._xorg=0;
       }
       if(win_rect.top < 0) {
           win_rect.bottom -= win_rect.top; win_rect.top = 0;
           _props._yorg=0;
       }

       SetWindowPos(_dxgsg->scrn.hWnd, NULL, win_rect.left,win_rect.top, RECT_XSIZE(win_rect), RECT_YSIZE(win_rect), SWP_NOZORDER | SWP_NOSENDCHANGING);
       // WM_ERASEBKGND will be ignored, because _WindowAdjustingType!=NotAdjusting because
       // we dont want to redraw as user is manually resizing window, so need to force explicit
       // background clear for the programmatic resize fn call
        _WindowAdjustingType=NotAdjusting;

        //window_proc(_mwindow, WM_ERASEBKGND,(WPARAM)_hdc,0x0);  // this doesnt seem to be working in toontown resize, so I put ddraw blackblt in handle_windowed_resize instead

       return handle_windowed_resize(_dxgsg->scrn.hWnd,true);
    }

    assert(IS_VALID_PTR(_dxgsg));

    if(wdxdisplay_cat.is_info())
      wdxdisplay_cat.info() << "fullscrn resize("<<xsize<<","<<ysize<<") called\n";

    _dxgsg->SetDXReady(false);

    bool bCouldntFindValidZBuf;
    D3DFORMAT pixFmt;
    bool bNeedZBuffer = (_dxgsg->scrn.PresParams.EnableAutoDepthStencil!=false);
    bool bNeedStencilBuffer = IS_STENCIL_FORMAT(_dxgsg->scrn.PresParams.AutoDepthStencilFormat);

    bool bIsGoodMode=false;

    if(!special_check_fullscreen_resolution(xsize,ysize)) {
         // bypass the lowvidmem test below for certain "lowmem" cards we know have valid modes

        // wdxdisplay_cat.info() << "1111111 lowvidmemcard="<< _dxgsg->scrn.bIsLowVidMemCard << endl;
        if(_dxgsg->scrn.bIsLowVidMemCard && (!((xsize==640) && (ysize==480)))) {
            wdxdisplay_cat.error() << "resize() failed: will not try to resize low vidmem device #" << _dxgsg->scrn.CardIDNum << " to non-640x480!\n";
            goto Error_Return;
        }
    }

    // must ALWAYS use search_for_valid_displaymode even if we know a-priori that res is valid so we can 
    // get a valid pixfmt
    search_for_valid_displaymode(xsize,ysize,bNeedZBuffer,bNeedStencilBuffer,
                                 &_dxgsg->scrn.SupportedScreenDepthsMask,&bCouldntFindValidZBuf,
                                      &pixFmt);
    bIsGoodMode=(pixFmt!=D3DFMT_UNKNOWN);

    if(!bIsGoodMode) {
        wdxdisplay_cat.error() << "resize() failed: "
          << (bCouldntFindValidZBuf ? "Couldnt find valid zbuffer format to go with FullScreen mode" : "No supported FullScreen modes")
          << " at " << xsize << "x" << ysize << " for device #" << _dxgsg->scrn.CardIDNum <<endl;
      goto Error_Return;
    }

    // reset_device_resize_window handles both windowed & fullscrn,
    // so need to set new displaymode manually here
    _dxgsg->scrn.DisplayMode.Width=xsize;
    _dxgsg->scrn.DisplayMode.Height=ysize;
    _dxgsg->scrn.DisplayMode.Format = pixFmt;
    _dxgsg->scrn.DisplayMode.RefreshRate = D3DPRESENT_RATE_DEFAULT;

    _dxgsg->scrn.PresParams.BackBufferFormat = pixFmt;   // make reset_device_resize use presparams or displaymode??

    //    resized(xsize,ysize);  not needed?

    bResizeSucceeded=reset_device_resize_window(xsize,ysize);

    if(!bResizeSucceeded) {
       wdxdisplay_cat.error() << "resize() failed with OUT-OF-MEMORY error!\n";

       if((!IS_16BPP_DISPLAY_FORMAT(_dxgsg->scrn.PresParams.BackBufferFormat)) &&
                                  (_dxgsg->scrn.SupportedScreenDepthsMask & (R5G6B5_FLAG|X1R5G5B5_FLAG))) {
            // fallback strategy, if we trying >16bpp, fallback to 16bpp buffers
            _dxgsg->scrn.DisplayMode.Format = ((_dxgsg->scrn.SupportedScreenDepthsMask & R5G6B5_FLAG) ? D3DFMT_R5G6B5 : D3DFMT_X1R5G5B5);
            dx_force_16bpp_zbuffer=true;
            if(wdxdisplay_cat.info())
               wdxdisplay_cat.info() << "CreateDevice failed with out-of-vidmem, retrying w/16bpp buffers on device #"<< _dxgsg->scrn.CardIDNum << endl;

            bResizeSucceeded= reset_device_resize_window(xsize,ysize);  // create the new resized rendertargets
       }
    }

  Error_Return:

    _dxgsg->SetDXReady(true);

    if(wdxdisplay_cat.is_debug())
      wdxdisplay_cat.debug() << "fullscrn resize("<<xsize<<","<<ysize<<") " << (bResizeSucceeded ? "succeeds\n" : "fails\n");

    return bResizeSucceeded;
}

// overrides of the general estimator for known working cases
bool wdxGraphicsWindow::
special_check_fullscreen_resolution(UINT xsize,UINT ysize) {
    assert(IS_VALID_PTR(_dxgsg));

    DWORD VendorId=_dxgsg->scrn.DXDeviceID.VendorId;
    DWORD DeviceId=_dxgsg->scrn.DXDeviceID.DeviceId;
    switch(VendorId) {
        case 0x8086:  // Intel
            /*for now, just validate all the intel cards at these resolutions.
              I dont have a complete list of intel deviceIDs (missing 82830, 845, etc)
            // Intel i810,i815,82810
            if((DeviceId==0x7121)||(DeviceId==0x7123)||(DeviceId==0x7125)||
               (DeviceId==0x1132)) 
             */
            {
                if((xsize==640)&&(ysize==480))
                    return true;
                if((xsize==800)&&(ysize==600))
                    return true;
                if((xsize==1024)&&(ysize==768))
                    return true;
            }
            break;
    }

    return false;
}

UINT wdxGraphicsWindow::
verify_window_sizes(UINT numsizes,UINT *dimen) {
   // unfortunately this only works AFTER you make the window initially,
   // so its really mostly useful for resizes only
   assert(IS_VALID_PTR(_dxgsg));

   UINT num_valid_modes=0;

   // not requesting same refresh rate since changing res might not support same refresh rate at new size

   UINT *pCurDim=(UINT *)dimen;

   for(UINT i=0;i<numsizes;i++,pCurDim+=2) {
      UINT xsize=pCurDim[0];
      UINT ysize=pCurDim[1];

      bool bIsGoodMode=false;
      bool CouldntFindAnyValidZBuf;
      D3DFORMAT newPixFmt=D3DFMT_UNKNOWN;

      if(special_check_fullscreen_resolution(xsize,ysize)) {
           // bypass the test below for certain cards we know have valid modes
           bIsGoodMode=true;
      } else {
          if(_dxgsg->scrn.bIsLowVidMemCard) {
              bIsGoodMode=((xsize==640) && (ysize==480));
          } else  {
              search_for_valid_displaymode(xsize,ysize,_dxgsg->scrn.PresParams.EnableAutoDepthStencil!=false,
                                        IS_STENCIL_FORMAT(_dxgsg->scrn.PresParams.AutoDepthStencilFormat),
                                        &_dxgsg->scrn.SupportedScreenDepthsMask,&CouldntFindAnyValidZBuf,
                                        &newPixFmt);
              bIsGoodMode=(newPixFmt!=D3DFMT_UNKNOWN);
          }
      }

      if(bIsGoodMode) {
         num_valid_modes++;
       } else {
            // tell caller the mode is invalid
            pCurDim[0] = 0;
            pCurDim[1] = 0;
       }

       if(wdxdisplay_cat.is_spam())
           wdxdisplay_cat.spam() << "Fullscrn Mode (" << xsize << "," << ysize << ")\t" << (bIsGoodMode ? "V" : "Inv") <<"alid\n";
   }

   return num_valid_modes;
}

BOOL WINAPI DX7_DriverEnumCallback( GUID* pGUID, TCHAR* strDesc,TCHAR* strName,VOID *argptr, HMONITOR hm) {
//    #define PRNT(XX) ((XX!=NULL) ? XX : "NULL")
//    cout << "strDesc: "<< PRNT(strDesc) << "  strName: "<< PRNT(strName)<<endl;

    CardID CurCardID;

    ZeroMemory(&CurCardID,sizeof(CardID));

    if(hm==NULL) {
        CurCardID.hMon=MonitorFromWindow(GetDesktopWindow(),MONITOR_DEFAULTTOPRIMARY);
    } else {
        CurCardID.hMon=hm;
    }

    if(pGUID!=NULL)
        memcpy(&CurCardID.DX7_DeviceGUID,pGUID,sizeof(GUID));

    if(g_pCardIDVec==NULL) {
       g_pCardIDVec= new CardIDVec;
    }

    CurCardID.MaxAvailVidMem = UNKNOWN_VIDMEM_SIZE;

    g_pCardIDVec->push_back(CurCardID);
    return DDENUMRET_OK;
}

void wdxGraphicsWindowGroup::
find_all_card_memavails(void) {
    if(g_pCardIDVec!=NULL)  // we've already got the info
        return;

    #define DDRAW_NAME "ddraw.dll"

    HINSTANCE hDDrawDLL = LoadLibrary(DDRAW_NAME);
    if(hDDrawDLL == 0) {
        wdxdisplay_cat.fatal() << "LoadLibrary(" << DDRAW_NAME <<") failed!, error=" << GetLastError() << endl;
        exit(1);
    }

    LPDIRECTDRAWCREATEEX pDDCreateEx;

    pDDCreateEx = (LPDIRECTDRAWCREATEEX) GetProcAddress(hDDrawDLL,"DirectDrawCreateEx");
    if(pDDCreateEx == NULL) {
        wdxdisplay_cat.fatal() << "GetProcAddr failed for DDCreateEx" << endl;
        exit(1);
    }

    LPDIRECTDRAWENUMERATEEX pDDEnumEx = (LPDIRECTDRAWENUMERATEEX) GetProcAddress(hDDrawDLL,"DirectDrawEnumerateExA");
    if(pDDEnumEx == NULL) {
         wdxdisplay_cat.fatal() << "GetProcAddr failed for DirectDrawEnumerateEx! (win95 system?)\n";
         exit(1);
    }

    HRESULT hr = (*pDDEnumEx)(DX7_DriverEnumCallback, NULL, DDENUM_ATTACHEDSECONDARYDEVICES | DDENUM_NONDISPLAYDEVICES);
    if(FAILED(hr)) {
       wdxdisplay_cat.fatal()   << "DirectDrawEnumerateEx failed" << D3DERRORSTRING(hr);
       exit(1);
    }

    if(g_pCardIDVec==NULL) {
      wdxdisplay_cat.error() << "DirectDrawEnumerateEx enum'ed no devices!\n";
      return;
    }

    GUID ZeroGUID;
    ZeroMemory(&ZeroGUID,sizeof(GUID));

    if(g_pCardIDVec->size()>1) {
        assert(IsEqualGUID(ZeroGUID,(*g_pCardIDVec)[0].DX7_DeviceGUID));
        // delete enum of primary display (always the first), since it is duplicated by explicit entry
        g_pCardIDVec->erase(g_pCardIDVec->begin());
    }

    for(UINT i=0;i<g_pCardIDVec->size();i++) {
        LPDIRECTDRAW7 pDD;
        BYTE ddd_space[sizeof(DDDEVICEIDENTIFIER2)+4];  //bug in DX7 requires 4 extra bytes for GetDeviceID
        DDDEVICEIDENTIFIER2 *pDX7DeviceID=(DDDEVICEIDENTIFIER2 *)&ddd_space[0];
        GUID *pGUID= &((*g_pCardIDVec)[i].DX7_DeviceGUID);

        if(IsEqualGUID(*pGUID,ZeroGUID))
            pGUID=NULL;

        // Create the Direct Draw Object
        hr = (*pDDCreateEx)(pGUID,(void **)&pDD, IID_IDirectDraw7, NULL);
        if(FAILED(hr)) {
              wdxdisplay_cat.fatal() << "DirectDrawCreateEx failed for device ("<< i << ")" << D3DERRORSTRING(hr);
              continue;
        }

        ZeroMemory(ddd_space,sizeof(DDDEVICEIDENTIFIER2));

        hr = pDD->GetDeviceIdentifier(pDX7DeviceID,0x0);
        if(FAILED(hr)) {
              wdxdisplay_cat.fatal() << "GetDeviceID failed for device ("<< i << ")" << D3DERRORSTRING(hr);
              continue;
        }

        (*g_pCardIDVec)[i].DeviceID = pDX7DeviceID->dwDeviceId;
        (*g_pCardIDVec)[i].VendorID = pDX7DeviceID->dwVendorId;

        // Get Current VidMem avail.  Note this is only an estimate, when we switch to fullscreen
        // mode from desktop, more vidmem will be available (typically 1.2 meg).  I dont want
        // to switch to fullscreen more than once due to the annoying monitor flicker, so try
        // to figure out optimal mode using this estimate
        DDSCAPS2 ddsGAVMCaps;
        DWORD dwVidMemTotal,dwVidMemFree;
        dwVidMemTotal=dwVidMemFree=0;
        {
            // print out total INCLUDING AGP just for information purposes and future use
            // The real value I'm interested in for purposes of measuring possible valid screen sizes
            // shouldnt include AGP
            ZeroMemory(&ddsGAVMCaps,sizeof(DDSCAPS2));
            ddsGAVMCaps.dwCaps = DDSCAPS_VIDEOMEMORY;

            if(FAILED(hr = pDD->GetAvailableVidMem(&ddsGAVMCaps,&dwVidMemTotal,&dwVidMemFree))) {
               wdxdisplay_cat.error() << "GetAvailableVidMem failed for device #"<< i << D3DERRORSTRING(hr);
               // goto skip_device;
               exit(1);  // probably want to exit, since it may be my fault
            }
        }
        wdxdisplay_cat.info() << "GetAvailableVidMem (including AGP) returns Total: "<<dwVidMemTotal <<", Free: " << dwVidMemFree << " for device #"<<i<< endl;

        ZeroMemory(&ddsGAVMCaps,sizeof(DDSCAPS2));
        // just want to measure localvidmem, not AGP texmem
        ddsGAVMCaps.dwCaps = DDSCAPS_VIDEOMEMORY | DDSCAPS_LOCALVIDMEM;
        if(FAILED(hr = pDD->GetAvailableVidMem(&ddsGAVMCaps,&dwVidMemTotal,&dwVidMemFree))) {
           wdxdisplay_cat.error() << "GetAvailableVidMem failed for device #"<< i<< D3DERRORSTRING(hr);
           // goto skip_device;
           exit(1);  // probably want to exit, since it may be my fault
        }

        wdxdisplay_cat.info() << "GetAvailableVidMem (no AGP) returns Total: "<<dwVidMemTotal <<", Free: " << dwVidMemFree << " for device #"<< i<< endl;

        pDD->Release();  // release DD obj, since this is all we needed it for

        if(!dx_do_vidmemsize_check) {
           // still calling the DD stuff to get deviceID, etc.  is this necessary?
           (*g_pCardIDVec)[i].MaxAvailVidMem = UNKNOWN_VIDMEM_SIZE;
           (*g_pCardIDVec)[i].bIsLowVidMemCard = false;
           continue;
        }

        if(dwVidMemTotal==0) {  // unreliable driver
            dwVidMemTotal=UNKNOWN_VIDMEM_SIZE;
        } else {
             if(!ISPOW2(dwVidMemTotal)) {
                 // assume they wont return a proper max value, so
                 // round up to next pow of 2
                 UINT count=0;
                 while((dwVidMemTotal >> count)!=0x0)
                    count++;
                 dwVidMemTotal = (1 << count);
             }
         }

        // after SetDisplayMode, GetAvailVidMem totalmem seems to go down by 1.2 meg (contradicting above
        // comment and what I think would be correct behavior (shouldnt FS mode release the desktop vidmem?),
        // so this is the true value
        (*g_pCardIDVec)[i].MaxAvailVidMem = dwVidMemTotal;

        // I can never get this stuff to work reliably, so I'm just rounding up to nearest pow2.
        // Could try to get HardwareInformation.MemorySize MB number from registry like video control panel,
        // but its not clear how to find the proper registry location for a given card

        // #define LOWVIDMEMTHRESHOLD 3500000
        // #define CRAPPY_DRIVER_IS_LYING_VIDMEMTHRESHOLD 1000000

        #define LOWVIDMEMTHRESHOLD 5000000  // 4MB cards should fall below this
        #define CRAPPY_DRIVER_IS_LYING_VIDMEMTHRESHOLD 1000000  // if # is > 1MB, card is lying and I cant tell what it is

        // assume buggy drivers (this means you, FireGL2) may return zero (or small amts) for dwVidMemTotal, so ignore value if its < CRAPPY_DRIVER_IS_LYING_VIDMEMTHRESHOLD
        bool bLowVidMemFlag = ((dwVidMemTotal>CRAPPY_DRIVER_IS_LYING_VIDMEMTHRESHOLD) && (dwVidMemTotal< LOWVIDMEMTHRESHOLD));

        (*g_pCardIDVec)[i].bIsLowVidMemCard = bLowVidMemFlag;
        wdxdisplay_cat.info() << "SetLowVidMem flag to "<< bLowVidMemFlag<< " based on adjusted VidMemTotal: " <<dwVidMemTotal << endl;
    }

    FreeLibrary(hDDrawDLL);
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

        if(_props._fullscreen) {
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

int D3DFMT_to_DepthBits(D3DFORMAT fmt) {
    switch(fmt) {
        case D3DFMT_D16:  return 16;
        case D3DFMT_D24X8:
        case D3DFMT_D24X4S4:
        case D3DFMT_D24S8:  return 24;
        case D3DFMT_D32:  return 32;
        case D3DFMT_D15S1:  return 15;
        default:
          wdxdisplay_cat.debug() << "D3DFMT_DepthBits: unhandled D3DFMT!\n";
          return 0;
    }
}

bool wdxGraphicsWindow::FindBestDepthFormat(DXScreenData &Display,D3DDISPLAYMODE &TestDisplayMode,D3DFORMAT *pBestFmt,bool bWantStencil,bool bForce16bpp) const {
    // list fmts in order of preference
    #define NUM_TEST_ZFMTS 3
    static D3DFORMAT NoStencilPrefList[NUM_TEST_ZFMTS]={D3DFMT_D32,D3DFMT_D24X8,D3DFMT_D16};
    static D3DFORMAT StencilPrefList[NUM_TEST_ZFMTS]={D3DFMT_D24S8,D3DFMT_D24X4S4,D3DFMT_D15S1};

    // do not use Display.DisplayMode since that is probably not set yet, use TestDisplayMode instead

    // int want_color_bits = _props._want_color_bits;
    // int want_depth_bits = _props._want_depth_bits;  should we pay attn to these so panda user can select bitdepth?

    *pBestFmt = D3DFMT_UNKNOWN;
    HRESULT hr;

    // nvidia likes zbuf depth to match rendertarget depth
    bool bOnlySelect16bpp= (dx_force_16bpp_zbuffer || bForce16bpp ||
        (IS_NVIDIA(Display.DXDeviceID) && IS_16BPP_DISPLAY_FORMAT(TestDisplayMode.Format)));

    for(int i=0;i<NUM_TEST_ZFMTS;i++) {
        D3DFORMAT TestDepthFmt = (bWantStencil ? StencilPrefList[i] : NoStencilPrefList[i]);

        if(bOnlySelect16bpp && !IS_16BPP_ZBUFFER(TestDepthFmt))
            continue;

        hr = Display.pD3D8->CheckDeviceFormat(Display.CardIDNum,D3DDEVTYPE_HAL,TestDisplayMode.Format,
                                              D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,TestDepthFmt);

        if(FAILED(hr)) {
          if(hr==D3DERR_NOTAVAILABLE)
              continue;

          wdxdisplay_cat.error() << "unexpected CheckDeviceFormat failure" << D3DERRORSTRING(hr);
          exit(1);
        }

        hr = Display.pD3D8->CheckDepthStencilMatch(Display.CardIDNum,D3DDEVTYPE_HAL,
                                           TestDisplayMode.Format,   // adapter format
                                           TestDisplayMode.Format,   // backbuffer fmt  (should be the same in my apps)
                                           TestDepthFmt);

        if(FAILED(hr) && (hr!=D3DERR_NOTAVAILABLE)) {
          wdxdisplay_cat.error() << "unexpected CheckDepthStencilMatch failure" << D3DERRORSTRING(hr);
          exit(1);
        }

        if(SUCCEEDED(hr)) {
           *pBestFmt = TestDepthFmt;
           break;
        }
    }

    return (*pBestFmt != D3DFMT_UNKNOWN);
}

// all ptr args are output parameters
// if no valid mode found, returns *pSuggestedPixFmt = D3DFMT_UNKNOWN;
void wdxGraphicsWindow::search_for_valid_displaymode(UINT RequestedXsize,UINT RequestedYsize,bool bWantZBuffer,bool bWantStencil,
                                                     UINT *pSupportedScreenDepthsMask,bool *pCouldntFindAnyValidZBuf,
                                                     D3DFORMAT *pSuggestedPixFmt) {
    assert(IS_VALID_PTR(_dxgsg));
    assert(IS_VALID_PTR(_dxgsg->scrn.pD3D8));
    HRESULT hr;

    #ifndef NDEBUG
//   no longer true, due to special_check_fullscreen_res, where lowvidmem cards are allowed higher resolutions
//    if(_dxgsg->scrn.bIsLowVidMemCard)
//        nassertv((RequestedXsize==640)&&(RequestedYsize==480));
    #endif

    *pSuggestedPixFmt = D3DFMT_UNKNOWN;

    *pSupportedScreenDepthsMask = 0x0;
    int cNumModes=_dxgsg->scrn.pD3D8->GetAdapterModeCount(_dxgsg->scrn.CardIDNum);
    D3DDISPLAYMODE BestDispMode;
    ZeroMemory(&BestDispMode,sizeof(BestDispMode));

    *pCouldntFindAnyValidZBuf=false;

    for(int i=0;i<cNumModes;i++) {
        D3DDISPLAYMODE dispmode;
        hr = _dxgsg->scrn.pD3D8->EnumAdapterModes(_dxgsg->scrn.CardIDNum,i,&dispmode);
        if(FAILED(hr)) {
            wdxdisplay_cat.error() << "EnumAdapterDisplayMode failed for device #"<<_dxgsg->scrn.CardIDNum<< D3DERRORSTRING(hr);
            exit(1);
        }

        if((dispmode.Width!=RequestedXsize) || (dispmode.Height!=RequestedYsize))
            continue;

        if((dispmode.RefreshRate<60) && (dispmode.RefreshRate>1)) {
            // dont want refresh rates under 60Hz, but 0 or 1 might indicate a default refresh rate, which is usually >=60
            continue;
        }

        // Note no attempt is made to verify if format will work at requested size, so even if this call
        // succeeds, could still get an out-of-video-mem error

        hr = _dxgsg->scrn.pD3D8->CheckDeviceFormat(_dxgsg->scrn.CardIDNum,D3DDEVTYPE_HAL,dispmode.Format,
                                                   D3DUSAGE_RENDERTARGET,D3DRTYPE_SURFACE,dispmode.Format);
        if(FAILED(hr)) {
           if(hr==D3DERR_NOTAVAILABLE)
               continue;
             else {
                 wdxdisplay_cat.error() << "CheckDeviceFormat failed for device #" <<_dxgsg->scrn.CardIDNum << D3DERRORSTRING(hr);
                 exit(1);
             }
        } 

        bool bIs16bppRenderTgt = IS_16BPP_DISPLAY_FORMAT(dispmode.Format);
        float RendTgtMinMemReqmt;

        // if we have a valid memavail value, try to determine if we have enough space
        if( (_dxgsg->scrn.MaxAvailVidMem!=UNKNOWN_VIDMEM_SIZE) && 
            (!(special_check_fullscreen_resolution(RequestedXsize,RequestedYsize)))) {
            // assume user is testing fullscreen, not windowed, so use the dwTotal value
            // see if 3 scrnbufs (front/back/z)at 16bpp at xsize*ysize will fit with a few
            // extra megs for texmem

            // 8MB Rage Pro says it has 6.8 megs Total free and will run at 1024x768, so
            // formula makes it so that is OK

             #define REQD_TEXMEM 1800000

             float bytes_per_pixel = (bIs16bppRenderTgt ? 2 : 4);
             assert(_props._mask & W_DOUBLE);

             // *2 for double buffer

             RendTgtMinMemReqmt = ((float)RequestedXsize)*((float)RequestedYsize)*bytes_per_pixel*2+REQD_TEXMEM;

             if(wdxdisplay_cat.is_spam())
                wdxdisplay_cat.spam() << "Testing Mode (" <<RequestedXsize<<"x" << RequestedYsize <<","
                 << D3DFormatStr(dispmode.Format) << ")\nReqdVidMem: "<< (int)RendTgtMinMemReqmt << " AvailVidMem: " << _dxgsg->scrn.MaxAvailVidMem << endl;

             if(RendTgtMinMemReqmt>_dxgsg->scrn.MaxAvailVidMem) {
                 if(wdxdisplay_cat.is_debug())
                     wdxdisplay_cat.debug() << "not enough VidMem for render tgt, skipping display fmt " << D3DFormatStr(dispmode.Format)
                                            << " (" << (int)RendTgtMinMemReqmt << " > " << _dxgsg->scrn.MaxAvailVidMem << ")\n";
                 continue;
             }
        }

        if(bWantZBuffer) {
            D3DFORMAT zformat;
            if(!FindBestDepthFormat(_dxgsg->scrn,dispmode,&zformat,bWantStencil,false)) {
                *pCouldntFindAnyValidZBuf=true;
                continue;
            }

            if((_dxgsg->scrn.MaxAvailVidMem!=UNKNOWN_VIDMEM_SIZE) &&
               (!(special_check_fullscreen_resolution(RequestedXsize,RequestedYsize)))) {
                // test memory again, this time including zbuf size
                float zbytes_per_pixel = (IS_16BPP_ZBUFFER(zformat) ? 2 : 4);
                float MinMemReqmt = RendTgtMinMemReqmt + ((float)RequestedXsize)*((float)RequestedYsize)*zbytes_per_pixel;

                if(wdxdisplay_cat.is_spam())
                 wdxdisplay_cat.spam() << "Testing Mode w/Z (" <<RequestedXsize<<"x" << RequestedYsize <<","
                    << D3DFormatStr(dispmode.Format) << ")\nReqdVidMem: "<< (int)MinMemReqmt << " AvailVidMem: " << _dxgsg->scrn.MaxAvailVidMem << endl;

                if(MinMemReqmt>_dxgsg->scrn.MaxAvailVidMem) {
                    if(wdxdisplay_cat.is_debug())
                        wdxdisplay_cat.debug() << "not enough VidMem for RendTgt+zbuf, skipping display fmt " << D3DFormatStr(dispmode.Format)
                                               << " (" << (int)MinMemReqmt << " > " << _dxgsg->scrn.MaxAvailVidMem << ")\n";
                    continue;
                }

                if(MinMemReqmt<_dxgsg->scrn.MaxAvailVidMem) {
                    if(!IS_16BPP_ZBUFFER(zformat)) {
                        // see if things fit with a 16bpp zbuffer

                        if(!FindBestDepthFormat(_dxgsg->scrn,dispmode,&zformat,bWantStencil,true)) {
                            *pCouldntFindAnyValidZBuf=true;
                            continue;
                        }

                        // right now I'm not going to use these flags, just let the create fail out-of-mem and retry at 16bpp
                        *pSupportedScreenDepthsMask |= (IS_16BPP_DISPLAY_FORMAT(dispmode.Format) ? DISPLAY_16BPP_REQUIRES_16BPP_ZBUFFER_FLAG : DISPLAY_32BPP_REQUIRES_16BPP_ZBUFFER_FLAG);
                    }
                }
            }
        }

        switch(dispmode.Format) {
            case D3DFMT_X1R5G5B5:
                *pSupportedScreenDepthsMask |= X1R5G5B5_FLAG;
                break;
            case D3DFMT_X8R8G8B8:
                *pSupportedScreenDepthsMask |= X8R8G8B8_FLAG;
                break;
            case D3DFMT_R8G8B8:
                *pSupportedScreenDepthsMask |= R8G8B8_FLAG;
                break;
            case D3DFMT_R5G6B5:
                *pSupportedScreenDepthsMask |= R5G6B5_FLAG;
                break;
            default:
                //Render target formats should be only D3DFMT_X1R5G5B5, D3DFMT_R5G6B5, D3DFMT_X8R8G8B8 (or R8G8B8?)
                wdxdisplay_cat.debug() << "unrecognized supported screen D3DFMT returned by EnumAdapterDisplayModes!\n";
        }
    }

    // note: this chooses 32bpp, which may not be preferred over 16 for memory & speed reasons on some older cards in particular
    if(*pSupportedScreenDepthsMask & X8R8G8B8_FLAG)
        *pSuggestedPixFmt = D3DFMT_X8R8G8B8;
    else if(*pSupportedScreenDepthsMask & R8G8B8_FLAG)
        *pSuggestedPixFmt = D3DFMT_R8G8B8;
    else if(*pSupportedScreenDepthsMask & R5G6B5_FLAG)
        *pSuggestedPixFmt = D3DFMT_R5G6B5;
    else if(*pSupportedScreenDepthsMask & X1R5G5B5_FLAG)
        *pSuggestedPixFmt = D3DFMT_X1R5G5B5;
}

bool is_badvidmem_card(D3DADAPTER_IDENTIFIER8 *pDevID) {
  // dont trust Intel cards since they often use regular memory as vidmem
  if(pDevID->VendorId==0x00008086)
      return true;

   return false;
}

// returns true if successful
bool wdxGraphicsWindow::search_for_device(LPDIRECT3D8 pD3D8,DXDeviceInfo *pDevInfo) {
    DWORD dwRenderWidth  = _props._xsize;
    DWORD dwRenderHeight = _props._ysize;
    HRESULT hr;

    assert(_dxgsg!=NULL);
    _dxgsg->scrn.pD3D8 = pD3D8;
    _dxgsg->scrn.CardIDNum=pDevInfo->cardID;  // could this change by end?

    bool bWantStencil = ((_props._mask & W_STENCIL)!=0);

    hr = pD3D8->GetAdapterIdentifier(pDevInfo->cardID,D3DENUM_NO_WHQL_LEVEL,&_dxgsg->scrn.DXDeviceID);
    if(FAILED(hr)) {
       wdxdisplay_cat.fatal() << "D3D GetAdapterID failed" << D3DERRORSTRING(hr);
    }

    D3DCAPS8 d3dcaps;
    hr = pD3D8->GetDeviceCaps(pDevInfo->cardID,D3DDEVTYPE_HAL,&d3dcaps);
    if(FAILED(hr)) {
         if((hr==D3DERR_INVALIDDEVICE)||(hr==D3DERR_NOTAVAILABLE)) {
             wdxdisplay_cat.fatal() << "No DirectX 8 D3D-capable 3D hardware detected for device # "<<pDevInfo->cardID<<" ("<<pDevInfo->szDescription <<  ")!  Exiting...\n";
         } else {
             wdxdisplay_cat.fatal() << "GetDeviceCaps failed" << D3DERRORSTRING(hr);
         }
         exit(1);
    }

    //search_for_valid_displaymode needs these to be set
    memcpy(&_dxgsg->scrn.d3dcaps,&d3dcaps,sizeof(D3DCAPS8));
    _dxgsg->scrn.CardIDNum=pDevInfo->cardID;

    _dxgsg->scrn.MaxAvailVidMem = UNKNOWN_VIDMEM_SIZE;
    _dxgsg->scrn.bIsLowVidMemCard = false;

    // bugbug:  wouldnt we like to do GetAVailVidMem so we can do upper-limit memory computation for dx8 cards too?
    //          otherwise verify_window_sizes cant do much
    if((d3dcaps.MaxStreams==0)|| dx_pick_best_screenres) {
       if(wdxdisplay_cat.is_debug())
           wdxdisplay_cat.debug() << "checking vidmem size\n";
       assert(IS_VALID_PTR(_pParentWindowGroup));

       // look for low memory video cards
       _pParentWindowGroup->find_all_card_memavails();

       UINT IDnum;

       // simple linear search to match DX7 card info w/DX8 card ID
       for(IDnum=0;IDnum<g_pCardIDVec->size();IDnum++) {
//         wdxdisplay_cat.info() << "comparing '" << (*g_pCardIDVec)[IDnum].Driver << "' to '" << _dxgsg->scrn.DXDeviceID.Driver << "'\n";
           if(//(stricmp((*g_pCardIDVec)[IDnum].szDriver,pDevInfo->szDriver)==0) &&
              (pDevInfo->VendorID==(*g_pCardIDVec)[IDnum].VendorID) &&
              (pDevInfo->DeviceID==(*g_pCardIDVec)[IDnum].DeviceID) &&
              (pDevInfo->hMon==(*g_pCardIDVec)[IDnum].hMon))
             break;
       }

       if(IDnum<g_pCardIDVec->size()) {
           _dxgsg->scrn.MaxAvailVidMem = (*g_pCardIDVec)[IDnum].MaxAvailVidMem;
           _dxgsg->scrn.bIsLowVidMemCard = (*g_pCardIDVec)[IDnum].bIsLowVidMemCard;
       } else wdxdisplay_cat.error() << "Error: couldnt find a CardID match in DX7 info, assuming card is not a lowmem card\n";
    }

    if((bWantStencil) && (d3dcaps.StencilCaps==0x0)) {
            wdxdisplay_cat.fatal() << "Stencil ability requested, but device #" << pDevInfo->cardID << " (" << _dxgsg->scrn.DXDeviceID.Description<<"), has no stencil capability!\n";
            exit(1);
    }

    // just because TNL is true, it doesnt mean vtx shaders are supported in HW (see GF2)
    // for this case, you probably want MIXED processing to use HW for fixed-fn vertex processing
    // and SW for vtx shaders
    _dxgsg->scrn.bIsTNLDevice=((d3dcaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)!=0);
    _dxgsg->scrn.bCanUseHWVertexShaders = (d3dcaps.VertexShaderVersion >= D3DVS_VERSION(1,0));
    _dxgsg->scrn.bCanUsePixelShaders = (d3dcaps.PixelShaderVersion >= D3DPS_VERSION(1,0));

    bool bNeedZBuffer = ((!(d3dcaps.RasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR ))
                         && (_props._mask & W_DEPTH));

    _dxgsg->scrn.PresParams.EnableAutoDepthStencil=bNeedZBuffer;

    D3DFORMAT pixFmt=D3DFMT_UNKNOWN;

    if(_props._fullscreen) {
        _props._xorg = _props._yorg = 0;

        bool bCouldntFindValidZBuf;
        if(!_dxgsg->scrn.bIsLowVidMemCard) {

            bool bUseDefaultSize=dx_pick_best_screenres&&
                                 ((_dxgsg->scrn.MaxAvailVidMem == UNKNOWN_VIDMEM_SIZE) || 
                                  is_badvidmem_card(&_dxgsg->scrn.DXDeviceID));
                
            if(dx_pick_best_screenres && !bUseDefaultSize) {
                typedef struct {
                    UINT memlimit;
                    DWORD scrnX,scrnY;
                } Memlimres;

                const Memlimres MemRes[] = {
                                             {       0,  640, 480},
                                             { 8000000,  800, 600},
                                             {16000000, 1024, 768},
                                             {32000000, 1280,1024},  // 32MB+ cards will choose this

                                            // some monitors have trouble w/1600x1200, so dont pick this by deflt,
                                            // even though 64MB cards should handle it
                                             {64000000, 1280,1024}   // 64MB+ cards will choose this
                                           };
                const NumResLims = (sizeof(MemRes)/sizeof(Memlimres));

                for(int i=NumResLims-1;i>=0;i--) {
                    // find biggest slot card can handle
                    if(_dxgsg->scrn.MaxAvailVidMem > MemRes[i].memlimit) {
                        dwRenderWidth=MemRes[i].scrnX;
                        dwRenderHeight=MemRes[i].scrnY;

                        wdxdisplay_cat.info() << "pick_best_screenres: picked " << dwRenderWidth << "x" << dwRenderHeight << " based on " << _dxgsg->scrn.MaxAvailVidMem << " bytes avail\n";

                        search_for_valid_displaymode(dwRenderWidth,dwRenderHeight,bNeedZBuffer,bWantStencil,
                                         &_dxgsg->scrn.SupportedScreenDepthsMask,
                                         &bCouldntFindValidZBuf,
                                         &pixFmt);


                        // note I'm not saving refresh rate, will just use adapter default at given res for now

                        if(pixFmt!=D3DFMT_UNKNOWN)
                           break;

                        wdxdisplay_cat.debug() << "skipping scrnres; "
                                << (bCouldntFindValidZBuf ? "Couldnt find valid zbuffer format to go with FullScreen mode" : "No supported FullScreen modes")
                                << " at " << dwRenderWidth << "x" << dwRenderHeight << " for device #" << _dxgsg->scrn.CardIDNum << endl;
                    }
                }
                // otherwise just go with whatever was specified (we probably shouldve marked this card as lowmem if it gets to end of loop w/o breaking
            }

            if(pixFmt==D3DFMT_UNKNOWN) {

                if(bUseDefaultSize) {
                    wdxdisplay_cat.info() << "pick_best_screenres: defaulted 800x600 based on no reliable vidmem size\n";
                    dwRenderWidth=800;  dwRenderHeight=600;
                }

                search_for_valid_displaymode(dwRenderWidth,dwRenderHeight,bNeedZBuffer,bWantStencil,
                                         &_dxgsg->scrn.SupportedScreenDepthsMask,
                                         &bCouldntFindValidZBuf,
                                         &pixFmt);

                // note I'm not saving refresh rate, will just use adapter default at given res for now

                if(pixFmt==D3DFMT_UNKNOWN) {
                    wdxdisplay_cat.fatal()
                        << (bCouldntFindValidZBuf ? "Couldnt find valid zbuffer format to go with FullScreen mode" : "No supported FullScreen modes")
                        << " at " << dwRenderWidth << "x" << dwRenderHeight << " for device #" << _dxgsg->scrn.CardIDNum <<endl;
                   return false;
                }
            }
        } else {
            // Low Memory card
               dwRenderWidth=640;
               dwRenderHeight=480;
               dx_force_16bpptextures = true;

               // force 16bpp zbuf? or let user get extra bits if they have the mem?

               search_for_valid_displaymode(dwRenderWidth,dwRenderHeight,bNeedZBuffer,bWantStencil,
                                     &_dxgsg->scrn.SupportedScreenDepthsMask,
                                     &bCouldntFindValidZBuf,
                                     &pixFmt);

               // hack: figuring out exactly what res to use is tricky, instead I will
               // just use 640x480 if we have < 3 meg avail

               if(_dxgsg->scrn.SupportedScreenDepthsMask & R5G6B5_FLAG)
                   pixFmt = D3DFMT_R5G6B5;
               else if(_dxgsg->scrn.SupportedScreenDepthsMask & X1R5G5B5_FLAG)
                   pixFmt = D3DFMT_X1R5G5B5;
               else {
                   wdxdisplay_cat.fatal() << "Low Memory VidCard has no supported FullScreen 16bpp resolutions at "<< dwRenderWidth << "x" << dwRenderHeight
                   << " for device #" << pDevInfo->cardID << " (" <<  _dxgsg->scrn.DXDeviceID.Description <<"), skipping device...\n";
                   return false;
               }

               if(wdxdisplay_cat.is_info())
                   wdxdisplay_cat.info() << "Available VidMem (" << _dxgsg->scrn.MaxAvailVidMem << ") is under " << LOWVIDMEMTHRESHOLD <<", using 640x480 16bpp rendertargets to save tex vidmem.\n";
        }
    } else {
        // Windowed Mode

        D3DDISPLAYMODE dispmode;
        hr=pD3D8->GetAdapterDisplayMode(pDevInfo->cardID,&dispmode);
        if(FAILED(hr)) {
            wdxdisplay_cat.fatal() << "GetAdapterDisplayMode(" << pDevInfo->cardID << ") failed" << D3DERRORSTRING(hr);
            exit(1);
        }
        pixFmt = dispmode.Format;
    }

    _dxgsg->scrn.DisplayMode.Width=dwRenderWidth;
    _dxgsg->scrn.DisplayMode.Height=dwRenderHeight;
    _dxgsg->scrn.DisplayMode.Format = pixFmt;
    _dxgsg->scrn.DisplayMode.RefreshRate = D3DPRESENT_RATE_DEFAULT;
    _dxgsg->scrn.hMon=pDevInfo->hMon;
    return true;
}

//return true if successful
void wdxGraphicsWindow::
CreateScreenBuffersAndDevice(DXScreenData &Display) {

    // only want this to apply to initial startup
    dx_pick_best_screenres=false;

    DWORD dwRenderWidth=Display.DisplayMode.Width;
    DWORD dwRenderHeight=Display.DisplayMode.Height;
    LPDIRECT3D8 pD3D8=Display.pD3D8;
    D3DCAPS8 *pD3DCaps = &Display.d3dcaps;
    D3DPRESENT_PARAMETERS* pPresParams = &Display.PresParams;
    RECT view_rect;
    HRESULT hr;
    bool bWantStencil = ((_props._mask & W_STENCIL)!=0);

    assert(pD3D8!=NULL);
    assert(pD3DCaps->DevCaps & D3DDEVCAPS_HWRASTERIZATION);

    #ifndef NDEBUG
      if(!(_props._mask & W_DEPTH)) {
        wdxdisplay_cat.info() << "no zbuffer requested, skipping zbuffer creation\n";
      }
    #endif

    pPresParams->BackBufferFormat = Display.DisplayMode.Format;  // dont need dest alpha, so just use adapter format

    if(dx_sync_video && !(pD3DCaps->Caps & D3DCAPS_READ_SCANLINE)) {
        wdxdisplay_cat.info() << "HW doesnt support syncing to vertical refresh, ignoring dx_sync_video\n";
        dx_sync_video=false;
    }

    // verify the rendertarget fmt one last time
    if(FAILED(pD3D8->CheckDeviceFormat(Display.CardIDNum, D3DDEVTYPE_HAL, Display.DisplayMode.Format,D3DUSAGE_RENDERTARGET,
                           D3DRTYPE_SURFACE, pPresParams->BackBufferFormat))) {
        wdxdisplay_cat.error() << "device #"<<Display.CardIDNum<< " CheckDeviceFmt failed for surface fmt "<< D3DFormatStr(pPresParams->BackBufferFormat) << endl;
        goto Fallback_to_16bpp_buffers;
    }

    if(FAILED(pD3D8->CheckDeviceType(Display.CardIDNum,D3DDEVTYPE_HAL, Display.DisplayMode.Format,pPresParams->BackBufferFormat,
                                    _props._fullscreen))) {
        wdxdisplay_cat.error() << "device #"<<Display.CardIDNum<< " CheckDeviceType failed for surface fmt "<< D3DFormatStr(pPresParams->BackBufferFormat) << endl;
        goto Fallback_to_16bpp_buffers;
    }

    if(Display.PresParams.EnableAutoDepthStencil) {
        if(!FindBestDepthFormat(Display,Display.DisplayMode,&Display.PresParams.AutoDepthStencilFormat,bWantStencil,false)) {
            wdxdisplay_cat.error() << "FindBestDepthFormat failed in CreateScreenBuffers for device #"<<Display.CardIDNum<< endl;
            goto Fallback_to_16bpp_buffers;
        }
        _depth_buffer_bpp=D3DFMT_to_DepthBits(Display.PresParams.AutoDepthStencilFormat);
    } else {
        _depth_buffer_bpp=0;
    }

    pPresParams->Windowed = !_props._fullscreen;

    if(dx_multisample_antialiasing_level>1) {
    // need to check both rendertarget and zbuffer fmts
        hr = pD3D8->CheckDeviceMultiSampleType(Display.CardIDNum, D3DDEVTYPE_HAL, Display.DisplayMode.Format,
                                                     _props._fullscreen, D3DMULTISAMPLE_TYPE(dx_multisample_antialiasing_level));
        if(FAILED(hr)) {
            wdxdisplay_cat.fatal() << "device #"<<Display.CardIDNum<< " doesnt support multisample level "<<dx_multisample_antialiasing_level <<"surface fmt "<< D3DFormatStr(Display.DisplayMode.Format) <<endl;
            exit(1);
        }

        if(Display.PresParams.EnableAutoDepthStencil) {
            hr = pD3D8->CheckDeviceMultiSampleType(Display.CardIDNum, D3DDEVTYPE_HAL, Display.PresParams.AutoDepthStencilFormat,
                                             _props._fullscreen, D3DMULTISAMPLE_TYPE(dx_multisample_antialiasing_level));
            if(FAILED(hr)) {
                wdxdisplay_cat.fatal() << "device #"<<Display.CardIDNum<< " doesnt support multisample level "<<dx_multisample_antialiasing_level <<"surface fmt "<< D3DFormatStr(Display.PresParams.AutoDepthStencilFormat) <<endl;
                exit(1);
            }
        }

        pPresParams->MultiSampleType = D3DMULTISAMPLE_TYPE(dx_multisample_antialiasing_level);

        if(wdxdisplay_cat.is_info())
            wdxdisplay_cat.info() << "device #"<<Display.CardIDNum<< " using multisample antialiasing level "<<dx_multisample_antialiasing_level <<endl;
    }

    pPresParams->BackBufferCount = 1;
    pPresParams->Flags = 0x0;
    pPresParams->hDeviceWindow = Display.hWnd;
    pPresParams->BackBufferWidth = Display.DisplayMode.Width;
    pPresParams->BackBufferHeight = Display.DisplayMode.Height;
    DWORD dwBehaviorFlags=0x0;

    if(_dxgsg->scrn.bIsTNLDevice) {
       dwBehaviorFlags|=D3DCREATE_HARDWARE_VERTEXPROCESSING;
      // note: we could create a pure device in this case if I eliminated the GetRenderState calls in dxgsg

      // also, no software vertex processing available since I specify D3DCREATE_HARDWARE_VERTEXPROCESSING
      // and not D3DCREATE_MIXED_VERTEXPROCESSING
    } else {
       dwBehaviorFlags|=D3DCREATE_SOFTWARE_VERTEXPROCESSING;
    }

    if(dx_preserve_fpu_state)
       dwBehaviorFlags|=D3DCREATE_FPU_PRESERVE;

    // if window is not foreground in exclusive mode, ddraw thinks you are 'not active', so
    // it changes your WM_ACTIVATEAPP from true to false, causing us
    // to go into a 'wait-for WM_ACTIVATEAPP true' loop, and the event never comes so we hang
    // in fullscreen wait.  also doing this for windowed mode since it was requested.
    SetForegroundWindow(Display.hWnd);

    if(_props._fullscreen) {
        pPresParams->SwapEffect = D3DSWAPEFFECT_DISCARD;  // we dont care about preserving contents of old frame
        pPresParams->FullScreen_PresentationInterval = (dx_sync_video ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE);
        pPresParams->FullScreen_RefreshRateInHz = Display.DisplayMode.RefreshRate;

#if 0
/* this is incorrect, we dont need rendertarget to hold alpha to do normal alpha blending, since blending is usually just
   based on source alpha.  so for now its OK to pick rendertargets with same format as display

        // assume app requires alpha-blending
        // will fullscrn alphablend work with DISCARD anyway even if this cap is not set?  I dont see it being set by latest drivers
        // for any card.  need to test both dx8.0 and dx8.1, it may be that dx8.1 works even if cap is not set, but 8.0 doesnt
        if((!Display.bIsDX81) || (!(d3dcaps.Caps3 & D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD))) {
            pPresParams->SwapEffect = (dx_sync_video ? D3DSWAPEFFECT_COPY_VSYNC : D3DSWAPEFFECT_COPY);

            if(pPresParams->MultiSampleType != D3DMULTISAMPLE_NONE) {
                wdxdisplay_cat.fatal() << "device #"<<Display.CardIDNum<< " cant support multisample antialiasing and fullscrn alphablending because of driver limitations and/or lack of DX8.1"<<dx_multisample_antialiasing_level <<endl;
                exit(1);
            }
        }
*/
#endif
        #ifdef _DEBUG
        if(pPresParams->MultiSampleType != D3DMULTISAMPLE_NONE)
            assert(pPresParams->SwapEffect == D3DSWAPEFFECT_DISCARD);  // only valid effect for multisample
        #endif

        ClearToBlack(Display.hWnd,_props);

        hr = pD3D8->CreateDevice(Display.CardIDNum, D3DDEVTYPE_HAL, _pParentWindowGroup->_hParentWindow,
                                 dwBehaviorFlags, pPresParams, &Display.pD3DDevice);

        if(FAILED(hr)) {
            wdxdisplay_cat.fatal() << "D3D CreateDevice failed for device #" << Display.CardIDNum << ", " << D3DERRORSTRING(hr);

            if(hr == D3DERR_OUTOFVIDEOMEMORY)
                goto Fallback_to_16bpp_buffers;
        }

        SetRect(&view_rect, 0, 0, dwRenderWidth, dwRenderHeight);
    }   // end create full screen buffers

    else {          // CREATE WINDOWED BUFFERS

        if(!(pD3DCaps->Caps2 & D3DCAPS2_CANRENDERWINDOWED)) {
            wdxdisplay_cat.fatal() << "the 3D HW cannot render windowed, exiting..." << endl;
            exit(1);
        }

        D3DDISPLAYMODE dispmode;
        hr = Display.pD3D8->GetAdapterDisplayMode(Display.CardIDNum, &dispmode);

        if(FAILED(hr)) {
            wdxdisplay_cat.fatal() << "GetAdapterDisplayMode failed" << D3DERRORSTRING(hr);
            exit(1);
        }

        if(dispmode.Format == D3DFMT_P8) {
            wdxdisplay_cat.fatal() << "Can't run windowed in an 8-bit or less display mode" << endl;
            exit(1);
        }

        pPresParams->FullScreen_PresentationInterval = 0;

        if(dx_multisample_antialiasing_level<2) {
            if(dx_sync_video) {
                pPresParams->SwapEffect = D3DSWAPEFFECT_COPY_VSYNC;
            } else {
                pPresParams->SwapEffect = D3DSWAPEFFECT_DISCARD;  //D3DSWAPEFFECT_COPY;  does this make any difference?
            }
        } else {
            pPresParams->SwapEffect = D3DSWAPEFFECT_DISCARD;
        }

        assert((dwRenderWidth==pPresParams->BackBufferWidth)&&(dwRenderHeight==pPresParams->BackBufferHeight));

        hr = pD3D8->CreateDevice(Display.CardIDNum, D3DDEVTYPE_HAL, _pParentWindowGroup->_hParentWindow,
                                 dwBehaviorFlags, pPresParams, &Display.pD3DDevice);

        if(FAILED(hr)) {
            wdxdisplay_cat.fatal() << "D3D CreateDevice failed for device #" << Display.CardIDNum << D3DERRORSTRING(hr);
            exit(1);
        }
    }  // end create windowed buffers

//  ========================================================

    PRINT_REFCNT(wdxdisplay,_dxgsg->scrn.pD3DDevice);

    if(pPresParams->EnableAutoDepthStencil) {
        _dxgsg->_buffer_mask |= RenderBuffer::T_depth;
        if(IS_STENCIL_FORMAT(pPresParams->AutoDepthStencilFormat))
            _dxgsg->_buffer_mask |= RenderBuffer::T_stencil;
    }

    init_resized_window();

    return;

Fallback_to_16bpp_buffers:

    if((!IS_16BPP_DISPLAY_FORMAT(pPresParams->BackBufferFormat)) &&
       (Display.SupportedScreenDepthsMask & (R5G6B5_FLAG|X1R5G5B5_FLAG))) {
            // fallback strategy, if we trying >16bpp, fallback to 16bpp buffers

            Display.DisplayMode.Format = ((Display.SupportedScreenDepthsMask & R5G6B5_FLAG) ? D3DFMT_R5G6B5 : D3DFMT_X1R5G5B5);

            dx_force_16bpp_zbuffer=true;
            if(wdxdisplay_cat.info())
               wdxdisplay_cat.info() << "CreateDevice failed with out-of-vidmem, retrying w/16bpp buffers on device #"<< Display.CardIDNum << endl;
            CreateScreenBuffersAndDevice(Display);
            return;
    } else if(!((dwRenderWidth==640)&&(dwRenderHeight==480))) {
        if(wdxdisplay_cat.info())
           wdxdisplay_cat.info() << "CreateDevice failed w/out-of-vidmem, retrying at 640x480 w/16bpp buffers on device #"<< Display.CardIDNum << endl;
        // try final fallback to 640x480x16
        dx_force_16bpp_zbuffer=true;
        Display.DisplayMode.Width=640;
        Display.DisplayMode.Height=480;
        CreateScreenBuffersAndDevice(Display);
        return;
    } else {
        exit(1);
    }
}

// assumes CreateDevice or Device->Reset() has just been called,
// and the new size is specified in _dxgsg->scrn.PresParams
void wdxGraphicsWindow::init_resized_window(void) {
    DXScreenData *pDisplay=&_dxgsg->scrn;
    HRESULT hr;

    DWORD newWidth = pDisplay->PresParams.BackBufferWidth;
    DWORD newHeight = pDisplay->PresParams.BackBufferHeight;

    if(pDisplay->PresParams.Windowed) {
        POINT ul,lr;
        RECT client_rect;

        // need to figure out x,y origin offset of window client area on screen
        // (we already know the client area size)

        GetClientRect(pDisplay->hWnd, &client_rect);
        ul.x=client_rect.left;  ul.y=client_rect.top;
        lr.x=client_rect.right;  lr.y=client_rect.bottom;
        ClientToScreen(pDisplay->hWnd, &ul);
        ClientToScreen(pDisplay->hWnd, &lr);
        client_rect.left=ul.x; client_rect.top=ul.y;
        client_rect.right=lr.x; client_rect.bottom=lr.y;
        _props._xorg = client_rect.left;  // _props should reflect view rectangle
        _props._yorg = client_rect.top;

        #ifdef _DEBUG
          // try to make sure GDI and DX agree on window client area size
          // but client rect will not include any offscreen areas, so dont
          // do check if window was bigger than screen (there are other bad
          // cases too, like when window is positioned partly offscreen,
          // or if window trim border make size bigger than screen)

          RECT desktop_rect;
          GetClientRect(GetDesktopWindow(), &desktop_rect);
          if((_props._xsize<RECT_XSIZE(desktop_rect)) && (_props._ysize<RECT_YSIZE(desktop_rect)))
              assert((RECT_XSIZE(client_rect)==newWidth)&&(RECT_YSIZE(client_rect)==newHeight));
        #endif
    }

    resized(newWidth,newHeight);  // update panda channel/display rgn info, _props.xsize, _props.ysize

    // clear window to black ASAP
    ClearToBlack(pDisplay->hWnd,_props);

    // clear textures and VB's out of video&AGP mem, so cache is reset
    hr = pDisplay->pD3DDevice->ResourceManagerDiscardBytes(0);
    if(FAILED(hr)) {
        wdxdisplay_cat.error() << "ResourceManagerDiscardBytes failed for device #" << pDisplay->CardIDNum << D3DERRORSTRING(hr);
    }

    // Create the viewport
    D3DVIEWPORT8 vp = {0,0,_props._xsize,_props._ysize,0.0f,1.0f};
    hr = pDisplay->pD3DDevice->SetViewport( &vp );
    if(FAILED(hr)) {
        wdxdisplay_cat.fatal() << "SetViewport failed for device #" << pDisplay->CardIDNum << D3DERRORSTRING(hr);
        exit(1);
    }

    _dxgsg->dx_init(_pParentWindowGroup->_hMouseCursor);
    // do not SetDXReady() yet since caller may want to do more work before letting rendering proceed

    SetCursor(_pParentWindowGroup->_hMouseCursor);
    set_cursor_visibility(true);
}

// does NOT override _props._bCursorIsVisible
INLINE void wdxGraphicsWindow::set_cursor_visibility(bool bVisible) {
  bool bValidDXPtrs;

  if(_use_dx8_cursor)
    bValidDXPtrs = (IS_VALID_PTR(_dxgsg) && IS_VALID_PTR(_dxgsg->scrn.pD3DDevice));

  if(_props._bCursorIsVisible) {
      if(_use_dx8_cursor) {
          ShowCursor(false);
          if(bValidDXPtrs)
              _dxgsg->scrn.pD3DDevice->ShowCursor(bVisible);
      } else {
         ShowCursor(bVisible);
      }
  } else {
      ShowCursor(false);
      if(_use_dx8_cursor && bValidDXPtrs)
          _dxgsg->scrn.pD3DDevice->ShowCursor(false);
  }
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

/*  dont need to override the defaults (which just go to gsg->begin_frame())
////////////////////////////////////////////////////////////////////
//     Function: begin_frame
//       Access:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::begin_frame(void) {
    GraphicsWindow::begin_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: end_frame
//       Access:
//  Description:  timer info, incs frame #
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::end_frame(void) {
    GraphicsWindow::end_frame();
}
*/

////////////////////////////////////////////////////////////////////
//     Function: handle_window_move
//       Access:
//  Description: we receive the new x and y position of the client
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::handle_window_move(int x, int y) {
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
void wdxGraphicsWindow::handle_mouse_exit(void) {
    _input_devices[0].set_pointer_out_of_window();
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

      while((!_window_active) && (!_return_control_to_app)) {
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
    register_type(_type_handle, "wdxGraphicsWindow8",
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

int wdxGraphicsWindow::
get_depth_bitwidth(void) {
    assert(_dxgsg!=NULL);
    if(_dxgsg->scrn.PresParams.EnableAutoDepthStencil)
       return _dxgsg->scrn.depth_buffer_bitdepth;
     else return 0;

// GetSurfaceDesc is not reliable, on GF2, GetSurfDesc returns 32bpp when you created a 24bpp zbuf
// instead store the depth used at creation time

//    DX_DECLARE_CLEAN(DDSURFACEDESC2, ddsd);
//    _dxgsg->_zbuf->GetSurfaceDesc(&ddsd);
//  return ddsd.ddpfPixelFormat.dwRGBBitCount;
}

/*
void wdxGraphicsWindow::
get_framebuffer_format(PixelBuffer::Type &fb_type, PixelBuffer::Format &fb_format) {
    assert(_dxgsg!=NULL);

    fb_type = PixelBuffer::T_unsigned_byte; 
    // this is sortof incorrect, since for F_rgb5 it's really 5 bits per channel
    //would have to change a lot of texture stuff to make this correct though

    if(IS_16BPP_DISPLAY_FORMAT(_dxgsg->scrn.PresParams.BackBufferFormat)) 
        fb_format = PixelBuffer::F_rgb5; 
     else fb_format = PixelBuffer::F_rgb; 
}
*/
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

    assert(_windows.size()>0);
    _hOldForegroundWindow=GetForegroundWindow();
    _bClosingAllWindows= false;

    UINT num_windows=_windows.size();

    #define D3D8_NAME "d3d8.dll"
    #define D3DCREATE8 "Direct3DCreate8"

    _hD3D8_DLL = LoadLibrary(D3D8_NAME);
    if(_hD3D8_DLL == 0) {
        wdxdisplay_cat.fatal() << "PandaDX8 requires DX8, can't locate " << D3D8_NAME <<"!\n";
        exit(1);
    }

    _hMouseCursor = NULL;
    _bLoadedCustomCursor = false;

    _pDInputInfo = NULL;

    // can only get multimon HW acceleration in fullscrn on DX7

    UINT numMonitors = GetSystemMetrics(SM_CMONITORS);

    if(numMonitors < num_windows) {
        if(numMonitors==0) {
             numMonitors=1;   //win95 system will fail this call
          } else {
              wdxdisplay_cat.fatal() << "system has only "<< numMonitors << " monitors attached, couldn't find enough devices to meet multi window reqmt of " << num_windows << endl;
              exit(1);
          }
    }

   // Do all DX7 stuff first
   //  find_all_card_memavails();

    LPDIRECT3D8 pD3D8;

    typedef LPDIRECT3D8 (WINAPI *Direct3DCreate8_ProcPtr)(UINT SDKVersion);

    // dont want to statically link to possibly non-existent d3d8 dll, so must call D3DCr8 indirectly
    Direct3DCreate8_ProcPtr D3DCreate8_Ptr =
        (Direct3DCreate8_ProcPtr) GetProcAddress(_hD3D8_DLL, D3DCREATE8);

    if(D3DCreate8_Ptr == NULL) {
        wdxdisplay_cat.fatal() << "GetProcAddress for "<< D3DCREATE8 << "failed!" << endl;
        exit(1);
    }

// these were taken from the 8.0 and 8.1 d3d8.h SDK headers
#define D3D_SDK_VERSION_8_0  120
#define D3D_SDK_VERSION_8_1  220

    // are we using 8.0 or 8.1?
    WIN32_FIND_DATA TempFindData;
    HANDLE hFind;
    char tmppath[MAX_PATH];
    GetSystemDirectory(tmppath,MAX_PATH);
    strcat(tmppath,"\\dpnhpast.dll");
    hFind = FindFirstFile ( tmppath,&TempFindData );
    if(hFind != INVALID_HANDLE_VALUE) {
         FindClose(hFind);
         _bIsDX81=true;
         pD3D8 = (*D3DCreate8_Ptr)(D3D_SDK_VERSION_8_1);
    } else {
        _bIsDX81=false;
        pD3D8 = (*D3DCreate8_Ptr)(D3D_SDK_VERSION_8_0);
    }

    if(pD3D8==NULL) {
        wdxdisplay_cat.fatal() << D3DCREATE8 << " failed!\n";
        exit(1);
    }

    _numAdapters = pD3D8->GetAdapterCount();
    if(_numAdapters < num_windows) {
        wdxdisplay_cat.fatal() << "couldn't find enough devices attached to meet multi window reqmt of " << num_windows << endl;
        exit(1);
    }

    for(UINT i=0;i<_numAdapters;i++) {
        D3DADAPTER_IDENTIFIER8 adapter_info;
        ZeroMemory(&adapter_info,sizeof(D3DADAPTER_IDENTIFIER8));
        hr = pD3D8->GetAdapterIdentifier(i,D3DENUM_NO_WHQL_LEVEL,&adapter_info);
        if(FAILED(hr)) {
            wdxdisplay_cat.fatal() << "D3D GetAdapterID failed" << D3DERRORSTRING(hr);
        }

        LARGE_INTEGER *DrvVer=&adapter_info.DriverVersion;

        wdxdisplay_cat.info() << "D3D8 Adapter[" << i << "]: " << adapter_info.Description <<
                               ", Driver: " << adapter_info.Driver << ", DriverVersion: ("
            << HIWORD(DrvVer->HighPart) << "." << LOWORD(DrvVer->HighPart) << "."
            << HIWORD(DrvVer->LowPart) << "." << LOWORD(DrvVer->LowPart) << ")\nVendorID: 0x"
            << (void*) adapter_info.VendorId << " DeviceID: 0x" <<  (void*) adapter_info.DeviceId
            << " SubsysID: 0x" << (void*) adapter_info.SubSysId << " Revision: 0x"
            << (void*) adapter_info.Revision << endl;

        HMONITOR hMon=pD3D8->GetAdapterMonitor(i);
        if(hMon==NULL) {
            wdxdisplay_cat.info() << "D3D8 Adapter[" << i << "]: seems to be disabled, skipping it\n";
            continue;
        }

        DXDeviceInfo devinfo;
        ZeroMemory(&devinfo,sizeof(devinfo));
        memcpy(&devinfo.guidDeviceIdentifier,&adapter_info.DeviceIdentifier,sizeof(GUID));
        strncpy(devinfo.szDescription,adapter_info.Description,MAX_DEVICE_IDENTIFIER_STRING);
        strncpy(devinfo.szDriver,adapter_info.Driver,MAX_DEVICE_IDENTIFIER_STRING);
        devinfo.VendorID=adapter_info.VendorId;
        devinfo.DeviceID=adapter_info.DeviceId;
        devinfo.hMon=hMon;
        devinfo.cardID=i;

        _DeviceInfoVec.push_back(devinfo);
    }

    for(UINT i=0;i<num_windows;i++) {
        _windows[i]->config_window(this);
    }

    UINT good_device_count=0;

    if(num_windows==1) {
        UINT D3DAdapterNum = D3DADAPTER_DEFAULT;

        if(dx_preferred_deviceID!=-1) {
            if(dx_preferred_deviceID>=(int)_numAdapters) {
                wdxdisplay_cat.fatal() << "invalid 'dx-preferred-device-id', valid values are 0-" << _numAdapters-1 << ", using default adapter 0 instead\n";
            } else D3DAdapterNum=dx_preferred_deviceID;
        }
        if(_windows[0]->search_for_device(pD3D8,&(_DeviceInfoVec[D3DAdapterNum])))
            good_device_count=1;
    } else {
        for(UINT devnum=0;devnum<_DeviceInfoVec.size() && (good_device_count < num_windows);devnum++) {
            if(_windows[devnum]->search_for_device(pD3D8,&(_DeviceInfoVec[devnum])))
                good_device_count++;
        }
    }

    if(good_device_count < num_windows) {
      if(good_device_count==0)
         wdxdisplay_cat.fatal() << "no usable display devices, exiting...\n";
       else wdxdisplay_cat.fatal() << "multi-device request for " << num_windows << "devices, found only "<< good_device_count << " usable ones, exiting!";
      exit(1);
    }

    _DeviceInfoVec.clear();  // dont need this anymore

    if(wdxdisplay_cat.is_debug() && (g_pCardIDVec!=NULL)) {
      // print out the MaxAvailVidMems
      for(UINT i=0;i<_windows.size();i++) {
        D3DADAPTER_IDENTIFIER8 adapter_info;
        pD3D8->GetAdapterIdentifier(_windows[i]->_dxgsg->scrn.CardIDNum,D3DENUM_NO_WHQL_LEVEL,&adapter_info);
        wdxdisplay_cat.info() << "D3D8 Adapter[" << i << "]: " << adapter_info.Description
                              << ", MaxAvailVideoMem: " << _windows[i]->_dxgsg->scrn.MaxAvailVidMem
                              << ", IsLowVidMemCard: " << (_windows[i]->_dxgsg->scrn.bIsLowVidMemCard ? "true" : "false") << endl;
      }
    }

    CreateWindows();  // creates win32 windows  (need to do this before Setting coopLvls and display modes,
                      // but after we have all the monitor handles needed by CreateWindow()

//    SetCoopLevelsAndDisplayModes();

    if(dx_show_fps_meter)
       _windows[0]->_dxgsg->_bShowFPSMeter = true;  // just show fps on 1st mon

    for(UINT i=0;i<num_windows;i++) {
        _windows[i]->CreateScreenBuffersAndDevice(_windows[i]->_dxgsg->scrn);
    }

    for(UINT i=0;i<num_windows;i++) {
        _windows[i]->finish_window_setup();
    }

    SAFE_DELETE(g_pCardIDVec);  // dont need this anymore

    for(UINT i=0;i<num_windows;i++) {
        _windows[i]->_dxgsg->SetDXReady(true);
    }

    dx_pick_best_screenres = false;   // only want to do this on startup, not resize

  #ifdef DINPUT_DEBUG_POLL
    if(dx_use_joystick) {
        _pDInputInfo = new DInput8Info;
        assert(_pDInputInfo !=NULL);
       if(!_pDInputInfo->InitDirectInput()) {
           wdxdisplay_cat.error() << "InitDirectInput failed!\n";
           exit(1);
       }

       if(!_pDInputInfo->CreateJoystickOrPad(_hParentWindow)) {  // associate w/parent window of group for now
           wdxdisplay_cat.error() << "CreateJoystickOrPad failed!\n";
           exit(1);
       }

        // for now, just set up a WM_TIMER to poll the joystick.
        // could configure it to do event-based input, and that is default w/action mapping
        // which would be better, less processor intensive

        #define POLL_FREQUENCY_HZ  3
        _pDInputInfo->_JoystickPollTimer = SetTimer(_hParentWindow, JOYSTICK_POLL_TIMER_ID, 1000/POLL_FREQUENCY_HZ, NULL);
        if(_pDInputInfo->_JoystickPollTimer!=JOYSTICK_POLL_TIMER_ID) {
           wdxdisplay_cat.error() << "Error in joystick SetTimer!\n";
       }
    }
  #endif
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

    SAFE_DELETE(_pDInputInfo);

    for(UINT i=0;i<_windows.size();i++) {
        _windows[i]->close_window();
    }

    if((_hOldForegroundWindow!=NULL) /*&& (scrn.hWnd==GetForegroundWindow())*/) {
        SetForegroundWindow(_hOldForegroundWindow);
    }

    if(_bLoadedCustomCursor && (_hMouseCursor!=NULL))
      DestroyCursor(_hMouseCursor);

    if(_hD3D8_DLL != NULL) {
       FreeLibrary(_hD3D8_DLL);
       _hD3D8_DLL = NULL;
    }
}

