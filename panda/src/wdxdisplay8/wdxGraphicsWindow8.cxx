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

#define D3D_OVERLOADS
//#define  INITGUID  dont want this if linking w/dxguid.lib
#include <d3d8.h>
#include <ddraw.h>

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

#define MOUSE_ENTERED 0
#define MOUSE_EXITED 1
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
        
            SET_MOUSE_COORD(x,LOWORD(lparam));
            SET_MOUSE_COORD(y,HIWORD(lparam));

            if(mouse_motion_enabled()
               && wparam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)) {
                handle_mouse_motion(x, y);
            } else if(mouse_passive_motion_enabled() &&
                      ((wparam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)) == 0)) {
                handle_mouse_motion(x, y);
            }
            return 0;
      #if 0
        case WM_SYSCHAR:
        case WM_CHAR:  // shouldnt receive WM_CHAR unless WM_KEYDOWN stops returning 0 and passes on to DefWindProc
            break;
      #endif

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN: {

            POINT point;

            GetCursorPos(&point);
            ScreenToClient(hwnd, &point);

          #ifdef NDEBUG
               handle_keypress(lookup_key(wparam), point.x, point.y);
          #else
            // handle Cntrl-V paste from clipboard
            if(!((wparam=='V') && (GetKeyState(VK_CONTROL) < 0))) {
               handle_keypress(lookup_key(wparam), point.x, point.y);
            } else {
                HGLOBAL hglb; 
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
          #endif
            // want to use defwindproc on Alt syskey so Alt-F4 works, etc
            // but do want to bypass defwindproc F10 behavior (it activates the
            // main menu, but we have none)
            if((msg==WM_SYSKEYDOWN)&&(wparam!=VK_F10))
              break;
             else return 0;
        }

        case WM_SYSKEYUP:
        case WM_KEYUP: {
            handle_keyrelease(lookup_key(wparam));
            return 0;
        }

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

        case WM_SETCURSOR:
            // Turn off any GDI window cursor 
            SetCursor( NULL );
            _dxgsg->scrn.pD3DDevice->ShowCursor(true);
            return TRUE; // prevent Windows from setting cursor to window class cursor (see docs on WM_SETCURSOR)
            break;

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
                if(_dxgsg==NULL)
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

            //    if(_props._fullscreen && _window_inactive) {
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
            // wdxdisplay_cat.info() << "got WM_SETFOCUS\n";
            if(!DXREADY) {
              break;
            }

            if(_mouse_entry_enabled)
                handle_mouse_entry(MOUSE_ENTERED,_pParentWindowGroup->_hMouseCursor);

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
            // wdxdisplay_cat.info() << "got WM_KILLFOCUS\n";
            if(!DXREADY) {
              break;
            }

            if(_mouse_entry_enabled)
                  handle_mouse_entry(MOUSE_EXITED,_pParentWindowGroup->_hMouseCursor);

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

            if((wparam==_PandaPausedTimer) && (_window_inactive||_active_minimized_fullscreen)) {
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

        //case WM_CREATE:
        //        break;

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

   if(_window_inactive || _exiting_window || _active_minimized_fullscreen) {
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

       _window_inactive = true;
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
    if(_window_inactive) {
    
        // first see if dx cooperative level is OK for reactivation
    //    if(!_dxgsg->CheckCooperativeLevel())
    //        return;
    
        if(wdxdisplay_cat.is_spam())
            wdxdisplay_cat.spam() << "WDX window re-activated...\n";
    
        _window_inactive = false;
    
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
    _bLoadedCustomCursor=false;

    // Clear before filling in window structure!
    ZeroMemory(&wc, sizeof(WNDCLASS));
    wc.style      = CS_HREDRAW | CS_VREDRAW; //CS_OWNDC;
    wc.lpfnWndProc    = (WNDPROC) static_window_proc;
    wc.hInstance      = hProgramInstance;
    wc.hCursor      = NULL;  // for DX8 we handle the cursor messages ourself


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

    DWORD base_window_style = WS_POPUP | WS_SYSMENU;  // for CreateWindow


    global_wdxwinptr = _windows[0];  // for use during createwin()  bugbug look at this again

    HINSTANCE hUser32 = NULL;

    // rect now contains the coords for the entire window, not the client
    if(dx_full_screen) {
        // get upper-left corner coords using GetMonitorInfo

        // GetMonInfo doesnt exist on w95, so dont statically link to it
        hUser32 = (HINSTANCE) LoadLibrary("user32.dll");
        assert(hUser32);
        typedef BOOL (WINAPI* LPGETMONITORINFO)(HMONITOR, LPMONITORINFO);   
        LPGETMONITORINFO pfnGetMonitorInfo = (LPGETMONITORINFO) GetProcAddress(hUser32, "GetMonitorInfoA");
    }

    // extra windows must be parented to the first so app doesnt minimize when user selects them

    for(int devnum=0;devnum<_windows.size();devnum++) {
        DWORD xleft,ytop,xsize,ysize;
        GraphicsWindow::Properties *props = &_windows[devnum]->_props;
        DWORD final_window_style;

        final_window_style=base_window_style;

        if(dx_full_screen) {
            MONITORINFO minfo;
            ZeroMemory(&minfo, sizeof(MONITORINFO));
            minfo.cbSize = sizeof(MONITORINFO);
            if(pfnGetMonitorInfo)
                (*pfnGetMonitorInfo)(_windows[devnum]->_dxgsg->scrn.hMon, &minfo);
             else {
                 minfo.rcMonitor.left = minfo.rcMonitor.top = 0;
             }
            xleft=minfo.rcMonitor.left;
            ytop=minfo.rcMonitor.top;
            xsize=props->_xsize;
            ysize=props->_ysize;
        } else {
            RECT win_rect;
            SetRect(&win_rect, props->_xorg,  props->_yorg, props->_xorg + props->_xsize,
                    props->_yorg + props->_ysize);
    
            if(props->_border)
                final_window_style |= WS_OVERLAPPEDWINDOW;  // should we just use WS_THICKFRAME instead?
    
            AdjustWindowRect(&win_rect, window_style, FALSE);  //compute window size based on desired client area size
    
            // make sure origin is on screen
            if(win_rect.left < 0) {
                win_rect.right -= win_rect.left; win_rect.left = 0;
            }
            if(win_rect.top < 0) {
                win_rect.bottom -= win_rect.top; win_rect.top = 0;
            }

            xleft=win_rect.left;
            ytop= win_rect.top;
            xsize=win_rect.right-win_rect.left;
            ysize=win_rect.bottom-win_rect.top;
        }

        // BUGBUG: this sets window posns based on desktop arrangement of monitors (that is what GetMonInfo is for). 
        // need to move to chancfg stuff instead (i.e. use the properties x/yorg's) when that is ready
        HWND hWin = CreateWindow(WDX_WINDOWCLASSNAME, props->_title.c_str(),
                                  final_window_style, xleft,ytop,xsize,ysize,
                                  _hParentWindow, NULL, hProgramInstance, 0);

        if(!hWin) {
            wdxdisplay_cat.fatal() << "CreateWindow failed for monitor " << devnum << "!, LastError=" << GetLastError() << endl;
            #ifdef _DEBUG
               PrintErrorMessage(LAST_ERROR);
            #endif
            exit(1);
        }

        _windows[devnum]->_dxgsg->scrn.hWnd = hWin;
        if(devnum==0)
            _hParentWindow=hWin;
    }

    if(hUser32)
      FreeLibrary(hUser32);
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
        _windows[0]->_dxgsg->scrn.hWnd = _hParentWindow;
    }

    if(_hParentWindow==NULL) {
        wdxdisplay_cat.fatal() << "CreateWindow failed!\n";
        exit(1);
    }
*/
    for(int devnum=0;devnum<_windows.size();devnum++) {
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
    _window_inactive = false;
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
    _dxgsg->scrn.bIsDX81 = pParentGroup->_bIsDX81;
}

void wdxGraphicsWindow::finish_window_setup(void) {
    // init panda input handling
    _mouse_input_enabled = false;
    _mouse_motion_enabled = false;
    _mouse_passive_motion_enabled = false;
    _mouse_entry_enabled = false;

    // Enable detection of mouse input
    enable_mouse_input(true);
    enable_mouse_motion(true);
    enable_mouse_passive_motion(true);
    //  enable_mouse_entry(true);   re-enable this??

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


#if 0
HRESULT CALLBACK EnumDevicesCallback(LPSTR pDeviceDescription, LPSTR pDeviceName,
                                     LPD3DDEVICEDESC7 pD3DDeviceDesc,LPVOID pContext) {
    D3DDEVICEDESC7 *pd3ddevs = (D3DDEVICEDESC7 *)pContext;
#ifdef _DEBUG
    wdxdisplay_cat.spam() << "Enumerating Device " << pDeviceName << " : " << pDeviceDescription << endl;
#endif

#define REGHALIDX 0
#define TNLHALIDX 1

    // only saves hal and tnl devs, not sw rasts

    if(IsEqualGUID(pD3DDeviceDesc->deviceGUID,IID_IDirect3DHALDevice)) {
        CopyMemory(&pd3ddevs[REGHALIDX],pD3DDeviceDesc,sizeof(D3DDEVICEDESC7));
    } else if(IsEqualGUID(pD3DDeviceDesc->deviceGUID,IID_IDirect3DTnLHalDevice)) {
        CopyMemory(&pd3ddevs[TNLHALIDX],pD3DDeviceDesc,sizeof(D3DDEVICEDESC7));
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
#endif
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
*/
void wdxGraphicsWindow::resize(unsigned int xsize,unsigned int ysize) {

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
        return;
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
       return;
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
       return;
   }

   if(FAILED(hr = _dxgsg->scrn.pDD->TestCooperativeLevel())) {
        wdxdisplay_cat.error() << "TestCooperativeLevel failed : result = " << ConvD3DErrorToString(hr) << endl;
        wdxdisplay_cat.error() << "Full screen app failed to get exclusive mode on resize, exiting..\n";
        return;
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

bool wdxGraphicsWindow::FindBestDepthFormat(DXScreenData &Display,D3DFORMAT *pBestFmt,bool bWantStencil) {
    // list fmts in order of preference
    #define NUM_TEST_ZFMTS 3
    const static D3DFORMAT NoStencilPrefList={D3DFMT_D32,D3DFMT_D24X8,D3DFMT_D16};
    const static D3DFORMAT StencilPrefList={D3DFMT_D24S8,D3DFMT_D24X4S4,D3DFMT_D15S1};
    
    // int want_depth_bits = _props._want_depth_bits;  should we pay attn to these at some point?
    // int want_color_bits = _props._want_color_bits;

    *pBestFmt = D3DFMT_UNKNOWN;
    HRESULT hr;

    // nvidia likes zbuf depth to match rendertarget depth
    bool bOnlySelect16bpp= (dx_force_16bpp_zbuffer ||
        ((IS_NVIDIA(Display.DXDeviceID) && ((Display.DisplayMode.Format==D3DFMT_X1R5G5B5) ||
                                           (Display.DisplayMode.Format==D3DFMT_R5G5B5))));
    
    for(int i=0;i<NUM_TEST_ZFMTS;i++) {
        D3DFORMAT TestDepthFmt = (bWantStencil ? StencilPrefList[i] : NoStencilPrefList[i]);

        if(bOnlySelect16bpp && (TestDepthFmt!=D3DFMT_D16) && (TestDepthFmt!=D3DFMT_D15S1))
            continue;

        hr = Display.pD3D8->CheckDeviceFormat(Display.CardIDNum,D3DDEVTYPE_HAL,Display.DisplayMode.Format,
                                  D3DUSAGE_DEPTHSTENCIL,D3DRTYPE_SURFACE,TestDepthFmt);

        if(FAILED(hr)) 
          continue;
    
        hr = Display.pD3D8->CheckDepthStencilMatch(Display.CardIDNum,D3DDEVTYPE_HAL,
                                      Display.DisplayMode.Format,   // adapter format
                                      Display.DisplayMode.Format,   // backbuffer fmt  (should be the same in my apps)
                                      TestDepthFmt);
        if(!FAILED(hr)) {
           *pBestFmt = TestDepthFmt;
           break;
        }
    }

    return (*pBestFmt != D3DFMT_UNKNOWN);
}
    

// returns true if successful
bool wdxGraphicsWindow::search_for_device(LPDIRECT3D8 pD3D8,DXDeviceInfo *pDevinfo) {
    DWORD dwRenderWidth  = _props._xsize;
    DWORD dwRenderHeight = _props._ysize;
    HRESULT hr;

    assert(_dxgsg!=NULL);
    _dxgsg->scrn.pD3D8 = pD3D8;

    bool bWantStencil = ((_props._mask & W_STENCIL)!=0);

    hr = pD3D8->GetAdapterIdentifier(pDevInfo->cardID,D3DENUM_NO_WHQL_LEVEL,&_dxgsg->scrn.DXDeviceID);
    if(FAILED(hr)) {
       wdxdisplay_cat.fatal() << "D3D GetAdapterID failed" << D3DERRORSTRING(hr);
    }

    D3DCAPS8 d3dcaps;
    hr = pD3D8->GetDeviceCaps(pDevInfo->cardID,D3DDEVTYPE_HAL,&d3dcaps);
    if(FAILED(hr)) {
         if(hr==D3DERR_INVALIDDEVICE) {
             wdxdisplay_cat.fatal() << "No D3D-capable 3D hardware detected!  Exiting...\n";
         } else {
             wdxdisplay_cat.fatal() << "GetDeviceCaps failed, hr = " << D3DERRORSTRING(hr);
         }
         exit(1);
    }

    if(d3dcaps.MaxStreams==0) {
       wdxdisplay_cat.info() << "Warning: video driver predates DX8\n";
    }

    if(bWantStencil & (d3dcaps.StencilCaps==0x0)) {
       wdxdisplay_cat.fatal() << "Stencil ability requested, but device #" << pDevInfo->cardID << " (" << _dxgsg->scrn.DXDeviceID.szDescription<<"), has no stencil capability!\n";
       exit(1);
    }

/*
    LPDIRECTDRAW7 pDD;
    D3DDEVICEDESC7 d3ddevs[2];  // put HAL in 0, TnLHAL in 1

    // just look for HAL and TnL devices right now.  I dont think
    // we have any interest in the sw rasts at this point

    ZeroMemory(d3ddevs,2*sizeof(D3DDEVICEDESC7));

    hr = _dxgsg->scrn.pD3D->EnumDevices(EnumDevicesCallback,d3ddevs);
    if(hr != DD_OK) {
       wdxdisplay_cat.fatal() << "EnumDevices failed : result = " << ConvD3DErrorToString(hr) << endl;
        goto error_exit;
    }
    
    WORD DeviceIdx;
    DeviceIdx=REGHALIDX;

    if(!(d3ddevs[REGHALIDX].dwDevCaps & D3DDEVCAPS_HWRASTERIZATION )) {
       // should never get here because enum devices should filter out non-HAL devices
       wdxdisplay_cat.error() << "No 3D HW present on device #"<<devnum<<", skipping it... (" << _dxgsg->scrn.DXDeviceID.szDescription<<")\n";
       goto error_exit;
    }

    memcpy(&_dxgsg->scrn.D3DDevDesc,&d3ddevs[DeviceIdx],sizeof(D3DDEVICEDESC7));
*/      

    _dxgsg->scrn.bIsTNLDevice=((d3dcaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)!=0);

    // only way to get vidmem estimate prior to device creation and displaymode changing on DX8 is to use DX7 ddraw

    // Create the Direct Draw Objects
    hr = (*_pParentWindowGroup->_pDDCreateEx)(&pDevInfo->guidDeviceIdentifier,(void **)&pDD, IID_IDirectDraw7, NULL);
    if(hr != DD_OK) {
          wdxdisplay_cat.fatal() << "DirectDrawCreateEx failed for device ("<<pDevInfo->cardID<< "): result = " << D3DERRORSTRING(hr);
          return false;
    }
    
    // Get Current VidMem avail.  Note this is only an estimate, when we switch to fullscreen
    // mode from desktop, more vidmem will be available (typically 1.2 meg).  I dont want
    // to switch to fullscreen more than once due to the annoying monitor flicker, so try
    // to figure out optimal mode using this estimate
    DDSCAPS2 ddsGAVMCaps;
    DWORD dwVidMemTotal,dwVidMemFree;
    dwVidMemTotal=dwVidMemFree=0;
    ZeroMemory(&ddsGAVMCaps,sizeof(DDSCAPS2));
    ddsGAVMCaps.dwCaps = DDSCAPS_VIDEOMEMORY; //set internally by DX anyway, dont think this any different than 0x0
    if(FAILED(hr = pDD->GetAvailableVidMem(&ddsGAVMCaps,&dwVidMemTotal,&dwVidMemFree))) {
       wdxdisplay_cat.error() << "GetAvailableVidMem failed for device #"<<pDevInfo->cardID<<": result = " << D3DERRORSTRING(hr);
       // goto skip_device;
       exit(1);  // probably want to exit, since it may be my fault
    }

    pDD->Release();  // release DD obj, since this is all we needed it for
    
    // after SetDisplayMode, GetAvailVidMem totalmem seems to go down by 1.2 meg (contradicting above
    // comment and what I think would be correct behavior (shouldnt FS mode release the desktop vidmem?),
    // so this is the true value
    _dxgsg->scrn.MaxAvailVidMem = dwVidMemTotal;
    
    #define LOWVIDMEMTHRESHOLD 3500000
    #define CRAPPY_DRIVER_IS_LYING_VIDMEMTHRESHOLD 1000000
    
    // assume buggy drivers (this means you, FireGL2) may return zero for dwVidMemTotal, so ignore value if its < CRAPPY_DRIVER_IS_LYING_VIDMEMTHRESHOLD
    _dxgsg->scrn.bIsLowVidMemCard = ((dwVidMemTotal>CRAPPYDRIVERISLYING_VIDMEMTHRESHOLD) && (dwVidMemTotal< LOWVIDMEMTHRESHOLD));

    bool bNeedZBuffer = ((!(Display.d3dcaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR )) 
                         && (_props._mask & W_DEPTH));

    Display.PresentParams.EnableAutoDepthStencil=bNeedZBuffer;

    if(dx_full_screen) {
        _props._xorg = _props._yorg = 0;

        DWORD SupportedBitDepthMask = 0x0;
        int cNumModes=pD3D8->GetAdapterModeCount(pDevInfo->cardID);
        D3DDISPLAYMODE BestDispMode;        
        ZeroMemory(BestDispMode,sizeof(BestDispMode));

        bool bCouldntFindValidZBuf=false;

        for(i=0;i<cNumModes;i++) {
            D3DDISPLAYMODE dispmode;
            if(FAILED(hr = pD3D8->EnumAdapterDisplayModes(pDevInfo->cardID,i,&dispmode))) {
                wdxdisplay_cat.error() << "EnumAdapterDisplayMode failed for device #"<<pDevInfo->cardID<<": result = " << D3DERRORSTRING(hr);
                exit(1); 
            }

            if((dispmode.RefreshRate<60) && (dispmode.RefreshRate>1)) {
                // dont want refresh rates under 60Hz, but 0 or 1 might indicate a default refresh rate, which is usually >=60
                continue;
            }

            if((dispmode.Width==dwRenderWidth) && (dispmode.Height==dwRenderHeight))  {
                if(FAILED(hr = pD3D8->CheckDeviceFormat(pDevInfo->cardID,D3DDEVTYPE_HAL,dispmode.Format,
                                                        D3DUSAGE_RENDERTARGET,D3DRTYPE_SURFACE,dispmode.Format)) {
                   if(hr==D3DERR_NOTAVAILABLE)
                       continue;
                     else {
                         wdxdisplay_cat.error() << "CheckDeviceFormat failed for device #"<<pDevInfo->cardID<<": result = " << D3DERRORSTRING(hr);
                         exit(1); 
                     }
                }

                if(bNeedZBuffer) {
                    D3DFORMAT junk;
                    if(!FindBestDepthFormat(Display,&junk,bWantStencil)) {
                        bCouldntFindValidZBuf=true;
                        continue;
                    }
                }

                switch(dispmode.Format) {
                    case D3DFMT_X1R5G5B5:
                        _dxgsg->scrn.dwSupportedScreenDepths |= X1R5G5B5_FLAG;
                        break;
                    case D3DFMT_X8R8G8B8:
                        _dxgsg->scrn.dwSupportedScreenDepths |= X8R8G8B8_FLAG;
                        break;
                    case D3DFMT_R8G8B8:
                        _dxgsg->scrn.dwSupportedScreenDepths |= R8G8B8_FLAG;
                        break;
                    case D3DFMT_R5G6B5:
                        _dxgsg->scrn.dwSupportedScreenDepths |= R5G6B5_FLAG;
                        break;
                    default:
                        //Render target formats should be only D3DFMT_X1R5G5B5, D3DFMT_R5G6B5, D3DFMT_X8R8G8B8 (or R8G8B8?)
                        wdxdisplay_cat.debug() << "unrecognized supported screen D3DFMT returned by EnumAdapterDisplayModes!\n";
                }
            }
        }
/*        
        if(wdxdisplay_cat.is_info())
           wdxdisplay_cat.info() << "Before fullscreen switch: GetAvailableVidMem for device #"<<devnum<<" returns Total: " << dwVidMemTotal/1000000.0 << "  Free: " << dwVidMemFree/1000000.0 << endl;
        
        // Now we try to figure out if we can use requested screen resolution and best
        // rendertarget bpp and still have at least 2 meg of texture vidmem
        
        DMI.supportedBitDepths &= _dxgsg->scrn.D3DDevDesc.dwDeviceRenderBitDepth;
*/
        // note I'm not saving refresh rate, will just use adapter default at given res for now

        // note: this chooses 32bpp, which may not be preferred over 16 for memory & speed reasons on some older cards in particular

        D3DFORMAT pixFmt;

        if(_dxgsg->scrn.dwSupportedScreenDepths & X8R8G8B8_FLAG) 
            pixFmt = D3DFORMAT_X8R8G8B8;
        else if(_dxgsg->scrn.dwSupportedScreenDepths & R8G8B8_FLAG) 
            pixFmt = D3DFORMAT_R8G8B8;
        else if(_dxgsg->scrn.dwSupportedScreenDepths & R5G6B5_FLAG) 
            pixFmt = D3DFORMAT_R5G6B5;
        else if(_dxgsg->scrn.dwSupportedScreenDepths & X1R5G5B5_FLAG)
            pixFmt = D3DFORMAT_X1R5G5B5;
        else {
           if(bCouldntFindValidZBuf) {
               wdxdisplay_cat.fatal() << "Couldnt find valid zbuffer format to go with FullScreen mode at " << dwRenderWidth << "x" << dwRenderHeight 
                   << " for device #" << pDevInfo->cardID << " (" << _dxgsg->scrn.DXDeviceID.szDescription<<"), skipping device...\n";
           } else
               wdxdisplay_cat.fatal() << "No supported FullScreen modes at " << dwRenderWidth << "x" << dwRenderHeight 
                   << " for device #" << pDevInfo->cardID << " (" << _dxgsg->scrn.DXDeviceID.szDescription<<"), skipping device...\n";
           return false;
        }
        
        if(_dxgsg->scrn.bIsLowVidMemCard) {
               // hack: figuring out exactly what res to use is tricky, instead I will
               // just use 640x480 if we have < 3 meg avail
    
               if(_dxgsg->scrn.dwSupportedScreenDepths & R5G6B5_FLAG)
                   pixFmt = D3DFORMAT_R5G6B5;
               else if(_dxgsg->scrn.dwSupportedScreenDepths & X1R5G5B5_FLAG)
                   pixFmt = D3DFORMAT_X1R5G5B5;
               else {
                   wdxdisplay_cat.fatal() << "Low Memory VidCard has no supported FullScreen 16bpp resolutions at "<< dwRenderWidth << "x" << dwRenderHeight   
                   << " for device #" << pDevInfo->cardID << " (" << _dxgsg->scrn.DXDeviceID.szDescription<<"), skipping device...\n";
                   return false;
               }
               dwRenderWidth=640;
               dwRenderHeight=480;
               dx_force_16bpptextures = true;
        
               if(wdxdisplay_cat.is_info())
                   wdxdisplay_cat.info() << "Available VidMem (" << dwVidMemFree<<") is under " << LOWVIDMEMTHRESHOLD <<", using 640x480 16bpp rendertargets to save tex vidmem.\n";
        }
    }
    
    _dxgsg->scrn.DisplayMode.Width=dwRenderWidth;
    _dxgsg->scrn.DisplayMode.Height=dwRenderHeight;
    _dxgsg->scrn.DisplayMode.Format = pixFmt;
    _dxgsg->scrn.DisplayMode.RefreshRate = D3DPRESENT_RATE_DEFAULT;  
    _dxgsg->scrn.hMon=pDevinfo->hMon;
    _dxgsg->scrn.CardIDNum=pDevInfo->cardID; 
    return true;
}

#if 0
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

    for(int devnum=0;devnum<_windows.size();devnum++) {
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
#endif

//return true if successful
void wdxGraphicsWindow::
CreateScreenBuffersAndDevice(DXScreenData &Display) {

    DWORD dwRenderWidth=Display.dwRenderWidth;
    DWORD dwRenderHeight=Display.dwRenderHeight;
    LPDIRECT3D8 pD3D8=Display.pD3D;
//    LPDIRECTDRAW7 pDD=Display.pDD;
//    D3DDEVICEDESC7 *pD3DDevDesc=&Display.D3DDevDesc;
    LPD3DCAPS8 pD3Dcaps = &Display.d3dcaps;
    D3DPRESENT_PARAMETERS* pPresParams = &Display.PresParams;
    LPDIRECTDRAWSURFACE7 pPrimaryDDSurf,pBackDDSurf,pZDDSurf;
    LPDIRECT3DDEVICE7 pD3DDevice;
    RECT view_rect;
    int i;
    HRESULT hr;
    DX_DECLARE_CLEAN( DDSURFACEDESC2, SurfaceDesc );
    bool bWantStencil = ((_props._mask & W_STENCIL)!=0);
    DWORD dwBehaviorFlags;

    assert(pD3D8!=NULL);
    assert(pD3Dcaps->dwDevCaps & D3DDEVCAPS_HWRASTERIZATION);

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

//   DX_DECLARE_CLEAN(DDCAPS,DDCaps);
//   pDD->GetCaps(&DDCaps,NULL);

    // find zbuffer format

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
    if(FAILED(CheckDeviceFormat(Display.CardIDNum, D3DDEVTYPE_HAL, Display.DisplayMode.Format,D3DUSAGE_RENDERTARGET,
                           D3DRTYPE_SURFACE, pPresParams->BackBufferFormat))) {
        wdxdisplay_cat.error() << "device #"<<Display.cardIDNum<< " CheckDeviceFmt failed for surface fmt "<< D3DFormatStr(pPresParams->BackBufferFormat) << endl;
        goto Fallback_to_16bpp_buffers;
    }

    if(FAILED(pD3D8->CheckDeviceType(Display.CardIDNum,D3DDEVTYPE_HAL, Display.DisplayMode.Format,pPresParams->BackBufferFormat,
                                    dx_full_screen))) {
        wdxdisplay_cat.error() << "device #"<<Display.cardIDNum<< " CheckDeviceType failed for surface fmt "<< D3DFormatStr(pPresParams->BackBufferFormat) << endl;
        goto Fallback_to_16bpp_buffers;
    }
    
    if(Display.PresParams.EnableAutoDepthStencil) {
        if(!FindBestDepthFormat(Display,&Display.PresParams.AutoDepthStencilFormat,bWantStencil)) {
            wdxdisplay_cat.error() << "FindBestDepthFormat failed in CreateScreenBuffers for device #"<<Display.cardIDNum<< endl;
            goto Fallback_to_16bpp_buffers;
        }
    }

    pPresParams->Windowed = dx_full_screen;
    DWORD dwBehaviorFlags=0x0;
  
    if(dx_multisample_antialiasing_level>1) {
    // need to check both rendertarget and zbuffer fmts
        if( FAILED(pD3D8->CheckDeviceMultiSampleType(Display.CardIDNum, D3DDEVTYPE_HAL, Display.DisplayMode.Format,
                                                     dx_full_screen, D3DMULTISAMPLE_TYPE(dx_multisample_antialiasing_level)) {
            wdxdisplay_cat.fatal() << "device #"<<Display.cardIDNum<< " doesnt support multisample level "<<dx_multisample_antialiasing_level <<"surface fmt "<< D3DFormatStr(Display.DisplayMode.Format) <<endl;
            exit(1); 
        }
    
        if(Display.PresParams.EnableAutoDepthStencil) {
            if( FAILED(pD3D8->CheckDeviceMultiSampleType(Display.CardIDNum, D3DDEVTYPE_HAL, Display.PresParams.AutoDepthStencilFormat,
                                             dx_full_screen, D3DMULTISAMPLE_TYPE(dx_multisample_antialiasing_level)) {
                wdxdisplay_cat.fatal() << "device #"<<Display.cardIDNum<< " doesnt support multisample level "<<dx_multisample_antialiasing_level <<"surface fmt "<< D3DFormatStr(Display.PresParams.AutoDepthStencilFormat) <<endl;
                exit(1); 
            }
        }

        pPresParams->MultiSampleType = D3DMULTISAMPLE_TYPE(dx_multisample_antialiasing_level);
    
        if(wdxdisplay_cat.is_info())
            wdxdisplay_cat.info() << "device #"<<Display.cardIDNum<< " using multisample antialiasing level "<<dx_multisample_antialiasing_level <<endl;
    }

    pPresParams->BackBufferCount = 1;
    pPresParams->Flags = 0x0;
    pPresParams->hDeviceWindow = Display.hWnd;
    pPresParams->BackBufferWidth = Display.DisplayMode.Width;
    pPresParams->BackBufferHeight = Display.DisplayMode.Height;
    
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

    if(dx_full_screen) {
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
                wdxdisplay_cat.fatal() << "device #"<<Display.cardIDNum<< " cant support multisample antialiasing and fullscrn alphablending because of driver limitations and/or lack of DX8.1"<<dx_multisample_antialiasing_level <<endl;
                exit(1);
            }
        }
*/      
#endif
        #ifdef _DEBUG
        if(pPresParams->MultiSampleType != D3DMULTISAMPLE_NONE)
            assert(pPresParams->SwapEffect == D3DSWAPEFFECT_DISCARD);  // only valid effect for multisample
        #endif

        hr = pD3D8->CreateDevice(Display.CardIDNum, D3DDEVTYPE_HAL, _pParentWindowGroup->_hParentWindow,   
                                 dwBehaviorFlags, pPresParams, &Display.pD3DDevice);

        if(FAILED(hr)) {
            wdxdisplay_cat.fatal() << "D3D CreateDevice failed for device #" << Display.CardIDnum << ", " << D3DERRORSTRING(hr);

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
            wdxdisplay_cat.fatal() << "GetAdapterDisplayMode failed result = " << D3DERRORSTRING(hr);
            exit(1);
        }

        if(dispmode.Format == D3DFMT_P8) {
            wdxdisplay_cat.fatal() << "Can't run windowed in an 8-bit or less display mode" << endl;
            exit(1);
        }

        pPresParams->FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;   // I believe this irrelevant for windowed mode, but whatever

        if(dx_multisample_antialiasing_level<2) {
            if(dx_sync_video) {
                pPresParams->SwapEffect = D3DSWAPEFFECT_COPY_VSYNC;
            } else {
                pPresParams->SwapEffect = D3DSWAPEFFECT_DISCARD;  //D3DSWAPEFFECT_COPY;  does this make any difference?
            }
        } else {
            pPresParams->SwapEffect = D3DSWAPEFFECT_DISCARD;
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

        hr = pD3D8->CreateDevice(Display.CardIDNum, D3DDEVTYPE_HAL, _pParentWindowGroup->_hParentWindow,   
                                 dwBehaviorFlags, pPresParams, &Display.pD3DDevice);

        if(FAILED(hr)) {
            wdxdisplay_cat.fatal() << "D3D CreateDevice failed for device #" << Display.CardIDnum << ", " << D3DERRORSTRING(hr);
            exit(1);
        }
    }  // end create windowed buffers

    hr = Display.pD3DDevice->ResourceManagerDiscardBytes(0);
    if(hr != DD_OK) {
        wdxdisplay_cat.fatal() << "ResourceManagerDiscardBytes failed for device #" << Display.CardIDnum << ", " << D3DERRORSTRING(hr);
    }

    // clear to transparent black to get rid of visible garbage 
    hr = Display.pD3DDevice->Clear(0,NULL,D3DCLEAR_TARGET,0,0.0,0);
    if(hr != DD_OK) {
        wdxdisplay_cat.fatal() << "init Clear() to black failed for device #" << Display.CardIDnum << ", " << D3DERRORSTRING(hr);
    }

//  ========================================================

    resized(dwRenderWidth,dwRenderHeight);  // update panda channel/display rgn info

    // Create the viewport
    D3DVIEWPORT8 vp = { 0, 0, _props._xsize, _props._ysize, 0.0f, 1.0f};
    hr = pD3DDevice->SetViewport( &vp );
    if(hr != DD_OK) {
        wdxdisplay_cat.fatal() << "SetViewport failed for device #" << Display.CardIDnum << ", " << D3DERRORSTRING(hr);
        exit(1);
    }

    Display.pD3DDevice=pD3DDevice;
    Display.view_rect = view_rect;

    _dxgsg->dx_init();
    // do not SetDXReady() yet since caller may want to do more work before letting rendering proceed

    return;

Fallback_to_16bpp_buffers:

    if(!IS_16BPP_FORMAT(pPresParams->BackBufferFormat) &&
       (Display.dwSupportedScreenDepths & (R5G6B5_FLAG|X1R5G5B5_FLAG))) {
            // fallback strategy, if we trying >16bpp, fallback to 16bpp buffers

            if(Display.dwSupportedScreenDepths & R5G6B5_FLAG)
              Display.DisplayMode.Format = D3DFORMAT_R5G6B5;
              else Display.DisplayMode.Format = D3DFORMAT_X1R5G5B5;

            dx_force_16bpp_zbuffer=true;
            if(wdxdisplay_cat.info())
               wdxdisplay_cat.info() << "CreateDevice failed, retrying w/16bpp buffers on device #"<< Display.CardIDNum<< ", hr = " << D3DERRORSTRING(hr);
            CreateScreenBuffersAndDevice(Display);
            return;
    } else {
        exit(1);
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
//     Function: handle_mouse_entry
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::handle_mouse_entry(int state,HCURSOR hCursor) {
    if(state == MOUSE_EXITED) {
        _input_devices[0].set_pointer_out_of_window();
    } else {
//        SetCursor(hCursor);  believe this is not necessary, handled by windows
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

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow::supports_update
//       Access: Public, Virtual
//  Description: Returns true if this particular kind of
//               GraphicsWindow supports use of the update() function
//               to update the graphics one frame at a time, so that
//               the window does not need to be the program's main
//               loop.  Returns false if the only way to update the
//               window is to call main_loop().
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow::
supports_update() const {
    return true;
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
//     Function: update
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::update(void) {
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
void wdxGraphicsWindow::enable_mouse_input(bool val) {
    _mouse_input_enabled = val;
}

////////////////////////////////////////////////////////////////////
//     Function: enable_mouse_motion
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::enable_mouse_motion(bool val) {
    _mouse_motion_enabled = val;
}

////////////////////////////////////////////////////////////////////
//     Function: enable_mouse_passive_motion
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::enable_mouse_passive_motion(bool val) {
    _mouse_passive_motion_enabled = val;
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
            if(isascii(key) && key != 0) {
                bool bCapsLockDown=((GetKeyState(VK_CAPITAL) & 0x1)!=0);
                bool bShiftUp = (GetKeyState(VK_SHIFT) >= 0);
                if(bShiftUp) {
                    if(bCapsLockDown) 
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
                            if(bCapsLockDown) 
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
    int i;

    assert(_windows.size()>0);
    _hOldForegroundWindow=GetForegroundWindow();
    _bClosingAllWindows= false;

    int num_windows=_windows.size();

    #define D3D8_NAME "d3d8.dll"

    _hD3D8_DLL = LoadLibrary(D3D8_NAME);
    if(_hD3D8_DLL == 0) {
        wdxdisplay_cat.fatal() << "PandaDX8 requires DX8, can't locate " << D3D8_NAME <<"!\n";
        exit(1);
    }

    #define DDRAW_NAME "ddraw.dll"

    _hDDrawDLL = LoadLibrary(DDRAW_NAME);
    if(_hDDrawDLL == 0) {
        wdxdisplay_cat.fatal() << "can't locate " << DDRAW_NAME <<"!\n";
        exit(1);
    }

    _pDDCreateEx = (LPDIRECTDRAWCREATEEX) GetProcAddress(_hDDrawDLL,"DirectDrawCreateEx");
    if(_pDDCreateEx == NULL) {
        wdxdisplay_cat.fatal() << "Panda currently requires at least DirectX 7.0!\n";
        exit(1);
    }

    _hMouseCursor = NULL;
    _bLoadedCustomCursor = false;

    // can only get multimon HW acceleration in fullscrn on DX7

    int numMonitors = GetSystemMetrics(SM_CMONITORS);

    if(numMonitors < num_windows) {
        if(numMonitors==0) {
             numMonitors=1;   //win95 system will fail this call
          } else {
              wdxdisplay_cat.fatal() << "system has only "<< numMonitors << " monitors attached, couldn't find enough devices to meet multi window reqmt of " << num_windows << endl;
              exit(1);
          }
    }     

    LPDIRECT3D8 pD3D8;

    typedef LPDIRECT3D8 (WINAPI *Direct3DCreate8_ProcPtr)(UINT SDKVersion);
 
    // dont want to statically link to possibly non-existent d3d8 dll, so must call D3DCr8 indirectly
    Direct3DCreate8_ProcPtr D3DCreate8_Ptr = 
        (Direct3DCreate8_ProcPtr) GetProcAddress(_hD3D8_DLL, "Direct3DCreate8" );

    if(D3DCreate8_Ptr == NULL) {
        wdxdisplay_cat.fatal() << "GetProcAddress for D3DCreate8 failed!" << endl;
        exit(1);
    }

// these were taken from the 8.0 and 8.1 d3d8.h SDK headers
#define D3D_SDK_VERSION_8_0  120
#define D3D_SDK_VERSION_8_1  220

    // are we using 8.0 or 8.1?
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind = FindFirstFile("dpnhpast.dll",&FindFileData);  // this dll exists on 8.1 but not 8.0

    if(hFind != INVALID_HANDLE_VALUE) {
         FindClose(hFind);
         _bIsDX81=true;
         pD3D8 = (*D3DCreate8_Ptr)(D3D_SDK_VERSION_8_1);
    } else {
        _bIsDX81=false;
        pD3D8 = (*D3DCreate8_Ptr)(D3D_SDK_VERSION_8_0);
    }

    if(pD3D8==NULL) {
        wdxdisplay_cat.fatal() << "Direct3DCreate8 failed!\n";
        exit(1);
    }


    _numAdapters = pD3DI->GetAdapterCount();
    if(_numAdapters < num_windows) {
        wdxdisplay_cat.fatal() << "couldn't find enough devices attached to meet multi window reqmt of " << num_windows << endl;
        exit(1);
    }

    for(int i=0;i<_numAdapters;i++) {
        D3DADAPTER_IDENTIFIER8 adapter_info;
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
            << " SubsysID: 0x" << (void*) adapter_info.SubsysId << " Revision: 0x"
            << (void*) adapter_info.Revision << endl;

        DXDeviceInfo devinfo;
        ZeroMemory(&devinfo,sizeof(devinfo));
        memcpy(&devinfo.guidDeviceIdentifier,&adapter_info.DeviceIdentifier,sizeof(GUID));
        strncpy(devinfo.szDescription,&adapter_info.Description,MAX_DDDEVICEID_STRING);
        strncpy(devinfo.szDriver,&adapter_info.Driver,MAX_DDDEVICEID_STRING);
        devinfo.hMon=pD3D8->GetAdapterMonitor(i);
        devinfo.cardID=i;
        _DeviceInfoVec.push_back(devinfo);
    }

    for(i=0;i<num_windows;i++) {
        _windows[i]->config_window(this);
    }

    int good_device_count=0;

    if(num_windows==1) {
        UINT D3DAdapterNum = D3DADAPTER_DEFAULT;

        if(dx_preferred_deviceID!=-1) {
            if(dx_preferred_deviceID>=_numAdapters) {
                wdxdisplay_cat.fatal() << "invalid dx-preferred-deviceID, valid values are 0-" << _numAdapters << ", using default adapter instead\n";
            } else D3DAdapterNum=dx_preferred_deviceID;
        }
        if(_windows[0]->search_for_device(D3DAdapterNum,&(_DeviceInfoVec[devnum])))
            good_device_count=1;
    } else {
        for(int devnum=0;devnum<_DeviceInfoVec.size() && (good_device_count < num_windows);devnum++) {
            if(_windows[devnum]->search_for_device(devnum,&(_DeviceInfoVec[devnum])))
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

    CreateWindows();  // creates win32 windows  (need to do this before Setting coopLvls and display modes, 
                      // but after we have all the monitor handles needed by CreateWindow()

//    SetCoopLevelsAndDisplayModes();

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
   // just call the GraphicsWindow constructor, have to hold off rest of init
}

wdxGraphicsWindowGroup::~wdxGraphicsWindowGroup() {
    // this fn must be called before windows are actually closed
    _bClosingAllWindows= true;

    for(int i=0;i<_windows.size();i++) {
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

    if(_hD3D8_DLL != NULL) {
       FreeLibrary(_hD3D8_DLL);
       _hD3D8_DLL = NULL;
    }
}

