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
#include "dxGraphicsStateGuardian.h"
#include "config_wdxdisplay.h"

#include <keyboardButton.h>
#include <mouseButton.h>

#include <throw_event.h>

#ifdef DO_PSTATS
#include <pStatTimer.h>
#endif

#define D3D_OVERLOADS
//#define  INITGUID  dont want this if linking w/dxguid.lib
#include <d3d.h>

#include <map>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle wdxGraphicsWindow::_type_handle;

#define LAST_ERROR 0
#define ERRORBOX_TITLE "Panda3D Error"
#define WDX_WINDOWCLASSNAME "wdxDisplay"

typedef map<HWND,wdxGraphicsWindow *> HWND_PANDAWIN_MAP;

HWND_PANDAWIN_MAP hwnd_pandawin_map;
wdxGraphicsWindow* global_wdxwinptr = NULL;  // need this for temporary windproc

#define MAX_DEVICES 20

extern bool dx_full_screen_antialiasing;  // defined in dxgsg_config.cxx

#define MOUSE_ENTERED 0
#define MOUSE_EXITED 1
#define PAUSED_TIMER_ID  7   // completely arbitrary choice
#define DXREADY ((_dxgsg!=NULL)&&(_dxgsg->GetDXReady()))

LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam,LPARAM lparam);

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

  if(_dxgsg!=NULL) {
      _dxgsg->dx_cleanup(_props._fullscreen, bAtExitFnCalled);
      _dxgsg=NULL;
  }

  if(_hdc!=NULL) {
    ReleaseDC(_mwindow,_hdc);
    _hdc = NULL;
  }

  if((_hOldForegroundWindow!=NULL) && (_mwindow==GetForegroundWindow())) {
      SetForegroundWindow(_hOldForegroundWindow);
  }

  if(_mwindow!=NULL) {
      if(_bLoadedCustomCursor && _hMouseCursor!=NULL)
          DestroyCursor(_hMouseCursor);

    DestroyWindow(_mwindow);
    hwnd_pandawin_map.erase(_mwindow);
    _mwindow = NULL;
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
                if((_mwindow==NULL) || dx_full_screen || ((wparam != SIZE_RESTORED) && (wparam != SIZE_MAXIMIZED)))
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
                handle_mouse_entry(MOUSE_ENTERED,_hMouseCursor);

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
                  handle_mouse_entry(MOUSE_EXITED,_hMouseCursor);

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
          close_window();

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

        if(_dxgsg->GetBackBuffer()==NULL) {
            //assume this is initial creation reshape and ignore this call
            return;
        }

        // Clear the back/primary surface to black
        DX_DECLARE_CLEAN(DDBLTFX, bltfx)
        bltfx.dwDDFX |= DDBLTFX_NOTEARING;
        hr = _dxgsg->_pri->Blt(NULL,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bltfx);
        if(FAILED( hr )) {
            wdxdisplay_cat.fatal() << "Blt to Black of Primary Surf failed! : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        if(FAILED(hr = _dxgsg->_pDD->TestCooperativeLevel())) {
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

        _dxgsg->GetBackBuffer()->GetSurfaceDesc(&ddsd);
        LPDIRECTDRAW7 pDD = _dxgsg->GetDDInterface();
    
        ddsd.dwFlags &= ~DDSD_PITCH;
        ddsd.dwWidth  = 1; ddsd.dwHeight = 1;
        ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER | DDSCAPS_BACKBUFFER);
    
        PRINTVIDMEM(pDD,&ddsd.ddsCaps,"dummy backbuf");
    
        if(FAILED( hr = pDD->CreateSurface( &ddsd, &pddsDummy, NULL ) )) {
            wdxdisplay_cat.fatal() << "Resize CreateSurface for temp backbuf failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }
    
        DX_DECLARE_CLEAN( DDSURFACEDESC2, ddsdZ );
        _dxgsg->GetZBuffer()->GetSurfaceDesc(&ddsdZ);
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
    
        if(FAILED( hr = _dxgsg->GetD3DDevice()->SetRenderTarget( pddsDummy, 0x0 ))) {
            wdxdisplay_cat.fatal()
            << "Resize failed to set render target to temporary surface, result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }
        RELEASE(pddsDummyZ,wdxdisplay,"dummy resize zbuffer",false);
        RELEASE(pddsDummy,wdxdisplay,"dummy resize rendertarget buffer",false);
    }
    
    RECT view_rect;
    GetClientRect( _mwindow, &view_rect );
    ClientToScreen( _mwindow, (POINT*)&view_rect.left );   // translates top,left pnt
    ClientToScreen( _mwindow, (POINT*)&view_rect.right );  // translates right,bottom pnt
    
    // change _props xsize,ysize
    resized((view_rect.right - view_rect.left),(view_rect.bottom - view_rect.top));
    
    _props._xorg = view_rect.left;  // _props origin should reflect upper left of view rectangle
    _props._yorg = view_rect.top;
    
    if(wdxdisplay_cat.is_spam()) {
      wdxdisplay_cat.spam() << "reshape to origin: (" << _props._xorg << "," << _props._yorg << "), size: (" << _props._xsize << "," << _props._ysize << ")\n";
    }
    
    if(_dxgsg!=NULL) {
      if(bDoDxReset)
          _dxgsg->dx_setup_after_resize(view_rect,_mwindow);  // create the new resized rendertargets
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
       
       if(!GetWindowPlacement(_mwindow,&wndpl)) {
           wdxdisplay_cat.error() << "GetWindowPlacement failed!\n";
           return;
       }
       if((wndpl.showCmd!=SW_MINIMIZE)&&(wndpl.showCmd!=SW_SHOWMINIMIZED)) {
           ShowWindow(_mwindow, SW_MINIMIZE);
       }

       throw_event("PandaPaused"); // right now this is used to signal python event handler to disable audio
   }

//   if(!bResponsive_minimized_fullscreen_window) {
   // need this even in responsive-mode to trigger the dxgsg check of cooplvl, i think?
       _PandaPausedTimer = SetTimer(_mwindow,PAUSED_TIMER_ID,500,NULL);
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
            KillTimer(_mwindow,_PandaPausedTimer);
            _PandaPausedTimer = NULL;
        }
    
        // move window to top of zorder
    //  if(_props._fullscreen)
    //      SetWindowPos(_mwindow, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOOWNERZORDER);
        GdiFlush();

    } else if(_active_minimized_fullscreen) {
        if(wdxdisplay_cat.is_spam())
            wdxdisplay_cat.spam() << "WDX window unminimized from active-minimized state...\n";
    
        if(_PandaPausedTimer!=NULL) {
            KillTimer(_mwindow,_PandaPausedTimer);
            _PandaPausedTimer = NULL;
        }

        _active_minimized_fullscreen = false;
    
        // move window to top of zorder
    //  if(_props._fullscreen)
    //      SetWindowPos(_mwindow, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOOWNERZORDER);
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
wdxGraphicsWindow::
wdxGraphicsWindow(GraphicsPipe* pipe) : GraphicsWindow(pipe) {
    config();
}

////////////////////////////////////////////////////////////////////
//     Function: Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
wdxGraphicsWindow::
wdxGraphicsWindow(GraphicsPipe* pipe, const
                  GraphicsWindow::Properties& props) : GraphicsWindow(pipe, props) {
    config();
}

////////////////////////////////////////////////////////////////////
//     Function: config
//       Access:
//  Description:  Set up win32 window
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::config(void) {
    GraphicsWindow::config();

    global_wdxwinptr = this;  // for use during createwin()

    _hDDraw_DLL = NULL;
    _hdc = NULL;
    _mwindow = NULL;
    _gsg = _dxgsg = NULL;
    _exiting_window = false;
    _window_inactive = false;
    _return_control_to_app = false;
    _active_minimized_fullscreen = false;
    _bIsLowVidMemCard = false;
    _MaxAvailVidMem = 0;

    _hOldForegroundWindow=GetForegroundWindow();

    if(dx_full_screen || _props._fullscreen) {
        _props._fullscreen = dx_full_screen = true;
    }

    _WindowAdjustingType = NotAdjusting;
    _hMouseCursor = NULL;
    _bSizeIsMaximized=FALSE;

    // Create a GSG to manage the graphics
    make_gsg();
    if(_gsg==NULL) {
        wdxdisplay_cat.error() << "DXGSG creation failed!\n";
        exit(1);
    }
    _dxgsg = DCAST(DXGraphicsStateGuardian, _gsg);

    HINSTANCE hinstance = GetModuleHandle(NULL);

    WNDCLASS wc;

    // Clear before filling in window structure!
    ZeroMemory(&wc, sizeof(WNDCLASS));
    wc.style      = CS_HREDRAW | CS_VREDRAW; //CS_OWNDC;
    wc.lpfnWndProc    = (WNDPROC) static_window_proc;
    wc.hInstance      = hinstance;

    string windows_icon_filename = get_icon_filename().to_os_specific();
    string windows_mono_cursor_filename = get_mono_cursor_filename().to_os_specific();

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
            wdxdisplay_cat.warning() << "windows cursor filename '" << windows_mono_cursor_filename << "' not found!!\n";
            _hMouseCursor = LoadCursor(NULL, IDC_ARROW);
        }
        _bLoadedCustomCursor=true;
    } else {
        _hMouseCursor = LoadCursor(NULL, IDC_ARROW);
    }

    wc.hCursor = _hMouseCursor;
    wc.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszMenuName   = NULL;
    wc.lpszClassName  = WDX_WINDOWCLASSNAME;

    if(!RegisterClass(&wc)) {
        wdxdisplay_cat.fatal() << "could not register window class!" << endl;
        exit(1);
    }

    DWORD window_style = WS_POPUP | WS_SYSMENU;  // for CreateWindow

    // rect now contains the coords for the entire window, not the client
    if(dx_full_screen) {

        _mwindow = CreateWindow(WDX_WINDOWCLASSNAME, _props._title.c_str(),
                                window_style, 0, 0, _props._xsize,_props._ysize,
                                NULL, NULL, hinstance, 0);
    } else {
        RECT win_rect;
        SetRect(&win_rect, _props._xorg,  _props._yorg, _props._xorg + _props._xsize,
                _props._yorg + _props._ysize);

        if(_props._border)
            window_style |= WS_OVERLAPPEDWINDOW;  // should we just use WS_THICKFRAME instead?

        AdjustWindowRect(&win_rect, window_style, FALSE);  //compute window size based on desired client area size

        // make sure origin is on screen
        if(win_rect.left < 0) {
            win_rect.right -= win_rect.left; win_rect.left = 0;
        }
        if(win_rect.top < 0) {
            win_rect.bottom -= win_rect.top; win_rect.top = 0;
        }

        _mwindow = CreateWindow(WDX_WINDOWCLASSNAME, _props._title.c_str(),
                                window_style, win_rect.left, win_rect.top, win_rect.right-win_rect.left,
                                win_rect.bottom-win_rect.top,
                                NULL, NULL, hinstance, 0);
    }

    if(!_mwindow) {
        wdxdisplay_cat.fatal() << "config() - failed to create window" << endl;
        exit(1);
    }

    hwnd_pandawin_map[_mwindow] = this;
    global_wdxwinptr = NULL;  // get rid of any reference to this obj

    // move window to top of zorder
    SetWindowPos(_mwindow, HWND_TOP, 0,0,0,0, SWP_NOMOVE | SWP_NOSENDCHANGING | SWP_NOSIZE);

    _hdc = GetDC(_mwindow);

    dx_setup();

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

    ShowWindow(_mwindow, SW_SHOWNORMAL);
    ShowWindow(_mwindow, SW_SHOWNORMAL);  // call twice to override STARTUPINFO value, which may be set to hidden initially by emacs
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

BOOL WINAPI DriverEnumCallback( GUID* pGUID, TCHAR* strDesc,TCHAR* strName,
                                VOID *argptr, HMONITOR hm) {
    if(hm!=NULL)  // skip over non-primary display devices
        return DDENUMRET_OK;

    // primary display driver will have NULL guid
    // ignore that and save any non-null value, whic
    // indicates a secondary driver, which is usually voodoo1/2
    if(pGUID!=NULL) {
        memcpy(argptr,pGUID,sizeof(GUID));
    }

    return DDENUMRET_OK;
}

void wdxGraphicsWindow::resize(unsigned int xsize,unsigned int ysize) {
   if(wdxdisplay_cat.is_debug())
      wdxdisplay_cat.debug() << "resize("<<xsize<<","<<ysize<<") called\n";

   if (!_props._fullscreen) {
        // is this enough?
        SetWindowPos(_mwindow, NULL, 0,0, xsize, ysize, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSENDCHANGING);
        // WM_ERASEBKGND will be ignored, because _WindowAdjustingType!=NotAdjusting because 
        // we dont want to redraw as user is manually resizing window, so need to force explicit
        // background clear for the programmatic resize fn call
         _WindowAdjustingType=NotAdjusting;
        
         // this doesnt seem to be working in toontown resize, so I put ddraw blackblt in handle_reshape instead
         //window_proc(_mwindow, WM_ERASEBKGND,(WPARAM)_hdc,0x0);  
        handle_reshape(true);
        return;
   }

   _dxgsg->SetDXReady(false);

   HRESULT hr;

   DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd_curmode);

   if(FAILED(hr = _dxgsg->_pDD->GetDisplayMode(&ddsd_curmode))) {
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

   if(FAILED(hr = _dxgsg->_pDD->EnumDisplayModes(DDEDM_REFRESHRATES,&ddsd_search,&DMI,EnumDisplayModesCallBack))) {
       wdxdisplay_cat.fatal() << "resize() - EnumDisplayModes failed, result = " << ConvD3DErrorToString(hr) << endl;
       return;
   }

   DMI.supportedBitDepths &= _dxgsg->_D3DDevDesc.dwDeviceRenderBitDepth;

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

   if(FAILED(hr = _dxgsg->_pDD->TestCooperativeLevel())) {
        wdxdisplay_cat.error() << "TestCooperativeLevel failed : result = " << ConvD3DErrorToString(hr) << endl;
        wdxdisplay_cat.error() << "Full screen app failed to get exclusive mode on resize, exiting..\n";
        return;
   }

   _dxgsg->free_dxgsg_objects();

   // let driver choose default refresh rate (hopefully its >=60Hz)   
   if(FAILED( hr = _dxgsg->_pDD->SetDisplayMode( xsize,ysize,dwFullScreenBitDepth, 0L, 0L ))) {
        wdxdisplay_cat.error() << "resize failed to reset display mode to (" << xsize <<"x"<<ysize<<"x"<<dwFullScreenBitDepth<<"): result = " << ConvD3DErrorToString(hr) << endl;
   }

   if(wdxdisplay_cat.is_debug()) {
      DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd34); 
      _dxgsg->_pDD->GetDisplayMode(&ddsd34);
      wdxdisplay_cat.debug() << "set displaymode to " << ddsd34.dwWidth << "x" << ddsd34.dwHeight << " at "<< ddsd34.ddpfPixelFormat.dwRGBBitCount << "bpp, " << ddsd34.dwRefreshRate<< "Hz\n";
   }

   CreateScreenBuffersAndDevice(xsize,ysize,_dxgsg->_pDD,_dxgsg->_d3d,NULL);
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
       ddsd_search.dwWidth=xsize;  ddsd_search.dwHeight=ysize;
    
       ZeroMemory(&DDSD_Arr,sizeof(DDSD_Arr));
       ZeroMemory(&DMI,sizeof(DMI));
       DMI.maxWidth=xsize;  DMI.maxHeight=ysize;
       DMI.pDDSD_Arr=DDSD_Arr;
    
       if(FAILED(hr = _dxgsg->_pDD->EnumDisplayModes(DDEDM_REFRESHRATES,&ddsd_search,&DMI,EnumDisplayModesCallBack))) {
           wdxdisplay_cat.fatal() << "resize() - EnumDisplayModes failed, result = " << ConvD3DErrorToString(hr) << endl;
           return 0;
       }
    
       // get rid of bpp's we cant render at
       DMI.supportedBitDepths &= _dxgsg->_D3DDevDesc.dwDeviceRenderBitDepth;

       bool bIsGoodMode=false;

       if(_bIsLowVidMemCard) 
           bIsGoodMode=(((float)xsize*(float)ysize)<=(float)(640*480));
         else if(DMI.supportedBitDepths & (DDBD_16 | DDBD_24 | DDBD_32)) {
             // assume user is testing fullscreen, not windowed, so use the dwTotal value
             // see if 3 scrnbufs (front/back/z)at 16bpp at xsize*ysize will fit with a few 
             // extra megs for texmem

             // 8MB Rage Pro says it has 6.8 megs Total free and will run at 1024x768, so
             // formula makes it so that is OK

             #define REQD_TEXMEM 1800000.0f  

             if(_MaxAvailVidMem==0) {
                 //assume buggy drivers return bad val of 0 and everything will be OK
                 bIsGoodMode=true;
             } else {
                 bIsGoodMode = ((((float)xsize*(float)ysize)*6+REQD_TEXMEM) < (float)_MaxAvailVidMem);
             }
         }

       wdxdisplay_cat.fatal() << "XXXXXXXXXXXXXXXXXXXXX:   " <<  ((1024.0f*768.0f)*6+REQD_TEXMEM)
                              << "       " << _MaxAvailVidMem  << endl; 

       if(bIsGoodMode)
         num_valid_modes++;
        else {
            pCurDim[0] = 0;
            pCurDim[1] = 0;
        }
   }

   return num_valid_modes;
}

// imperfect method to ID NVid? could also scan desc str, but that isnt fullproof either
#define IS_NVIDIA(DDDEVICEID) ((DDDEVICEID.dwVendorId==0x10DE) || (DDDEVICEID.dwVendorId==0x12D2))
#define IS_ATI(DDDEVICEID) (DDDEVICEID.dwVendorId==0x1002) 
#define IS_MATROX(DDDEVICEID) (DDDEVICEID.dwVendorId==0x102B)

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

////////////////////////////////////////////////////////////////////
//     Function: dx_setup
//  Description: Set up the DirectX environment.  The size of the
//               rendered area will be computed from the Client area
//               of the window (if in windowed mode) and the _props
//               structure will be set accordingly.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::
dx_setup() {
    LPDIRECT3D7   pD3DI;
    LPDIRECTDRAW7 pDD;
    HRESULT hr;
    DX_DECLARE_CLEAN( DDSURFACEDESC2, SurfaceDesc );

    // Check for DirectX 7 by looking for DirectDrawCreateEx

    _hDDraw_DLL = LoadLibrary("ddraw.dll");
    if(_hDDraw_DLL == 0) {
        wdxdisplay_cat.fatal() << "can't locate DDRAW.DLL!\n";
        exit(1);
    }

    // Note: I dont want to mess with manually doing FreeLibrary(ddraw.dll) for now.  The OS will clean
    // this up when the app exits, if we need to I'll add this later.

    typedef HRESULT (WINAPI * LPDIRECTDRAWCREATEEX)(GUID FAR * lpGuid, LPVOID  *lplpDD, REFIID  iid,IUnknown FAR *pUnkOuter);

    // load all ddraw exports dynamically to avoid static link to ddraw.dll, in case system doesnt have it

    LPDIRECTDRAWCREATEEX pDDCreateEx = (LPDIRECTDRAWCREATEEX) GetProcAddress(_hDDraw_DLL,"DirectDrawCreateEx");
    if(pDDCreateEx == NULL) {
        wdxdisplay_cat.fatal() << "Panda currently requires at least DirectX 7.0!\n";
        exit(1);
    }

    LPDIRECTDRAWENUMERATEEX pDDEnumEx = (LPDIRECTDRAWENUMERATEEX) GetProcAddress(_hDDraw_DLL,"DirectDrawEnumerateExA");
    if(pDDEnumEx == NULL) {
        wdxdisplay_cat.fatal() << "GetProcAddr failed for DirectDrawEnumerateEx!\n";
        exit(1);
    }

    GUID DriverGUID;
    ZeroMemory(&DriverGUID,sizeof(GUID));

    // search for early voodoo-type non-primary display drivers
    // if they exist, use them for 3D  (could examine 3D devices on all
    // drivers and pick the best one, but I'll assume the computer setuper knows what he's doing)
    if(hr = (*pDDEnumEx)( DriverEnumCallback, &DriverGUID, DDENUM_NONDISPLAYDEVICES )) {
        wdxdisplay_cat.fatal()   << "config() - DirectDrawEnumerateEx failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    GUID *pOurDriverGUID=NULL;
    if(DriverGUID.Data1 != 0x0) {    // assumes no driver guid ever starts with 0, so 0 means Enum found no voodoo-type device
        pOurDriverGUID=&DriverGUID;
    }

      // Create the Direct Draw Object
    hr = (*pDDCreateEx)(pOurDriverGUID, (void **)&pDD, IID_IDirectDraw7, NULL);
    if(hr != DD_OK) {
        wdxdisplay_cat.fatal()
        << "config() - DirectDrawCreateEx failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    pDD->GetDeviceIdentifier(&_DXDeviceID,0x0);

#ifdef _DEBUG
    wdxdisplay_cat.debug() << " GfxCard: " << _DXDeviceID.szDescription <<  "; DriverFile: '" << _DXDeviceID.szDriver  << "'; VendorID: " <<_DXDeviceID.dwVendorId <<"; DriverVer: " << HIWORD(_DXDeviceID.liDriverVersion.HighPart) << "." << LOWORD(_DXDeviceID.liDriverVersion.HighPart) << "." << HIWORD(_DXDeviceID.liDriverVersion.LowPart) << "." << LOWORD(_DXDeviceID.liDriverVersion.LowPart) << endl;
#endif
    
    check_for_color_cursor_support();
    
    // Query DirectDraw for access to Direct3D

    hr = pDD->QueryInterface( IID_IDirect3D7, (VOID**)&pD3DI );
    if(hr != DD_OK) {
        wdxdisplay_cat.fatal() << "QI for D3D failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    D3DDEVICEDESC7 d3ddevs[MAX_DEVICES];  // put HAL in 0, TnLHAL in 1

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

    D3DDEVICEDESC7 *pD3DDevDesc=&d3ddevs[DeviceIdx];

    DWORD dwRenderWidth=0, dwRenderHeight=0;

    // Get Current VidMem avail.  Note this is only an estimate, when we switch to fullscreen
    // mode from desktop, more vidmem will be available (typically 1.2 meg).  I dont want
    // to switch to fullscreen more than once due to the annoying monitor flicker, so try
    // to figure out optimal mode using this estimate
    DDSCAPS2 ddsGAVMCaps;
    DWORD dwVidMemTotal=0,dwVidMemFree=0;
    ZeroMemory(&ddsGAVMCaps,sizeof(DDSCAPS2));
    ddsGAVMCaps.dwCaps = DDSCAPS_VIDEOMEMORY; //set internally by DX anyway, dont think this any different than 0x0
    if(FAILED(hr = pDD->GetAvailableVidMem(&ddsGAVMCaps,&dwVidMemTotal,&dwVidMemFree))) {
        wdxdisplay_cat.error() << "GetAvailableVidMem failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    // after SetDisplayMode, GetAvailVidMem totalmem seems to go down by 1.2 meg (contradicting above
    // comment and what I think would be correct behavior (shouldnt FS mode release the desktop vidmem?),
    // so this is the true value
    _MaxAvailVidMem = dwVidMemTotal;  

    if(dx_full_screen) {
           dwRenderWidth  = _props._xsize;
           dwRenderHeight = _props._ysize;
           _props._xorg = _props._yorg = 0;

           // CREATE FULL SCREEN BUFFERS
           // Store the rectangle which contains the renderer

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
               wdxdisplay_cat.fatal() << "EnumDisplayModes failed, result = " << ConvD3DErrorToString(hr) << endl;
               exit(1);
           }

           #ifdef _DEBUG
               wdxdisplay_cat.debug() << "before fullscreen switch: GetAvailableVidMem returns Total: " << dwVidMemTotal/1000000.0 << "  Free: " << dwVidMemFree/1000000.0 << endl;
           #endif

           DWORD dwFullScreenBitDepth;

           // Now we try to figure out if we can use requested screen resolution and best
           // rendertarget bpp and still have at least 2 meg of texture vidmem

           DMI.supportedBitDepths &= pD3DDevDesc->dwDeviceRenderBitDepth;

           // note: this chooses 32bpp, which may not be preferred over 16 for memory & speed reasons
           if(DMI.supportedBitDepths & DDBD_32) {
               dwFullScreenBitDepth=32;              // go for 32bpp if its avail
           } else if(DMI.supportedBitDepths & DDBD_24) {
               dwFullScreenBitDepth=24;              // go for 24bpp if its avail
           } else if(DMI.supportedBitDepths & DDBD_16) {
               dwFullScreenBitDepth=16;              // do 16bpp
           } else {
               wdxdisplay_cat.fatal()
               << "No Supported FullScreen resolutions at " << dwRenderWidth << "x" << dwRenderHeight << endl;
               exit(1);
           }

           if(dwVidMemFree>0) {  // assume buggy drivers (this means you, FireGL2) may return zero for dwTotal, so ignore value if its 0

               // hack: figuring out exactly what res to use is tricky, instead I will
               // just use 640x480 if we have < 3 meg avail

       #define LOWVIDMEMTHRESHOLD 3500000
               if(dwVidMemFree< LOWVIDMEMTHRESHOLD) {
                   // we're going to need 800x600 or 640x480 at 16 bit to save enough tex vidmem
                   dwFullScreenBitDepth=16;              // do 16bpp
                   dwRenderWidth=640;
                   dwRenderHeight=480;
                   _bIsLowVidMemCard = true;
                   wdxdisplay_cat.debug() << " " << dwVidMemFree << " Available VidMem is under " << LOWVIDMEMTHRESHOLD <<", using 640x480 16bpp rendertargets to save tex vidmem.\n";
               }

       #if 0
       // cant do this without more accurate way to estimate mem used before actually switching
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
                       wdxdisplay_cat.debug() << " XXXXXX WARNING: cant reserve 2MB of tex vidmem. only " << memleft << " bytes available. Need to rewrite wdxdisplay to try lower resolutions  XXXXXXXXXXXXXXXXXXXX\n";
               }
       #endif
           }

           DWORD SCL_FPUFlag;
           if(dx_preserve_fpu_state)
              SCL_FPUFlag = DDSCL_FPUPRESERVE;  // tell d3d to preserve the fpu state across calls.  this hurts perf, but is good for dbgging
            else SCL_FPUFlag = DDSCL_FPUSETUP;

           DWORD SCL_FLAGS = SCL_FPUFlag | DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT;

           // s3 savage2000 on w95 seems to set EXCLUSIVE_MODE only if you call SetCoopLevel twice.
           // so we do it, it really shouldnt be necessary if drivers werent buggy
           for(int jj=0;jj<2;jj++) {
               if(FAILED(hr = pDD->SetCooperativeLevel(_mwindow, SCL_FLAGS))) {
                   wdxdisplay_cat.fatal() << "SetCooperativeLevel failed : result = " << ConvD3DErrorToString(hr) << endl;
                   exit(1);
               }
           }

           if(FAILED(hr = pDD->TestCooperativeLevel())) {
               wdxdisplay_cat.fatal() << "TestCooperativeLevel failed : result = " << ConvD3DErrorToString(hr) << endl;
               wdxdisplay_cat.fatal() << "Full screen app failed to get exclusive mode on init, exiting..\n";
               exit(1);
           }

           // let driver choose default refresh rate (hopefully its >=60Hz)
           if(FAILED( hr = pDD->SetDisplayMode( dwRenderWidth, dwRenderHeight,
                                                dwFullScreenBitDepth, 0L, 0L ))) {
               wdxdisplay_cat.fatal() << "failed to reset display mode to ("<<dwRenderWidth<<"x"<<dwRenderHeight<<"x"<<dwFullScreenBitDepth<<"): result = " << ConvD3DErrorToString(hr) << endl;
               exit(1);
           }

          if(wdxdisplay_cat.is_debug()) {
              DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd34); 
              pDD->GetDisplayMode(&ddsd34);
              wdxdisplay_cat.debug() << "set displaymode to " << ddsd34.dwWidth << "x" << ddsd34.dwHeight << " at "<< ddsd34.ddpfPixelFormat.dwRGBBitCount << "bpp, " << ddsd34.dwRefreshRate<< "Hz\n";

           #ifdef _DEBUG
              if(FAILED(hr = pDD->GetAvailableVidMem(&ddsGAVMCaps,&dwVidMemTotal,&dwVidMemFree))) {
                  wdxdisplay_cat.debug() << "GetAvailableVidMem failed : result = " << ConvD3DErrorToString(hr) << endl;
                  exit(1);
              }
              wdxdisplay_cat.debug() << "before fullscreen switch: GetAvailableVidMem returns Total: " << dwVidMemTotal/1000000.0 << "  Free: " << dwVidMemFree/1000000.0 << endl;
           #endif
           }
    }

    CreateScreenBuffersAndDevice(dwRenderWidth,dwRenderHeight,pDD,pD3DI,pD3DDevDesc);

    _dxgsg->SetDXReady(true);
}


void wdxGraphicsWindow::
CreateScreenBuffersAndDevice(DWORD dwRenderWidth, DWORD dwRenderHeight,LPDIRECTDRAW7 pDD,
                             LPDIRECT3D7 pD3DI,D3DDEVICEDESC7 *pD3DDevDesc) {
    LPDIRECTDRAWSURFACE7  pPrimaryDDSurf,pBackDDSurf,pZDDSurf;
    LPDIRECT3DDEVICE7     pD3DDevice;
    RECT view_rect;
    int i;
    HRESULT hr;
    DX_DECLARE_CLEAN( DDSURFACEDESC2, SurfaceDesc );
    D3DDEVICEDESC7 d3ddevs[2];  // put HAL in 0, TnLHAL in 1

    assert(pDD!=NULL);
    assert(pD3DI!=NULL);

    // select the best device if the caller does not provide one
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

    DX_DECLARE_CLEAN(DDCAPS,DDCaps);
    pDD->GetCaps(&DDCaps,NULL);

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
            exit(1);
        }

        // Clear the primary surface to black

        DX_DECLARE_CLEAN(DDBLTFX, bltfx)
        bltfx.dwDDFX |= DDBLTFX_NOTEARING;
        hr = pPrimaryDDSurf->Blt(NULL,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bltfx);

        if(FAILED( hr )) {
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
        assert(dwRenderWidth==0 && dwRenderHeight==0);  // these params are ignored for windowed mode

        if(!(DDCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED)) {
            wdxdisplay_cat.fatal() << "the 3D HW cannot render windowed, exiting..." << endl;
            exit(1);
        }

        if(FAILED(hr = pDD->GetDisplayMode( &SurfaceDesc ))) {
            wdxdisplay_cat.fatal()
            << "GetDisplayMode failed result = " << ConvD3DErrorToString(hr) << endl;
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

        DWORD SCL_FPUFlag;
        if(dx_preserve_fpu_state)
           SCL_FPUFlag = DDSCL_FPUPRESERVE;  // tell d3d to preserve the fpu state across calls.  this hurts perf, but is good for dbgging
         else SCL_FPUFlag = DDSCL_FPUSETUP;

        if(FAILED(hr = pDD->SetCooperativeLevel(_mwindow, SCL_FPUFlag | DDSCL_NORMAL))) {
            wdxdisplay_cat.fatal() << "SetCooperativeLevel failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }
        // Get the dimensions of the viewport and screen bounds

        GetClientRect( _mwindow, &view_rect );
        POINT ul,lr;
        ul.x=view_rect.left;  ul.y=view_rect.top;
        lr.x=view_rect.right;  lr.y=view_rect.bottom;
        ClientToScreen( _mwindow, &ul );
        ClientToScreen( _mwindow, &lr );
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
            wdxdisplay_cat.fatal()
            << "CreateClipper failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        // Associate the clipper with our window. Note that, afterwards, the
        // clipper is internally referenced by the primary surface, so it is safe
        // to release our local reference to it.
        Clipper->SetHWnd( 0, _mwindow );
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

        // Create the backbuffer. (might want to handle failure due to running out of video memory.
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
        ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;

        DDPIXELFORMAT ZBufPixFmts[MAX_DX_ZBUF_FMTS];
        cNumZBufFmts=0;

        // Get an appropiate pixel format from enumeration of the formats. On the
        // first pass, we look for a zbuffer dpeth which is equal to the frame
        // buffer depth (as some cards unfornately require this).
        if(FAILED(pD3DI->EnumZBufferFormats(  IID_IDirect3DHALDevice, EnumZBufFmtsCallback,
                                              (VOID*)&ZBufPixFmts ))) {
            wdxdisplay_cat.fatal() << "EnumZBufferFormats failed " << endl;
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

        #define SET_ZBUF_DEPTH(DEPTH) { assert(pz##DEPTH != NULL); _depth_buffer_bpp=DEPTH; ddsd.ddpfPixelFormat = *pz##DEPTH;}

        if(IS_NVIDIA(_DXDeviceID)) {
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
            exit(1);
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

    _dxgsg->Set_HDC(_hdc);
    _dxgsg->dx_init(pDD, pPrimaryDDSurf, pBackDDSurf, pZDDSurf, pD3DI, pD3DDevice, view_rect);
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
    if((_dxgsg==NULL) || (_dxgsg->_zbuf==NULL))
      return -1;

    return _depth_buffer_bpp;

// this is not reliable, on GF2, GetSurfDesc returns 32bpp when you created a 24bpp zbuf
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

