// Filename: wdxGraphicsWindow8.cxx
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
#include "config_dxgsg8.h"

#include "keyboardButton.h"
#include "mouseButton.h"
#include "throw_event.h"

#ifdef DO_PSTATS
#include "pStatTimer.h"
#endif

#include <ddraw.h>
#include <map>

TypeHandle wdxGraphicsWindow8::_type_handle;

#define LAST_ERROR 0
#define ERRORBOX_TITLE "Panda3D Error"
#define WDX_WINDOWCLASSNAME "wdxDisplay"
#define WDX_WINDOWCLASSNAME_NOCURSOR WDX_WINDOWCLASSNAME "_NoCursor"
#define DEFAULT_CURSOR IDC_ARROW


// define this to enable debug testing of dinput joystick
//#define DINPUT_DEBUG_POLL

typedef map<HWND,wdxGraphicsWindow8 *> HWND_PANDAWIN_MAP;

HWND_PANDAWIN_MAP hwnd_pandawin_map;
wdxGraphicsWindow8* global_wdxwinptr = NULL;  // need this for temporary windproc

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

// pops up MsgBox w/system error msg
#define LAST_ERROR 0
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
  wdxdisplay8_cat.fatal() << "System error msg: " << pMessageBuffer << endl;
  LocalFree( pMessageBuffer );
}

void ClearToBlack(HWND hWnd, const WindowProperties &props) {
  // clear to black
  HDC hDC=GetDC(hWnd);  // GetDC is not particularly fast.  if this needs to be super-quick, we should cache GetDC's hDC
  RECT clrRect = {
    props.get_x_origin(), props.get_y_origin(),
    props.get_x_origin() + props.get_x_size(),
    props.get_y_origin() + props.get_y_size()
  };
  FillRect(hDC,&clrRect,(HBRUSH)GetStockObject(BLACK_BRUSH));
  ReleaseDC(hWnd,hDC);
  GdiFlush();
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
wdxGraphicsWindow8::
wdxGraphicsWindow8(GraphicsPipe *pipe) :
  WinGraphicsWindow(pipe) 
{
  _dxgsg = (DXGraphicsStateGuardian8 *)NULL;
  _depth_buffer_bpp = 0;
  _awaiting_restore = false;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
wdxGraphicsWindow8::
~wdxGraphicsWindow8() {
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::make_gsg
//       Access: Public, Virtual
//  Description: Creates a new GSG for the window and stores it in the
//               _gsg pointer.  This should only be called from within
//               the draw thread.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow8::
make_gsg() {
  wdxGraphicsPipe8 *dxpipe;
  DCAST_INTO_V(dxpipe, _pipe);

  nassertv(_gsg == (GraphicsStateGuardian *)NULL);
  _dxgsg = new DXGraphicsStateGuardian8(this);
  _gsg = _dxgsg;

  // Tell the associated dxGSG about the window handle.
  _dxgsg->scrn.hWnd = _mwindow;

  // Create a Direct3D object.
  LPDIRECT3D8 pD3D8;

  // these were taken from the 8.0 and 8.1 d3d8.h SDK headers
#define D3D_SDK_VERSION_8_0  120
#define D3D_SDK_VERSION_8_1  220

  // are we using 8.0 or 8.1?
  WIN32_FIND_DATA TempFindData;
  HANDLE hFind;
  char tmppath[MAX_PATH + 128];
  GetSystemDirectory(tmppath, MAX_PATH);
  strcat(tmppath, "\\dpnhpast.dll");
  hFind = FindFirstFile (tmppath, &TempFindData);
  if (hFind != INVALID_HANDLE_VALUE) {
    FindClose(hFind);
    _dxgsg->scrn.bIsDX81 = true;
    pD3D8 = (*dxpipe->_Direct3DCreate8)(D3D_SDK_VERSION_8_1);
  } else {
    _dxgsg->scrn.bIsDX81 = false;
    pD3D8 = (*dxpipe->_Direct3DCreate8)(D3D_SDK_VERSION_8_0);
  }

  if (pD3D8 == NULL) {
    wdxdisplay8_cat.error()
      << "Direct3DCreate8 failed!\n";
    release_gsg();
    return;
  }

  if (!choose_adapter(pD3D8)) {
    wdxdisplay8_cat.error()
      << "Unable to find suitable rendering device.\n";
    release_gsg();
    return;
  }

  create_screen_buffers_and_device(_dxgsg->scrn, dx_force_16bpp_zbuffer);
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::release_gsg
//       Access: Public, Virtual
//  Description: Releases the current GSG pointer, if it is currently
//               held, and resets the GSG to NULL.  This should only
//               be called from within the draw thread.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow8::
release_gsg() {
  if (_gsg != (GraphicsStateGuardian *)NULL) {
    GraphicsWindow::release_gsg();
  }
} 

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::verify_window_sizes
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
int wdxGraphicsWindow8::
verify_window_sizes(int numsizes, int *dimen) {
  // unfortunately this only works AFTER you make the window
  // initially, so its really mostly useful for resizes only
  assert(IS_VALID_PTR(_dxgsg));

  int num_valid_modes = 0;

  // not requesting same refresh rate since changing res might not
  // support same refresh rate at new size

  int *pCurDim = dimen;

  for (int i=0; i < numsizes; i++, pCurDim += 2) {
    int x_size = pCurDim[0];
    int y_size = pCurDim[1];

    bool bIsGoodMode = false;
    bool CouldntFindAnyValidZBuf;
    D3DFORMAT newPixFmt = D3DFMT_UNKNOWN;

    if (special_check_fullscreen_resolution(x_size, y_size)) {
      // bypass the test below for certain cards we know have valid modes
      bIsGoodMode=true;

    } else {
      if (_dxgsg->scrn.bIsLowVidMemCard) {
        bIsGoodMode = ((x_size == 640) && (y_size == 480));
      } else  {
        search_for_valid_displaymode(x_size, y_size, _dxgsg->scrn.PresParams.EnableAutoDepthStencil != false,
                                     IS_STENCIL_FORMAT(_dxgsg->scrn.PresParams.AutoDepthStencilFormat),
                                     &_dxgsg->scrn.SupportedScreenDepthsMask,
                                     &CouldntFindAnyValidZBuf, &newPixFmt);
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

    if (wdxdisplay8_cat.is_spam()) {
      wdxdisplay8_cat.spam()
        << "Fullscrn Mode (" << x_size << "," << y_size << ")\t" 
        << (bIsGoodMode ? "V" : "Inv") <<"alid\n";
    }
  }

  return num_valid_modes;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::begin_frame
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               before beginning rendering for a given frame.  It
//               should do whatever setup is required, and return true
//               if the frame should be rendered, or false if it
//               should be skipped.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow8::
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

  return WinGraphicsWindow::begin_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::end_flip
//       Access: Public, Virtual
//  Description: This function will be called within the draw thread
//               after begin_flip() has been called on all windows, to
//               finish the exchange of the front and back buffers.
//
//               This should cause the window to wait for the flip, if
//               necessary.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow8::
end_flip() {
  if (_dxgsg != (DXGraphicsStateGuardian8 *)NULL && is_active()) {
    _dxgsg->show_frame();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::fullscreen_restored
//       Access: Protected, Virtual
//  Description: This is a hook for derived classes to do something
//               special, if necessary, when a fullscreen window has
//               been restored after being minimized.  The given
//               WindowProperties struct will be applied to this
//               window's properties after this function returns.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow8::
fullscreen_restored(WindowProperties &properties) {
  // In DX8, unlike DX7, for some reason we can't immediately start
  // rendering as soon as the window is restored, even though
  // BeginScene() says we can.  Instead, we have to wait until
  // TestCooperativeLevel() lets us in.  We need to set a flag so we
  // can handle this special case in begin_frame().
  if (_dxgsg != NULL) {
    _awaiting_restore = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::handle_reshape
//       Access: Protected, Virtual
//  Description: Called in the window thread when the window size or
//               location is changed, this updates the properties
//               structure accordingly.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow8::
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
      if (wdxdisplay8_cat.is_debug()) {
        wdxdisplay8_cat.debug()
          << "windowed_resize to size: (" << x_size << "," << y_size
          << ") failed due to out-of-memory\n";
      } else {
        if (wdxdisplay8_cat.is_debug()) {
          int x_origin = props.get_x_origin();
          int y_origin = props.get_y_origin();
          wdxdisplay8_cat.debug()
            << "windowed_resize to origin: (" << x_origin << ","
            << y_origin << "), size: (" << x_size
            << "," << y_size << ")\n";
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::do_fullscreen_resize
//       Access: Protected, Virtual
//  Description: Called in the window thread to resize a fullscreen
//               window.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow8::
do_fullscreen_resize(int x_size, int y_size) {
  bool bCouldntFindValidZBuf;
  D3DFORMAT pixFmt;
  bool bNeedZBuffer = (_dxgsg->scrn.PresParams.EnableAutoDepthStencil!=false);
  bool bNeedStencilBuffer = IS_STENCIL_FORMAT(_dxgsg->scrn.PresParams.AutoDepthStencilFormat);

  bool bIsGoodMode=false;

  if (!special_check_fullscreen_resolution(x_size,y_size)) {
    // bypass the lowvidmem test below for certain "lowmem" cards we know have valid modes

    // wdxdisplay8_cat.info() << "1111111 lowvidmemcard="<< _dxgsg->scrn.bIsLowVidMemCard << endl;
    if (_dxgsg->scrn.bIsLowVidMemCard && (!((x_size==640) && (y_size==480)))) {
      wdxdisplay8_cat.error() << "resize() failed: will not try to resize low vidmem device #" << _dxgsg->scrn.CardIDNum << " to non-640x480!\n";
      goto Error_Return;
    }
  }

  // must ALWAYS use search_for_valid_displaymode even if we know
  // a-priori that res is valid so we can get a valid pixfmt
  search_for_valid_displaymode(x_size, y_size, 
                               bNeedZBuffer, bNeedStencilBuffer,
                               &_dxgsg->scrn.SupportedScreenDepthsMask,
                               &bCouldntFindValidZBuf,
                               &pixFmt);
  bIsGoodMode=(pixFmt!=D3DFMT_UNKNOWN);

  if (!bIsGoodMode) {
    wdxdisplay8_cat.error() << "resize() failed: "
                           << (bCouldntFindValidZBuf ? "Couldnt find valid zbuffer format to go with FullScreen mode" : "No supported FullScreen modes")
                           << " at " << x_size << "x" << y_size << " for device #" << _dxgsg->scrn.CardIDNum <<endl;
    goto Error_Return;
  }

  // reset_device_resize_window handles both windowed & fullscrn,
  // so need to set new displaymode manually here
  _dxgsg->scrn.DisplayMode.Width=x_size;
  _dxgsg->scrn.DisplayMode.Height=y_size;
  _dxgsg->scrn.DisplayMode.Format = pixFmt;
  _dxgsg->scrn.DisplayMode.RefreshRate = D3DPRESENT_RATE_DEFAULT;

  _dxgsg->scrn.PresParams.BackBufferFormat = pixFmt;   // make reset_device_resize use presparams or displaymode??

  bool bResizeSucceeded = reset_device_resize_window(x_size, y_size);

  if (!bResizeSucceeded) {
    wdxdisplay8_cat.error() << "resize() failed with OUT-OF-MEMORY error!\n";

    if ((!IS_16BPP_DISPLAY_FORMAT(_dxgsg->scrn.PresParams.BackBufferFormat)) &&
       (_dxgsg->scrn.SupportedScreenDepthsMask & (R5G6B5_FLAG|X1R5G5B5_FLAG))) {
      // fallback strategy, if we trying >16bpp, fallback to 16bpp buffers
      _dxgsg->scrn.DisplayMode.Format = ((_dxgsg->scrn.SupportedScreenDepthsMask & R5G6B5_FLAG) ? D3DFMT_R5G6B5 : D3DFMT_X1R5G5B5);
      dx_force_16bpp_zbuffer=true;
      if (wdxdisplay8_cat.info())
        wdxdisplay8_cat.info() << "CreateDevice failed with out-of-vidmem, retrying w/16bpp buffers on device #"<< _dxgsg->scrn.CardIDNum << endl;

      bResizeSucceeded= reset_device_resize_window(x_size, y_size);  // create the new resized rendertargets
    }
  }

 Error_Return:

  if (wdxdisplay8_cat.is_debug())
    wdxdisplay8_cat.debug() << "fullscrn resize("<<x_size<<","<<y_size<<") " << (bResizeSucceeded ? "succeeds\n" : "fails\n");

  return bResizeSucceeded;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::create_screen_buffers_and_device
//       Access: Private
//  Description: Called whenever the window is resized, this recreates
//               the necessary buffers for rendering.
//
//               Sets _depth_buffer_bpp appropriately.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow8::
create_screen_buffers_and_device(DXScreenData &Display, bool force_16bpp_zbuffer) {
  // only want this to apply to initial startup
  dx_pick_best_screenres = false;

  DWORD dwRenderWidth=Display.DisplayMode.Width;
  DWORD dwRenderHeight=Display.DisplayMode.Height;
  LPDIRECT3D8 pD3D8=Display.pD3D8;
  D3DCAPS8 *pD3DCaps = &Display.d3dcaps;
  D3DPRESENT_PARAMETERS* pPresParams = &Display.PresParams;
  RECT view_rect;
  HRESULT hr;
  int framebuffer_mode = get_properties().get_framebuffer_mode();
  bool bWantStencil = ((framebuffer_mode & WindowProperties::FM_stencil) != 0);

  assert(pD3D8!=NULL);
  assert(pD3DCaps->DevCaps & D3DDEVCAPS_HWRASTERIZATION);

  pPresParams->BackBufferFormat = Display.DisplayMode.Format;  // dont need dest alpha, so just use adapter format

  if (dx_sync_video && !(pD3DCaps->Caps & D3DCAPS_READ_SCANLINE)) {
    wdxdisplay8_cat.info() << "HW doesnt support syncing to vertical refresh, ignoring dx_sync_video\n";
    dx_sync_video=false;
  }

  // verify the rendertarget fmt one last time
  if (FAILED(pD3D8->CheckDeviceFormat(Display.CardIDNum, D3DDEVTYPE_HAL, Display.DisplayMode.Format,D3DUSAGE_RENDERTARGET,
                                     D3DRTYPE_SURFACE, pPresParams->BackBufferFormat))) {
    wdxdisplay8_cat.error() << "device #"<<Display.CardIDNum<< " CheckDeviceFmt failed for surface fmt "<< D3DFormatStr(pPresParams->BackBufferFormat) << endl;
    goto Fallback_to_16bpp_buffers;
  }

  if (FAILED(pD3D8->CheckDeviceType(Display.CardIDNum,D3DDEVTYPE_HAL, Display.DisplayMode.Format,pPresParams->BackBufferFormat,
                                   is_fullscreen()))) {
    wdxdisplay8_cat.error() << "device #"<<Display.CardIDNum<< " CheckDeviceType failed for surface fmt "<< D3DFormatStr(pPresParams->BackBufferFormat) << endl;
    goto Fallback_to_16bpp_buffers;
  }

  if (Display.PresParams.EnableAutoDepthStencil) {
    if (!find_best_depth_format(Display, Display.DisplayMode,
                               &Display.PresParams.AutoDepthStencilFormat,
                               bWantStencil, false)) {
      wdxdisplay8_cat.error()
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
    hr = pD3D8->CheckDeviceMultiSampleType(Display.CardIDNum, D3DDEVTYPE_HAL, Display.DisplayMode.Format,
                                           is_fullscreen(), D3DMULTISAMPLE_TYPE(dx_multisample_antialiasing_level));
    if (FAILED(hr)) {
      wdxdisplay8_cat.fatal() << "device #"<<Display.CardIDNum<< " doesnt support multisample level "<<dx_multisample_antialiasing_level <<"surface fmt "<< D3DFormatStr(Display.DisplayMode.Format) <<endl;
      exit(1);
    }

    if (Display.PresParams.EnableAutoDepthStencil) {
      hr = pD3D8->CheckDeviceMultiSampleType(Display.CardIDNum, D3DDEVTYPE_HAL, Display.PresParams.AutoDepthStencilFormat,
                                             is_fullscreen(), D3DMULTISAMPLE_TYPE(dx_multisample_antialiasing_level));
      if (FAILED(hr)) {
        wdxdisplay8_cat.fatal() << "device #"<<Display.CardIDNum<< " doesnt support multisample level "<<dx_multisample_antialiasing_level <<"surface fmt "<< D3DFormatStr(Display.PresParams.AutoDepthStencilFormat) <<endl;
        exit(1);
      }
    }

    pPresParams->MultiSampleType = D3DMULTISAMPLE_TYPE(dx_multisample_antialiasing_level);

    if (wdxdisplay8_cat.is_info())
      wdxdisplay8_cat.info() << "device #"<<Display.CardIDNum<< " using multisample antialiasing level "<<dx_multisample_antialiasing_level <<endl;
  }

  pPresParams->BackBufferCount = 1;
  pPresParams->Flags = 0x0;
  pPresParams->hDeviceWindow = Display.hWnd;
  pPresParams->BackBufferWidth = Display.DisplayMode.Width;
  pPresParams->BackBufferHeight = Display.DisplayMode.Height;
  DWORD dwBehaviorFlags=0x0;

  if (_dxgsg->scrn.bIsTNLDevice) {
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
    wdxdisplay8_cat.warning() << "SetForegroundWindow() failed!\n";
  }

  if (is_fullscreen()) {
    pPresParams->SwapEffect = D3DSWAPEFFECT_DISCARD;  // we dont care about preserving contents of old frame
    pPresParams->FullScreen_PresentationInterval = (dx_sync_video ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE);
    pPresParams->FullScreen_RefreshRateInHz = Display.DisplayMode.RefreshRate;

#ifdef _DEBUG
    if (pPresParams->MultiSampleType != D3DMULTISAMPLE_NONE)
      assert(pPresParams->SwapEffect == D3DSWAPEFFECT_DISCARD);  // only valid effect for multisample
#endif

    ClearToBlack(Display.hWnd, get_properties());

    hr = pD3D8->CreateDevice(Display.CardIDNum, D3DDEVTYPE_HAL, _mwindow,
                             dwBehaviorFlags, pPresParams, &Display.pD3DDevice);

    if (FAILED(hr)) {
      wdxdisplay8_cat.fatal() << "D3D CreateDevice failed for device #" << Display.CardIDNum << ", " << D3DERRORSTRING(hr);

      if (hr == D3DERR_OUTOFVIDEOMEMORY)
        goto Fallback_to_16bpp_buffers;
    }

    SetRect(&view_rect, 0, 0, dwRenderWidth, dwRenderHeight);
  }   // end create full screen buffers

  else {          // CREATE WINDOWED BUFFERS

    if (!(pD3DCaps->Caps2 & D3DCAPS2_CANRENDERWINDOWED)) {
      wdxdisplay8_cat.fatal() << "the 3D HW cannot render windowed, exiting..." << endl;
      exit(1);
    }

    D3DDISPLAYMODE dispmode;
    hr = Display.pD3D8->GetAdapterDisplayMode(Display.CardIDNum, &dispmode);

    if (FAILED(hr)) {
      wdxdisplay8_cat.fatal() << "GetAdapterDisplayMode failed" << D3DERRORSTRING(hr);
      exit(1);
    }

    if (dispmode.Format == D3DFMT_P8) {
      wdxdisplay8_cat.fatal() << "Can't run windowed in an 8-bit or less display mode" << endl;
      exit(1);
    }

    pPresParams->FullScreen_PresentationInterval = 0;

    if (dx_multisample_antialiasing_level<2) {
      if (dx_sync_video) {
        pPresParams->SwapEffect = D3DSWAPEFFECT_COPY_VSYNC;
      } else {
        pPresParams->SwapEffect = D3DSWAPEFFECT_DISCARD;  //D3DSWAPEFFECT_COPY;  does this make any difference?
      }
    } else {
      pPresParams->SwapEffect = D3DSWAPEFFECT_DISCARD;
    }

    assert((dwRenderWidth==pPresParams->BackBufferWidth)&&(dwRenderHeight==pPresParams->BackBufferHeight));

    hr = pD3D8->CreateDevice(Display.CardIDNum, D3DDEVTYPE_HAL, _mwindow,
                             dwBehaviorFlags, pPresParams, &Display.pD3DDevice);

    if (FAILED(hr)) {
      wdxdisplay8_cat.fatal() << "D3D CreateDevice failed for device #" << Display.CardIDNum << D3DERRORSTRING(hr);
      exit(1);
    }
  }  // end create windowed buffers

  //  ========================================================

  PRINT_REFCNT(wdxdisplay,_dxgsg->scrn.pD3DDevice);

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

    if (wdxdisplay8_cat.info()) {
      wdxdisplay8_cat.info()
        << "CreateDevice failed with out-of-vidmem, retrying w/16bpp buffers on device #"
        << Display.CardIDNum << endl;
    }
    create_screen_buffers_and_device(Display, true);
    return;

  } else if (!((dwRenderWidth==640)&&(dwRenderHeight==480))) {
    if (wdxdisplay8_cat.info())
      wdxdisplay8_cat.info() << "CreateDevice failed w/out-of-vidmem, retrying at 640x480 w/16bpp buffers on device #"<< Display.CardIDNum << endl;
    // try final fallback to 640x480x16
    Display.DisplayMode.Width=640;
    Display.DisplayMode.Height=480;
    create_screen_buffers_and_device(Display, true);
    return;

  } else {
    wdxdisplay8_cat.fatal() 
      << "Can't create any screen buffers, bailing out.\n";
    exit(1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::choose_adapter
//       Access: Private
//  Description: Looks at the list of available graphics adapters and
//               chooses a suitable one for the window.
//
//               Returns true if successful, false on failure.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow8::
choose_adapter(LPDIRECT3D8 pD3D8) {
  HRESULT hr;

  int num_adapters = pD3D8->GetAdapterCount();
  DXDeviceInfoVec device_infos;

  for (int i = 0; i < num_adapters; i++) {
    D3DADAPTER_IDENTIFIER8 adapter_info;
    ZeroMemory(&adapter_info, sizeof(D3DADAPTER_IDENTIFIER8));
    hr = pD3D8->GetAdapterIdentifier(i, D3DENUM_NO_WHQL_LEVEL, &adapter_info);
    if (FAILED(hr)) {
      wdxdisplay8_cat.fatal()
        << "D3D GetAdapterID(" << i << ") failed: "
        << D3DERRORSTRING(hr) << endl;
      continue;
    }
     
    LARGE_INTEGER *DrvVer = &adapter_info.DriverVersion;

    wdxdisplay8_cat.info()
      << "D3D8 Adapter[" << i << "]: " << adapter_info.Description 
      << ", Driver: " << adapter_info.Driver << ", DriverVersion: ("
      << HIWORD(DrvVer->HighPart) << "." << LOWORD(DrvVer->HighPart) << "."
      << HIWORD(DrvVer->LowPart) << "." << LOWORD(DrvVer->LowPart)
      << ")\nVendorID: 0x" << (void*) adapter_info.VendorId 
      << " DeviceID: 0x" <<  (void*) adapter_info.DeviceId
      << " SubsysID: 0x" << (void*) adapter_info.SubSysId
      << " Revision: 0x" << (void*) adapter_info.Revision << endl;
    
    HMONITOR hMon = pD3D8->GetAdapterMonitor(i);
    if (hMon == NULL) {
      wdxdisplay8_cat.info()
        << "D3D8 Adapter[" << i << "]: seems to be disabled, skipping it\n";
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
    wdxdisplay8_cat.error()
      << "No available D3D8 devices found.\n";
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
      wdxdisplay8_cat.error()
        << "invalid 'dx-preferred-device-id', valid values are 0-" 
        << num_adapters - 1 << ", using default adapter instead.\n";
    } else {
      adapter_num = dx_preferred_device_id;
    }
  }

  return search_for_device(pD3D8, &device_infos[adapter_num]);
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::search_for_device
//       Access: Private
//  Description: Searches for a suitable hardware device for
//               rendering.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow8::
search_for_device(LPDIRECT3D8 pD3D8, DXDeviceInfo *device_info) {
  wdxGraphicsPipe8 *dxpipe;
  DCAST_INTO_R(dxpipe, _pipe, false);

  WindowProperties properties = get_properties();
  DWORD dwRenderWidth = properties.get_x_size();
  DWORD dwRenderHeight = properties.get_y_size();
  HRESULT hr;

  assert(_dxgsg != NULL);
  _dxgsg->scrn.pD3D8 = pD3D8;
  _dxgsg->scrn.CardIDNum = device_info->cardID;  // could this change by end?

  int framebuffer_mode = get_properties().get_framebuffer_mode();
  bool bWantStencil = ((framebuffer_mode & WindowProperties::FM_stencil) != 0);
  
  hr = pD3D8->GetAdapterIdentifier(device_info->cardID, D3DENUM_NO_WHQL_LEVEL,
                                   &_dxgsg->scrn.DXDeviceID);
  if (FAILED(hr)) {
    wdxdisplay8_cat.error()
      << "D3D GetAdapterID failed" << D3DERRORSTRING(hr);
    return false;
  }
  
  D3DCAPS8 d3dcaps;
  hr = pD3D8->GetDeviceCaps(device_info->cardID,D3DDEVTYPE_HAL,&d3dcaps);
  if (FAILED(hr)) {
    if ((hr==D3DERR_INVALIDDEVICE)||(hr==D3DERR_NOTAVAILABLE)) {
      wdxdisplay8_cat.error()
        << "No DirectX 8 D3D-capable 3D hardware detected for device # "
        << device_info->cardID << " (" <<device_info->szDescription 
        << ")!\n";
    } else {
      wdxdisplay8_cat.error()
        << "GetDeviceCaps failed: " << D3DERRORSTRING(hr) << endl;
    }
    return false;
  }
  
  //search_for_valid_displaymode needs these to be set
  memcpy(&_dxgsg->scrn.d3dcaps, &d3dcaps,sizeof(D3DCAPS8));
  _dxgsg->scrn.CardIDNum = device_info->cardID;
  
  _dxgsg->scrn.MaxAvailVidMem = UNKNOWN_VIDMEM_SIZE;
  _dxgsg->scrn.bIsLowVidMemCard = false;
  
  // bugbug: wouldnt we like to do GetAVailVidMem so we can do
  // upper-limit memory computation for dx8 cards too?  otherwise
  // verify_window_sizes cant do much
  if ((d3dcaps.MaxStreams==0) || dx_pick_best_screenres) {
    if (wdxdisplay8_cat.is_debug()) {
      wdxdisplay8_cat.debug()
        << "checking vidmem size\n";
    }
    //    assert(IS_VALID_PTR(_pParentWindowGroup));
    
    // look for low memory video cards
    //    _pParentWindowGroup->find_all_card_memavails();
    
    UINT IDnum;
    
    // simple linear search to match DX7 card info w/DX8 card ID
    for (IDnum=0; IDnum < dxpipe->_card_ids.size(); IDnum++) {
      //      wdxdisplay8_cat.info()
      //        << "comparing '" << dxpipe->_card_ids[IDnum].Driver
      //        << "' to '" << _dxgsg->scrn.DXDeviceID.Driver << "'\n";
      if (//(stricmp(dxpipe->_card_ids[IDnum].szDriver,device_info->szDriver)==0) &&
         (device_info->VendorID==dxpipe->_card_ids[IDnum].VendorID) &&
         (device_info->DeviceID==dxpipe->_card_ids[IDnum].DeviceID) &&
         (device_info->hMon==dxpipe->_card_ids[IDnum].hMon))
        break;
    }
    
    if (IDnum < dxpipe->_card_ids.size()) {
      _dxgsg->scrn.MaxAvailVidMem = dxpipe->_card_ids[IDnum].MaxAvailVidMem;
      _dxgsg->scrn.bIsLowVidMemCard = dxpipe->_card_ids[IDnum].bIsLowVidMemCard;
    } else {
      wdxdisplay8_cat.error()
        << "Error: couldnt find a CardID match in DX7 info, assuming card is not a lowmem card\n";
    }
  }

  if ((bWantStencil) && (d3dcaps.StencilCaps==0x0)) {
    wdxdisplay8_cat.fatal()
      << "Stencil ability requested, but device #" << device_info->cardID
      << " (" << _dxgsg->scrn.DXDeviceID.Description
      << "), has no stencil capability!\n";
    return false;
  }

  // just because TNL is true, it doesnt mean vtx shaders are
  // supported in HW (see GF2) for this case, you probably want MIXED
  // processing to use HW for fixed-fn vertex processing and SW for
  // vtx shaders
  _dxgsg->scrn.bIsTNLDevice = 
    ((d3dcaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0);
  _dxgsg->scrn.bCanUseHWVertexShaders = 
    (d3dcaps.VertexShaderVersion >= D3DVS_VERSION(1, 0));
  _dxgsg->scrn.bCanUsePixelShaders = 
    (d3dcaps.PixelShaderVersion >= D3DPS_VERSION(1, 0));

  bool bNeedZBuffer = 
    ((!(d3dcaps.RasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR )) &&
     ((framebuffer_mode & WindowProperties::FM_depth) != 0));

  _dxgsg->scrn.PresParams.EnableAutoDepthStencil = bNeedZBuffer;

  D3DFORMAT pixFmt = D3DFMT_UNKNOWN;

  if (is_fullscreen()) {
    bool bCouldntFindValidZBuf;
    if (!_dxgsg->scrn.bIsLowVidMemCard) {
      bool bUseDefaultSize = dx_pick_best_screenres &&
        ((_dxgsg->scrn.MaxAvailVidMem == UNKNOWN_VIDMEM_SIZE) ||
         is_badvidmem_card(&_dxgsg->scrn.DXDeviceID));

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
          if (_dxgsg->scrn.MaxAvailVidMem > MemRes[i].memlimit) {
            dwRenderWidth = MemRes[i].scrnX;
            dwRenderHeight = MemRes[i].scrnY;

            wdxdisplay8_cat.info()
              << "pick_best_screenres: trying " << dwRenderWidth 
              << "x" << dwRenderHeight << " based on "
              << _dxgsg->scrn.MaxAvailVidMem << " bytes avail\n";

            search_for_valid_displaymode(dwRenderWidth, dwRenderHeight, 
                                         bNeedZBuffer, bWantStencil,
                                         &_dxgsg->scrn.SupportedScreenDepthsMask,
                                         &bCouldntFindValidZBuf,
                                         &pixFmt);

            // note I'm not saving refresh rate, will just use adapter
            // default at given res for now

            if (pixFmt != D3DFMT_UNKNOWN) {
              break;
            }

            wdxdisplay8_cat.info()
              << "skipping scrnres; "
              << (bCouldntFindValidZBuf ? "Couldnt find valid zbuffer format to go with FullScreen mode" : "No supported FullScreen modes")
              << " at " << dwRenderWidth << "x" << dwRenderHeight
              << " for device #" << _dxgsg->scrn.CardIDNum << endl;
          }
        }
        // otherwise just go with whatever was specified (we probably shouldve marked this card as lowmem if it gets to end of loop w/o breaking
      }

      if (pixFmt == D3DFMT_UNKNOWN) {
        if (bUseDefaultSize) {
          wdxdisplay8_cat.info()
            << "pick_best_screenres: defaulted 800x600 based on no reliable vidmem size\n";
          dwRenderWidth=800;
          dwRenderHeight=600;
        }

        search_for_valid_displaymode(dwRenderWidth, dwRenderHeight,
                                     bNeedZBuffer, bWantStencil,
                                     &_dxgsg->scrn.SupportedScreenDepthsMask,
                                     &bCouldntFindValidZBuf,
                                     &pixFmt);

        // note I'm not saving refresh rate, will just use adapter
        // default at given res for now

        if (pixFmt == D3DFMT_UNKNOWN) {
          wdxdisplay8_cat.error()
            << (bCouldntFindValidZBuf ? "Couldnt find valid zbuffer format to go with FullScreen mode" : "No supported FullScreen modes")
            << " at " << dwRenderWidth << "x" << dwRenderHeight << " for device #" << _dxgsg->scrn.CardIDNum <<endl;

          // run it again in verbose mode to get more dbg info to log
          search_for_valid_displaymode(dwRenderWidth, dwRenderHeight,
                                       bNeedZBuffer, bWantStencil,
                                       &_dxgsg->scrn.SupportedScreenDepthsMask,
                                       &bCouldntFindValidZBuf,
                                       &pixFmt, true);
          return false;
        }
      }
    } else {
      // Low Memory card
      dwRenderWidth=640;
      dwRenderHeight=480;
      dx_force_16bpptextures = true;

      // force 16bpp zbuf? or let user get extra bits if they have the mem?

      search_for_valid_displaymode(dwRenderWidth, dwRenderHeight,
                                   bNeedZBuffer, bWantStencil,
                                   &_dxgsg->scrn.SupportedScreenDepthsMask,
                                   &bCouldntFindValidZBuf,
                                   &pixFmt);

      // hack: figuring out exactly what res to use is tricky, instead I will
      // just use 640x480 if we have < 3 meg avail

      if (_dxgsg->scrn.SupportedScreenDepthsMask & R5G6B5_FLAG) {
        pixFmt = D3DFMT_R5G6B5;
      } else if (_dxgsg->scrn.SupportedScreenDepthsMask & X1R5G5B5_FLAG) {
        pixFmt = D3DFMT_X1R5G5B5;
      } else {
        wdxdisplay8_cat.fatal()
          << "Low Memory VidCard has no supported FullScreen 16bpp resolutions at "
          << dwRenderWidth << "x" << dwRenderHeight << " for device #"
          << device_info->cardID << " (" 
          << _dxgsg->scrn.DXDeviceID.Description << "), skipping device...\n";

        // run it again in verbose mode to get more dbg info to log
        search_for_valid_displaymode(dwRenderWidth, dwRenderHeight,
                                     bNeedZBuffer, bWantStencil,
                                     &_dxgsg->scrn.SupportedScreenDepthsMask,
                                     &bCouldntFindValidZBuf,
                                     &pixFmt, true);
        return false;
      }

      if (wdxdisplay8_cat.is_info()) {
        wdxdisplay8_cat.info()
          << "Available VidMem (" << _dxgsg->scrn.MaxAvailVidMem
          << ") is under threshold, using 640x480 16bpp rendertargets to save tex vidmem.\n";
      }
    }
  } else {
    // Windowed Mode

    D3DDISPLAYMODE dispmode;
    hr = pD3D8->GetAdapterDisplayMode(device_info->cardID,&dispmode);
    if (FAILED(hr)) {
      wdxdisplay8_cat.error()
        << "GetAdapterDisplayMode(" << device_info->cardID
        << ") failed" << D3DERRORSTRING(hr);
      return false;
    }
    pixFmt = dispmode.Format;
  }

  _dxgsg->scrn.DisplayMode.Width = dwRenderWidth;
  _dxgsg->scrn.DisplayMode.Height = dwRenderHeight;
  _dxgsg->scrn.DisplayMode.Format = pixFmt;
  _dxgsg->scrn.DisplayMode.RefreshRate = D3DPRESENT_RATE_DEFAULT;
  _dxgsg->scrn.hMon = device_info->hMon;

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
//     Function: wdxGraphicsWindow8::special_check_fullscreen_resolution
//       Access: Private
//  Description: overrides of the general estimator for known working
//               cases
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow8::
special_check_fullscreen_resolution(UINT x_size,UINT y_size) {
  assert(IS_VALID_PTR(_dxgsg));

  DWORD VendorId = _dxgsg->scrn.DXDeviceID.VendorId;
  DWORD DeviceId = _dxgsg->scrn.DXDeviceID.DeviceId;
  switch (VendorId) {
  case 0x8086:  // Intel
    /*for now, just validate all the intel cards at these resolutions.
      I dont have a complete list of intel deviceIDs (missing 82830, 845, etc)
      // Intel i810,i815,82810
      if ((DeviceId==0x7121)||(DeviceId==0x7123)||(DeviceId==0x7125)||
      (DeviceId==0x1132))
    */
    if ((x_size == 640) && (y_size == 480)) {
      return true;
    }
    if ((x_size == 800) && (y_size == 600)) {
      return true;
    }
    if ((x_size == 1024) && (y_size == 768)) {
      return true;
    }
    break;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::search_for_valid_displaymode
//       Access: Private
//  Description: All ptr args are output parameters.  If no valid mode
//               found, returns *pSuggestedPixFmt = D3DFMT_UNKNOWN;
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow8::
search_for_valid_displaymode(UINT RequestedX_Size, UINT RequestedY_Size,
                             bool bWantZBuffer, bool bWantStencil,
                             UINT *pSupportedScreenDepthsMask,
                             bool *pCouldntFindAnyValidZBuf,
                             D3DFORMAT *pSuggestedPixFmt,
                             bool bVerboseMode) {
  assert(IS_VALID_PTR(_dxgsg));
  assert(IS_VALID_PTR(_dxgsg->scrn.pD3D8));
  HRESULT hr;

#ifndef NDEBUG
  //   no longer true, due to special_check_fullscreen_res, where lowvidmem cards are allowed higher resolutions
  //    if (_dxgsg->scrn.bIsLowVidMemCard)
  //        nassertv((RequestedX_Size==640)&&(RequestedY_Size==480));
#endif

  *pSuggestedPixFmt = D3DFMT_UNKNOWN;
  *pSupportedScreenDepthsMask = 0x0;
  *pCouldntFindAnyValidZBuf = false;

  int cNumModes = _dxgsg->scrn.pD3D8->GetAdapterModeCount(_dxgsg->scrn.CardIDNum);
  D3DDISPLAYMODE BestDispMode;
  ZeroMemory(&BestDispMode,sizeof(BestDispMode));

  if (bVerboseMode) {
    wdxdisplay8_cat.info()
      << "searching for valid display modes at res: ("
      << RequestedX_Size << "," << RequestedY_Size
      << "), TotalModes: " << cNumModes << endl;
  }

  // ignore memory based checks for min res 640x480.  some cards just
  // dont give accurate memavails.  (should I do the check anyway for
  // 640x480 32bpp?)
  bool bDoMemBasedChecks = 
    ((!((RequestedX_Size==640)&&(RequestedY_Size==480))) &&
     (_dxgsg->scrn.MaxAvailVidMem!=UNKNOWN_VIDMEM_SIZE) &&
     (!special_check_fullscreen_resolution(RequestedX_Size,RequestedY_Size)));

  if (bVerboseMode || wdxdisplay8_cat.is_spam()) {
    wdxdisplay8_cat.info()
      << "DoMemBasedChecks = " << bDoMemBasedChecks << endl;
  }

  for (int i=0; i < cNumModes; i++) {
    D3DDISPLAYMODE dispmode;
    hr = _dxgsg->scrn.pD3D8->EnumAdapterModes(_dxgsg->scrn.CardIDNum,i,&dispmode);
    if (FAILED(hr)) {
      wdxdisplay8_cat.error()
        << "EnumAdapterDisplayMode failed for device #"
        << _dxgsg->scrn.CardIDNum << D3DERRORSTRING(hr);
      exit(1);
    }

    if ((dispmode.Width!=RequestedX_Size) ||
        (dispmode.Height!=RequestedY_Size)) {
      continue;
    }

    if ((dispmode.RefreshRate<60) && (dispmode.RefreshRate>1)) {
      // dont want refresh rates under 60Hz, but 0 or 1 might indicate
      // a default refresh rate, which is usually >=60
      if (bVerboseMode) {
        wdxdisplay8_cat.info()
          << "skipping mode[" << i << "], bad refresh rate: " 
          << dispmode.RefreshRate << endl;
      }
      continue;
    }

    // Note no attempt is made to verify if format will work at
    // requested size, so even if this call succeeds, could still get
    // an out-of-video-mem error

    hr = _dxgsg->scrn.pD3D8->CheckDeviceFormat(_dxgsg->scrn.CardIDNum, 
                                               D3DDEVTYPE_HAL, dispmode.Format,
                                               D3DUSAGE_RENDERTARGET, 
                                               D3DRTYPE_SURFACE,
                                               dispmode.Format);
    if (FAILED(hr)) {
      if (hr==D3DERR_NOTAVAILABLE) {
        if (bVerboseMode) {
          wdxdisplay8_cat.info()
            << "skipping mode[" << i 
            << "], CheckDevFmt returns NotAvail for fmt: "
            << D3DFormatStr(dispmode.Format) << endl;
        }
        continue;
      } else {
        wdxdisplay8_cat.error()
          << "CheckDeviceFormat failed for device #" 
          << _dxgsg->scrn.CardIDNum << D3DERRORSTRING(hr);
        exit(1);
      }
    }

    bool bIs16bppRenderTgt = IS_16BPP_DISPLAY_FORMAT(dispmode.Format);
    float RendTgtMinMemReqmt;

    // if we have a valid memavail value, try to determine if we have
    // enough space
    if (bDoMemBasedChecks) {
      // assume user is testing fullscreen, not windowed, so use the
      // dwTotal value see if 3 scrnbufs (front/back/z)at 16bpp at
      // x_size*y_size will fit with a few extra megs for texmem

      // 8MB Rage Pro says it has 6.8 megs Total free and will run at
      // 1024x768, so formula makes it so that is OK

#define REQD_TEXMEM 1800000

      float bytes_per_pixel = (bIs16bppRenderTgt ? 2 : 4);
      assert((get_properties().get_framebuffer_mode() & WindowProperties::FM_double_buffer) != 0);

      // *2 for double buffer

      RendTgtMinMemReqmt = 
        ((float)RequestedX_Size) * ((float)RequestedY_Size) * 
        bytes_per_pixel * 2 + REQD_TEXMEM;

      if (bVerboseMode || wdxdisplay8_cat.is_spam())
        wdxdisplay8_cat.info()
          << "Testing Mode (" <<RequestedX_Size<<"x" << RequestedY_Size 
          << "," << D3DFormatStr(dispmode.Format) << ")\nReqdVidMem: "
          << (int)RendTgtMinMemReqmt << " AvailVidMem: " 
          << _dxgsg->scrn.MaxAvailVidMem << endl;

      if (RendTgtMinMemReqmt > _dxgsg->scrn.MaxAvailVidMem) {
        if (bVerboseMode || wdxdisplay8_cat.is_debug())
          wdxdisplay8_cat.info()
            << "not enough VidMem for render tgt, skipping display fmt "
            << D3DFormatStr(dispmode.Format) << " (" 
            << (int)RendTgtMinMemReqmt << " > " 
            << _dxgsg->scrn.MaxAvailVidMem << ")\n";
        continue;
      }
    }

    if (bWantZBuffer) {
      D3DFORMAT zformat;
      if (!find_best_depth_format(_dxgsg->scrn,dispmode, &zformat,
                                  bWantStencil, false)) {
        *pCouldntFindAnyValidZBuf=true;
        continue;
      }

      float MinMemReqmt = 0.0f;

      if (bDoMemBasedChecks) {
        // test memory again, this time including zbuf size
        float zbytes_per_pixel = (IS_16BPP_ZBUFFER(zformat) ? 2 : 4);
        float MinMemReqmt = RendTgtMinMemReqmt + ((float)RequestedX_Size)*((float)RequestedY_Size)*zbytes_per_pixel;

        if (bVerboseMode || wdxdisplay8_cat.is_spam())
          wdxdisplay8_cat.info()
            << "Testing Mode w/Z (" << RequestedX_Size << "x"
            << RequestedY_Size << "," << D3DFormatStr(dispmode.Format)
            << ")\nReqdVidMem: "<< (int)MinMemReqmt << " AvailVidMem: " 
            << _dxgsg->scrn.MaxAvailVidMem << endl;

        if (MinMemReqmt > _dxgsg->scrn.MaxAvailVidMem) {
          if (bVerboseMode || wdxdisplay8_cat.is_debug())
            wdxdisplay8_cat.info()
              << "not enough VidMem for RendTgt+zbuf, skipping display fmt "
              << D3DFormatStr(dispmode.Format) << " (" << (int)MinMemReqmt
              << " > " << _dxgsg->scrn.MaxAvailVidMem << ")\n";
          continue;
        }
      }

      if ((!bDoMemBasedChecks) || (MinMemReqmt<_dxgsg->scrn.MaxAvailVidMem)) {
        if (!IS_16BPP_ZBUFFER(zformat)) {
          // see if things fit with a 16bpp zbuffer

          if (!find_best_depth_format(_dxgsg->scrn, dispmode, &zformat, 
                                      bWantStencil, true, bVerboseMode)) {
            if (bVerboseMode)
              wdxdisplay8_cat.info()
                << "FindBestDepthFmt rejected Mode[" << i << "] (" 
                << RequestedX_Size << "x" << RequestedY_Size 
                << "," << D3DFormatStr(dispmode.Format) << endl;
            *pCouldntFindAnyValidZBuf=true;
            continue;
          }
          
          // right now I'm not going to use these flags, just let the
          // create fail out-of-mem and retry at 16bpp
          *pSupportedScreenDepthsMask |= 
            (IS_16BPP_DISPLAY_FORMAT(dispmode.Format) ? DISPLAY_16BPP_REQUIRES_16BPP_ZBUFFER_FLAG : DISPLAY_32BPP_REQUIRES_16BPP_ZBUFFER_FLAG);
        }
      }
    }

    if (bVerboseMode || wdxdisplay8_cat.is_spam())
      wdxdisplay8_cat.info()
        << "Validated Mode (" << RequestedX_Size << "x" 
        << RequestedY_Size << "," << D3DFormatStr(dispmode.Format) << endl;

    switch (dispmode.Format) {
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
      // Render target formats should be only D3DFMT_X1R5G5B5,
      // D3DFMT_R5G6B5, D3DFMT_X8R8G8B8 (or R8G8B8?)
      wdxdisplay8_cat.error()
        << "unrecognized supported fmt "<< D3DFormatStr(dispmode.Format)
        << " returned by EnumAdapterDisplayModes!\n";
    }
  }

  // note: this chooses 32bpp, which may not be preferred over 16 for
  // memory & speed reasons on some older cards in particular
  if (*pSupportedScreenDepthsMask & X8R8G8B8_FLAG) {
    *pSuggestedPixFmt = D3DFMT_X8R8G8B8;
  } else if (*pSupportedScreenDepthsMask & R8G8B8_FLAG) {
    *pSuggestedPixFmt = D3DFMT_R8G8B8;
  } else if (*pSupportedScreenDepthsMask & R5G6B5_FLAG) {
    *pSuggestedPixFmt = D3DFMT_R5G6B5;
  } else if (*pSupportedScreenDepthsMask & X1R5G5B5_FLAG) {
    *pSuggestedPixFmt = D3DFMT_X1R5G5B5;
  }

  if (bVerboseMode || wdxdisplay8_cat.is_spam()) {
    wdxdisplay8_cat.info() 
      << "search_for_valid_device returns fmt: "
      << D3DFormatStr(*pSuggestedPixFmt) << endl;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::reset_device_resize_window
//       Access: Private
//  Description: Called after a window (either fullscreen or windowed)
//               has been resized, this recreates the D3D structures
//               to match the new size.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow8::
reset_device_resize_window(UINT new_xsize, UINT new_ysize) {
  DXScreenData *pScrn = &_dxgsg->scrn;
  assert((new_xsize > 0) && (new_ysize > 0));
  bool bRetval = true;

  D3DPRESENT_PARAMETERS d3dpp;
  memcpy(&d3dpp, &pScrn->PresParams, sizeof(D3DPRESENT_PARAMETERS));
  d3dpp.BackBufferWidth = new_xsize;
  d3dpp.BackBufferHeight = new_ysize;
  HRESULT hr = _dxgsg->reset_d3d_device(&d3dpp);
  
  if (FAILED(hr)) {
    bRetval = false;
    wdxdisplay8_cat.error()
      << "reset_device_resize_window Reset() failed" << D3DERRORSTRING(hr);
    if (hr == D3DERR_OUTOFVIDEOMEMORY) {
      hr = _dxgsg->reset_d3d_device(&pScrn->PresParams);
      if (FAILED(hr)) {
        wdxdisplay8_cat.error()
          << "reset_device_resize_window Reset() failed OutOfVidmem, then failed again doing Reset w/original params:" << D3DERRORSTRING(hr);
        exit(1);
      } else {
        if (wdxdisplay8_cat.is_info())
          wdxdisplay8_cat.info()
            << "reset of original size (" << pScrn->PresParams.BackBufferWidth
            << "," << pScrn->PresParams.BackBufferHeight << ") succeeded\n";
      }
    } else {
      wdxdisplay8_cat.fatal() 
        << "Can't reset device, bailing out.\n";
      exit(1);
    }
  }
  
  init_resized_window();
  return bRetval;
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::init_resized_window
//       Access: Private
//  Description: Reinitializes the window after it has been resized,
//               or after it is first created.
//
//               Assumes CreateDevice or Device->Reset() has just been
//               called, and the new size is specified in
//               _dxgsg->scrn.PresParams.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow8::
init_resized_window() {
  DXScreenData *pDisplay=&_dxgsg->scrn;
  HRESULT hr;

  DWORD newWidth = pDisplay->PresParams.BackBufferWidth;
  DWORD newHeight = pDisplay->PresParams.BackBufferHeight;

  if (pDisplay->PresParams.Windowed) {
    POINT ul,lr;
    RECT client_rect;

    // need to figure out x,y origin offset of window client area on screen
    // (we already know the client area size)

    GetClientRect(pDisplay->hWnd, &client_rect);
    ul.x = client_rect.left;
    ul.y = client_rect.top;
    lr.x = client_rect.right;
    lr.y=client_rect.bottom;
    ClientToScreen(pDisplay->hWnd, &ul);
    ClientToScreen(pDisplay->hWnd, &lr);
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
  ClearToBlack(pDisplay->hWnd, get_properties());

  // clear textures and VB's out of video&AGP mem, so cache is reset
  hr = pDisplay->pD3DDevice->ResourceManagerDiscardBytes(0);
  if (FAILED(hr)) {
    wdxdisplay8_cat.error()
      << "ResourceManagerDiscardBytes failed for device #" 
      << pDisplay->CardIDNum << D3DERRORSTRING(hr);
  }

  _dxgsg->dx_init(_mouse_cursor);
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::find_best_depth_format
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow8::
find_best_depth_format(DXScreenData &Display, D3DDISPLAYMODE &TestDisplayMode,
                       D3DFORMAT *pBestFmt, bool bWantStencil,
                       bool bForce16bpp, bool bVerboseMode) const {
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
  bool bOnlySelect16bpp = (dx_force_16bpp_zbuffer || bForce16bpp ||
                           (IS_NVIDIA(Display.DXDeviceID) && IS_16BPP_DISPLAY_FORMAT(TestDisplayMode.Format)));

  if (bVerboseMode) {
    wdxdisplay8_cat.info()
      << "FindBestDepthFmt: bSelectOnly16bpp: " << bOnlySelect16bpp << endl;
  }

  for (int i=0; i < NUM_TEST_ZFMTS; i++) {
    D3DFORMAT TestDepthFmt = 
      (bWantStencil ? StencilPrefList[i] : NoStencilPrefList[i]);

    if (bOnlySelect16bpp && !IS_16BPP_ZBUFFER(TestDepthFmt)) {
      continue;
    }

    hr = Display.pD3D8->CheckDeviceFormat(Display.CardIDNum,
                                          D3DDEVTYPE_HAL,
                                          TestDisplayMode.Format,
                                          D3DUSAGE_DEPTHSTENCIL,
                                          D3DRTYPE_SURFACE,TestDepthFmt);

    if (FAILED(hr)) {
      if (hr == D3DERR_NOTAVAILABLE) {
        if (bVerboseMode)
          wdxdisplay8_cat.info() 
            << "FindBestDepthFmt: ChkDevFmt returns NotAvail for " 
            << D3DFormatStr(TestDepthFmt) << endl;
        continue;
      }

      wdxdisplay8_cat.error()
        << "unexpected CheckDeviceFormat failure" << D3DERRORSTRING(hr) 
        << endl;
      exit(1);
    }

    hr = Display.pD3D8->CheckDepthStencilMatch(Display.CardIDNum,
                                               D3DDEVTYPE_HAL,
                                               TestDisplayMode.Format,   // adapter format
                                               TestDisplayMode.Format,   // backbuffer fmt  (should be the same in my apps)
                                               TestDepthFmt);
    if (SUCCEEDED(hr)) {
      *pBestFmt = TestDepthFmt;
      break;
    } else {
      if (hr==D3DERR_NOTAVAILABLE) {
        if (bVerboseMode) {
          wdxdisplay8_cat.info()
            << "FindBestDepthFmt: ChkDepMatch returns NotAvail for "
            << D3DFormatStr(TestDisplayMode.Format) << ", " 
            << D3DFormatStr(TestDepthFmt) << endl;
        }
      } else {
        wdxdisplay8_cat.error()
          << "unexpected CheckDepthStencilMatch failure for "
          << D3DFormatStr(TestDisplayMode.Format) << ", " 
          << D3DFormatStr(TestDepthFmt) << endl;
        exit(1);
      }
    }
  }

  if (bVerboseMode) {
    wdxdisplay8_cat.info()
      << "FindBestDepthFmt returns fmt " << D3DFormatStr(*pBestFmt) << endl;
  }

  return (*pBestFmt != D3DFMT_UNKNOWN);
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::D3DFMT_to_DepthBits
//       Access: Private, Static
//  Description: Returns the number of depth bits represented by the
//               indicated D3DFORMAT value.
////////////////////////////////////////////////////////////////////
int wdxGraphicsWindow8::
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
    wdxdisplay8_cat.debug()
      << "D3DFMT_DepthBits: unhandled D3DFMT!\n";
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: wdxGraphicsWindow8::is_badvidmem_card
//       Access: Private, Static
//  Description: Returns true if the indicated video adapter card is
//               known to report an inaccurate figure for available
//               video memory.
////////////////////////////////////////////////////////////////////
bool wdxGraphicsWindow8::
is_badvidmem_card(D3DADAPTER_IDENTIFIER8 *pDevID) {
  // dont trust Intel cards since they often use regular memory as vidmem
  if (pDevID->VendorId == 0x00008086) {
    return true;
  }

  return false;
}
