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

extern bool dx_full_screen_antialiasing;  // defined in dxgsg_config.cxx

#define MOUSE_ENTERED 0
#define MOUSE_EXITED 1
#define PAUSED_TIMER_ID  7   // completely arbitrary choice
#define DXREADY ((_dxgsg!=NULL)&&(_dxgsg->GetDXReady()))

LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam,LPARAM lparam);

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

#if defined(NOTIFY_DEBUG) || defined(DO_PSTATS)
extern void dbgPrintVidMem(LPDIRECTDRAW7 pDD, LPDDSCAPS2 lpddsCaps,const char *pMsg) {
    DWORD dwTotal,dwFree;
    HRESULT hr;

/*
 * These Caps bits arent allowed to be specified when calling GetAvailVidMem.
 * They don't affect surface allocation in a vram heap.
 */

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

  #ifdef NOTIFY_DEBUG
  // Write a debug message to the console reporting the texture memory.
    char tmpstr[100],tmpstr2[100];
    sprintf(tmpstr,"%.4g",dwTotal/1000000.0);
    sprintf(tmpstr2,"%.4g",dwFree/1000000.0);
    wdxdisplay_cat.debug() << "AvailableVidMem before creating "<< pMsg << ",(megs) total: " << tmpstr << "  free:" << tmpstr2 <<endl;
  #endif

  #ifdef DO_PSTATS
  // Tell PStats about the state of the texture memory.
  GraphicsStateGuardian::_total_texmem_pcollector.set_level(dwTotal);
  GraphicsStateGuardian::_used_texmem_pcollector.set_level(dwTotal - dwFree);
  #endif
}
#endif

#define MAX_DX_ZBUF_FMTS 20
static int cNumZBufFmts=0;

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
        case WM_CREATE:
            break;

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

        case WM_PAINT: {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);

            if(DXREADY)
                show_frame();
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
            POINT point;

            GetCursorPos(&point);
            ScreenToClient(hwnd, &point);
            handle_keyrelease(lookup_key(wparam), point.x, point.y);
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
            // Win32 doesn't return the same numbers as X does when the mouse
            // goes beyond the upper or left side of the window
            x = LOWORD(lparam);
            y = HIWORD(lparam);
            if(x & 1 << 15) x -= (1 << 16);
            if(y & 1 << 15) y -= (1 << 16);
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
            x = LOWORD(lparam);
            y = HIWORD(lparam);
            if(x & 1 << 15) x -= (1 << 16);
            if(y & 1 << 15) y -= (1 << 16);
            handle_keyrelease(MouseButton::button(button), x, y);
            return 0;

        case WM_MOUSEMOVE:
            if(!DXREADY)
                break;

            x = LOWORD(lparam);
            y = HIWORD(lparam);
            if(x & 1 << 15) x -= (1 << 16);
            if(y & 1 << 15) y -= (1 << 16);
            if(mouse_motion_enabled()
               && wparam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)) {
                handle_mouse_motion(x, y);
            } else if(mouse_passive_motion_enabled() &&
                      ((wparam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)) == 0)) {
                handle_mouse_motion(x, y);
            }
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

                GdiFlush();

                if(_dxgsg!=NULL) {
                    _dxgsg->SetDXReady(false);  // disable rendering whilst we mess with surfs
    
                    // Want to change rendertarget size without destroying d3d device.  To save vid memory
                    // (and make resizing work on memory-starved 4MB cards), we need to construct
                    // a temporary mini-sized render target for the d3d device (it cannot point to a
                    // NULL rendertarget) before creating the fully resized buffers.  The old
                    // rendertargets will be freed when these temp targets are set, and that will give
                    // us the memory to create the resized target
    
                    LPDIRECTDRAWSURFACE7 pddsDummy = NULL, pddsDummyZ = NULL;
                    HRESULT hr;
    
                    DX_DECLARE_CLEAN( DDSURFACEDESC2, ddsd );
    
                    if(_dxgsg->GetBackBuffer()==NULL)  // bugbug why is this ever true??
                        return DefWindowProc(hwnd, msg, wparam, lparam);
    
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
                    RELEASE(pddsDummyZ);
                    RELEASE(pddsDummy);
                }

                handle_reshape(true);

                if(_dxgsg!=NULL) {
                   _dxgsg->SetDXReady(true);
                }
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
                if((_mwindow==NULL) || ((wparam != SIZE_RESTORED) && (wparam != SIZE_MAXIMIZED)))
                    break;

                width = LOWORD(lparam);  height = HIWORD(lparam);

                if((_props._xsize != width) || (_props._ysize != height)) {
                    _WindowAdjustingType = Resizing;

                 // for maximized,unmaximize, need to call resize code artificially
                 // since no WM_EXITSIZEMOVE is generated
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

        case WM_SETFOCUS:
            if(!DXREADY)
              break;
            if(_mouse_entry_enabled)
                handle_mouse_entry(MOUSE_ENTERED,_hMouseCursor);
            return 0;

        case WM_KILLFOCUS:
            if(!DXREADY)
              break;
            if(_mouse_entry_enabled)
                handle_mouse_entry(MOUSE_EXITED,_hMouseCursor);
            return 0;

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


            if((wparam==_PandaPausedTimer) && _window_inactive) {
                assert(_dxgsg!=NULL);
                _dxgsg->CheckCooperativeLevel(DO_REACTIVATE_WINDOW);

                // wdxdisplay_cat.spam() << "periodic return of control to app\n";
                _return_control_to_app = true;
                // throw_event("PandaPaused");   
                // do we still need to do this since I return control to app periodically using timer msgs?
                // does app need to know to avoid major computation?
            }
            return 0;
    }

    return DefWindowProc(hwnd, msg, wparam, lparam);
}

  
////////////////////////////////////////////////////////////////////
//     Function: handle_reshape
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::handle_reshape(bool bDoDxReset) {
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

      if((_dxgsg!=NULL) && bDoDxReset) {
          _dxgsg->dx_setup_after_resize(view_rect,_mwindow);  // create the new resized rendertargets
      }
}

void wdxGraphicsWindow::deactivate_window(void) {
    // current policy is to suspend minimized or deactivated fullscreen windows, but leave
    // regular windows running normally

   if(_window_inactive || _exiting_window)
     return;

   if(wdxdisplay_cat.is_spam())
       wdxdisplay_cat.spam() << "WDX window deactivated, waiting...\n";
   _window_inactive = true;

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
   }

   _PandaPausedTimer = SetTimer(_mwindow,PAUSED_TIMER_ID,500,NULL);
   if(_PandaPausedTimer!=PAUSED_TIMER_ID) {
       wdxdisplay_cat.error() << "Error in SetTimer!\n";
   }
}

void wdxGraphicsWindow::reactivate_window(void) {
    if(!_window_inactive)
        return;

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

    _hdc = NULL;
    _mwindow = NULL;
    _gsg = _dxgsg = NULL;
    _exiting_window = false;
    _window_inactive = false;
    _return_control_to_app = false;

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
    _entry_state = -1;

    // Enable detection of mouse input
    enable_mouse_input(true);
    enable_mouse_motion(true);
    enable_mouse_passive_motion(true);

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

HRESULT WINAPI EnumDisplayModesCallBack(LPDDSURFACEDESC2 lpDDSurfaceDesc,LPVOID lpContext) {
    DWORD *pDDBDMask = (DWORD*)lpContext;

    *pDDBDMask |= BitDepth_2_DDBDMask(lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount);
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

////////////////////////////////////////////////////////////////////
//     Function: dx_setup
//  Description: Set up the DirectX environment.  The size of the
//               rendered area will be computed from the Client area
//               of the window (if in windowed mode) and the _props
//               structure will be set accordingly.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::
dx_setup() {
    LPDIRECTDRAWSURFACE7  pBackDDSurf;
    LPDIRECTDRAWSURFACE7  pZDDSurf;
    LPDIRECT3D7           pD3DI;
    LPDIRECT3DDEVICE7     pD3DDevice;
    LPDIRECTDRAWSURFACE7  pPrimaryDDSurf;
    LPDIRECTDRAW7         pDD;
    RECT view_rect;
    int i;
    HRESULT hr;
    DX_DECLARE_CLEAN( DDSURFACEDESC2, SurfaceDesc );

      // Check for DirectX 7 by looking for DirectDrawCreateEx

    HINSTANCE DDHinst = LoadLibrary( "ddraw.dll" );
    if(DDHinst == 0) {
        wdxdisplay_cat.fatal() << "config() - DDRAW.DLL doesn't exist!" << endl;
        exit(1);
    }

    if(NULL == GetProcAddress( DDHinst, "DirectDrawCreateEx" )) {
        wdxdisplay_cat.fatal() << "config() - Panda currently requires at least DirectX 7.0!" << endl;
        exit(1);
    }

    GUID DriverGUID;
    ZeroMemory(&DriverGUID,sizeof(GUID));

      // search for early voodoo-type non-primary display drivers
      // if they exist, use them for 3D  (could examine 3D devices on all
      // drivers and pick the best one, but I'll assume the computer setuper knows what he's doing)
    if(hr = DirectDrawEnumerateEx( DriverEnumCallback, &DriverGUID, DDENUM_NONDISPLAYDEVICES )) {
        wdxdisplay_cat.fatal()   << "config() - DirectDrawEnumerateEx failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    GUID *pOurDriverGUID=NULL;
    if(DriverGUID.Data1 != 0x0) {    // assumes no driver guid ever starts with 0, so 0 means Enum found no voodoo-type device
        pOurDriverGUID=&DriverGUID;
    }

      // Create the Direct Draw Object
    hr = DirectDrawCreateEx(pOurDriverGUID, (void **)&pDD, IID_IDirectDraw7, NULL);
    if(hr != DD_OK) {
        wdxdisplay_cat.fatal()
        << "config() - DirectDrawCreateEx failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    FreeLibrary(DDHinst);    //undo LoadLib above, decrement ddrawl.dll refcnt (after DDrawCreate, since dont want to unload/reload)

    DDDEVICEIDENTIFIER2 dddi;
    pDD->GetDeviceIdentifier(&dddi,0x0);

#ifdef _DEBUG
    wdxdisplay_cat.debug()
    << " GfxCard: " << dddi.szDescription <<  "; VendorID: " <<dddi.dwVendorId <<"; DriverVer: " << HIWORD(dddi.liDriverVersion.HighPart) << "." << LOWORD(dddi.liDriverVersion.HighPart) << "." << HIWORD(dddi.liDriverVersion.LowPart) << "." << LOWORD(dddi.liDriverVersion.LowPart) << endl;
#endif

      // imperfect method to ID NVid, could also scan desc str, but that isnt fullproof either
    BOOL bIsNvidia = (dddi.dwVendorId==4318) || (dddi.dwVendorId==4818);

      // Query DirectDraw for access to Direct3D

    hr = pDD->QueryInterface( IID_IDirect3D7, (VOID**)&pD3DI );
    if(hr != DD_OK) {
        wdxdisplay_cat.fatal()
        << "config() - QI for D3D failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

       // just look for HAL and TnL devices right now.  I dont think
       // we have any interest in the sw rasts at this point

    D3DDEVICEDESC7 d3ddevs[2];     // put HAL in 0, TnLHAL in 1
    ZeroMemory(d3ddevs,2*sizeof(D3DDEVICEDESC7));

    hr = pD3DI->EnumDevices(EnumDevicesCallback,d3ddevs);
    if(hr != DD_OK) {
        wdxdisplay_cat.fatal()
        << "config() - EnumDevices failed : result = " << ConvD3DErrorToString(hr) << endl;
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

    DX_DECLARE_CLEAN(DDCAPS,DDCaps);
    pDD->GetCaps(&DDCaps,NULL);

    DWORD dwRenderWidth,dwRenderHeight;

    if(dx_full_screen) {
        dwRenderWidth  = _props._xsize;
        dwRenderHeight = _props._ysize;
        _props._xorg = _props._yorg = 0;

        // CREATE FULL SCREEN BUFFERS
        // Store the rectangle which contains the renderer

        DWORD dwSupportedBitDepthMask=0x0;  // uses DDBD_...
        DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd_search);
        ddsd_search.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
        ddsd_search.dwWidth=dwRenderWidth;  ddsd_search.dwHeight=dwRenderHeight;

        if(FAILED(hr= pDD->EnumDisplayModes(0x0,&ddsd_search,&dwSupportedBitDepthMask,EnumDisplayModesCallBack))) {
            wdxdisplay_cat.fatal()
            << "EnumDisplayModes failed, result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        DWORD dwFullScreenBitDepth;

        // Now we try to figure out if we can use requested screen resolution and best
        // rendertarget bpp and still have at least 2 meg of texture vidmem

        // Get Current VidMem avail.  Note this is only an estimate, when we switch to fullscreen
        // mode from desktop, more vidmem will be available (typically 1.2 meg).  I dont want
        // to switch to fullscreen more than once due to the annoying monitor flicker, so try
        // to figure out optimal mode using this estimate
        DDSCAPS2 ddsCaps;
        DWORD dwTotal,dwFree;
        ZeroMemory(&ddsCaps,sizeof(DDSCAPS2));
        ddsCaps.dwCaps = DDSCAPS_VIDEOMEMORY; //set internally by DX anyway, dont think this any different than 0x0

        if(FAILED(  hr = pDD->GetAvailableVidMem(&ddsCaps,&dwTotal,&dwFree))) {
            wdxdisplay_cat.debug() << "GetAvailableVidMem failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

#ifdef _DEBUG
        wdxdisplay_cat.debug() << "before FullScreen switch: GetAvailableVidMem returns Total: " << dwTotal/1000000.0 << "  Free: " << dwFree/1000000.0 << endl;
#endif

        // note: this chooses 32bpp, which may not be preferred over 16 for memory & speed reasons
        if((dwSupportedBitDepthMask & DDBD_32) && (d3ddevs[DeviceIdx].dwDeviceRenderBitDepth & DDBD_32)) {
            dwFullScreenBitDepth=32;              // go for 32bpp if its avail
        } else if((dwSupportedBitDepthMask & DDBD_24) && (d3ddevs[DeviceIdx].dwDeviceRenderBitDepth & DDBD_24)) {
            dwFullScreenBitDepth=24;              // go for 24bpp if its avail
        } else if((dwSupportedBitDepthMask & DDBD_16) && (d3ddevs[DeviceIdx].dwDeviceRenderBitDepth & DDBD_16)) {
            dwFullScreenBitDepth=16;              // do 16bpp
        } else {
            wdxdisplay_cat.fatal()
            << "No Supported FullScreen resolutions at " << dwRenderWidth << "x" << dwRenderHeight << endl;
            exit(1);
        }

        // hack: figuring out exactly what res to use is tricky, instead I will
        // just use 640x480 if we have < 3 meg avail

#define LOWVIDMEMTHRESHOLD 3500000
        if(dwFree< LOWVIDMEMTHRESHOLD) {
            // we're going to need 800x600 or 640x480 at 16 bit to save enough tex vidmem
            dwFullScreenBitDepth=16;              // do 16bpp
            dwRenderWidth=640;
            dwRenderHeight=480;
            wdxdisplay_cat.debug() << " "<<dwFree <<" Available VidMem is under "<< LOWVIDMEMTHRESHOLD <<", using 640x480 16bpp rendertargets to save tex vidmem.\n";
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
            assert((dwSupportedBitDepthMask & DDBD_16) && (d3ddevs[DeviceIdx].dwDeviceRenderBitDepth & DDBD_16));   // probably a safe assumption
            rendertargetmem=dwRenderWidth*dwRenderHeight*(dwFullScreenBitDepth>>3);
            memleft = dwFree-rendertargetmem*2;

             // BUGBUG:  if we still cant reserve 2 megs of vidmem, need to auto-reduce the scrn res
            if(memleft < RESERVEDTEXVIDMEM)
                wdxdisplay_cat.debug() << " XXXXXX WARNING: cant reserve 2MB of tex vidmem. only " << memleft << " bytes available. Need to rewrite wdxdisplay to try lower resolutions  XXXXXXXXXXXXXXXXXXXX\n";
        }
#endif


//      extern bool dx_preserve_fpu_state;
        DWORD SCL_FPUFlag = DDSCL_FPUSETUP;

//      if(dx_preserve_fpu_state)
//         SCL_FPUFlag = DDSCL_FPUPRESERVE;  // tell d3d to preserve the fpu state across calls.  this hurts perf, but is good for dbgging

        DWORD SCL_FLAGS = SCL_FPUFlag | DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT;

        if(FAILED(hr = pDD->SetCooperativeLevel(_mwindow, SCL_FLAGS))) {
            wdxdisplay_cat.fatal()
            << "config() - SetCooperativeLevel failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        // s3 savage2000 on w95 seems to set EXCLUSIVE_MODE only if you call SetCoopLevel twice.
        // so we do it, it really shouldnt be necessary if drivers werent buggy
        if(FAILED(hr = pDD->SetCooperativeLevel(_mwindow, SCL_FLAGS))) {
            wdxdisplay_cat.fatal()
            << "config() - SetCooperativeLevel failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        if(FAILED(hr = pDD->TestCooperativeLevel())) {
            wdxdisplay_cat.fatal()
            << "config() - TestCooperativeLevel failed : result = " << ConvD3DErrorToString(hr) << endl;
            wdxdisplay_cat.fatal()
            << "config() - Full screen app failed to get exclusive mode on init, exiting..\n";
            exit(1);
        }

        if(FAILED( hr = pDD->SetDisplayMode( dwRenderWidth, dwRenderHeight,
                                             dwFullScreenBitDepth, 0L, 0L ))) {
            wdxdisplay_cat.fatal() << "CreateFullscreenBuffers() - Can't set display mode : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

       if(wdxdisplay_cat.is_debug()) {
           DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd34); 
           pDD->GetDisplayMode(&ddsd34);
           wdxdisplay_cat.debug() << "set displaymode to " << ddsd34.dwWidth << "x" << ddsd34.dwHeight << " at "<< ddsd34.ddpfPixelFormat.dwRGBBitCount << "bpp, " << ddsd34.dwRefreshRate<< "Hz\n";
       }

#ifdef _DEBUG
        if(FAILED(  hr = pDD->GetAvailableVidMem(&ddsCaps,&dwTotal,&dwFree))) {
            wdxdisplay_cat.debug() << "GetAvailableVidMem failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }
        wdxdisplay_cat.debug() << "after FullScreen switch: GetAvailableVidMem returns Total: " << dwTotal/1000000.0 << "  Free: " << dwFree/1000000.0 << endl;
#endif

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
            wdxdisplay_cat.fatal()
            << "CreateFullscreenBuffers() - CreateSurface failed for primary surface: result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        // Clear the primary surface to black

        DX_DECLARE_CLEAN(DDBLTFX, bltfx)
        bltfx.dwDDFX |= DDBLTFX_NOTEARING;
        hr = pPrimaryDDSurf->Blt(NULL,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bltfx);

        if(FAILED( hr )) {
            wdxdisplay_cat.fatal()
            << "Blt to Black of Primary Surf failed! : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        // Get the backbuffer, which was created along with the primary.
        DDSCAPS2 ddscaps = { DDSCAPS_BACKBUFFER, 0, 0, 0};
        if(FAILED( hr = pPrimaryDDSurf->GetAttachedSurface( &ddscaps, &pBackDDSurf ) )) {
            wdxdisplay_cat.fatal()
            << "CreateFullscreenBuffers() - Can't get the backbuffer: result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        if(FAILED( hr = pBackDDSurf->Blt(NULL,NULL,NULL,DDBLT_COLORFILL | DDBLT_WAIT,&bltfx))) {
            wdxdisplay_cat.fatal()
            << "Blt to Black of Back Surf failed! : result = " << ConvD3DErrorToString(hr) << endl;
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
            wdxdisplay_cat.fatal()
            << "GetDisplayMode failed result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }
        if(SurfaceDesc.ddpfPixelFormat.dwRGBBitCount <= 8) {
            wdxdisplay_cat.fatal()
            << "config() - Can't run windowed in an 8-bit or less display mode" << endl;
            exit(1);
        }

        if(!(BitDepth_2_DDBDMask(SurfaceDesc.ddpfPixelFormat.dwRGBBitCount) & d3ddevs[DeviceIdx].dwDeviceRenderBitDepth)) {
            wdxdisplay_cat.fatal() << "3D Device doesnt support rendering at " << SurfaceDesc.ddpfPixelFormat.dwRGBBitCount << "bpp (current desktop bitdepth)" << endl;
            exit(1);
        }

//      extern bool dx_preserve_fpu_state;
        DWORD SCL_FPUFlag = DDSCL_FPUSETUP;

//      if(dx_preserve_fpu_state)
//         SCL_FPUFlag = DDSCL_FPUPRESERVE;  // tell d3d to preserve the fpu state across calls.  this hurts perf, but is good for dbgging

        if(FAILED(hr = pDD->SetCooperativeLevel(_mwindow, SCL_FPUFlag | DDSCL_NORMAL))) {
            wdxdisplay_cat.fatal()
            << "config() - SetCooperativeLevel failed : result = " << ConvD3DErrorToString(hr) << endl;
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

    // Check if the device supports z-bufferless hidden surface removal. If so,
    // we don't really need a z-buffer
    if(!(d3ddevs[DeviceIdx].dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR )) {

        // Get z-buffer dimensions from the render target
        DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd);
        pBackDDSurf->GetSurfaceDesc( &ddsd );

        // Setup the surface desc for the z-buffer.
        ddsd.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
        ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER | DDSCAPS_VIDEOMEMORY;

        DDPIXELFORMAT ZBufPixFmts[MAX_DX_ZBUF_FMTS];

        // Get an appropiate pixel format from enumeration of the formats. On the
        // first pass, we look for a zbuffer dpeth which is equal to the frame
        // buffer depth (as some cards unfornately require this).
        if(FAILED(pD3DI->EnumZBufferFormats(  IID_IDirect3DHALDevice, EnumZBufFmtsCallback,
                                              (VOID*)&ZBufPixFmts ))) {
            wdxdisplay_cat.fatal()
            << "config() - EnumZBufferFormats failed " << endl;
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

        LPDDPIXELFORMAT pCurPixFmt,pz16=NULL,pz24=NULL,pz32=NULL;
        for(i=0,pCurPixFmt=ZBufPixFmts;i<cNumZBufFmts;i++,pCurPixFmt++) {
            switch(pCurPixFmt->dwRGBBitCount) {
                case 16:
                    if(!(pCurPixFmt->dwFlags & DDPF_STENCILBUFFER))
                        pz16=pCurPixFmt;
                    break;
                case 24:
                    assert(!(pCurPixFmt->dwFlags & DDPF_STENCILBUFFER));  // shouldnt be stencil at 24bpp
                    pz24=pCurPixFmt;
                    break;
                case 32:
                    if(!(pCurPixFmt->dwFlags & DDPF_STENCILBUFFER))
                        pz32=pCurPixFmt;
                    break;
            }
        }

        if(bIsNvidia) {
            DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd_pri)
            pPrimaryDDSurf->GetSurfaceDesc(&ddsd_pri);

            // must pick zbuf depth to match primary surface depth for nvidia
            if(ddsd_pri.ddpfPixelFormat.dwRGBBitCount==16) {
                assert(pz16!=NULL);
                ddsd.ddpfPixelFormat = *pz16;
            } else {
                assert(pz24!=NULL);
                ddsd.ddpfPixelFormat = *pz24;  //take the no-stencil version of the 32-bit Zbuf
            }
        } else {
            // pick the largest non-stencil zbuffer format avail (wont support stenciling
            // until we definitely need it).  Note: this is choosing to waste memory
            // and possibly perf for more accuracy at long distance (std 16bpp would be smaller/
            // maybe faster)


            if(pz32!=NULL) {
                ddsd.ddpfPixelFormat = *pz32;
            } else if(pz24!=NULL) {
                ddsd.ddpfPixelFormat = *pz24;
            } else if(pz16!=NULL) {
                ddsd.ddpfPixelFormat = *pz16;
            } else {
                wdxdisplay_cat.fatal() << "config() - could find a valid zbuffer format!\n";
                exit(1);
            }
        }

        PRINTVIDMEM(pDD,&ddsd.ddsCaps,"initial zbuf");

        // Create and attach a z-buffer
        if(FAILED( hr = pDD->CreateSurface( &ddsd, &pZDDSurf, NULL ) )) {
            wdxdisplay_cat.fatal()
            << "config() - CreateSurface failed for Z buffer: result = " <<  ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

        if(FAILED( hr = pBackDDSurf->AddAttachedSurface( pZDDSurf ) )) {
            wdxdisplay_cat.fatal()
            << "config() - AddAttachedSurface failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

#ifdef _DEBUG
        wdxdisplay_cat.debug() << "Creating " << ddsd.ddpfPixelFormat.dwRGBBitCount << "bpp zbuffer\n";
#endif
    }

    // Create the device. The device is created off of our back buffer, which
    // becomes the render target for the newly created device.
    hr = pD3DI->CreateDevice(d3ddevs[DeviceIdx].deviceGUID, pBackDDSurf, &pD3DDevice );
    if(hr != DD_OK) {
        wdxdisplay_cat.fatal()
        << "config() - CreateDevice failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    // Create the viewport
    D3DVIEWPORT7 vp = { 0, 0, _props._xsize, _props._ysize, 0.0f, 1.0f};
    hr = pD3DDevice->SetViewport( &vp );
    if(hr != DD_OK) {
        wdxdisplay_cat.fatal()
        << "config() - SetViewport failed : result = " << ConvD3DErrorToString(hr) << endl;
        exit(1);
    }

    _dxgsg->Set_HDC(_hdc);
    _dxgsg->init_dx(pDD, pPrimaryDDSurf, pBackDDSurf, pZDDSurf, pD3DI, pD3DDevice, view_rect);

    _dxgsg->SetDXReady(true);
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
        SetCursor(hCursor);
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
handle_keyrelease(ButtonHandle key, int, int) {
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
ButtonHandle  wdxGraphicsWindow::
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
                if(GetKeyState(VK_SHIFT) >= 0)
                    key = tolower(key);
                else {
                    switch(key) {
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
