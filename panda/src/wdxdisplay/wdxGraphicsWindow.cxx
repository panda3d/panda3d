// Filename: wdxGraphicsWindow.cxx
// Created by:  mike (09Jan00)
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#ifndef STRICT
#define STRICT
#endif
#define D3D_OVERLOADS
//#define  INITGUID  dont need this if linking w/dxguid.lib
#include <d3d.h>
#include "wdxGraphicsWindow.h"
#include "wdxGraphicsPipe.h"
#include "config_wdxdisplay.h"

#include <keyboardButton.h>
#include <mouseButton.h>
#include <pStatTimer.h>
#include "dxGraphicsStateGuardian.h"

#include <errno.h>
#include <time.h>
#include <math.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle wdxGraphicsWindow::_type_handle;

// kluge
wdxGraphicsWindow* global_dxwin = NULL;

#define MOUSE_ENTERED 0
#define MOUSE_EXITED 1

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

#define MAX_DX_ZBUF_FMTS 20

static int cNumZBufFmts=0;

HRESULT CALLBACK EnumZBufFmtsCallback( LPDDPIXELFORMAT pddpf, VOID* param )  
{
    DDPIXELFORMAT *ZBufFmtsArr = (DDPIXELFORMAT *) param;
    assert(cNumZBufFmts < MAX_DX_ZBUF_FMTS);
    memcpy( &(ZBufFmtsArr[cNumZBufFmts]), pddpf, sizeof(DDPIXELFORMAT) );
    cNumZBufFmts++;
    return DDENUMRET_OK;
}

////////////////////////////////////////////////////////////////////
//     Function: lookup_key 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
ButtonHandle  wdxGraphicsWindow::
lookup_key(WPARAM wparam) const {
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
      return KeyboardButton::ascii_key((uchar)key);
    }
    break;
  }
  return ButtonHandle::none();
}

DXGraphicsStateGuardian *wdxGraphicsWindow::get_dxgsg(void) {
    return (_gsg==NULL) ? NULL : DCAST(DXGraphicsStateGuardian, _gsg);
}

void AtExitFn() {
#ifdef _DEBUG
  wdxdisplay_cat.debug() << "AtExitFn called\n";
  OutputDebugString("AtExitFn called\n");
#endif
  if(global_dxwin==NULL)
    return;
  if(global_dxwin->_hParentWindow!=NULL) {
      SetForegroundWindow(global_dxwin->_hParentWindow);
  }
  DXGraphicsStateGuardian* dxgsg = global_dxwin->get_dxgsg();
  if(dxgsg!=NULL)
      dxgsg->dx_cleanup();

//  global_dxwin->_dx_ready = false;
//  delete dxgsg; does panda do this itself?
}

////////////////////////////////////////////////////////////////////
//     Function: static_window_proc 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
LONG WINAPI wdxGraphicsWindow::static_window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  return global_dxwin->window_proc(hwnd, msg, wparam, lparam);
}

#ifdef _DEBUG
extern void dbgPrintVidMem(LPDIRECTDRAW7 pDD, LPDDSCAPS2 lpddsCaps,const char *pMsg) {
  DWORD dwTotal,dwFree;
  char tmpstr[100],tmpstr2[100];
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

  if( FAILED(  hr = pDD->GetAvailableVidMem(&ddsCaps,&dwTotal,&dwFree))) {
     wdxdisplay_cat.debug() << "wdxGraphicsWindow::GetAvailableVidMem failed : result = " << ConvD3DErrorToString(hr) << endl;
     exit(1);
  }
  sprintf(tmpstr,"%.4g",dwTotal/1000000.0);
  sprintf(tmpstr2,"%.4g",dwFree/1000000.0);
  wdxdisplay_cat.debug() << "AvailableVidMem before creating "<< pMsg << ",(megs) total: " << tmpstr << "  free:" << tmpstr2 <<endl;
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: window_proc 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
LONG wdxGraphicsWindow::
window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
  PAINTSTRUCT ps;
  POINT point;
  int button = -1;
  int x, y, width, height;
  typedef enum {NotAdjusting,MovingOrResizing,Resizing} WindowAdjustType;
  static WindowAdjustType bWindowAdjustingType = NotAdjusting;
  static HCURSOR hMouseCrossIcon = NULL;

  switch (msg) {
	case WM_CREATE:
       hMouseCrossIcon = LoadCursor(NULL, IDC_CROSS);
       SetCursor(hMouseCrossIcon);
//       atexit(AtExitFn);  _gsg ptr seems to be bogus in AtExitFn for esc-press if you do this
//       to enable this, need to delete call to AtExitFn from under GetMessage loop
       return 0;

    case WM_CLOSE: 
          PostQuitMessage(0);
          return 0;
        
    case WM_PAINT:
       BeginPaint(hwnd, &ps);

	   if (_dx_ready)
  		 show_frame();
       EndPaint(hwnd, &ps);
      return 0;
#if 0
      // shouldnt be necessary anymore
  case WM_SETCURSOR:
      SetCursor(hMouseCrossIcon);
      return 0;
#endif

    case WM_SYSCHAR:
    case WM_CHAR:
      return 0;    
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
		GetCursorPos(&point);
		ScreenToClient(hwnd, &point);
		handle_keypress(lookup_key(wparam), point.x, point.y);
      return 0;
    case WM_SYSKEYUP:
    case WM_KEYUP:
		GetCursorPos(&point);
		ScreenToClient(hwnd, &point);
		handle_keyrelease(lookup_key(wparam), point.x, point.y);
      return 0;

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
      if (x & 1 << 15) x -= (1 << 16);
      if (y & 1 << 15) y -= (1 << 16);
	  if (_dx_ready) handle_keypress(MouseButton::button(button), x, y);
      return 0;

    case WM_LBUTTONUP:
      button = 0;
    case WM_MBUTTONUP:
      if (button < 0)
	        button = 1;
    case WM_RBUTTONUP:
      if (button < 0)
			button = 2;
      ReleaseCapture();
	  x = LOWORD(lparam);
      y = HIWORD(lparam);
	  if (x & 1 << 15) x -= (1 << 16);
	  if (y & 1 << 15) y -= (1 << 16);
	  handle_keyrelease(MouseButton::button(button), x, y);
      return 0;

    case WM_MOUSEMOVE:
	  if (_dx_ready) 
		{
		x = LOWORD(lparam);
    	y = HIWORD(lparam);
		if (x & 1 << 15) x -= (1 << 16);
		if (y & 1 << 15) y -= (1 << 16);
		if (mouse_motion_enabled() 
		&& wparam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON))
			{
		    handle_mouse_motion(x, y);
			}
		else if (mouse_passive_motion_enabled() && 
		((wparam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)) == 0))
			{
		    handle_mouse_motion(x, y);
			}
		}
      return 0;

    case WM_MOVE:
	  if (_dx_ready) handle_window_move(LOWORD(lparam), HIWORD(lparam) );
      return 0;

  case WM_EXITSIZEMOVE: 
#ifdef _DEBUG
      wdxdisplay_cat.spam()  << "WM_EXITSIZEMOVE received"  << endl;
#endif

      if(bWindowAdjustingType==Resizing) {
    	  DXGraphicsStateGuardian* dxgsg = DCAST(DXGraphicsStateGuardian, _gsg);
          _dx_ready = false;
          dxgsg->SetDXStatus(false);      // disable rendering whilst we much with surfs

          // Want to change rendertarget size without destroying d3d device.  To save vid memory 
          // (and make resizing work on memory-starved 4MB cards), we need to construct
          // a temporary mini-sized render target for the d3d device (it cannot point to a 
          // NULL rendertarget) before creating the fully resized buffers.  The old
          // rendertargets will be freed when these temp targets are set, and that will give
          // us the memory to create the resized target

          LPDIRECTDRAWSURFACE7 pddsDummy = NULL,pddsDummyZ = NULL;
          HRESULT hr;
            
            DX_DECLARE_CLEAN( DDSURFACEDESC2, ddsd );

            if(dxgsg->GetBackBuffer()==NULL)  // bugbug why is this ever true??
                return DefWindowProc(hwnd, msg, wparam, lparam);

            dxgsg->GetBackBuffer()->GetSurfaceDesc(&ddsd);
            LPDIRECTDRAW7 pDD = dxgsg->GetDDInterface();

            ddsd.dwFlags &= ~DDSD_PITCH;
            ddsd.dwWidth  = 1; ddsd.dwHeight = 1;
            ddsd.ddsCaps.dwCaps &= ~(DDSCAPS_COMPLEX | DDSCAPS_FLIP | DDSCAPS_FRONTBUFFER | DDSCAPS_BACKBUFFER);

            PRINTVIDMEM(pDD,&ddsd.ddsCaps,"dummy backbuf");

            if( FAILED( hr = pDD->CreateSurface( &ddsd, &pddsDummy, NULL ) ) )
            {
                wdxdisplay_cat.fatal()
                << "wdxGraphicsWindow::Resize CreateSurface for temp backbuf failed : result = " << ConvD3DErrorToString(hr) << endl;
                exit(1);
            }

            DX_DECLARE_CLEAN( DDSURFACEDESC2, ddsdZ );
            dxgsg->GetZBuffer()->GetSurfaceDesc(&ddsdZ);
            ddsdZ.dwFlags &= ~DDSD_PITCH;
            ddsdZ.dwWidth  = 1;   ddsdZ.dwHeight = 1;

            PRINTVIDMEM(pDD,&ddsdZ.ddsCaps,"dummy zbuf");
            
            if( FAILED( hr = pDD->CreateSurface( &ddsdZ, &pddsDummyZ, NULL ) ) ) {
                wdxdisplay_cat.fatal() << "wdxGraphicsWindow::Resize CreateSurface for temp zbuf failed : result = " << ConvD3DErrorToString(hr) << endl;
                exit(1);
            }

            if( FAILED( hr = pddsDummy->AddAttachedSurface( pddsDummyZ ) ) ) {
                wdxdisplay_cat.fatal() << "wdxGraphicsWindow::Resize AddAttachedSurf for temp zbuf failed : result = " << ConvD3DErrorToString(hr) << endl;
                exit(1);
            }
    
            if( FAILED( hr = dxgsg->GetD3DDevice()->SetRenderTarget( pddsDummy, 0x0 )) )
            {
                wdxdisplay_cat.fatal()
                << "wdxGraphicsWindow::Resize failed to set render target to temporary surface, result = " << ConvD3DErrorToString(hr) << endl;
                exit(1);
            }
            RELEASE(pddsDummyZ);
            RELEASE(pddsDummy);            

            RECT view_rect;
            GetClientRect( _mwindow, &view_rect );
            ClientToScreen( _mwindow, (POINT*)&view_rect.left );
            ClientToScreen( _mwindow, (POINT*)&view_rect.right );

            dxgsg->dx_setup_after_resize(view_rect,_mwindow);  // create the new resized rendertargets

            _dx_ready = true;
            dxgsg->SetDXStatus(true);      
      }
      bWindowAdjustingType = NotAdjusting;
      return 0;

    case WM_ENTERSIZEMOVE: {
    	  DXGraphicsStateGuardian* dxgsg = DCAST(DXGraphicsStateGuardian, _gsg);
          _dx_ready = true;
          dxgsg->SetDXStatus(true);   // dont disable here because I want to see pic as I resize
          bWindowAdjustingType = MovingOrResizing;
      }
      return DefWindowProc(hwnd, msg, wparam, lparam);

    case WM_DISPLAYCHANGE:
        if(!dx_full_screen) {
            wdxdisplay_cat.fatal() << "WM_DISPLAYCHANGE received:  fatal error, desktop bitdepth change not handled\n";
            exit(1);
        }

    case WM_SIZE: {
        if(_mwindow==NULL)
           return DefWindowProc(hwnd, msg, wparam, lparam);

		width = LOWORD(lparam);  height = HIWORD(lparam);
#ifdef _DEBUG
        wdxdisplay_cat.spam() << "WM_SIZE received with width:" << width << "  height: " << height << endl;
#endif
        if (_props._xsize != width || _props._ysize != height) {
        	DXGraphicsStateGuardian* dxgsg = DCAST(DXGraphicsStateGuardian, _gsg);
            bWindowAdjustingType = Resizing;
            assert(_mwindow!=NULL);
        
        	RECT view_rect;
        	GetClientRect( _mwindow, &view_rect );
        	ClientToScreen( _mwindow, (POINT*)&view_rect.left );
        	ClientToScreen( _mwindow, (POINT*)&view_rect.right );
        	assert(width  == (view_rect.right - view_rect.left));
        	assert(height == (view_rect.bottom - view_rect.top));
        
            resized(width, height);         // change _props xsize,ysize
        	_props._xorg = view_rect.left;	// _props should reflect view rectangle
        	_props._yorg = view_rect.top;
        }
            
      return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    case WM_SETFOCUS:
	  if (_dx_ready && _mouse_entry_enabled) 
		  handle_mouse_entry(MOUSE_ENTERED,hMouseCrossIcon);
      return 0;

#if 0
    case WM_WINDOWPOSCHANGING: {
       LPWINDOWPOS pWindPos = (LPWINDOWPOS) lparam;
       wdxdisplay_cat.spam() << "WM_WINDOWPOSCHANGING received, flags 0x" << pWindPos->flags <<  endl;
       return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    case WM_GETMINMAXINFO:
       wdxdisplay_cat.spam() << "WM_GETMINMAXINFO received\n" <<  endl;
       return DefWindowProc(hwnd, msg, wparam, lparam);
#endif

    case WM_ERASEBKGND:
       if(bWindowAdjustingType)
        return DefWindowProc(hwnd, msg, wparam, lparam);
       else return 0;  // dont let GDI waste time redrawing the deflt background

    case WM_KILLFOCUS:
	  if (_dx_ready && _mouse_entry_enabled) 
          handle_mouse_entry(MOUSE_EXITED,hMouseCrossIcon);
      return 0;

    default:
      return DefWindowProc(hwnd, msg, wparam, lparam);
  }
//      IDirectDraw7::TestCooperativeLevel 

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
//     Function: Destructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
wdxGraphicsWindow::~wdxGraphicsWindow(void) {
#ifdef WBD_GL_MODE
  free(_visual);
#endif //WBD_GL_MODE
}



#ifdef WBD_GL_MODE
////////////////////////////////////////////////////////////////////
//     Function: get_config 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::
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
PIXELFORMATDESCRIPTOR* wdxGraphicsWindow::
try_for_visual(/*wdxGraphicsPipe *pipe,*/ int mask,
	       int want_depth_bits, int want_color_bits) {
  static const int max_attrib_list = 32;
  int attrib_list[max_attrib_list];
  int n=0;

  wdxdisplay_cat.debug()
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
    wdxdisplay_cat.debug(false) << " ALPHA";
    attrib_list[n++] = GLX_ALPHA_SIZE;
    attrib_list[n++] = want_color_component_bits;
  }
  if (mask & W_DOUBLE) {
    wdxdisplay_cat.debug(false) << " DOUBLEBUFFER";
    attrib_list[n++] = GLX_DOUBLEBUFFER;
  }
  if (mask & W_STEREO) {
    wdxdisplay_cat.debug(false) << " STEREO";
    attrib_list[n++] = GLX_STEREO;
  }
  if (mask & W_DEPTH) {
    wdxdisplay_cat.debug(false) << " DEPTH(" << want_depth_bits << ")";
    attrib_list[n++] = GLX_DEPTH_SIZE;
    attrib_list[n++] = want_depth_bits;
  }
  if (mask & W_STENCIL) {
    wdxdisplay_cat.debug(false) << " STENCIL";
    attrib_list[n++] = GLX_STENCIL_SIZE;
    attrib_list[n++] = 1;
  }
  if (mask & W_ACCUM) {
    wdxdisplay_cat.debug(false) << " ACCUM";
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
	wdxdisplay_cat.info()
	  << "wdxGraphicsWindow::try_for_visual() - request for "
		<< "stereo failed" << endl;	
      }
    }
  }

  return match;
}


////////////////////////////////////////////////////////////////////
//     Function: choose visual
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::choose_visual(void) {
  // don't need pipe... wdxGraphicsPipe* pipe = DCAST(wdxGraphicsPipe, _pipe);

  int mask = _props._mask;
  int want_depth_bits = _props._want_depth_bits;
  int want_color_bits = _props._want_color_bits;

  if (mask & W_MULTISAMPLE) {
    wdxdisplay_cat.info()
      << "wdxGraphicsWindow::choose_visual() - multisample not supported"
	<< endl;
    mask &= ~W_MULTISAMPLE;
  }

  _visual = try_for_visual(/*pipe,*/ mask, want_depth_bits, want_color_bits);

  // This is the severity level at which we'll report the details of
  // the visual we actually do find.  Normally, it's debug-level
  // information: we don't care about that much detail.
  NotifySeverity show_visual_severity = NS_debug;

  if (_visual == NULL) {
    wdxdisplay_cat.info()
      << "wdxGraphicsWindow::choose_visual() - visual with requested\n"
      << "   capabilities not found; trying for lesser visual.\n";

    // If we're unable to get the visual we asked for, however, we
    // probably *do* care to know the details about what we actually
    // got, even if we don't have debug mode set.  So we'll report the
    // visual at a higher level.
    show_visual_severity = NS_info;

    bool special_size_request =
      (want_depth_bits != 1 || want_color_bits != 1);
    
    // We try to be smart about choosing a close match for the visual.
    // First, we'll eliminate some of the more esoteric options one at
    // a time, then two at a time, and finally we'll try just the bare
    // minimum.

    if (special_size_request) {
      // Actually, first we'll eliminate all of the minimum sizes, to
      // try to open a window with all of the requested options, but
      // maybe not as many bits in some options as we'd like.
      _visual = try_for_visual(/*pipe,*/ mask);
    }

    if (_visual == NULL) {
      // Ok, not good enough.  Now try to eliminate options, but keep
      // as many bits as we asked for.
      
      // This array keeps the bitmasks of options that we pull out of
      // the requested mask, in order.
      
      static const int strip_properties[] = { 
	// One esoteric option removed.
	W_MULTISAMPLE,
	W_STENCIL,
	W_ACCUM,
	W_ALPHA,
	W_STEREO,
	
	// Two esoteric options removed.
	W_STENCIL | W_MULTISAMPLE,
	W_ACCUM | W_MULTISAMPLE,
	W_ALPHA | W_MULTISAMPLE,
	W_STEREO | W_MULTISAMPLE,
	W_STENCIL | W_ACCUM,
	W_ALPHA | W_STEREO,
	W_STENCIL | W_ACCUM | W_MULTISAMPLE,
	W_ALPHA | W_STEREO | W_MULTISAMPLE,
	
	// All esoteric options removed.
	W_STENCIL | W_ACCUM | W_ALPHA | W_STEREO | W_MULTISAMPLE,

	// All esoteric options, plus some we'd really really prefer,
	// removed.
	W_STENCIL | W_ACCUM | W_ALPHA | W_STEREO | W_MULTISAMPLE | W_DOUBLE,
	
	// A zero marks the end of the array.
	0
      };

      set<int> tried_masks;
      tried_masks.insert(mask);

      int i;
      for (i = 0; _visual == NULL && strip_properties[i] != 0; i++) {
	int new_mask = mask & ~strip_properties[i];
	if (tried_masks.insert(new_mask).second) {
	  _visual = try_for_visual(/*pipe,*/ new_mask, want_depth_bits,
				   want_color_bits);
	}
      }

      if (special_size_request) {
	tried_masks.clear();
	tried_masks.insert(mask);
	
	if (_visual == NULL) {
	  // Try once more, this time eliminating all of the size
	  // requests.
	  for (i = 0; _visual == NULL && strip_properties[i] != 0; i++) {
	    int new_mask = mask & ~strip_properties[i];
	    if (tried_masks.insert(new_mask).second) {
	      _visual = try_for_visual(/*pipe,*/ new_mask);
	    }
	  }
	}
      }
	
      if (_visual == NULL) {
	// Here's our last-ditch desparation attempt: give us any GLX
	// visual at all!
	_visual = try_for_visual(/*pipe,*/ 0);
      }
	
      if (_visual == NULL) {
	wdxdisplay_cat.fatal()
	  << "wdxGraphicsWindow::choose_visual() - could not get any "
	  	"visual." << endl;
	exit(1);
      }
    }
  }

  if (wdxdisplay_cat.is_on(show_visual_severity)) {
    int render_mode, double_buffer, stereo, red_size, green_size, blue_size,
      alpha_size, ared_size, agreen_size, ablue_size, aalpha_size,
      depth_size, stencil_size;
    
    get_config(_visual, GLX_RGBA, &render_mode);
    get_config(_visual, GLX_DOUBLEBUFFER, &double_buffer);
    get_config(_visual, GLX_STEREO, &stereo);
    get_config(_visual, GLX_RED_SIZE, &red_size);
    get_config(_visual, GLX_GREEN_SIZE, &green_size);
    get_config(_visual, GLX_BLUE_SIZE, &blue_size);
    get_config(_visual, GLX_ALPHA_SIZE, &alpha_size);
    get_config(_visual, GLX_ACCUM_RED_SIZE, &ared_size);
    get_config(_visual, GLX_ACCUM_GREEN_SIZE, &agreen_size);
    get_config(_visual, GLX_ACCUM_BLUE_SIZE, &ablue_size);
    get_config(_visual, GLX_ACCUM_ALPHA_SIZE, &aalpha_size);
    get_config(_visual, GLX_DEPTH_SIZE, &depth_size);
    get_config(_visual, GLX_STENCIL_SIZE, &stencil_size);

    wdxdisplay_cat.out(show_visual_severity)
      << "GLX Visual Info (# bits of each):" << endl
      << " RGBA: " << red_size << " " << green_size << " " << blue_size
      << " " << alpha_size << endl
      << " Accum RGBA: " << ared_size << " " << agreen_size << " "
      << ablue_size << " " << aalpha_size << endl
      << " Depth: " << depth_size << endl
      << " Stencil: " << stencil_size << endl
      << " DoubleBuffer? " << double_buffer << endl
      << " Stereo? " << stereo << endl;
  }
}


/*
#else		// DX Version of try_for_visual called from the Callback function to EnumDisplayModes
bool wdxGraphicsWindow::
try_for_visual(GraphicsWindow::Properties *props)
{
  static const int max_attrib_list = 32;
  int attrib_list[max_attrib_list];
  int n=0;

  wdxdisplay_cat.debug()
    << "Trying for visual with: RGB(" << want_color_bits << ")";

  int want_color_component_bits;
  if (mask & W_ALPHA) {
    want_color_component_bits = max(props->want_color_bits / 4, 1);
  } else {
    want_color_component_bits = max(props->want_color_bits / 3, 1);
  }
      
  attrib_list[n++] = GLX_RGBA;
  attrib_list[n++] = GLX_RED_SIZE;
  attrib_list[n++] = want_color_component_bits;
  attrib_list[n++] = GLX_GREEN_SIZE;
  attrib_list[n++] = want_color_component_bits;
  attrib_list[n++] = GLX_BLUE_SIZE;
  attrib_list[n++] = want_color_component_bits;

  if (mask & W_ALPHA) {
    wdxdisplay_cat.debug(false) << " ALPHA";
    attrib_list[n++] = GLX_ALPHA_SIZE;
    attrib_list[n++] = want_color_component_bits;
  }
  if (mask & W_DOUBLE) {
    wdxdisplay_cat.debug(false) << " DOUBLEBUFFER";
    attrib_list[n++] = GLX_DOUBLEBUFFER;
  }
  if (mask & W_STEREO) {
    wdxdisplay_cat.debug(false) << " STEREO";
    attrib_list[n++] = GLX_STEREO;
  }
  if (mask & W_DEPTH) {
    wdxdisplay_cat.debug(false) << " DEPTH(" << want_depth_bits << ")";
    attrib_list[n++] = GLX_DEPTH_SIZE;
    attrib_list[n++] = want_depth_bits;
  }
  if (mask & W_STENCIL) {
    wdxdisplay_cat.debug(false) << " STENCIL";
    attrib_list[n++] = GLX_STENCIL_SIZE;
    attrib_list[n++] = 1;
  }
  if (mask & W_ACCUM) {
    wdxdisplay_cat.debug(false) << " ACCUM";
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

  bool stereo = false;

  int *p = attrib_list;
  while (*p) {
    switch (*p) {
      case GLX_LEVEL:
			break;
      case GLX_RGBA:
			break;
      case GLX_DOUBLEBUFFER:
			break;
      case GLX_STEREO:
			stereo = true;
			break;
      case GLX_AUX_BUFFERS:
			break;
      case GLX_RED_SIZE:
			++p;
			break;
      case GLX_GREEN_SIZE:
			++p;
			break;
      case GLX_BLUE_SIZE:
			++p;
			break;
      case GLX_ALPHA_SIZE:
			++p;
			break;
      case GLX_DEPTH_SIZE:
			++p;
			break;
      case GLX_STENCIL_SIZE:
			++p;
			break;
      case GLX_ACCUM_RED_SIZE:
      case GLX_ACCUM_GREEN_SIZE:
      case GLX_ACCUM_BLUE_SIZE:
      case GLX_ACCUM_ALPHA_SIZE:
			++p;
			break;
    }
    ++p;
  }

  return TRUE;
}
*/
#endif	//WBD_GL_MODE



////////////////////////////////////////////////////////////////////
//     Function: config 
//       Access:
//  Description:  Set up Direct X
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::config(void) {
  GraphicsWindow::config();

  global_dxwin = this;		// kludge to get window proc working

  _dx_ready = false;
  _mwindow = NULL;
  _gsg = NULL;
  _hParentWindow = NULL;

  // Create a GSG to manage the graphics
  make_gsg();

  WNDCLASS wc;
  HINSTANCE hinstance = GetModuleHandle(NULL);

  // Clear before filling in window structure!
  ZeroMemory(&wc, sizeof(WNDCLASS));
  wc.style		= CS_HREDRAW | CS_VREDRAW;   //CS_OWNDC;
  wc.lpfnWndProc	= (WNDPROC) static_window_proc;
  wc.hInstance		= hinstance;
  wc.hIcon		= LoadIcon(hinstance, IDI_WINLOGO);
  wc.hCursor		= LoadCursor(NULL/*hinstance*/, IDC_CROSS);
  wc.hbrBackground	= (HBRUSH)GetStockObject(BLACK_BRUSH);
  wc.lpszMenuName	= NULL;
  wc.lpszClassName	= "wdxDisplay";

  if (!RegisterClass(&wc)) {
    wdxdisplay_cat.fatal() << "wdxGraphicsWindow:: could not register window class" << endl;
    exit(1);
  }

  wdxGraphicsPipe* pipe = DCAST(wdxGraphicsPipe, _pipe);

  int style;
  RECT win_rect;
  SetRect(&win_rect, _props._xorg,  _props._yorg, _props._xorg + _props._xsize, 
				 _props._yorg + _props._ysize);

  // rect now contains the coords for the entire window, not the client
  if (dx_full_screen) {
	  _mwindow = CreateWindow("wdxDisplay", _props._title.c_str(),
                              WS_POPUP, 0, 0, _props._xsize,_props._ysize,
                              NULL, NULL, hinstance, 0);
  } else  {
	  if (_props._border)
		    style = WS_OVERLAPPEDWINDOW; 
	  else
		    style = WS_POPUP | WS_MAXIMIZE;

	  AdjustWindowRect(&win_rect, style, FALSE);  //compute window size based on desired client area size

	  // make sure origin is on screen
	  if (win_rect.left < 0)  { win_rect.right -= win_rect.left; win_rect.left = 0; }
	  if (win_rect.top < 0)  { win_rect.bottom -= win_rect.top; win_rect.top = 0; }

	  _mwindow = CreateWindow("wdxDisplay", _props._title.c_str(),
		style, win_rect.left, win_rect.top, win_rect.right-win_rect.left, 
		win_rect.bottom-win_rect.top,
		NULL, NULL, hinstance, 0);
  }

  if (!_mwindow) {
    wdxdisplay_cat.fatal() << "wdxGraphicsWindow::config() - failed to create Mwindow" << endl;
    exit(1);
  }

  _hParentWindow=GetForegroundWindow();

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
  GraphicsWindowInputDevice device =
    GraphicsWindowInputDevice::pointer_and_keyboard("keyboard/mouse");
  _input_devices.push_back(device);

  ShowWindow(_mwindow, SW_SHOWNORMAL);
  ShowWindow(_mwindow, SW_SHOWNORMAL);  // call twice to override STARTUPINFO value, which may be set to hidden initially by emacs
//  UpdateWindow( _mwindow );
}

HRESULT CALLBACK EnumDevicesCallback(LPSTR pDeviceDescription, LPSTR pDeviceName, 
                                         LPD3DDEVICEDESC7 pD3DDeviceDesc,LPVOID pContext) {
    D3DDEVICEDESC7 *pd3ddevs = (D3DDEVICEDESC7 *)pContext;
#ifdef _DEBUG
  	wdxdisplay_cat.spam() << "wdxGraphicsWindow: Enumerating Device " << pDeviceName << " : " << pDeviceDescription << endl;
#endif

    // only saves hal and tnl devs

    if(IsEqualGUID(pD3DDeviceDesc->deviceGUID,IID_IDirect3DHALDevice)) {
        CopyMemory(pd3ddevs,pD3DDeviceDesc,sizeof(D3DDEVICEDESC7));
    } else if(IsEqualGUID(pD3DDeviceDesc->deviceGUID,IID_IDirect3DTnLHalDevice)) {
        CopyMemory(&pd3ddevs[1],pD3DDeviceDesc,sizeof(D3DDEVICEDESC7));
    } 
    return DDENUMRET_OK;
}

HRESULT WINAPI EnumDisplayModesCallBack(LPDDSURFACEDESC2 lpDDSurfaceDesc,LPVOID lpContext) {
    DWORD *pDDBDMask = (DWORD*)lpContext;

    *pDDBDMask |= BitDepth_2_DDBDMask(lpDDSurfaceDesc->ddpfPixelFormat.dwRGBBitCount);
    return DDENUMRET_OK;    
}

////////////////////////////////////////////////////////////////////
//     Function: dx_setup
//  Description: Set up the DirectX environment.  The size of the
//				 rendered area will be computed from the Client area
//				 of the window (if in windowed mode) and the _props
//				 structure will be set accordingly.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::
dx_setup() 
{
      LPDIRECTDRAWSURFACE7  pBackDDSurf;
      LPDIRECTDRAWSURFACE7  pZDDSurf;
      LPDIRECT3D7           pD3DI;
      LPDIRECT3DDEVICE7     pD3DDevice;
      LPDIRECTDRAWSURFACE7  pPrimaryDDSurf;
      LPDIRECTDRAW7		    pDD;
      RECT view_rect;
      int i;
      DX_DECLARE_CLEAN( DDSURFACEDESC2, SurfaceDesc );    

      // Check for DirectX 7 by looking for DirectDrawCreateEx
    
      HINSTANCE DDHinst = LoadLibrary( "DDRAW.DLL" );
      if( DDHinst == 0 ) {
      	   wdxdisplay_cat.fatal() << "wdxGraphicsWindow::config() - DDRAW.DLL doesn't exist!" << endl;
           exit(1);
      }
    
      if(NULL == GetProcAddress( DDHinst, "DirectDrawCreateEx" )) {
      	   wdxdisplay_cat.fatal() << "wdxGraphicsWindow::config() - Panda currently requires DirectX 7.0!" << endl;
           exit(1);
       }
    
      // Create the Direct Draw Object
      HRESULT hr = DirectDrawCreateEx(NULL, (void **)&pDD, IID_IDirectDraw7, NULL);
      if (hr != DD_OK) {
    	  wdxdisplay_cat.fatal()
          << "wdxGraphicsWindow::config() - DirectDrawCreateEx failed : result = " << ConvD3DErrorToString(hr) << endl;
    	  exit(1);
      }
    
      FreeLibrary(DDHinst);  //undo LoadLib above, decrement ddrawl.dll refcnt (after DDrawCreate, since dont want to unload/reload)
    

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
      if (hr != DD_OK) {
    	  wdxdisplay_cat.fatal()
          << "wdxGraphicsWindow::config() - QI for D3D failed : result = " << ConvD3DErrorToString(hr) << endl;
    	  exit(1);
      }
    
       // just look for HAL and TnL devices right now.  I dont think 
       // we have any interest in the sw rasts at this point
       
       D3DDEVICEDESC7 d3ddevs[2];  // put HAL in 0, TnL in 1
       ZeroMemory(d3ddevs,2*sizeof(D3DDEVICEDESC7));
    
       hr = pD3DI->EnumDevices(EnumDevicesCallback,d3ddevs);
       if (hr != DD_OK) {
        	  wdxdisplay_cat.fatal()
              << "wdxGraphicsWindow::config() - EnumDevices failed : result = " << ConvD3DErrorToString(hr) << endl;
        	  exit(1);
       }
    
       if(!(d3ddevs[0].dwDevCaps & D3DDEVCAPS_HWRASTERIZATION )) {
        	  wdxdisplay_cat.fatal() << "wdxGraphicsWindow:: No 3D HW present, exiting..." << endl;
        	  exit(1);
       }
       
       DX_DECLARE_CLEAN(DDCAPS,DDCaps);
       pDD->GetCaps(&DDCaps,NULL);
       if((!(DDCaps.dwCaps2 & DDCAPS2_CANRENDERWINDOWED)) && !dx_full_screen) {
        	  wdxdisplay_cat.fatal() << "wdxGraphicsWindow:: 3D HW cannot render windowed, exiting..." << endl;
        	  exit(1);
       }

      if (dx_full_screen) {
		DWORD dwRenderWidth  = _props._xsize;
		DWORD dwRenderHeight = _props._ysize;
		_props._xorg = _props._yorg = 0;
		SetRect(&view_rect, _props._xorg, _props._yorg, _props._xsize, _props._ysize);

        // CREATE FULL SCREEN BUFFERS
	    // Store the rectangle which contains the renderer

        DWORD dwSupportedBitDepthMask=0x0;  // uses DDBD_...
        DX_DECLARE_CLEAN(DDSURFACEDESC2,ddsd_search);
        ddsd_search.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
        ddsd_search.dwWidth=dwRenderWidth;  ddsd_search.dwHeight=dwRenderHeight;

        if(FAILED(hr= pDD->EnumDisplayModes(0x0,&ddsd_search,&dwSupportedBitDepthMask,EnumDisplayModesCallBack))) {
			wdxdisplay_cat.fatal()
		      << "wdxGraphicsWindow::EnumDisplayModes failed, result = " << ConvD3DErrorToString(hr) << endl;
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

        if( FAILED(  hr = pDD->GetAvailableVidMem(&ddsCaps,&dwTotal,&dwFree))) {
            wdxdisplay_cat.debug() << "wdxGraphicsWindow::GetAvailableVidMem failed : result = " << ConvD3DErrorToString(hr) << endl;
            exit(1);
        }

#ifdef _DEBUG 
        wdxdisplay_cat.debug() << "before FullScreen switch: GetAvailableVidMem returns Total: " << dwTotal/1000000.0 << "  Free: " << dwFree/1000000.0 << endl;
#endif

        // note: this chooses 32bpp, which may not be preferred over 16 for memory & speed reasons
        if((dwSupportedBitDepthMask & DDBD_32) && (d3ddevs[0].dwDeviceRenderBitDepth & DDBD_32)) {
            dwFullScreenBitDepth=32;              // go for 32bpp if its avail
        } else if((dwSupportedBitDepthMask & DDBD_24) && (d3ddevs[0].dwDeviceRenderBitDepth & DDBD_24)) {
            dwFullScreenBitDepth=24;              // go for 24bpp if its avail
        } else if((dwSupportedBitDepthMask & DDBD_16) && (d3ddevs[0].dwDeviceRenderBitDepth & DDBD_16)) {
            dwFullScreenBitDepth=16;              // do 16bpp
        } else {
             wdxdisplay_cat.fatal()
             << "wdxGraphicsWindow::config() - No Supported FullScreen resolutions at " << dwRenderWidth << "x" << dwRenderHeight << endl;
             exit(1);
        }

        #define THREE_MEG 3000000
        if(dwFree< THREE_MEG) {
            // we're going to need 800x600 or 640x480 at 16 bit to save enough tex vidmem
            dwFullScreenBitDepth=16;              // do 16bpp
            wdxdisplay_cat.debug() << "wdxGraphicsWindow:: using 16bpp rendertargets to save tex vidmem.  XXX BUGBUG: auto-reduce-scrn-res not implemented yet XXX\n";
            // BUGBUG:  need to insert code to reduce dimensions here.  need to modify
            // _props scrn dimension values as well
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
             wdxdisplay_cat.debug() << "wdxGraphicsWindow:: using 16bpp rendertargets to save tex vidmem\n";
             assert((dwSupportedBitDepthMask & DDBD_16) && (d3ddevs[0].dwDeviceRenderBitDepth & DDBD_16));  // probably a safe assumption
             rendertargetmem=dwRenderWidth*dwRenderHeight*(dwFullScreenBitDepth>>3);
             memleft = dwFree-rendertargetmem*2;

             // BUGBUG:  if we still cant reserve 2 megs of vidmem, need to auto-reduce the scrn res
             if(memleft < RESERVEDTEXVIDMEM) 
                 wdxdisplay_cat.debug() << "wdxGraphicsWindow:: XXXXXX WARNING: cant reserve 2MB of tex vidmem. only " << memleft << " bytes available. Need to rewrite wdxdisplay to try lower resolutions  XXXXXXXXXXXXXXXXXXXX\n";
        }
#endif

        if(FAILED(hr = pDD->SetCooperativeLevel(_mwindow, DDSCL_FPUSETUP | DDSCL_FULLSCREEN | DDSCL_EXCLUSIVE | DDSCL_ALLOWREBOOT ))) {
             wdxdisplay_cat.fatal()
             << "wdxGraphicsWindow::config() - SetCooperativeLevel failed : result = " << ConvD3DErrorToString(hr) << endl;
             exit(1);
        }

	    if( FAILED( hr = pDD->SetDisplayMode( dwRenderWidth, dwRenderHeight,
								dwFullScreenBitDepth, 0L, 0L ))) {
			wdxdisplay_cat.fatal() << "wdxGraphicsWindow::CreateFullscreenBuffers() - Can't set display mode : result = " << ConvD3DErrorToString(hr) << endl;
			exit(1);
        }

#ifdef _DEBUG
		wdxdisplay_cat.debug() << "wdxGraphicsWindow::setting displaymode to " << dwRenderWidth << "x" << dwRenderHeight << " at "<< dwFullScreenBitDepth  << "bpp" <<endl;
        if( FAILED(  hr = pDD->GetAvailableVidMem(&ddsCaps,&dwTotal,&dwFree))) {
            wdxdisplay_cat.debug() << "wdxGraphicsWindow::GetAvailableVidMem failed : result = " << ConvD3DErrorToString(hr) << endl;
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

        PRINTVIDMEM(pDD,&ddsd.ddsCaps,"initial primary & backbuf");

	    // Create the primary surface
	    if( FAILED( hr = pDD->CreateSurface( &ddsd, &pPrimaryDDSurf, NULL ) ) )
	    {
		  wdxdisplay_cat.fatal()
	      << "wdxGraphicsWindow::CreateFullscreenBuffers() - CreateSurface failed for primary surface: result = " << ConvD3DErrorToString(hr) << endl;
		  exit(1);
	    }

	    // Get the backbuffer, which was created along with the primary.
	    DDSCAPS2 ddscaps = { DDSCAPS_BACKBUFFER, 0, 0, 0 };
	    if( FAILED( hr = pPrimaryDDSurf->GetAttachedSurface( &ddscaps, &pBackDDSurf ) ) )
	    {
		  wdxdisplay_cat.fatal()
	      << "wdxGraphicsWindow::CreateFullscreenBuffers() - Can't get the backbuffer: result = " << ConvD3DErrorToString(hr) << endl;
		  exit(1);
	    }
	  }	// end create full screen buffers

	else  {			// CREATE WINDOWED BUFFERS
        if(FAILED(hr = pDD->GetDisplayMode( &SurfaceDesc ))) {
		  wdxdisplay_cat.fatal()
	      << "wdxGraphicsWindow::GetDisplayMode failed result = " << ConvD3DErrorToString(hr) << endl;
		  exit(1);
        }
        if( SurfaceDesc.ddpfPixelFormat.dwRGBBitCount <= 8 ) {
            wdxdisplay_cat.fatal()
              << "wdxGraphicsWindow::config() - Can't run windowed in an 8-bit or less display mode" << endl;
            exit(1);
        }

        if(!(BitDepth_2_DDBDMask(SurfaceDesc.ddpfPixelFormat.dwRGBBitCount) & d3ddevs[0].dwDeviceRenderBitDepth)) {
                wdxdisplay_cat.fatal() << "wdxGraphicsWindow:: - 3D Device doesnt support rendering at " << SurfaceDesc.ddpfPixelFormat.dwRGBBitCount << "bpp (current desktop bitdepth)" << endl;
                exit(1);
        }

        if(FAILED(hr = pDD->SetCooperativeLevel(_mwindow, DDSCL_FPUSETUP | DDSCL_NORMAL))) {
             wdxdisplay_cat.fatal()
             << "wdxGraphicsWindow::config() - SetCooperativeLevel failed : result = " << ConvD3DErrorToString(hr) << endl;
             exit(1);
        }
	    // Get the dimensions of the viewport and screen bounds

		GetClientRect( _mwindow, &view_rect );
		ClientToScreen( _mwindow, (POINT*)&view_rect.left );
		ClientToScreen( _mwindow, (POINT*)&view_rect.right );
		DWORD dwRenderWidth  = view_rect.right - view_rect.left;
		DWORD dwRenderHeight = view_rect.bottom - view_rect.top;
		_props._xorg = view_rect.left;	// _props should reflect view rectangle
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
	      << "wdxGraphicsWindow:: CreateSurface failed for primary surface: result = " << ConvD3DErrorToString(hr) << endl;
		  exit(1);
        }

		// Create a clipper object which handles all our clipping for cases when
		// our window is partially obscured by other windows. 
		LPDIRECTDRAWCLIPPER Clipper;
		if(FAILED(hr = pDD->CreateClipper( 0, &Clipper, NULL ))) {
		  wdxdisplay_cat.fatal()
		     << "wdxGraphicsWindow:: CreateClipper failed : result = " << ConvD3DErrorToString(hr) << endl;
		  exit(1);
        }
	
		// Associate the clipper with our window. Note that, afterwards, the
		// clipper is internally referenced by the primary surface, so it is safe
		// to release our local reference to it.
		Clipper->SetHWnd( 0, _mwindow );
		pPrimaryDDSurf->SetClipper( Clipper );
		Clipper->Release();
	
		// Setup a surface description to create a backbuffer. 
		SurfaceDesc.dwFlags        = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS;
		SurfaceDesc.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE | DDSCAPS_VIDEOMEMORY;
		SurfaceDesc.dwWidth  = dwRenderWidth;
		SurfaceDesc.dwHeight = dwRenderHeight;

        PRINTVIDMEM(pDD,&SurfaceDesc.ddsCaps,"initial backbuf");
	
		// Create the backbuffer. (might want to handle failure due to running out of video memory.
		if(FAILED(hr = pDD->CreateSurface( &SurfaceDesc, &pBackDDSurf, NULL ))) {
		  wdxdisplay_cat.fatal()
	      << "wdxGraphicsWindow:: CreateSurface failed for backbuffer : result = " << ConvD3DErrorToString(hr) << endl;
		  exit(1);
        }

    }  // end create windowed buffers

//	========================================================

    // Check if the device supports z-bufferless hidden surface removal. If so,
    // we don't really need a z-buffer
    if(!(d3ddevs[0].dpcTriCaps.dwRasterCaps & D3DPRASTERCAPS_ZBUFFERLESSHSR )) {
    
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
        if( FAILED(pD3DI->EnumZBufferFormats(  IID_IDirect3DHALDevice, EnumZBufFmtsCallback,
                                    (VOID*)&ZBufPixFmts ))) {
    		  wdxdisplay_cat.fatal()
    	      << "wdxGraphicsWindow::config() - EnumZBufferFormats failed " << endl;
    		  exit(1);
        }
    
    #ifdef _DEBUG
        { static BOOL bPrinted=FALSE;
           if(!bPrinted) {
                for(i=0;i<cNumZBufFmts;i++) { DebugPrintPixFmt(&ZBufPixFmts[i]); }
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
            } else if(pz16!=NULL){
                ddsd.ddpfPixelFormat = *pz16;
            } else {
        	  wdxdisplay_cat.fatal() << "wdxGraphicsWindow::config() - could find a valid zbuffer format!\n";
        	  exit(1);
            }
        }
    
        PRINTVIDMEM(pDD,&ddsd.ddsCaps,"initial zbuf");
    
        // Create and attach a z-buffer
        if( FAILED( hr = pDD->CreateSurface( &ddsd, &pZDDSurf, NULL ) ) ) {
    	  wdxdisplay_cat.fatal()
          << "wdxGraphicsWindow::config() - CreateSurface failed for Z buffer: result = " <<  ConvD3DErrorToString(hr) << endl;
    	  exit(1);
        }
    
        if( FAILED( hr = pBackDDSurf->AddAttachedSurface( pZDDSurf ) ) ) {
    	  wdxdisplay_cat.fatal()
          << "wdxGraphicsWindow::config() - AddAttachedSurface failed : result = " << ConvD3DErrorToString(hr) << endl;
    	  exit(1);
        }

#ifdef _DEBUG
    wdxdisplay_cat.debug() << "Creating " << ddsd.ddpfPixelFormat.dwRGBBitCount << "bpp zbuffer\n";
#endif
    }

    // Create the device. The device is created off of our back buffer, which
	// becomes the render target for the newly created device. 
    hr = pD3DI->CreateDevice( IID_IDirect3DHALDevice, pBackDDSurf, &pD3DDevice );
    if (hr != DD_OK) {
	  wdxdisplay_cat.fatal()
      << "wdxGraphicsWindow::config() - CreateDevice failed : result = " << ConvD3DErrorToString(hr) << endl;
	  exit(1);
    }

    // Create the viewport
	D3DVIEWPORT7 vp = { 0, 0, _props._xsize, _props._ysize, 0.0f, 1.0f };
    hr = pD3DDevice->SetViewport( &vp );
    if (hr != DD_OK) {
	  wdxdisplay_cat.fatal()
	      << "wdxGraphicsWindow::config() - SetViewport failed : result = " << ConvD3DErrorToString(hr) << endl;
	  exit(1);
    }

  DXGraphicsStateGuardian* dxgsg = DCAST(DXGraphicsStateGuardian, _gsg);
  dxgsg->Set_HDC(_hdc);
  dxgsg->init_dx(pDD, pPrimaryDDSurf, pBackDDSurf, pZDDSurf, pD3DI, pD3DDevice, view_rect);

  _dx_ready = true;
  dxgsg->SetDXStatus(true);

}

////////////////////////////////////////////////////////////////////
//     Function: setup_colormap 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::setup_colormap(void) {
  wdxGraphicsPipe* pipe = DCAST(wdxGraphicsPipe, _pipe);

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
//  Description:  timer info, incs frame #
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::end_frame(void) {
  PStatTimer timer(_swap_pcollector);  // added to mimic wglGraphicsWindow.cxx
//  SwapBuffers(_hdc);
  GraphicsWindow::end_frame();
}


//-----------------------------------------------------------------------------
// Name: SetViewMatrix()
// Desc: Given an eye point, a lookat point, and an up vector, this
//       function builds a 4x4 view matrix.
//-----------------------------------------------------------------------------
HRESULT SetViewMatrix( D3DMATRIX& mat, D3DVECTOR& vFrom, D3DVECTOR& vAt,
                       D3DVECTOR& vWorldUp )
{
    // Get the z basis vector, which points straight ahead. This is the
    // difference from the eyepoint to the lookat point.
    D3DVECTOR vView = vAt - vFrom;

    FLOAT fLength = Magnitude( vView );
    if( fLength < 1e-6f )
        return E_INVALIDARG;

    // Normalize the z basis vector
    vView /= fLength;

    // Get the dot product, and calculate the projection of the z basis
    // vector onto the up vector. The projection is the y basis vector.
    FLOAT fDotProduct = DotProduct( vWorldUp, vView );

    D3DVECTOR vUp = vWorldUp - fDotProduct * vView;

    // If this vector has near-zero length because the input specified a
    // bogus up vector, let's try a default up vector
    if( 1e-6f > ( fLength = Magnitude( vUp ) ) )
    {
        vUp = D3DVECTOR( 0.0f, 1.0f, 0.0f ) - vView.y * vView;

        // If we still have near-zero length, resort to a different axis.
        if( 1e-6f > ( fLength = Magnitude( vUp ) ) )
        {
            vUp = D3DVECTOR( 0.0f, 0.0f, 1.0f ) - vView.z * vView;

            if( 1e-6f > ( fLength = Magnitude( vUp ) ) )
                return E_INVALIDARG;
        }
    }

    // Normalize the y basis vector
    vUp /= fLength;

    // The x basis vector is found simply with the cross product of the y
    // and z basis vectors
    D3DVECTOR vRight = CrossProduct( vUp, vView );
    
    // Start building the matrix. The first three rows contains the basis
    // vectors used to rotate the view to point at the lookat point
    mat._11 = vRight.x;  mat._12 = vUp.x;  mat._13 = vView.x;  mat._14 = 0.0f;
    mat._21 = vRight.y;  mat._22 = vUp.y;  mat._23 = vView.y;  mat._24 = 0.0f;
    mat._31 = vRight.z;  mat._32 = vUp.z;  mat._33 = vView.z;  mat._34 = 0.0f;

    // Do the translation values (rotations are still about the eyepoint)
    mat._41 = - DotProduct( vFrom, vRight );
    mat._42 = - DotProduct( vFrom, vUp );
    mat._43 = - DotProduct( vFrom, vView );
    mat._44 = 1.0f;

    return S_OK;
}




////////////////////////////////////////////////////////////////////
//     Function: begin_frame 
//       Access:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::begin_frame(void) {
  GraphicsWindow::begin_frame();
}


void wdxGraphicsWindow::show_frame(void) 
{
  DXGraphicsStateGuardian* dxgsg = DCAST(DXGraphicsStateGuardian, _gsg);
  dxgsg->show_frame();
}

/**
////////////////////////////////////////////////////////////////////
//     Function: end_frame 
//       Access:
//  Description: Swaps the front and back buffers.
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::end_frame(void) {

  BUGBUG:  do we want this here (as it is in GL)??
  {
    PStatTimer timer(_swap_pcollector);
    SwapBuffers(_hdc);
  }

  GraphicsWindow::end_frame();
}
**/  



////////////////////////////////////////////////////////////////////
//     Function: handle_window_move
//       Access:
//  Description: we receive the new x and y position of the client
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::handle_window_move(int x, int y) {
	DXGraphicsStateGuardian* dxgsg = DCAST(DXGraphicsStateGuardian, _gsg);
	dxgsg->adjust_view_rect(x,y);
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
void wdxGraphicsWindow::handle_mouse_entry(int state,HCURSOR hCrossIcon) {
  if (state == MOUSE_EXITED) {
    _input_devices[0].set_pointer_out_of_window();
  }else {
      SetCursor(hCrossIcon);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_keypress
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::
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
void wdxGraphicsWindow::
handle_keyrelease(ButtonHandle key, int, int) {
  if (key != ButtonHandle::none()) {
    _input_devices[0].button_up(key);
  }
}

#if 0
////////////////////////////////////////////////////////////////////
//     Function: handle_changes
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::handle_changes(void) {
  // As an optimization, check to see if change is anything other than a
  // redisplay
  if (_change_mask & WDXWIN_CONFIGURE) {
    RECT changes;
    POINT point;
    GetClientRect(_mwindow, &changes);
    point.x = 0;
    point.y = 0;
    ClientToScreen(_mwindow, &point);
    changes.left = point.x;
    changes.top = point.y;
    // Don't do this in full-screen mode
    AdjustWindowRect(&changes, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN, FALSE);
    SetWindowPos(_mwindow, HWND_TOP, changes.left, changes.top,
		changes.right - changes.left, changes.bottom - changes.top,
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER |
		SWP_NOSENDCHANGING | SWP_NOSIZE | SWP_NOZORDER);
  }
  _change_mask = 0;
}
#endif

////////////////////////////////////////////////////////////////////
//     Function: process_events 
//       Access://  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::process_events(void) {
#if 0
  MSG msg;

  if (!GetMessage(&msg, NULL, 0, 0))
      exit(0);
  do {
    // Translate virtual key messages
    TranslateMessage(&msg);
    // Call window_proc
    DispatchMessage(&msg);
  } while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
#endif
}

/*
////////////////////////////////////////////////////////////////////
//     Function: idle_wait 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::idle_wait(void) {
  MSG msg;
  if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
    process_events();

  call_idle_callback();
}
*/

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

////////////////////////////////////////////////////////////////////
//     Function: update
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wdxGraphicsWindow::update(void) {
  _show_code_pcollector.stop();
  PStatClient::main_tick();

  // Always ask for a redisplay for now

  MSG msg;
#if 0
  if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
    process_events();
  else  call_draw_callback(true);
#else

  while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
    if (!GetMessage(&msg, NULL, 0, 0)) {
        // WM_QUIT received
        AtExitFn();
        exit(0);
    }

    // Translate virtual key messages
    TranslateMessage(&msg);
    // Call window_proc
    DispatchMessage(&msg);
  } 

  call_draw_callback(true);
#endif
 
  call_idle_callback();
/*  
  if (_idle_callback) 
    idle_wait();
  else if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
    process_events();					  
*/
  _show_code_pcollector.start();
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
  if (!get_param_into(pipe_param, params)) {
    wdxdisplay_cat.error()
      << "No pipe specified for window creation!" << endl;
    return NULL;
  }

  GraphicsPipe *pipe = pipe_param->get_pipe();
  
  GraphicsWindow::WindowProps *props_param;
  if (!get_param_into(props_param, params)) {
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


