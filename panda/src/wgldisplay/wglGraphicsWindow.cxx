// Filename: wglGraphicsWindow.cxx
//
////////////////////////////////////////////////////////////////////
// Includes
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

#ifdef DO_PSTATS
#include <pStatTimer.h>
#endif

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle wglGraphicsWindow::_type_handle;

#define MOUSE_ENTERED 0
#define MOUSE_EXITED 1

#define FONT_BITMAP_OGLDISPLAYLISTNUM 1000    // some arbitrary #

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
//     Function: Destructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
wglGraphicsWindow::~wglGraphicsWindow(void) {

  if(gl_show_fps_meter)
	glDeleteLists(FONT_BITMAP_OGLDISPLAYLISTNUM, 128);

  if(_visual!=NULL)
    free(_visual);

  // The GL context is already gone.  Don't try to destroy it again.
  //  wglDeleteContext(_context);

  ReleaseDC(_mwindow,_hdc);
  DestroyWindow(_mwindow);
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
	  << "wglGraphicsWindow::try_for_visual() - request for stereo failed" << endl;	
      }
    }
  }

  return match;
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

#if 0
////////////////////////////////////////////////////////////////////
//     Function: choose visual
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
int wglGraphicsWindow::choose_visual(void) {
  wglGraphicsPipe* pipe = DCAST(wglGraphicsPipe, _pipe);

  int mask = _props._mask;

    wgldisplay_cat.info()
      << "wglGraphicsWindow::mask =0x" << (void*) _props._mask
	<< endl;

  int want_depth_bits = _props._want_depth_bits;
  int want_color_bits = _props._want_color_bits;

  if (mask & W_MULTISAMPLE) {
    wgldisplay_cat.info()
      << "wglGraphicsWindow::config() - multisample not supported"
	<< endl;
    mask &= ~W_MULTISAMPLE;
  }

  _visual = try_for_visual(pipe, mask, want_depth_bits, want_color_bits);

  // This is the severity level at which we'll report the details of
  // the visual we actually do find.  Normally, it's debug-level
  // information: we don't care about that much detail.
  NotifySeverity show_visual_severity = NS_debug;

  if (_visual == NULL) {
    wgldisplay_cat.info()
      << "wglGraphicsWindow::choose_visual() - visual with requested\n"
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
      _visual = try_for_visual(pipe, mask);
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
	  _visual = try_for_visual(pipe, new_mask, want_depth_bits,
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
	      _visual = try_for_visual(pipe, new_mask);
	    }
	  }
	}
      }
	
      if (_visual == NULL) {
	// Here's our last-ditch desparation attempt: give us any GLX
	// visual at all!
	_visual = try_for_visual(pipe, 0);
      }
	
      if (_visual == NULL) {
	wgldisplay_cat.fatal()
	  << "wglGraphicsWindow::choose_visual() - could not get any "
	  	"visual." << endl;
	exit(1);
      }
    }
  }

  if (wgldisplay_cat.is_on(show_visual_severity)) {
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

    wgldisplay_cat.out(show_visual_severity)
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
#else

typedef enum {Software, MCD, ICD} OGLDriverType;
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
  wglGraphicsPipe* pipe = DCAST(wglGraphicsPipe, _pipe);

  int mask = _props._mask;
  int want_depth_bits = _props._want_depth_bits;
  int want_color_bits = _props._want_color_bits;
  OGLDriverType drvtype;

  if (mask & W_MULTISAMPLE) {
    wgldisplay_cat.info()
      << "wglGraphicsWindow::config() - multisample not supported"<< endl;
    mask &= ~W_MULTISAMPLE;
  }
    wgldisplay_cat.info()
      << "wglGraphicsWindow::mask =0x" << (void*) mask
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

      if (wgldisplay_cat->is_debug())
	wgldisplay_cat->debug() << "----------------" << endl;

      if((pfd.dwFlags & PFD_GENERIC_ACCELERATED) && (pfd.dwFlags & PFD_GENERIC_FORMAT))
          drvtype=MCD;
       else if(!(pfd.dwFlags & PFD_GENERIC_ACCELERATED) && !(pfd.dwFlags & PFD_GENERIC_FORMAT))
          drvtype=ICD;
       else {
         drvtype=Software;
	 if (wgldisplay_cat->is_debug())
	   wgldisplay_cat->debug() << "skipping software driver" << endl;
         continue;  // skipping all SW fmts
       }

      if((pfd.iPixelType == PFD_TYPE_COLORINDEX) && !(mask & W_INDEX))
          continue;

#if 0
    // use wglinfo.exe instead
    char msg[200];
    sprintf(msg,"\nGL PixelFormat[%d]",i);
    PrintPFD(&pfd,msg);
#endif

       DWORD dwReqFlags=(PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW);

       if (wgldisplay_cat->is_debug()) {
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
      wgldisplay_cat.error() << "wglGraphicsWindow:: ERROR: couldn't find HW-accelerated OpenGL pixfmt appropriate for this desktop!!\n";
      if(cur_bpp>16) 
        wgldisplay_cat.error() << "wglGraphicsWindow:: try reducing the screen resolution or reducing the screen pixeldepth\n";
       else wgldisplay_cat.error() << "wglGraphicsWindow:: try reducing the screen resolution\n";
      exit(1);
  }

  _visual = (PIXELFORMATDESCRIPTOR*)malloc(sizeof(PIXELFORMATDESCRIPTOR));
  if(_visual==NULL) {
      wgldisplay_cat.error() << "wglGraphicsWindow:: couldnt alloc mem for PIXELFORMATDESCRIPTOR\n";
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

#endif

////////////////////////////////////////////////////////////////////
//     Function: adjust_coords 
//       Access:
//  Description: Adjust the window rectangle because Win32 thinks
//		 that the x, y, width, and height are the *entire*
//		 window, including decorations, whereas we assume
//		 the size is the *client* area of the window.
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::
adjust_coords(int &xorg, int &yorg, int &xsize, int &ysize) {
  RECT rect;
  rect.left = xorg;  rect.top = yorg;
  rect.right = xorg + xsize; 
  rect.bottom = yorg + ysize;

  // Style determines whether there are borders or not
  // False in third parameter indicates window has no menu bar
  AdjustWindowRect(&rect, WS_CLIPSIBLINGS | WS_CLIPCHILDREN |
			  WS_OVERLAPPEDWINDOW, false);

  // Readjust if the x and y are offscreen
  if (rect.left < 0)
    xorg = 0;
  else
    xorg = rect.left;

  if (rect.top < 0)
    yorg = 0;
  else
    yorg = rect.top;

  xsize = rect.right - rect.left;
  ysize = rect.bottom - rect.top;
}

////////////////////////////////////////////////////////////////////
//     Function: config 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::config(void) {
  GraphicsWindow::config();

  wglGraphicsPipe* pipe = DCAST(wglGraphicsPipe, _pipe);
  HINSTANCE hinstance = GetModuleHandle(NULL);
  HWND desktop = GetDesktopWindow();

  if (_props._fullscreen) {
      // Note: to set fullscreen, use 'fullscreen #t' in Configurc
      // This just runs fullscrn at current display res & depth
      // want to be smarter about this like wdxdisplay and pick a res
      // that will have HW acceleration and adequate texmem, but we cant
      // get the appropriate info yet in GL (unlike DX). 
      
      // BUGBUG: The Configrc res vars (_props.x/ysize) are ignored; 
      // instead we should consider switching to the specified res
      // (cant use DDraw for this though due to inconsistent driver support)

    _props._xorg = 0;
    _props._yorg = 0;
    _props._xsize = GetSystemMetrics(SM_CXSCREEN);
    _props._ysize = GetSystemMetrics(SM_CYSCREEN);
    _mwindow = CreateWindow("wglFullscreen", _props._title.c_str(),
                WS_POPUP | WS_MAXIMIZE, 
		_props._xorg, _props._yorg, _props._xsize, _props._ysize,
                desktop, NULL, hinstance, 0);

  } else {

    int xorg = _props._xorg;
    int yorg = _props._yorg;
    int xsize = _props._xsize;
    int ysize = _props._ysize;

    int style = WS_POPUP | WS_MAXIMIZE;
    if (_props._border == true) {
      style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW;
      adjust_coords(xorg, yorg, xsize, ysize);
    }

    _mwindow = CreateWindow("wglStandard", _props._title.c_str(),
                style, xorg, yorg, xsize, ysize,
                desktop, NULL, hinstance, 0);
  }

  if (!_mwindow) {
    wgldisplay_cat.fatal()
      << "wglGraphicsWindow::config() - failed to create Mwindow" << endl;
    exit(1);
  }

  _hdc = GetDC(_mwindow);

  // Configure the framebuffer according to parameters specified in _props
  // Initializes _visual
  int pfnum=choose_visual();

//  int pfnum=ChoosePixelFormat(_hdc, _visual);
  wgldisplay_cat.debug()
      << "wglGraphicsWindow::config() - picking pixfmt #"<< pfnum <<endl;

  if (!SetPixelFormat(_hdc, pfnum, _visual)) {
    wgldisplay_cat.fatal()
      << "wglGraphicsWindow::config() - SetPixelFormat failed after window create" << endl;
    exit(1);
  }

  // Initializes _colormap
  setup_colormap();

  _context = wglCreateContext(_hdc);
  if (!_context) {
    wgldisplay_cat.fatal()
      << "wglGraphicsWindow::config() - failed to create Win32 rendering "
      << "context" << endl;
    exit(1);
  }

  _change_mask = 0;

  _mouse_input_enabled = false;
  _mouse_motion_enabled = false;
  _mouse_passive_motion_enabled = false;
  _mouse_entry_enabled = false;
  _entry_state = -1;

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
			  wgldisplay_cat.debug() << "wglGraphicsWindow:: GL VendorID: " <<   glGetString(GL_VENDOR) << " (Software Rendering)" << endl;
		  } else {
			  wgldisplay_cat.debug() << "wglGraphicsWindow:: GL VendorID: " <<   glGetString(GL_VENDOR) << endl;
		  }
	  } else {
		 wgldisplay_cat.info() << "wglGraphicsWindow:: glGetString(GL_VENDOR) returns NULL!!!\n";
	  }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: setup_colormap 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::setup_colormap(void) {
  wglGraphicsPipe* pipe = DCAST(wglGraphicsPipe, _pipe);

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

	if(gl_show_fps_meter) {
		 DWORD now = timeGetTime();  // this is win32 fn

		 float time_delta = (now - _start_time) * 0.001f;

		 if(time_delta > gl_fps_meter_update_interval) {
			 // didnt use global clock object, it wasnt working properly when I tried,
			 // its probably slower due to cache faults, and I can easily track all the
			 // info I need in dxgsg
			 DWORD num_frames = _cur_frame_count - _start_frame_count;

			 _current_fps = num_frames / time_delta;
			 _start_time = now;
			 _start_frame_count = _cur_frame_count;
		 }

		 char fps_msg[15];
		 sprintf(fps_msg, "%.02f fps", _current_fps);

		 // Note: we cant use simple GDI TextOut calls to draw FPS meter chars (like DX fps meter)
		 // because WGL doesnt support GDI in double-buffered mode.  Instead we have to 
		 // use glBitMap display lists created by wglUseFontBitmaps

         glColor3f(0.0,1.0,1.0);

		 GLboolean tex_was_on = glIsEnabled(GL_TEXTURE_2D);

		 if(tex_was_on)
			 glDisable(GL_TEXTURE_2D);

		 glMatrixMode(GL_MODELVIEW);
		 glPushMatrix();
		 glLoadMatrixf(LMatrix4f::ident_mat().get_data());
		 glMatrixMode(GL_PROJECTION);
		 glPushMatrix();
		 glLoadMatrixf(LMatrix4f::ident_mat().get_data());

		 glOrtho(_props._xorg,_props._xorg+_props._xsize,
				 _props._yorg,_props._yorg+_props._ysize,-1.0,1.0);
					
		 glRasterPos2f(_props._xsize-70,_props._ysize-20);  // these seem to be good for default font

		 // set up for a string-drawing display list call 
		 glListBase(FONT_BITMAP_OGLDISPLAYLISTNUM);
 
		 // draw a string using font display lists.  chars index their corresponding displist name
		 glCallLists(strlen(fps_msg), GL_UNSIGNED_BYTE, fps_msg);

 		 glPopMatrix();
		 glMatrixMode(GL_MODELVIEW);
		 glPopMatrix();

		 if(tex_was_on)
			 glEnable(GL_TEXTURE_2D);

		 _cur_frame_count++;  // only used by fps meter right now
	}

  {
#ifdef DO_PSTATS
    PStatTimer timer(_swap_pcollector);
#endif
    SwapBuffers(_hdc);
  }
  GraphicsWindow::end_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: handle_reshape 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::handle_reshape(int w, int h) {
  // bugbug: this is not current working right??
  if (_props._xsize != w || _props._ysize != h) {
    _props._xsize = w;
    _props._ysize = h;
    make_current();
    GdiFlush(); 
    resized(w, h);
    _change_mask |= WGLWIN_CONFIGURE;
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
handle_keyrelease(ButtonHandle key, int, int) {
  if (key != ButtonHandle::none()) {
    _input_devices[0].button_up(key);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_changes
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::handle_changes(void) {
  // As an optimization, check to see if change is anything other than a
  // redisplay
  if (_change_mask & WGLWIN_CONFIGURE) {
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

////////////////////////////////////////////////////////////////////
//     Function: process_events 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::process_events(void) {
  MSG event, msg;

  do {
    if (!GetMessage(&event, NULL, 0, 0))
      exit(0);
    // Translate virtual key messages
    TranslateMessage(&event);
    // Call window_proc
    DispatchMessage(&event);
  }
  while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE));
}

////////////////////////////////////////////////////////////////////
//     Function: idle_wait 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::idle_wait(void) {
  MSG msg;
  if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
    process_events();

  call_idle_callback();
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
  PStatClient::main_tick();
#endif

  if (_change_mask)
    handle_changes();

  make_current();
  // Always ask for a redisplay for now
  call_draw_callback(true);

  if (_idle_callback) 
    idle_wait();
  else
    process_events();
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
#ifdef DO_PSTATS
  PStatTimer timer(_make_current_pcollector);
#endif
  HGLRC current_context = wglGetCurrentContext();
  HDC current_dc = wglGetCurrentDC();
  if (current_context != _context || current_dc != _hdc)
    wglMakeCurrent(_hdc, _context);
}

////////////////////////////////////////////////////////////////////
//     Function: unmake_current
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void wglGraphicsWindow::unmake_current(void) {
  wglMakeCurrent(NULL, NULL);
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
