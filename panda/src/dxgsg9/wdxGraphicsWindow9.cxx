// Filename: wdxGraphicsWindow8.cxx
// Created by:  masad (05Jan04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2004, Disney Enterprises, Inc.  All rights reserved
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
#include "wdxGraphicsPipe9.h"
#include "wdxGraphicsWindow9.h"
#include "config_dxgsg9.h"
#include "config_display.h"

#include "keyboardButton.h"
#include "mouseButton.h"
#include "throw_event.h"

#ifdef DO_PSTATS
#include "pStatTimer.h"
#endif

#include <ddraw.h>
#include <map>

TypeHandle wdxGraphicsWindow9::_type_handle;

#define WDX_WINDOWCLASSNAME "wdxDisplay"
#define WDX_WINDOWCLASSNAME_NOCURSOR WDX_WINDOWCLASSNAME "_NoCursor"
#define DEFAULT_CURSOR IDC_ARROW

// define this to enable debug testing of dinput joystick
//#define DINPUT_DEBUG_POLL

typedef map<HWND,wdxGraphicsWindow9 *> HWND_PANDAWIN_MAP;

HWND_PANDAWIN_MAP hwnd_pandawin_map;
wdxGraphicsWindow9* global_wdxwinptr = NULL;  // need this for temporary windproc

#define MAX_DISPLAYS 20

#define PAUSED_TIMER_ID        7   // completely arbitrary choice
#define JOYSTICK_POLL_TIMER_ID 8
#define DX_IS_READY ((_dxgsg!=NULL)&&(_dxgsg->GetDXReady()))

LONG WINAPI static_window_proc(HWND hwnd, UINT msg, WPARAM wparam,LPARAM lparam);

/*
// because we dont have access to ModifierButtons, as a hack just synchronize state of these
// keys on get/lose keybd focus
#define NUM_MODIFIER_KEYS 16
unsigned int hardcoded_modifier_buttons[NUM_MODIFIER_KEYS]={VK_SHIFT,VK_MENU,VK_CONTROL,VK_SPACE,VK_TAB,
                                                            VK_UP,VK_DOWN,VK_LEFT,VK_RIGHT,VK_PRIOR,VK_NEXT,VK_HOME,VK_END,
                                                            VK_INSERT,VK_DELETE,VK_ESCAPE};
*/
//#define UNKNOWN_VIDMEM_SIZE 0xFFFFFFFF

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
wdxGraphicsWindow9::
wdxGraphicsWindow9(GraphicsPipe *pipe, GraphicsStateGuardian *gsg) :
  WinGraphicsWindow(pipe, gsg) 
{
  // dont actually create the window in the constructor.  reason: multi-threading requires
  // panda C++ window object to exist in separate thread from actual API window

  _dxgsg = DCAST(DXGraphicsStateGuardian9, gsg);
  _depth_buffer_bpp = 0;
  _awaiting_restore = false;
  ZeroMemory(&_wcontext,sizeof(_wcontext));
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
wdxGraphicsWindow9::
~wdxGraphicsWindow9() {
}

void wdxGraphicsWindow9::
make_current(void) {
  DXGraphicsStateGuardian9 *dxgsg;
  DCAST_INTO_V(dxgsg, _gsg);
  //wglMakeCurrent(_hdc, wdxgsg->_context);
  dxgsg->set_context(&_wcontext);

  // Now that we have made the context current to a window, we can
  // reset the GSG state if this is the first time it has been used.
  // (We can't just call reset() when we construct the GSG, because
  // reset() requires having a current context.)
  dxgsg->reset_if_new();

  //wdxdisplay9_cat.debug() << "this is " << this << "\n";
}

/* BUGBUG:  need to reinstate these methods ASAP.  they were incorrectly moved from the GraphicsWindow to the GSG
            apps need to know the framebuffer format so they can create texture/rendertgt with same fmt
int wdxGraphicsWindow9::
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

void wdxGraphicsWindow9::
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

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::verify_window_sizes
//       Access: Public, Virtual
//  Description: Determines which of the indicated window sizes are
//               supported by available hardware (e.g. in fullscreen
//               mode).
//
//               On entry, dimen is an array containing contiguous x,y
//               pairs specifying possible display sizes; it is
//               numsizes*2 words long.  The function will zero out
//               any invalid x,y size pairs.  The return value is the
//               number of valid sizes that were found.
////////////////////////////////////////////////////////////////////
int wdxGraphicsWindow9::
verify_window_sizes(int numsizes, int *dimen) {
  // unfortunately this only works AFTER you make the window
  // initially, so its really mostly useful for resizes only
  assert(IS_VALID_PTR(_dxgsg));

  int num_valid_modes = 0;

  wdxGraphicsPipe9 *dxpipe;
  DCAST_INTO_R(dxpipe, _pipe, 0);

  // not requesting same refresh rate since changing res might not
  // support same refresh rate at new size

  int *pCurDim = dimen;

  for (int i=0; i < numsizes; i++, pCurDim += 2) {
    int x_size = pCurDim[0];
    int y_size = pCurDim[1];

    bool bIsGoodMode = false;
    bool CouldntFindAnyValidZBuf;
    D3DFORMAT newPixFmt = D3DFMT_UNKNOWN;

    if (dxpipe->special_check_fullscreen_resolution(_wcontext, x_size, y_size)) {
      // bypass the test below for certain cards we know have valid modes
      bIsGoodMode=true;

    } else {
      if (_wcontext.bIsLowVidMemCard) {
        bIsGoodMode = ((x_size == 640) && (y_size == 480));
      } else  {
        dxpipe->search_for_valid_displaymode(_wcontext, x_size, y_size, _wcontext.PresParams.EnableAutoDepthStencil != false,
                                     IS_STENCIL_FORMAT(_wcontext.PresParams.AutoDepthStencilFormat),
                                     &_wcontext.SupportedScreenDepthsMask,
                                     &CouldntFindAnyValidZBuf, &newPixFmt, dx_force_16bpp_zbuffer);
        bIsGoodMode = (newPixFmt != D3DFMT_UNKNOWN);
      }
    }

    if (bIsGoodMode) {
      num_valid_modes++;
    } else {
      // tell caller the mode is invalid
      pCurDim[0] = 0;
      pCurDim[1] = 0;
    }

    if (wdxdisplay9_cat.is_spam()) {
      wdxdisplay9_cat.spam()
        << "Fullscrn Mode (" << x_size << "," << y_size << ")\t" 
        << (bIsGoodMode ? "V" : "Inv") <<"alid\n";
    }
  }

  return num_valid_modes;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow9::
begin_frame() {
  if (_awaiting_restore) {
    // The fullscreen window was recently restored; we can't continue
    // until the GSG says we can.
    if (!_dxgsg->CheckCooperativeLevel()) {
      // Keep waiting.
      return false;
    }
    _awaiting_restore = false;

    init_resized_window();
  }

  bool return_val = WinGraphicsWindow::begin_frame();
  _dxgsg->set_render_target();
  return return_val;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::end_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after begin_flip() has been called on all windows, to
//               finish the exchange of the front and back buffers.
//
//               This should cause the window to wait for the flip, if
//               necessary.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow9::
end_flip() {
  if (_dxgsg != (DXGraphicsStateGuardian9 *)NULL && is_active()) {
    make_current();
    //wdxdisplay9_cat.debug() << "current swapchain from end_flip is " << _wcontext.pSwapChain << "\n";
    _dxgsg->show_frame();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::fullscreen_restored
//       Access: Protected, Virtual
//  Description: This is a hook for derived classes to do something
//               special, if necessary, when a fullscreen window has
//               been restored after being minimized.  The given
//               WindowProperties struct will be applied to this
//               window's properties after this function returns.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow9::
fullscreen_restored(WindowProperties &properties) {
  // In DX9, unlike DX7, for some reason we can't immediately start
  // rendering as soon as the window is restored, even though
  // BeginScene() says we can.  Instead, we have to wait until
  // TestCooperativeLevel() lets us in.  We need to set a flag so we
  // can handle this special case in begin_frame().
  if (_dxgsg != NULL) {
    _awaiting_restore = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::handle_reshape
//       Access: Protected, Virtual
//  Description: Called in the window thread when the window size or
//               location is changed, this updates the properties
//               structure accordingly.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow9::
handle_reshape() {
  GdiFlush();
  WinGraphicsWindow::handle_reshape();

  if (_dxgsg != NULL) {
    // create the new resized rendertargets
    WindowProperties props = get_properties();
    int x_size = props.get_x_size();
    int y_size = props.get_y_size();
    bool resize_succeeded = reset_device_resize_window(x_size, y_size);
    if (!resize_succeeded) {
      if (wdxdisplay9_cat.is_debug()) {
        wdxdisplay9_cat.debug()
          << "windowed_resize to size: (" << x_size << "," << y_size
          << ") failed due to out-of-memory\n";
      } else {
        if (wdxdisplay9_cat.is_debug()) {
          int x_origin = props.get_x_origin();
          int y_origin = props.get_y_origin();
          wdxdisplay9_cat.debug()
            << "windowed_resize to origin: (" << x_origin << ","
            << y_origin << "), size: (" << x_size
            << "," << y_size << ")\n";
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::do_fullscreen_resize
//       Access: Protected, Virtual
//  Description: Called in the window thread to resize a fullscreen
//               window.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow9::
do_fullscreen_resize(int x_size, int y_size) {
  bool bCouldntFindValidZBuf;
  D3DFORMAT pixFmt;
  bool bNeedZBuffer = (_wcontext.PresParams.EnableAutoDepthStencil!=false);
  bool bNeedStencilBuffer = IS_STENCIL_FORMAT(_wcontext.PresParams.AutoDepthStencilFormat);

  wdxGraphicsPipe9 *dxpipe;
  DCAST_INTO_R(dxpipe, _pipe, false);

  bool bIsGoodMode=false;
  bool bResizeSucceeded=false;

  if (!dxpipe->special_check_fullscreen_resolution(_wcontext, x_size,y_size)) {
    // bypass the lowvidmem test below for certain "lowmem" cards we know have valid modes

    // wdxdisplay9_cat.info() << "1111111 lowvidmemcard="<< _wcontext.bIsLowVidMemCard << endl;
    if (_wcontext.bIsLowVidMemCard && (!((x_size==640) && (y_size==480)))) {
      wdxdisplay9_cat.error() << "resize() failed: will not try to resize low vidmem device #" << _wcontext.CardIDNum << " to non-640x480!\n";
      goto Error_Return;
    }
  }

  // must ALWAYS use search_for_valid_displaymode even if we know
  // a-priori that res is valid so we can get a valid pixfmt
  dxpipe->search_for_valid_displaymode(_wcontext, x_size, y_size, 
                               bNeedZBuffer, bNeedStencilBuffer,
                               &_wcontext.SupportedScreenDepthsMask,
                               &bCouldntFindValidZBuf,
                               &pixFmt, dx_force_16bpp_zbuffer);
  bIsGoodMode=(pixFmt!=D3DFMT_UNKNOWN);

  if (!bIsGoodMode) {
    wdxdisplay9_cat.error() << "resize() failed: "
                           << (bCouldntFindValidZBuf ? "Couldnt find valid zbuffer format to go with FullScreen mode" : "No supported FullScreen modes")
                           << " at " << x_size << "x" << y_size << " for device #" << _wcontext.CardIDNum <<endl;
    goto Error_Return;
  }

  // reset_device_resize_window handles both windowed & fullscrn,
  // so need to set new displaymode manually here
  _wcontext.DisplayMode.Width=x_size;
  _wcontext.DisplayMode.Height=y_size;
  _wcontext.DisplayMode.Format = pixFmt;
  _wcontext.DisplayMode.RefreshRate = D3DPRESENT_RATE_DEFAULT;

  _wcontext.PresParams.BackBufferFormat = pixFmt;   // make reset_device_resize use presparams or displaymode??

  bResizeSucceeded = reset_device_resize_window(x_size, y_size);

  if (!bResizeSucceeded) {
    wdxdisplay9_cat.error() << "resize() failed with OUT-OF-MEMORY error!\n";

    if ((!IS_16BPP_DISPLAY_FORMAT(_wcontext.PresParams.BackBufferFormat)) &&
       (_wcontext.SupportedScreenDepthsMask & (R5G6B5_FLAG|X1R5G5B5_FLAG))) {
      // fallback strategy, if we trying >16bpp, fallback to 16bpp buffers
      _wcontext.DisplayMode.Format = ((_wcontext.SupportedScreenDepthsMask & R5G6B5_FLAG) ? D3DFMT_R5G6B5 : D3DFMT_X1R5G5B5);
      dx_force_16bpp_zbuffer=true;
      if (wdxdisplay9_cat.info())
        wdxdisplay9_cat.info() << "CreateDevice failed with out-of-vidmem, retrying w/16bpp buffers on device #"<< _wcontext.CardIDNum << endl;

      bResizeSucceeded= reset_device_resize_window(x_size, y_size);  // create the new resized rendertargets
    }
  }

 Error_Return:

  if (wdxdisplay9_cat.is_debug())
    wdxdisplay9_cat.debug() << "fullscrn resize("<<x_size<<","<<y_size<<") " << (bResizeSucceeded ? "succeeds\n" : "fails\n");

  return bResizeSucceeded;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::support_overlay_window
//       Access: Protected, Virtual
//  Description: Some windows graphics contexts (e.g. DirectX)
//               require special support to enable the displaying of
//               an overlay window (particularly the IME window) over
//               the fullscreen graphics window.  This is a hook for
//               the window to enable or disable that mode when
//               necessary.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow9::
support_overlay_window(bool flag) {
  if (_dxgsg != (DXGraphicsStateGuardian9 *)NULL) {
    _dxgsg->support_overlay_window(flag);
  }
}

#if 1
//////////////////////////////////////////////////////////////////
//     Function: WinGraphicsWindow::window_proc
//       Access: Private
//  Description: This is the nonstatic window_proc function.  It is
//               called to handle window events for this particular
//               window.
////////////////////////////////////////////////////////////////////
LONG wdxGraphicsWindow9::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  return WinGraphicsWindow::window_proc(hwnd,msg,wparam,lparam);
}

#else

////////////////////////////////////////////////////////////////////
//     Function: window_proc
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
LONG wdxGraphicsWindow9::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    int button = -1;
    int x, y, width, height;

    switch(msg) {
        case WM_SETCURSOR: {
            // Turn off any GDI window cursor
            //  dx9 cursor not working yet

            if(dx_use_dx_cursor && is_fullscreen()) {
                //            SetCursor( NULL );
            //                _dxgsg->scrn.pD3DDevice->ShowCursor(true);

                set_cursor_visibility(true);
                return TRUE; // prevent Windows from setting cursor to window class cursor (see docs on WM_SETCURSOR)
            }
            break;
        }

        case WM_PAINT: {
           // primarily seen when app window is 'uncovered'
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

        case WM_ENTERSIZEMOVE:
             if(_dxgsg!=NULL) 
                _dxgsg->SetDXReady(false);   // dont see pic during resize
             _WindowAdjustingType = MovingOrResizing;
          break;

        case WM_EXITSIZEMOVE: {
            #ifdef _DEBUG
              wdxdisplay_cat.spam()  << "WM_EXITSIZEMOVE received"  << endl;
            #endif

            if(_WindowAdjustingType==Resizing) {
                bool bSucceeded=handle_windowed_resize(hwnd,true);

                if(!bSucceeded) {
                    #if 0
                                bugbug need to fix this stuff
                                SetWindowPos(hwnd,NULL,0,0,lastxsize,lastysize,SWP_NOMOVE |
                    #endif
                }
            }

            _WindowAdjustingType = NotAdjusting;
            _dxgsg->SetDXReady(true);
            return 0;
        }

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
    
        case WM_ERASEBKGND: {
            // WM_ERASEBKGND will be ignored during resizing, because
            // we dont want WM_PAINT's generated as user is manually resizing window.
            
            // for the intermediate resizing images that WM_PAINT would show to be useful, 
            // the panda window parameters need to be reset on every
            // WM_SIZE event and that isnt happening yet

            if(_WindowAdjustingType)
                break;
            return 0;  // dont let GDI waste time redrawing the deflt background
        }

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

  return WinGraphicsWindow::window_proc(hwnd,msg,wparam,lparam);
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::create_screen_buffers_and_device
//       Access: Private
//  Description: Called whenever the window is resized, this recreates
//               the necessary buffers for rendering.
//
//               Sets _depth_buffer_bpp appropriately.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow9::
create_screen_buffers_and_device(DXScreenData &Display, bool force_16bpp_zbuffer) {
  wdxGraphicsPipe9 *dxpipe;
  DCAST_INTO_V(dxpipe, _pipe);

  // only want dx_pick_best_screenres to apply to initial startup, and 
  // since the initial res has already been picked, dont use auto-res-select in any future init sequence.
  dx_pick_best_screenres = false;

  DWORD dwRenderWidth=Display.DisplayMode.Width;
  DWORD dwRenderHeight=Display.DisplayMode.Height;
  DWORD dwBehaviorFlags=0x0;
  LPDIRECT3D9 pD3D9=Display.pD3D9;
  D3DCAPS9 *pD3DCaps = &Display.d3dcaps;
  D3DPRESENT_PARAMETERS* pPresParams = &Display.PresParams;
  RECT view_rect;
  HRESULT hr;

  // BUGBUG: need to change panda to put frame buffer properties with GraphicsWindow, not GSG!!
  int frame_buffer_mode = _gsg->get_properties().get_frame_buffer_mode();
  bool bWantStencil = ((frame_buffer_mode & FrameBufferProperties::FM_stencil) != 0);

  PRINT_REFCNT(wdxdisplay9,pD3D9);

  assert(pD3D9!=NULL);
  assert(pD3DCaps->DevCaps & D3DDEVCAPS_HWRASTERIZATION);

  pPresParams->BackBufferFormat = Display.DisplayMode.Format;  // dont need dest alpha, so just use adapter format

  if (dx_sync_video && !(pD3DCaps->Caps & D3DCAPS_READ_SCANLINE)) {
    wdxdisplay9_cat.info() << "HW doesnt support syncing to vertical refresh, ignoring dx_sync_video\n";
    dx_sync_video=false;
  }

  // verify the rendertarget fmt one last time
  if (FAILED(pD3D9->CheckDeviceFormat(Display.CardIDNum, D3DDEVTYPE_HAL, Display.DisplayMode.Format,D3DUSAGE_RENDERTARGET,
                                     D3DRTYPE_SURFACE, pPresParams->BackBufferFormat))) {
    wdxdisplay9_cat.error() << "device #"<<Display.CardIDNum<< " CheckDeviceFmt failed for surface fmt "<< D3DFormatStr(pPresParams->BackBufferFormat) << endl;
    goto Fallback_to_16bpp_buffers;
  }

  if (FAILED(pD3D9->CheckDeviceType(Display.CardIDNum,D3DDEVTYPE_HAL, Display.DisplayMode.Format,pPresParams->BackBufferFormat,
                                   is_fullscreen()))) {
    wdxdisplay9_cat.error() << "device #"<<Display.CardIDNum<< " CheckDeviceType failed for surface fmt "<< D3DFormatStr(pPresParams->BackBufferFormat) << endl;
    goto Fallback_to_16bpp_buffers;
  }

  if (Display.PresParams.EnableAutoDepthStencil) {
    if (!dxpipe->find_best_depth_format(Display, Display.DisplayMode,
                               &Display.PresParams.AutoDepthStencilFormat,
                               bWantStencil, false)) {
      wdxdisplay9_cat.error()
        << "find_best_depth_format failed in CreateScreenBuffers for device #"
        << Display.CardIDNum << endl;
      goto Fallback_to_16bpp_buffers;
    }
    _depth_buffer_bpp = D3DFMT_to_DepthBits(Display.PresParams.AutoDepthStencilFormat);
  } else {
    _depth_buffer_bpp = 0;
  }

  pPresParams->Windowed = !is_fullscreen();

  if (dx_multisample_antialiasing_level>1) {
    // need to check both rendertarget and zbuffer fmts
    hr = pD3D9->CheckDeviceMultiSampleType(Display.CardIDNum, D3DDEVTYPE_HAL, Display.DisplayMode.Format,
                                           is_fullscreen(), D3DMULTISAMPLE_TYPE(dx_multisample_antialiasing_level), NULL);
    if (FAILED(hr)) {
      wdxdisplay9_cat.fatal() << "device #"<<Display.CardIDNum<< " doesnt support multisample level "<<dx_multisample_antialiasing_level <<"surface fmt "<< D3DFormatStr(Display.DisplayMode.Format) <<endl;
      exit(1);
    }

    if (Display.PresParams.EnableAutoDepthStencil) {
      hr = pD3D9->CheckDeviceMultiSampleType(Display.CardIDNum, D3DDEVTYPE_HAL, Display.PresParams.AutoDepthStencilFormat,
                                             is_fullscreen(), D3DMULTISAMPLE_TYPE(dx_multisample_antialiasing_level), NULL);
      if (FAILED(hr)) {
        wdxdisplay9_cat.fatal() << "device #"<<Display.CardIDNum<< " doesnt support multisample level "<<dx_multisample_antialiasing_level <<"surface fmt "<< D3DFormatStr(Display.PresParams.AutoDepthStencilFormat) <<endl;
        exit(1);
      }
    }

    pPresParams->MultiSampleType = D3DMULTISAMPLE_TYPE(dx_multisample_antialiasing_level);

    if (wdxdisplay9_cat.is_info())
      wdxdisplay9_cat.info() << "device #"<<Display.CardIDNum<< " using multisample antialiasing level "<<dx_multisample_antialiasing_level <<endl;
  }

  pPresParams->BackBufferCount = 1;
  pPresParams->Flags = 0x0;
  pPresParams->hDeviceWindow = Display.hWnd;
  pPresParams->BackBufferWidth = Display.DisplayMode.Width;
  pPresParams->BackBufferHeight = Display.DisplayMode.Height;

#if 0
  GetClientRect(GetDesktopWindow(), &view_rect);
  pPresParams->BackBufferWidth = view_rect.right;
  pPresParams->BackBufferHeight = view_rect.bottom;
  wdxdisplay9_cat.debug()<<"width "<<view_rect.right<<" and height "<<view_rect.bottom<<"\n";
#endif

  if (_wcontext.bIsTNLDevice) {
    dwBehaviorFlags|=D3DCREATE_HARDWARE_VERTEXPROCESSING;
    // note: we could create a pure device in this case if I eliminated the GetRenderState calls in dxgsg

    // also, no software vertex processing available since I specify D3DCREATE_HARDWARE_VERTEXPROCESSING
    // and not D3DCREATE_MIXED_VERTEXPROCESSING
  } else {
    dwBehaviorFlags|=D3DCREATE_SOFTWARE_VERTEXPROCESSING;
  }

  if (dx_preserve_fpu_state)
    dwBehaviorFlags|=D3DCREATE_FPU_PRESERVE;

  // if window is not foreground in exclusive mode, ddraw thinks you are 'not active', so
  // it changes your WM_ACTIVATEAPP from true to false, causing us
  // to go into a 'wait-for WM_ACTIVATEAPP true' loop, and the event never comes so we hang
  // in fullscreen wait.  also doing this for windowed mode since it was requested.
  if (!SetForegroundWindow(Display.hWnd)) {
    wdxdisplay9_cat.warning() << "SetForegroundWindow() failed!\n";
  }

  if (is_fullscreen()) {
    pPresParams->SwapEffect = D3DSWAPEFFECT_DISCARD;  // we dont care about preserving contents of old frame
    pPresParams->PresentationInterval = (dx_sync_video ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE);
    pPresParams->FullScreen_RefreshRateInHz = Display.DisplayMode.RefreshRate;

#ifdef _DEBUG
    if (pPresParams->MultiSampleType != D3DMULTISAMPLE_NONE)
      assert(pPresParams->SwapEffect == D3DSWAPEFFECT_DISCARD);  // only valid effect for multisample
#endif

    ClearToBlack(Display.hWnd, get_properties());

    hr = pD3D9->CreateDevice(Display.CardIDNum, D3DDEVTYPE_HAL, _hWnd,
                             dwBehaviorFlags, pPresParams, &Display.pD3DDevice);

    if (FAILED(hr)) {
      wdxdisplay9_cat.fatal() << "D3D CreateDevice failed for device #" << Display.CardIDNum << ", " << D3DERRORSTRING(hr);

      if (hr == D3DERR_OUTOFVIDEOMEMORY)
        goto Fallback_to_16bpp_buffers;
    }

    SetRect(&view_rect, 0, 0, dwRenderWidth, dwRenderHeight);
  }   // end create full screen buffers

  else {          // CREATE WINDOWED BUFFERS

    /* not necessary anymore...all cards can do this now a days
    if (!(pD3DCaps->Caps2 & D3DCAPS2_CANRENDERWINDOWED)) {
      wdxdisplay9_cat.fatal() << "the 3D HW cannot render windowed, exiting..." << endl;
      exit(1);
    }
    */

    D3DDISPLAYMODE dispmode;
    hr = Display.pD3D9->GetAdapterDisplayMode(Display.CardIDNum, &dispmode);

    if (FAILED(hr)) {
      wdxdisplay9_cat.fatal() << "GetAdapterDisplayMode failed" << D3DERRORSTRING(hr);
      exit(1);
    }

    if (dispmode.Format == D3DFMT_P8) {
      wdxdisplay9_cat.fatal() << "Can't run windowed in an 8-bit or less display mode" << endl;
      exit(1);
    }

    pPresParams->PresentationInterval = 0;

    if (dx_multisample_antialiasing_level<2) {
      if (dx_sync_video) {
        pPresParams->SwapEffect = D3DSWAPEFFECT_COPY;
      } else {
        pPresParams->SwapEffect = D3DSWAPEFFECT_DISCARD;  //D3DSWAPEFFECT_COPY;  does this make any difference?
      }
    } else {
      pPresParams->SwapEffect = D3DSWAPEFFECT_DISCARD;
    }

    //    assert((dwRenderWidth==pPresParams->BackBufferWidth)&&(dwRenderHeight==pPresParams->BackBufferHeight));

    hr = pD3D9->CreateDevice(Display.CardIDNum, D3DDEVTYPE_HAL, _hWnd,
                             dwBehaviorFlags, pPresParams, &Display.pD3DDevice);

    if (FAILED(hr)) {
      wdxdisplay9_cat.fatal() << "D3D CreateDevice failed for device #" << Display.CardIDNum << D3DERRORSTRING(hr);
      exit(1);
    }
  }  // end create windowed buffers

#if 0
  pPresParams->BackBufferWidth = Display.DisplayMode.Width;
  pPresParams->BackBufferHeight = Display.DisplayMode.Height;
#endif

  //  ========================================================

  PRINT_REFCNT(wdxdisplay9,_wcontext.pD3DDevice);

  if (pPresParams->EnableAutoDepthStencil) {
    _dxgsg->_buffer_mask |= RenderBuffer::T_depth;
    if (IS_STENCIL_FORMAT(pPresParams->AutoDepthStencilFormat))
      _dxgsg->_buffer_mask |= RenderBuffer::T_stencil;
  }

  init_resized_window();

  return;

 Fallback_to_16bpp_buffers:

  if ((!IS_16BPP_DISPLAY_FORMAT(pPresParams->BackBufferFormat)) &&
     (Display.SupportedScreenDepthsMask & (R5G6B5_FLAG|X1R5G5B5_FLAG))) {
    // fallback strategy, if we trying >16bpp, fallback to 16bpp buffers

    Display.DisplayMode.Format = ((Display.SupportedScreenDepthsMask & R5G6B5_FLAG) ? D3DFMT_R5G6B5 : D3DFMT_X1R5G5B5);

    if (wdxdisplay9_cat.info()) {
      wdxdisplay9_cat.info()
        << "CreateDevice failed with out-of-vidmem, retrying w/16bpp buffers on device #"
        << Display.CardIDNum << endl;
    }
    create_screen_buffers_and_device(Display, true);
    return;

  } else if (!((dwRenderWidth==640)&&(dwRenderHeight==480))) {
    if (wdxdisplay9_cat.info())
      wdxdisplay9_cat.info() << "CreateDevice failed w/out-of-vidmem, retrying at 640x480 w/16bpp buffers on device #"<< Display.CardIDNum << endl;
    // try final fallback to 640x480x16
    Display.DisplayMode.Width=640;
    Display.DisplayMode.Height=480;
    create_screen_buffers_and_device(Display, true);
    return;

  } else {
    wdxdisplay9_cat.fatal() 
      << "Can't create any screen buffers, bailing out.\n";
    exit(1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::choose_device
//       Access: Private
//  Description: Looks at the list of available graphics adapters and
//               chooses a suitable one for the window.
//
//               Returns true if successful, false on failure.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow9::
choose_device(void) {
  HRESULT hr;

  wdxGraphicsPipe9 *dxpipe;
  DCAST_INTO_R(dxpipe, _pipe, false);

  int num_adapters = dxpipe->_pD3D9->GetAdapterCount();
  DXDeviceInfoVec device_infos;

  for (int i = 0; i < num_adapters; i++) {
    D3DADAPTER_IDENTIFIER9 adapter_info;
    ZeroMemory(&adapter_info, sizeof(D3DADAPTER_IDENTIFIER9));
    hr = dxpipe->_pD3D9->GetAdapterIdentifier(i, 0, &adapter_info);
    if (FAILED(hr)) {
      wdxdisplay9_cat.fatal()
        << "D3D GetAdapterID(" << i << ") failed: "
        << D3DERRORSTRING(hr) << endl;
      continue;
    }
     
    LARGE_INTEGER *DrvVer = &adapter_info.DriverVersion;

    wdxdisplay9_cat.info()
      << "D3D9." << (dxpipe->_bIsDX9 ?"a":"b") << " Adapter[" << i << "]: " << adapter_info.Description 
      << ", Driver: " << adapter_info.Driver << ", DriverVersion: ("
      << HIWORD(DrvVer->HighPart) << "." << LOWORD(DrvVer->HighPart) << "."
      << HIWORD(DrvVer->LowPart) << "." << LOWORD(DrvVer->LowPart)
      << ")\nVendorID: 0x" << (void*) adapter_info.VendorId 
      << " DeviceID: 0x" <<  (void*) adapter_info.DeviceId
      << " SubsysID: 0x" << (void*) adapter_info.SubSysId
      << " Revision: 0x" << (void*) adapter_info.Revision << endl;
    
    HMONITOR hMon = dxpipe->_pD3D9->GetAdapterMonitor(i);
    if (hMon == NULL) {
      wdxdisplay9_cat.info()
        << "D3D9 Adapter[" << i << "]: seems to be disabled, skipping it\n";
      continue;
    }

    DXDeviceInfo devinfo;
    ZeroMemory(&devinfo, sizeof(devinfo));
    memcpy(&devinfo.guidDeviceIdentifier, &adapter_info.DeviceIdentifier, 
           sizeof(GUID));
    strncpy(devinfo.szDescription, adapter_info.Description,
            MAX_DEVICE_IDENTIFIER_STRING);
    strncpy(devinfo.szDriver, adapter_info.Driver,
            MAX_DEVICE_IDENTIFIER_STRING);
    devinfo.VendorID = adapter_info.VendorId;
    devinfo.DeviceID = adapter_info.DeviceId;
    devinfo.hMon = hMon;
    devinfo.cardID = i;

    device_infos.push_back(devinfo);
  }

  if (device_infos.empty()) {
    wdxdisplay9_cat.error()
      << "No available D3D9 devices found.\n";
    return false;
  }

  // Since some adapters may have been disabled, we should re-obtain
  // the number of available adapters.
  num_adapters = (int)device_infos.size();

  // Now choose a suitable adapter.

  int adapter_num = D3DADAPTER_DEFAULT;

  // Eventually, we should have some interface for specifying a device
  // index interactively, instead of only via Configrc.
  if (dx_preferred_device_id != -1) {
    if (dx_preferred_device_id < 0 || dx_preferred_device_id >= num_adapters) {
      wdxdisplay9_cat.error()
        << "invalid 'dx-preferred-device-id', valid values are 0-" 
        << num_adapters - 1 << ", using default adapter instead.\n";
    } else {
      adapter_num = dx_preferred_device_id;
    }
  }

  UINT good_device_count=0;
  for(UINT devnum=0;devnum<device_infos.size() /*&& (good_device_count < num_windows)*/;devnum++) {
      if(search_for_device(dxpipe,&device_infos[devnum]))
          good_device_count++;
  }

  if(good_device_count==0) {
     wdxdisplay9_cat.fatal() << "no usable display devices, exiting...\n";
     return false;
  }

  return true;
}

/*
primary init sequence of old method, still need to integrate multi-window functionality
void wdxGraphicsWindow9Group::initWindowGroup(void) {
    HRESULT hr;

    assert(_windows.size()>0);
    _hOldForegroundWindow=GetForegroundWindow();
    _bClosingAllWindows= false;

    UINT num_windows=_windows.size();

    #define D3D9_NAME "d3d9.dll"
    #define D3DCREATE9 "Direct3DCreate9"

    _hD3D9_DLL = LoadLibrary(D3D9_NAME);
    if(_hD3D9_DLL == 0) {
        wdxdisplay_cat.fatal() << "PandaDX9 requires DX9, can't locate " << D3D9_NAME <<"!\n";
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
              wdxdisplay_cat.fatal() << "system has only " << numMonitors << " monitors attached, couldn't find enough devices to meet multi window reqmt of " << num_windows << endl;
              exit(1);
          }
    }

   // Do all DX7 stuff first
   //  find_all_card_memavails();

    LPDIRECT3D9 pD3D9;

    typedef LPDIRECT3D9 (WINAPI *Direct3DCreate9_ProcPtr)(UINT SDKVersion);

    // dont want to statically link to possibly non-existent d3d9 dll, so must call D3DCr9 indirectly
    Direct3DCreate9_ProcPtr D3DCreate9_Ptr =
        (Direct3DCreate9_ProcPtr) GetProcAddress(_hD3D9_DLL, D3DCREATE9);

    if(D3DCreate9_Ptr == NULL) {
        wdxdisplay_cat.fatal() << "GetProcAddress for "<< D3DCREATE9 << "failed!" << endl;
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
        pD3D8->GetAdapterIdentifier(_windows[i]->_wcontext.CardIDNum,D3DENUM_NO_WHQL_LEVEL,&adapter_info);
        wdxdisplay_cat.info() << "D3D8 Adapter[" << i << "]: " << adapter_info.Description
                              << ", MaxAvailVideoMem: " << _windows[i]->_wcontext.MaxAvailVidMem
                              << ", IsLowVidMemCard: " << (_windows[i]->_wcontext.bIsLowVidMemCard ? "true" : "false") << endl;
      }
    }

    CreateWindows();  // creates win32 windows  (need to do this before Setting coopLvls and display modes,
                      // but after we have all the monitor handles needed by CreateWindow()

//    SetCoopLevelsAndDisplayModes();

    if(dx_show_fps_meter)
       _windows[0]->_dxgsg->_bShowFPSMeter = true;  // just show fps on 1st mon

    for(UINT i=0;i<num_windows;i++) {
        _windows[i]->CreateScreenBuffersAndDevice(_windows[i]->_wcontext);
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
*/

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::search_for_device
//       Access: Private
//  Description: Searches for a suitable hardware device for
//               rendering.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow9::
search_for_device(wdxGraphicsPipe9 *dxpipe, DXDeviceInfo *device_info) {

  assert(dxpipe != NULL);  
  WindowProperties properties = get_properties();
  DWORD dwRenderWidth = properties.get_x_size();
  DWORD dwRenderHeight = properties.get_y_size();
  HRESULT hr;
  LPDIRECT3D9 pD3D9 = dxpipe->_pD3D9;

  assert(_dxgsg != NULL);  
  _wcontext.pD3D9 = pD3D9;
  _wcontext.bIsDX9 = dxpipe->_bIsDX9;
  _wcontext.CardIDNum = device_info->cardID;  // could this change by end?

  int frame_buffer_mode = _gsg->get_properties().get_frame_buffer_mode();
  bool bWantStencil = ((frame_buffer_mode & FrameBufferProperties::FM_stencil) != 0);
  
  hr = pD3D9->GetAdapterIdentifier(device_info->cardID, 0,
                                   &_wcontext.DXDeviceID);
  if (FAILED(hr)) {
    wdxdisplay9_cat.error()
      << "D3D GetAdapterID failed" << D3DERRORSTRING(hr);
    return false;
  }
  
  D3DCAPS9 d3dcaps;
  hr = pD3D9->GetDeviceCaps(device_info->cardID,D3DDEVTYPE_HAL,&d3dcaps);
  if (FAILED(hr)) {
    if ((hr==D3DERR_INVALIDDEVICE)||(hr==D3DERR_NOTAVAILABLE)) {
      wdxdisplay9_cat.error()
        << "No DirectX 9 D3D-capable 3D hardware detected for device # "
        << device_info->cardID << " (" <<device_info->szDescription 
        << ")!\n";
    } else {
      wdxdisplay9_cat.error()
        << "GetDeviceCaps failed: " << D3DERRORSTRING(hr) << endl;
    }
    return false;
  }
  
  //search_for_valid_displaymode needs these to be set
  memcpy(&_wcontext.d3dcaps, &d3dcaps,sizeof(D3DCAPS9));
  _wcontext.CardIDNum = device_info->cardID;
  
  _wcontext.MaxAvailVidMem = UNKNOWN_VIDMEM_SIZE;
  _wcontext.bIsLowVidMemCard = false;
  
  // bugbug: wouldnt we like to do GetAVailVidMem so we can do
  // upper-limit memory computation for dx9 cards too?  otherwise
  // verify_window_sizes cant do much
  if ((d3dcaps.MaxStreams==0) || dx_pick_best_screenres) {
    if (wdxdisplay9_cat.is_debug()) {
      wdxdisplay9_cat.debug()
        << "checking vidmem size\n";
    }
    //    assert(IS_VALID_PTR(_pParentWindowGroup));
    
    // look for low memory video cards
    //    _pParentWindowGroup->find_all_card_memavails();
    
    UINT IDnum;
    
    // simple linear search to match DX7 card info w/DX9 card ID
    for (IDnum=0; IDnum < dxpipe->_card_ids.size(); IDnum++) {
      //      wdxdisplay9_cat.info()
      //        << "comparing '" << dxpipe->_card_ids[IDnum].Driver
      //        << "' to '" << _wcontext.DXDeviceID.Driver << "'\n";
      if (//(stricmp(dxpipe->_card_ids[IDnum].szDriver,device_info->szDriver)==0) &&
         (device_info->VendorID==dxpipe->_card_ids[IDnum].VendorID) &&
         (device_info->DeviceID==dxpipe->_card_ids[IDnum].DeviceID) &&
         (device_info->hMon==dxpipe->_card_ids[IDnum].hMon))
        break;
    }
    
    if (IDnum < dxpipe->_card_ids.size()) {
      _wcontext.MaxAvailVidMem = dxpipe->_card_ids[IDnum].MaxAvailVidMem;
      _wcontext.bIsLowVidMemCard = dxpipe->_card_ids[IDnum].bIsLowVidMemCard;
    } else {
      wdxdisplay9_cat.error()
        << "Error: couldnt find a CardID match in DX7 info, assuming card is not a lowmem card\n";
    }
  }

  if ((bWantStencil) && (d3dcaps.StencilCaps==0x0)) {
    wdxdisplay9_cat.fatal()
      << "Stencil ability requested, but device #" << device_info->cardID
      << " (" << _wcontext.DXDeviceID.Description
      << "), has no stencil capability!\n";
    return false;
  }

  // just because TNL is true, it doesnt mean vtx shaders are
  // supported in HW (see GF2) for this case, you probably want MIXED
  // processing to use HW for fixed-fn vertex processing and SW for
  // vtx shaders
  _wcontext.bIsTNLDevice = 
    ((d3dcaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0);
  _wcontext.bCanUseHWVertexShaders = 
    (d3dcaps.VertexShaderVersion >= D3DVS_VERSION(1, 0));
  _wcontext.bCanUsePixelShaders = 
    (d3dcaps.PixelShaderVersion >= D3DPS_VERSION(1, 0));

  bool bNeedZBuffer = 
    ((!(d3dcaps.RasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR )) &&
     ((frame_buffer_mode & FrameBufferProperties::FM_depth) != 0));

  _wcontext.PresParams.EnableAutoDepthStencil = bNeedZBuffer;

  D3DFORMAT pixFmt = D3DFMT_UNKNOWN;

  if (is_fullscreen()) {
    bool bCouldntFindValidZBuf;
    if (!_wcontext.bIsLowVidMemCard) {
      bool bUseDefaultSize = dx_pick_best_screenres &&
        ((_wcontext.MaxAvailVidMem == UNKNOWN_VIDMEM_SIZE) ||
         is_badvidmem_card(&_wcontext.DXDeviceID));

      if (dx_pick_best_screenres && !bUseDefaultSize) {
        typedef struct {
          UINT memlimit;
          DWORD scrnX,scrnY;
        } Memlimres;

        const Memlimres MemRes[] = {
          {       0,  640, 480},
          { 8000000,  800, 600},
#if 0
          {16000000, 1024, 768},
          {32000000, 1280,1024},  // 32MB+ cards will choose this
#else
          // unfortunately the 32MB card perf varies greatly (TNT2-GF2),
          // so we need to be conservative since frame rate difference
          // can change from 15->30fps when going from 1280x1024->800x600
          // on low-end 32mb cards
          {16000000,  800, 600},
          {32000000,  800, 600},  // 32MB+ cards will choose this
#endif
          // some monitors have trouble w/1600x1200, so dont pick this by deflt,
          // even though 64MB cards should handle it              
          {64000000, 1280,1024}   // 64MB+ cards will choose this
        };
        const NumResLims = (sizeof(MemRes)/sizeof(Memlimres));

        for(int i = NumResLims - 1; i >= 0; i--) {
          // find biggest slot card can handle
          if (_wcontext.MaxAvailVidMem > MemRes[i].memlimit) {
            dwRenderWidth = MemRes[i].scrnX;
            dwRenderHeight = MemRes[i].scrnY;

            wdxdisplay9_cat.info()
              << "pick_best_screenres: trying " << dwRenderWidth 
              << "x" << dwRenderHeight << " based on "
              << _wcontext.MaxAvailVidMem << " bytes avail\n";

             dxpipe->search_for_valid_displaymode(_wcontext,dwRenderWidth, dwRenderHeight, 
                                         bNeedZBuffer, bWantStencil,
                                         &_wcontext.SupportedScreenDepthsMask,
                                         &bCouldntFindValidZBuf,
                                         &pixFmt, dx_force_16bpp_zbuffer);

            // note I'm not saving refresh rate, will just use adapter
            // default at given res for now

            if (pixFmt != D3DFMT_UNKNOWN) {
              break;
            }

            wdxdisplay9_cat.info()
              << "skipping scrnres; "
              << (bCouldntFindValidZBuf ? "Couldnt find valid zbuffer format to go with FullScreen mode" : "No supported FullScreen modes")
              << " at " << dwRenderWidth << "x" << dwRenderHeight
              << " for device #" << _wcontext.CardIDNum << endl;
          }
        }
        // otherwise just go with whatever was specified (we probably shouldve marked this card as lowmem if it gets to end of loop w/o breaking
      }

      if (pixFmt == D3DFMT_UNKNOWN) {
        if (bUseDefaultSize) {
          wdxdisplay9_cat.info()
            << "pick_best_screenres: defaulted 800x600 based on no reliable vidmem size\n";
          dwRenderWidth=800;
          dwRenderHeight=600;
        }

        dxpipe->search_for_valid_displaymode(_wcontext, dwRenderWidth, dwRenderHeight,
                                     bNeedZBuffer, bWantStencil,
                                     &_wcontext.SupportedScreenDepthsMask,
                                     &bCouldntFindValidZBuf,
                                     &pixFmt, dx_force_16bpp_zbuffer);

        // note I'm not saving refresh rate, will just use adapter
        // default at given res for now

        if (pixFmt == D3DFMT_UNKNOWN) {
          wdxdisplay9_cat.error()
            << (bCouldntFindValidZBuf ? "Couldnt find valid zbuffer format to go with FullScreen mode" : "No supported FullScreen modes")
            << " at " << dwRenderWidth << "x" << dwRenderHeight << " for device #" << _wcontext.CardIDNum <<endl;

          // run it again in verbose mode to get more dbg info to log
          dxpipe->search_for_valid_displaymode(_wcontext,dwRenderWidth, dwRenderHeight,
                                       bNeedZBuffer, bWantStencil,
                                       &_wcontext.SupportedScreenDepthsMask,
                                       &bCouldntFindValidZBuf,
                                       &pixFmt, dx_force_16bpp_zbuffer, true);
          return false;
        }
      }
    } else {
      // Low Memory card
      dwRenderWidth=640;
      dwRenderHeight=480;
      dx_force_16bpptextures = true;

      // need to autoforce 16bpp zbuf?  or let user use that extra mem for textures/framebuf res/etc?
      // most lowmem cards only do 16bpp Z anyway, but we wont force it for now

      dxpipe->search_for_valid_displaymode(_wcontext,dwRenderWidth, dwRenderHeight,
                                   bNeedZBuffer, bWantStencil,
                                   &_wcontext.SupportedScreenDepthsMask,
                                   &bCouldntFindValidZBuf,
                                   &pixFmt, dx_force_16bpp_zbuffer);

      // hack: figuring out exactly what res to use is tricky, instead I will
      // just use 640x480 if we have < 3 meg avail

      if (_wcontext.SupportedScreenDepthsMask & R5G6B5_FLAG) {
        pixFmt = D3DFMT_R5G6B5;
      } else if (_wcontext.SupportedScreenDepthsMask & X1R5G5B5_FLAG) {
        pixFmt = D3DFMT_X1R5G5B5;
      } else {
        wdxdisplay9_cat.fatal()
          << "Low Memory VidCard has no supported FullScreen 16bpp resolutions at "
          << dwRenderWidth << "x" << dwRenderHeight << " for device #"
          << device_info->cardID << " (" 
          << _wcontext.DXDeviceID.Description << "), skipping device...\n";

        // run it again in verbose mode to get more dbg info to log
        dxpipe->search_for_valid_displaymode(_wcontext, dwRenderWidth, dwRenderHeight,
                                     bNeedZBuffer, bWantStencil,
                                     &_wcontext.SupportedScreenDepthsMask,
                                     &bCouldntFindValidZBuf,
                                     &pixFmt, dx_force_16bpp_zbuffer, 
                                     true /* verbose mode on*/);
        return false;
      }

      if (wdxdisplay9_cat.is_info()) {
        wdxdisplay9_cat.info()
          << "Available VidMem (" << _wcontext.MaxAvailVidMem
          << ") is under threshold, using 640x480 16bpp rendertargets to save tex vidmem.\n";
      }
    }
  } else {
    // Windowed Mode

    D3DDISPLAYMODE dispmode;
    hr = pD3D9->GetAdapterDisplayMode(device_info->cardID,&dispmode);
    if (FAILED(hr)) {
      wdxdisplay9_cat.error()
        << "GetAdapterDisplayMode(" << device_info->cardID
        << ") failed" << D3DERRORSTRING(hr);
      return false;
    }
    pixFmt = dispmode.Format;
  }

  _wcontext.DisplayMode.Width = dwRenderWidth;
  _wcontext.DisplayMode.Height = dwRenderHeight;
  _wcontext.DisplayMode.Format = pixFmt;
  _wcontext.DisplayMode.RefreshRate = D3DPRESENT_RATE_DEFAULT;
  _wcontext.hMon = device_info->hMon;

  if (dwRenderWidth != properties.get_x_size() ||
      dwRenderHeight != properties.get_y_size()) {
    // This is probably not the best place to put this; I'm just putting
    // it here for now because if dx_pick_best_screenres is true, the
    // code above might have changed the size of the window
    // unexpectedly.  This code gets called when make_gsg() is called,
    // which means it is called in the draw thread, but this method
    // should really be called from the window thread.  In DirectX those
    // may always be the same threads anyway, so we may be all right.
    // Still, it's a little strange that the window may change size
    // after it has already been opened, at the time we create the GSG
    // for it; it would be better if we could find a way to do this
    // resolution-selection logic earlier, say at the time the window is
    // created.
    system_changed_size(dwRenderWidth, dwRenderHeight);
    WindowProperties resized_props;
    resized_props.set_size(dwRenderWidth, dwRenderHeight);
    _properties.add_properties(resized_props);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::reset_device_resize_window
//       Access: Private
//  Description: Called after a window (either fullscreen or windowed)
//               has been resized, this recreates the D3D structures
//               to match the new size.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow9::
reset_device_resize_window(UINT new_xsize, UINT new_ysize) {
  assert((new_xsize > 0) && (new_ysize > 0));
  bool bRetval = true;

  DXScreenData *pScrn;
  D3DPRESENT_PARAMETERS d3dpp;
  memcpy(&d3dpp, &_wcontext.PresParams, sizeof(D3DPRESENT_PARAMETERS));
  d3dpp.BackBufferWidth = new_xsize;
  d3dpp.BackBufferHeight = new_ysize;
  make_current();
  HRESULT hr = _dxgsg->reset_d3d_device(&d3dpp, &pScrn);
  
  if (FAILED(hr)) {
    bRetval = false;
    wdxdisplay9_cat.error()
      << "reset_device_resize_window Reset() failed" << D3DERRORSTRING(hr);
    if (hr == D3DERR_OUTOFVIDEOMEMORY) {
      hr = _dxgsg->reset_d3d_device(&_wcontext.PresParams, &pScrn);
      if (FAILED(hr)) {
        wdxdisplay9_cat.error()
          << "reset_device_resize_window Reset() failed OutOfVidmem, then failed again doing Reset w/original params:" << D3DERRORSTRING(hr);
        exit(1);
      } else {
        if (wdxdisplay9_cat.is_info())
          wdxdisplay9_cat.info()
            << "reset of original size (" << _wcontext.PresParams.BackBufferWidth
            << "," << _wcontext.PresParams.BackBufferHeight << ") succeeded\n";
      }
    } else {
      wdxdisplay9_cat.fatal() 
        << "Can't reset device, bailing out.\n";
      exit(1);
    }
  }
  // before you init_resized_window you need to copy certain changes to _wcontext
  _wcontext.pSwapChain = pScrn->pSwapChain;
  wdxdisplay9_cat.debug() << "swapchain is " << _wcontext.pSwapChain << "\n";
  init_resized_window();
  return bRetval;
}
////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::init_resized_window
//       Access: Private
//  Description: Reinitializes the window after it has been resized,
//               or after it is first created.
//
//               Assumes CreateDevice or Device->Reset() has just been
//               called, and the new size is specified in
//               _wcontext.PresParams.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow9::
init_resized_window() {
  HRESULT hr;

  DWORD newWidth = _wcontext.PresParams.BackBufferWidth;
  DWORD newHeight = _wcontext.PresParams.BackBufferHeight;

  assert((newWidth!=0) && (newHeight!=0));
  assert(_wcontext.hWnd!=NULL);

  if (_wcontext.PresParams.Windowed) {
    POINT ul,lr;
    RECT client_rect;

    // need to figure out x,y origin offset of window client area on screen
    // (we already know the client area size)

    GetClientRect(_wcontext.hWnd, &client_rect);
    ul.x = client_rect.left;
    ul.y = client_rect.top;
    lr.x = client_rect.right;
    lr.y=client_rect.bottom;
    ClientToScreen(_wcontext.hWnd, &ul);
    ClientToScreen(_wcontext.hWnd, &lr);
    client_rect.left = ul.x;
    client_rect.top = ul.y;
    client_rect.right = lr.x;
    client_rect.bottom = lr.y;
    //    _props._xorg = client_rect.left;  // _props should reflect view rectangle
    //    _props._yorg = client_rect.top;

    /*
#ifdef _DEBUG
    // try to make sure GDI and DX agree on window client area size
    // but client rect will not include any offscreen areas, so dont
    // do check if window was bigger than screen (there are other bad
    // cases too, like when window is positioned partly offscreen,
    // or if window trim border make size bigger than screen)

    RECT desktop_rect;
    GetClientRect(GetDesktopWindow(), &desktop_rect);
    int x_size = get_properties().get_x_size();
    int y_size = get_properties().get_y_size();
    if ((x_size < RECT_X_SIZE(desktop_rect)) && 
        (y_size < RECT_Y_SIZE(desktop_rect)))
      assert((RECT_X_SIZE(client_rect) == newWidth) &&
             (RECT_Y_SIZE(client_rect) == newHeight));
#endif
    */
  }

  //  resized(newWidth, newHeight);  // update panda channel/display rgn info, _props.x_size, _props.y_size

  // clear window to black ASAP
  assert(_wcontext.hWnd!=NULL);
  ClearToBlack(_wcontext.hWnd, get_properties());

  // clear textures and VB's out of video&AGP mem, so cache is reset
  hr = _wcontext.pD3DDevice->EvictManagedResources();
  if (FAILED(hr)) {
    wdxdisplay9_cat.error()
      << "ResourceManagerDiscardBytes failed for device #" 
      << _wcontext.CardIDNum << D3DERRORSTRING(hr);
  }

  _dxgsg->set_context(&_wcontext); 
  // Note: dx_init will fill in additional fields in _wcontext, like supportedtexfmts
  _dxgsg->dx_init();

  if(dx_use_dx_cursor && is_fullscreen()) {
      hr = CreateDX9Cursor(_wcontext.pD3DDevice,_mouse_cursor,dx_show_cursor_watermark);
      if(FAILED(hr))
          wdxdisplay9_cat.error() << "CreateDX9Cursor failed!" <<  D3DERRORSTRING(hr);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::D3DFMT_to_DepthBits
//       Access: Private, Static
//  Description: Returns the number of depth bits represented by the
//               indicated D3DFORMAT value.
////////////////////////////////////////////////////////////////////
int wdxGraphicsWindow9::
D3DFMT_to_DepthBits(D3DFORMAT fmt) {
  switch(fmt) {
  case D3DFMT_D16:
    return 16;

  case D3DFMT_D24X8:
  case D3DFMT_D24X4S4:
  case D3DFMT_D24S8:
    return 24;

  case D3DFMT_D32:
    return 32;

  case D3DFMT_D15S1:
    return 15;

  default:
    wdxdisplay9_cat.debug()
      << "D3DFMT_DepthBits: unhandled D3DFMT!\n";
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::is_badvidmem_card
//       Access: Private, Static
//  Description: Returns true if the indicated video adapter card is
//               known to report an inaccurate figure for available
//               video memory.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow9::
is_badvidmem_card(D3DADAPTER_IDENTIFIER9 *pDevID) {
  // dont trust Intel cards since they often use regular memory as vidmem
  if (pDevID->VendorId == 0x00008086) {
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::reset_window
//       Access: Public, Virtual
//  Description: Resets the window framebuffer right now.  Called 
//               from graphicsEngine. It releases the current swap
//               chain / creates a new one. If this is the initial
//               window and swapchain is false, then it calls reset_
//               main_device to Reset the device.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow9::
reset_window(bool swapchain) {
  DXGraphicsStateGuardian9 *dxgsg;
  DCAST_INTO_V(dxgsg,_gsg);
  if (swapchain) {
    if (_wcontext.pSwapChain) {
      dxgsg->create_swap_chain(&_wcontext);
      wdxdisplay9_cat.debug() << "created swapchain " << _wcontext.pSwapChain << "\n";
    }
  }
  else {
    if (_wcontext.pSwapChain) {
      dxgsg->release_swap_chain(&_wcontext);
      wdxdisplay9_cat.debug() << "released swapchain " << _wcontext.pSwapChain << "\n";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow9::open_window
//       Access: Protected, Virtual
//  Description: Opens the window right now.  Called from the window
//               thread.  Returns true if the window is successfully
//               opened, or false if there was a problem.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow9::
open_window(void) {
  PT(DXGraphicsDevice9) dxdev;
  DXGraphicsStateGuardian9 *dxgsg;
  DCAST_INTO_R(dxgsg,_gsg,false);
  WindowProperties props;

  if(!choose_device()) {
      return false;
  }
  if (dxgsg->get_pipe()->get_device() && !multiple_windows) {
    wdxdisplay9_cat.error() 
      << "Could not create window; multiple window support not enabled.\n";
    return false;
  }

  wdxdisplay9_cat.debug() << "_wcontext.hWnd is " << _wcontext.hWnd << "\n";
  if (!WinGraphicsWindow::open_window()) {
    return false;
  }
  _wcontext.hWnd = _hWnd;

  wdxdisplay9_cat.debug() << "_wcontext.hWnd is " << _wcontext.hWnd << "\n";
  
  // Here check if a device already exists. If so, then this open_window
  // call may be an extension to create multiple windows on same device
  // In that case just create an additional swapchain for this window

  if (dxgsg->get_pipe()->get_device() == NULL) {
    create_screen_buffers_and_device(_wcontext, dx_force_16bpp_zbuffer);
    dxgsg->get_pipe()->make_device((void*)(&_wcontext));
    dxgsg->copy_pres_reset(&_wcontext);
    if (multiple_windows) {
      // then we have no choice but to waste a framebuffer
      dxgsg->create_swap_chain(&_wcontext);
    }

  } else {
    // fill in the DXScreenData from dxdevice here and change the
    // reference to hWnd.
    dxdev = (DXGraphicsDevice9*)dxgsg->get_pipe()->get_device();
    props = get_properties();

    memcpy(&_wcontext,dxdev->_pScrn,sizeof(DXScreenData));
    _wcontext.hWnd = _hWnd;
    _wcontext.PresParams.hDeviceWindow = _hWnd;
    _wcontext.PresParams.BackBufferWidth = props.get_x_size();
    _wcontext.PresParams.BackBufferHeight = props.get_y_size();

    wdxdisplay9_cat.debug()<<"device width "<<_wcontext.PresParams.BackBufferWidth<<"\n";

    init_resized_window();

    if (wdxdisplay9_cat.is_debug()) {
      wdxdisplay9_cat.debug() << "Current device is " << dxdev << "\n";
    }
    dxgsg->create_swap_chain(&_wcontext);
  }
  wdxdisplay9_cat.debug() << "swapchain is " << _wcontext.pSwapChain << "\n";
  return true;
}

bool wdxGraphicsWindow9::
handle_mouse_motion(int x, int y) {
  (void) WinGraphicsWindow::handle_mouse_motion(x,y);
  if(dx_use_dx_cursor && is_fullscreen() && (_wcontext.pD3DDevice!=NULL)) {
      _wcontext.pD3DDevice->SetCursorPosition(x,y,D3DCURSOR_IMMEDIATE_UPDATE);
      // return true to indicate wind_proc should return 0 instead of going to DefaultWindowProc
      return true;
  }
  return false;
}

#if 0
// does NOT override _props._bCursorIsVisible
INLINE void wdxGraphicsWindow::
set_cursor_visibility(bool bVisible) {
  if(_props._bCursorIsVisible) {
      if(dx_use_dx_cursor) {
          ShowCursor(false);
          if(IS_VALID_PTR(_wcontext.pD3DDevice))
              _dxgsg->scrn.pD3DDevice->ShowCursor(bVisible);
      } else {
         ShowCursor(bVisible);
      }
  } else {
      ShowCursor(false);
      if(dx_use_dx_cursor && IS_VALID_PTR(_wcontext.pD3DDevice))
          _dxgsg->scrn.pD3DDevice->ShowCursor(false);
  }
}
#endif
