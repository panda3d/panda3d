// Filename: glxGraphicsWindow.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "glxGraphicsWindow.h"
#include "glxDisplay.h"
#include "config_glxdisplay.h"

#include <graphicsPipe.h>
#include <keyboardButton.h>
#include <mouseButton.h>
#include <glGraphicsStateGuardian.h>
#include <pStatTimer.h>

#include <errno.h>
#include <sys/time.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle glxGraphicsWindow::_type_handle;

const char* glxGraphicsWindow::_glx_extensions = NULL;

#define MOUSE_ENTERED 0
#define MOUSE_EXITED 1

////////////////////////////////////////////////////////////////////
//     Function: Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
glxGraphicsWindow::glxGraphicsWindow( GraphicsPipe* pipe ) : 
	GraphicsWindow( pipe )
{
  config();
}

////////////////////////////////////////////////////////////////////
//     Function: Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
glxGraphicsWindow::glxGraphicsWindow( GraphicsPipe* pipe, const 
	GraphicsWindow::Properties& props ) : GraphicsWindow( pipe, props )
{
  config();
}

////////////////////////////////////////////////////////////////////
//     Function: Destructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
glxGraphicsWindow::~glxGraphicsWindow(void) 
{
  // The GL context is already gone.  Don't try to destroy it again;
  // that will cause a seg fault on exit.
  //  unmake_current();
  //  glXDestroyContext(_display, _context);

  XDestroyWindow(_display, _xwindow);
  if (_colormap)
    XFreeColormap(_display, _colormap);
  XFree(_visual);
}

////////////////////////////////////////////////////////////////////
//     Function: glx_supports 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
bool glxGraphicsWindow::glx_supports(const char* extension)
{
#if defined(GLX_VERSION_1_1)
  const char* start;
  char* where, *terminator;
  int major, minor;

  glxDisplay *glx = _pipe->get_glx_display();
  nassertr(glx != (glxDisplay *)NULL, false);

  glXQueryVersion(_display, &major, &minor);
  if ((major == 1 && minor >= 1) || (major > 1)) {
    if (!_glx_extensions) {
      _glx_extensions = 
	glXQueryExtensionsString(_display, glx->get_screen());
    }
    start = _glx_extensions;
    for (;;) {
      where = strstr(start, extension);
      if (!where)
        return false;
      terminator = where + strlen(extension);
      if (where == start || *(where - 1) == ' ') {
	if (*terminator == ' ' || *terminator == '\0') {
	  return true;
	}
      }	
      start = terminator;
    }
  }
  return false;
#else
  return false;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: try_for_visual
//  Description: This is a static function that attempts to get the
//               requested visual, if it is available.  It's just a
//               wrapper around glXChooseVisual().  It returns the
//               visual information if possible, or NULL if it is not.
////////////////////////////////////////////////////////////////////
static XVisualInfo *
try_for_visual(glxDisplay *glx, int mask,
	       int want_depth_bits = 1, int want_color_bits = 1) {
  static const int max_attrib_list = 32;
  int attrib_list[max_attrib_list];
  int n=0;

  glxdisplay_cat.debug()
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
    glxdisplay_cat.debug(false) << " ALPHA";
    attrib_list[n++] = GLX_ALPHA_SIZE;
    attrib_list[n++] = want_color_component_bits;
  }
  if (mask & W_DOUBLE) {
    glxdisplay_cat.debug(false) << " DOUBLEBUFFER";
    attrib_list[n++] = GLX_DOUBLEBUFFER;
  }
  if (mask & W_STEREO) {
    glxdisplay_cat.debug(false) << " STEREO";
    attrib_list[n++] = GLX_STEREO;
  }
  if (mask & W_DEPTH) {
    glxdisplay_cat.debug(false) << " DEPTH(" << want_depth_bits << ")";
    attrib_list[n++] = GLX_DEPTH_SIZE;
    attrib_list[n++] = want_depth_bits;
  }
  if (mask & W_STENCIL) {
    glxdisplay_cat.debug(false) << " STENCIL";
    attrib_list[n++] = GLX_STENCIL_SIZE;
    attrib_list[n++] = 1;
  }
  if (mask & W_ACCUM) {
    glxdisplay_cat.debug(false) << " ACCUM";
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
#if defined(GLX_VERSION_1_1) && defined(GLX_SGIS_multisample)
  if (mask & W_MULTISAMPLE) {
    glxdisplay_cat.debug(false) << " MULTISAMPLE";
    attrib_list[n++] = GLX_SAMPLES_SGIS;
    // We decide 4 is minimum number of samples
    attrib_list[n++] = 4;
  }
#endif

  // Terminate the list
  nassertr(n < max_attrib_list, NULL);
  attrib_list[n] = (int)None;

  XVisualInfo *vinfo =
    glXChooseVisual(glx->get_display(), glx->get_screen(), attrib_list);

  if (glxdisplay_cat.is_debug()) {
    if (vinfo != NULL) {
      glxdisplay_cat.debug(false) << ", match found!\n";
    } else {
      glxdisplay_cat.debug(false) << ", no match.\n";
    }
  }

  return vinfo;
}

////////////////////////////////////////////////////////////////////
//     Function: choose visual
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::choose_visual(void)
{
  glxDisplay *glx = _pipe->get_glx_display();
  nassertv(glx != (glxDisplay *)NULL);

  int mask = _props._mask;
  int want_depth_bits = _props._want_depth_bits;
  int want_color_bits = _props._want_color_bits;

#if defined(GLX_VERSION_1_1) && defined(GLX_SGIS_multisample)
  if (mask & W_MULTISAMPLE) {
    if (!glx_supports("GLX_SGIS_multisample")) {
      glxdisplay_cat.info()
	<< "glxGraphicsWindow::config() - multisample not supported"
	<< endl;
      mask &= ~W_MULTISAMPLE;
    }
  }
#endif

  _visual = try_for_visual(glx, mask, want_depth_bits, want_color_bits);

  // This is the severity level at which we'll report the details of
  // the visual we actually do find.  Normally, it's debug-level
  // information: we don't care about that much detail.
  NotifySeverity show_visual_severity = NS_debug;

  if (_visual == NULL) {
    glxdisplay_cat.info()
      << "glxGraphicsWindow::choose_visual() - visual with requested\n"
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
      _visual = try_for_visual(glx, mask);
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
	  _visual = try_for_visual(glx, new_mask, want_depth_bits,
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
	      _visual = try_for_visual(glx, new_mask);
	    }
	  }
	}
      }
	
      if (_visual == NULL) {
	// Here's our last-ditch desparation attempt: give us any GLX
	// visual at all!
	_visual = try_for_visual(glx, 0);
      }
	
      if (_visual == NULL) {
	glxdisplay_cat.fatal()
	  << "glxGraphicsWindow::choose_visual() - could not get any "
	  "GLX visual." << endl;
	exit(1);
      }
    }
  }

  glxdisplay_cat.info()
    << "Got visual 0x" << hex << (int)_visual->visualid << dec << ".\n";

  if (glxdisplay_cat.is_on(show_visual_severity)) {
    int render_mode, double_buffer, stereo, red_size, green_size, blue_size,
      alpha_size, ared_size, agreen_size, ablue_size, aalpha_size,
      depth_size, stencil_size;
    
    glXGetConfig(_display, _visual, GLX_RGBA, &render_mode);
    glXGetConfig(_display, _visual, GLX_DOUBLEBUFFER, &double_buffer);
    glXGetConfig(_display, _visual, GLX_STEREO, &stereo);
    glXGetConfig(_display, _visual, GLX_RED_SIZE, &red_size);
    glXGetConfig(_display, _visual, GLX_GREEN_SIZE, &green_size);
    glXGetConfig(_display, _visual, GLX_BLUE_SIZE, &blue_size);
    glXGetConfig(_display, _visual, GLX_ALPHA_SIZE, &alpha_size);
    glXGetConfig(_display, _visual, GLX_ACCUM_RED_SIZE, &ared_size);
    glXGetConfig(_display, _visual, GLX_ACCUM_GREEN_SIZE, &agreen_size);
    glXGetConfig(_display, _visual, GLX_ACCUM_BLUE_SIZE, &ablue_size);
    glXGetConfig(_display, _visual, GLX_ACCUM_ALPHA_SIZE, &aalpha_size);
    glXGetConfig(_display, _visual, GLX_DEPTH_SIZE, &depth_size);
    glXGetConfig(_display, _visual, GLX_STENCIL_SIZE, &stencil_size);

    glxdisplay_cat.out(show_visual_severity)
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

////////////////////////////////////////////////////////////////////
//     Function: config 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::config( void )
{
  GraphicsWindow::config();

  glxDisplay *glx = _pipe->get_glx_display();
  nassertv(glx != (glxDisplay *)NULL);

  _display = glx->get_display();

  // Configure the framebuffer according to parameters specified in _props
  // Initializes _visual
  choose_visual();

  // Initializes _colormap
  setup_colormap();

  uint attrib_mask = CWBackPixmap | CWBorderPixel | CWColormap | CWEventMask;

  // Do we even need structure notify?
  _event_mask = StructureNotifyMask;

  // Initialize window attributes
  XSetWindowAttributes wa;
  wa.background_pixmap = None;
  wa.border_pixel = 0;
  wa.colormap = _colormap;
  wa.event_mask = _event_mask; 
  wa.do_not_propagate_mask = 0;

  _xwindow = XCreateWindow(_display, glx->get_root(),
	_props._xorg, _props._yorg, _props._xsize, _props._ysize, 0,
	_visual->depth, InputOutput, _visual->visual, attrib_mask, &wa); 
  if (!_xwindow) {
    glxdisplay_cat.fatal()
      << "glxGraphicsWindow::config() - failed to create Xwindow" << endl;
    exit(0);
  }

  setup_properties();

  _context = glXCreateContext(_display, _visual, None, GL_TRUE);
  if (!_context) {
    glxdisplay_cat.fatal()
      << "glxGraphicsWindow::config() - failed to create OpenGL rendering "
      << "context" << endl;
    exit(0);
  }

  if (!(glXIsDirect(_display, _context))) {
    glxdisplay_cat.info()
      << "Graphics context is not direct (it does not bypass X)."
      << endl;
  }

  _change_mask = 0;
  _configure_mask = 0;

  _mouse_input_enabled = false;
  _mouse_motion_enabled = false;
  _mouse_passive_motion_enabled = false;
  _mouse_entry_enabled = false;
  _entry_state = -1;

  XSelectInput(_display, _xwindow, _event_mask);
  XMapWindow(_display, _xwindow);

  _connection_fd = ConnectionNumber(_display);

  // Enable detection of mouse input
  enable_mouse_input(true);
  enable_mouse_motion(true);
  enable_mouse_passive_motion(true);
  enable_mouse_entry(true);

  // Enable detection of keyboard input
  add_event_mask(KeyPressMask | KeyReleaseMask);

  // Now indicate that we have our keyboard/mouse device ready.
  GraphicsWindowInputDevice device =
    GraphicsWindowInputDevice::pointer_and_keyboard("keyboard/mouse");
  _input_devices.push_back(device);

  // Create a GSG to manage the graphics
  // First make the new context and window the current one so GL knows how
  // to configure itself in the gsg
  make_current();
  make_gsg();
}

////////////////////////////////////////////////////////////////////
//     Function: setup_colormap 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::setup_colormap(void)
{
  int visual_class, rc, is_rgb;
#if defined(__cplusplus) || defined(c_plusplus)
  visual_class = _visual->c_class;
#else
  visual_class = _visual->class;
#endif  
  glxDisplay *glx = _pipe->get_glx_display();
  nassertv(glx != (glxDisplay *)NULL);
  
  switch (visual_class) {
    case PseudoColor:
      rc = glXGetConfig(_display, _visual, GLX_RGBA, &is_rgb);
      if (rc == 0 && is_rgb) {
        glxdisplay_cat.info()
	  << "glxGraphicsWindow::setup_colormap() - mesa pseudocolor "
	  << "not supported" << endl;
	// this is a terrible terrible hack, that seems to work
        _colormap = (Colormap)0;
      } else {
        _colormap = XCreateColormap(_display, glx->get_root(),
	  _visual->visual, AllocAll);	
      }
      break;
    case TrueColor:
    case DirectColor:
      _colormap = XCreateColormap(_display, glx->get_root(),
	_visual->visual, AllocNone); 
      break;
    case StaticColor:
    case StaticGray:
    case GrayScale:
      _colormap = XCreateColormap(_display, glx->get_root(),
	_visual->visual, AllocNone); 
      break;
    default:
      glxdisplay_cat.error()
	<< "glxGraphicsWindow::setup_colormap() - could not allocate a "
	<< "colormap for visual type: " << visual_class << endl;
      break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: setup_properties
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::setup_properties(void)
{
  // Name the window if there is a name
  XTextProperty window_name;
  if (!_props._title.empty()) {
    char *name = (char *)_props._title.c_str();
    if (XStringListToTextProperty(&name, 1, &window_name) == 0) {
      glxdisplay_cat.error()
	<< "glxGraphicsWindow::config() - failed to allocate window name "
	<< "structure" << endl;
    }
  }

  // Setup size hints
  XSizeHints size_hints = {0};
  if (_props._xorg >= 0 && _props._yorg >= 0) {
    size_hints.x = _props._xorg;
    size_hints.y = _props._yorg;
    size_hints.flags |= USPosition;
  }
  else
    size_hints.flags &= ~USPosition;
  if (_props._xsize >= 0 && _props._ysize >= 0) {
    size_hints.width = _props._xsize;
    size_hints.height = _props._ysize;
    size_hints.flags |= USSize;
  }
  else
    size_hints.flags &= ~USSize;

  // Setup window manager hints
  XWMHints* wm_hints;
  if (!(wm_hints = XAllocWMHints())) {
    glxdisplay_cat.fatal()
      << "failed to allocate wm hints" << endl;
    exit(-1);
  }
  wm_hints->initial_state = NormalState;
  wm_hints->flags = StateHint;

  XSetWMProperties(_display, _xwindow, &window_name, &window_name,
    NULL, 0, &size_hints, wm_hints, NULL);
  XFree(wm_hints);

  // If we asked for a window without a border, there's no sure-fire
  // way to arrange that.  It really depends on the user's window
  // manager of choice.  Instead, we'll totally punt and just set the
  // window's Class to "Undecorated", and let the user configure
  // his/her window manager not to put a border around windows of this
  // class.
  if (!_props._border) {
    XClassHint *class_hint = XAllocClassHint();
    class_hint->res_class = "Undecorated";
    XSetClassHint(_display, _xwindow, class_hint);
    XFree(class_hint);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: end_frame 
//       Access:
//  Description: Swaps the front and back buffers.
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::end_frame( void )
{
  {
    PStatTimer timer(_swap_pcollector);
    glXSwapBuffers(_display, _xwindow);
  }

  GraphicsWindow::end_frame();
}

////////////////////////////////////////////////////////////////////
//     Function: handle_reshape 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::handle_reshape(int w, int h)
{
  resized(w, h);
  // Do we want to reshape the glViewport here as well?
  //glViewport(0, 0, (GLsizei)_props._xsize, (GLsizei)_props._ysize);
}

////////////////////////////////////////////////////////////////////
//     Function: handle_mouse_motion
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::handle_mouse_motion(int x, int y) {
  _input_devices[0].set_pointer_in_window(x, y);
}

////////////////////////////////////////////////////////////////////
//     Function: handle_mouse_entry
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::handle_mouse_entry(int state) {
  if (state == MOUSE_EXITED) {
    _input_devices[0].set_pointer_out_of_window();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: handle_keypress
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::
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
void glxGraphicsWindow::
handle_keyrelease(ButtonHandle key, int, int) {
  if (key != ButtonHandle::none()) {
    _input_devices[0].button_up(key);
  }
}

ButtonHandle glxGraphicsWindow::
lookup_key(XEvent event) { 
  // First, get the string key name.  If it fits in one character, it
  // must be the ASCII equivalent for the key.
  char tmp[1];
  if (XLookupString(&event.xkey, tmp, 1, NULL, NULL)) {
    ButtonHandle button = KeyboardButton::ascii_key(tmp[0]);
    if (button != ButtonHandle::none()) {
      return button;
    }
  }

  // Otherwise, look it up the hard way.
  KeySym ks = XLookupKeysym((XKeyEvent *)&event, 0);

  switch (ks) {
  case XK_BackSpace: 
    return KeyboardButton::backspace();
  case XK_Tab: 
    return KeyboardButton::tab();
  case XK_Return: 
    return KeyboardButton::enter();
  case XK_Escape: 
    return KeyboardButton::escape();
  case XK_space: 
    return KeyboardButton::space();
  case XK_exclam: 
    return KeyboardButton::ascii_key('!');
  case XK_quotedbl: 
    return KeyboardButton::ascii_key('"');
  case XK_numbersign: 
    return KeyboardButton::ascii_key('#');
  case XK_dollar: 
    return KeyboardButton::ascii_key('$');
  case XK_percent: 
    return KeyboardButton::ascii_key('%');
  case XK_ampersand: 
    return KeyboardButton::ascii_key('&');
  case XK_apostrophe: // == XK_quoteright
    return KeyboardButton::ascii_key('\'');
  case XK_parenleft: 
    return KeyboardButton::ascii_key('(');
  case XK_parenright: 
    return KeyboardButton::ascii_key(')');
  case XK_asterisk: 
    return KeyboardButton::ascii_key('*');
  case XK_plus: 
    return KeyboardButton::ascii_key('+');
  case XK_comma: 
    return KeyboardButton::ascii_key(',');
  case XK_minus: 
    return KeyboardButton::ascii_key('-');
  case XK_period: 
    return KeyboardButton::ascii_key('.');
  case XK_slash: 
    return KeyboardButton::ascii_key('/');
  case XK_0: 
    return KeyboardButton::ascii_key('0');
  case XK_1: 
    return KeyboardButton::ascii_key('1');
  case XK_2: 
    return KeyboardButton::ascii_key('2');
  case XK_3: 
    return KeyboardButton::ascii_key('3');
  case XK_4: 
    return KeyboardButton::ascii_key('4');
  case XK_5: 
    return KeyboardButton::ascii_key('5');
  case XK_6: 
    return KeyboardButton::ascii_key('6');
  case XK_7: 
    return KeyboardButton::ascii_key('7');
  case XK_8: 
    return KeyboardButton::ascii_key('8');
  case XK_9: 
    return KeyboardButton::ascii_key('9');
  case XK_colon: 
    return KeyboardButton::ascii_key(':');
  case XK_semicolon: 
    return KeyboardButton::ascii_key(';');
  case XK_less: 
    return KeyboardButton::ascii_key('<');
  case XK_equal: 
    return KeyboardButton::ascii_key('=');
  case XK_greater: 
    return KeyboardButton::ascii_key('>');
  case XK_question: 
    return KeyboardButton::ascii_key('?');
  case XK_at: 
    return KeyboardButton::ascii_key('@');
  case XK_A: 
    return KeyboardButton::ascii_key('A');
  case XK_B: 
    return KeyboardButton::ascii_key('B');
  case XK_C: 
    return KeyboardButton::ascii_key('C');
  case XK_D: 
    return KeyboardButton::ascii_key('D');
  case XK_E: 
    return KeyboardButton::ascii_key('E');
  case XK_F: 
    return KeyboardButton::ascii_key('F');
  case XK_G: 
    return KeyboardButton::ascii_key('G');
  case XK_H: 
    return KeyboardButton::ascii_key('H');
  case XK_I: 
    return KeyboardButton::ascii_key('I');
  case XK_J: 
    return KeyboardButton::ascii_key('J');
  case XK_K: 
    return KeyboardButton::ascii_key('K');
  case XK_L: 
    return KeyboardButton::ascii_key('L');
  case XK_M: 
    return KeyboardButton::ascii_key('M');
  case XK_N: 
    return KeyboardButton::ascii_key('N');
  case XK_O: 
    return KeyboardButton::ascii_key('O');
  case XK_P: 
    return KeyboardButton::ascii_key('P');
  case XK_Q: 
    return KeyboardButton::ascii_key('Q');
  case XK_R: 
    return KeyboardButton::ascii_key('R');
  case XK_S: 
    return KeyboardButton::ascii_key('S');
  case XK_T: 
    return KeyboardButton::ascii_key('T');
  case XK_U: 
    return KeyboardButton::ascii_key('U');
  case XK_V: 
    return KeyboardButton::ascii_key('V');
  case XK_W: 
    return KeyboardButton::ascii_key('W');
  case XK_X: 
    return KeyboardButton::ascii_key('X');
  case XK_Y: 
    return KeyboardButton::ascii_key('Y');
  case XK_Z: 
    return KeyboardButton::ascii_key('Z');
  case XK_bracketleft: 
    return KeyboardButton::ascii_key('[');
  case XK_backslash: 
    return KeyboardButton::ascii_key('\\');
  case XK_bracketright: 
    return KeyboardButton::ascii_key(']');
  case XK_asciicircum: 
    return KeyboardButton::ascii_key('^');
  case XK_underscore: 
    return KeyboardButton::ascii_key('_');
  case XK_grave: // == XK_quoteleft
    return KeyboardButton::ascii_key('`');
  case XK_a: 
    return KeyboardButton::ascii_key('a');
  case XK_b: 
    return KeyboardButton::ascii_key('b');
  case XK_c: 
    return KeyboardButton::ascii_key('c');
  case XK_d: 
    return KeyboardButton::ascii_key('d');
  case XK_e: 
    return KeyboardButton::ascii_key('e');
  case XK_f: 
    return KeyboardButton::ascii_key('f');
  case XK_g: 
    return KeyboardButton::ascii_key('g');
  case XK_h: 
    return KeyboardButton::ascii_key('h');
  case XK_i: 
    return KeyboardButton::ascii_key('i');
  case XK_j: 
    return KeyboardButton::ascii_key('j');
  case XK_k: 
    return KeyboardButton::ascii_key('k');
  case XK_l: 
    return KeyboardButton::ascii_key('l');
  case XK_m: 
    return KeyboardButton::ascii_key('m');
  case XK_n: 
    return KeyboardButton::ascii_key('n');
  case XK_o: 
    return KeyboardButton::ascii_key('o');
  case XK_p: 
    return KeyboardButton::ascii_key('p');
  case XK_q: 
    return KeyboardButton::ascii_key('q');
  case XK_r: 
    return KeyboardButton::ascii_key('r');
  case XK_s: 
    return KeyboardButton::ascii_key('s');
  case XK_t: 
    return KeyboardButton::ascii_key('t');
  case XK_u: 
    return KeyboardButton::ascii_key('u');
  case XK_v: 
    return KeyboardButton::ascii_key('v');
  case XK_w: 
    return KeyboardButton::ascii_key('w');
  case XK_x: 
    return KeyboardButton::ascii_key('x');
  case XK_y: 
    return KeyboardButton::ascii_key('y');
  case XK_z: 
    return KeyboardButton::ascii_key('z');
  case XK_braceleft: 
    return KeyboardButton::ascii_key('{');
  case XK_bar: 
    return KeyboardButton::ascii_key('|');
  case XK_braceright: 
    return KeyboardButton::ascii_key('}');
  case XK_asciitilde: 
    return KeyboardButton::ascii_key('~');
  case XK_F1:
    return KeyboardButton::f1();
  case XK_F2:
    return KeyboardButton::f2();
  case XK_F3:
    return KeyboardButton::f3();
  case XK_F4:
    return KeyboardButton::f4();
  case XK_F5:
    return KeyboardButton::f5();
  case XK_F6:
    return KeyboardButton::f6();
  case XK_F7:
    return KeyboardButton::f7();
  case XK_F8:
    return KeyboardButton::f8();
  case XK_F9:
    return KeyboardButton::f9();
  case XK_F10:
    return KeyboardButton::f10();
  case XK_F11:
    return KeyboardButton::f11();
  case XK_F12:
    return KeyboardButton::f12();
  case XK_KP_Left:
  case XK_Left:
    return KeyboardButton::left();
  case XK_KP_Up:
  case XK_Up:
    return KeyboardButton::up();
  case XK_KP_Right:
  case XK_Right:
    return KeyboardButton::right();
  case XK_KP_Down:
  case XK_Down:
    return KeyboardButton::down();
  case XK_KP_Prior:
  case XK_Prior:
    return KeyboardButton::page_up();
  case XK_KP_Next:
  case XK_Next:
    return KeyboardButton::page_down();
  case XK_KP_Home:
  case XK_Home:
    return KeyboardButton::home();
  case XK_KP_End:
  case XK_End:
    return KeyboardButton::end();
  case XK_KP_Insert:
  case XK_Insert:
    return KeyboardButton::insert();
  case XK_KP_Delete:
  case XK_Delete:
    return KeyboardButton::del();
  case XK_Shift_L:
  case XK_Shift_R:
    return KeyboardButton::shift();
  case XK_Control_L:
  case XK_Control_R:
    return KeyboardButton::control();
  case XK_Alt_L:
  case XK_Alt_R:
    return KeyboardButton::alt();
  case XK_Meta_L:
  case XK_Meta_R:
    return KeyboardButton::meta();
  case XK_Caps_Lock:
    return KeyboardButton::caps_lock();
  case XK_Shift_Lock:
    return KeyboardButton::shift_lock();
  }

  return ButtonHandle::none();
}

////////////////////////////////////////////////////////////////////
//     Function: interruptible_xnextevent  
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
int glxGraphicsWindow::interruptible_xnextevent(Display* display, XEvent* event)
{
  XFlush(display);
  if (XPending(display)) {
    XNextEvent(display, event);
    return 1;
  } else 
    return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: handle_changes
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::handle_changes(void)
{
  // As an optimization, check to see if change is anything other than a
  // redisplay
  if (_change_mask & (GLXWIN_CONFIGURE | GLXWIN_EVENT)) {
    if (_change_mask & GLXWIN_CONFIGURE) {
      XWindowChanges changes;
      changes.x = _props._xorg;
      changes.y = _props._yorg;
      if (_configure_mask & (CWWidth | CWHeight)) {
        changes.width = _props._xsize;
        changes.height = _props._ysize;
      }
      XConfigureWindow(_display, _xwindow, _configure_mask, &changes);
      _configure_mask = 0;
    }
    if (_change_mask & GLXWIN_EVENT) {
      XSelectInput(_display, _xwindow, _event_mask);
    }
  }

  _change_mask = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: process_event
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::process_event(XEvent event)
{
  switch (event.type) {
  case MappingNotify:
    break;
  case ConfigureNotify:
    if (_props._xsize != event.xconfigure.width ||
	_props._ysize != event.xconfigure.height) {
      _props._xsize = event.xconfigure.width;
      _props._ysize = event.xconfigure.height;
      make_current();
      // Do not execute OpenGL out of sequence with respect to the
      // XResizeWindow request
      glXWaitX();
      handle_reshape(event.xconfigure.width, event.xconfigure.height);
      _change_mask |= GLXWIN_CONFIGURE;
    }
    break;
  case Expose:
    break;

  case ButtonPress:
    make_current();
    handle_keypress(MouseButton::button(event.xbutton.button - 1),
		    event.xkey.x, event.xkey.y);
    break;

  case ButtonRelease:
    make_current();
    handle_keyrelease(MouseButton::button(event.xbutton.button - 1),
		      event.xkey.x, event.xkey.y);
    break;

  case MotionNotify:
    if (_mouse_motion_enabled && event.xmotion.state &
	(Button1Mask | Button2Mask | Button3Mask)) {
      make_current();
      handle_mouse_motion(event.xmotion.x, event.xmotion.y);
    } else if (_mouse_passive_motion_enabled &&
	       ((event.xmotion.state &
		 (Button1Mask | Button2Mask | Button3Mask)) == 0)) {
      make_current();
      handle_mouse_motion(event.xmotion.x, event.xmotion.y);
    }
    break;

  case KeyPress:
    make_current();
    handle_keypress(lookup_key(event), event.xkey.x, event.xkey.y);
    break;

  case KeyRelease:
    make_current();
    handle_keyrelease(lookup_key(event), event.xkey.x, event.xkey.y);
    break;

  case EnterNotify:
  case LeaveNotify:
    if (_mouse_entry_enabled) {
      if (event.type == EnterNotify) {
	// Overlays can generate multiple enter events
	if (_entry_state != EnterNotify) {
	  _entry_state = EnterNotify;
	  make_current();
	  handle_mouse_entry(MOUSE_ENTERED);
	  if (_mouse_passive_motion_enabled) {
	    handle_mouse_motion(event.xcrossing.x, event.xcrossing.y);
	  }
	}
      } else { // event.type == LeaveNotify
	if (_entry_state != LeaveNotify) {
	  _entry_state = LeaveNotify;
	  make_current();
	  handle_mouse_entry(MOUSE_EXITED);
	}
      }
    } else if (_mouse_passive_motion_enabled) {
      make_current();
      handle_mouse_motion(event.xcrossing.x, event.xcrossing.y);
    }
    break;
  case UnmapNotify:
  case VisibilityNotify:
  case ClientMessage:
  case DestroyNotify:
  case CirculateNotify:
  case CreateNotify:
  case GravityNotify:
  case ReparentNotify:
    break;
  default:
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: process_events 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::process_events(void)
{
  XEvent event;
  int got_event;
  glxGraphicsWindow* window;

  glxDisplay *glx = _pipe->get_glx_display();
  nassertv(glx != (glxDisplay *)NULL);

  do {
    got_event = interruptible_xnextevent(_display, &event);
    if (got_event) {
      switch (event.type) {
        case MappingNotify:
	  XRefreshKeyboardMapping((XMappingEvent *) &event);
	  break;
        case ConfigureNotify:
	  if ((window = glx->find_window(event.xconfigure.window)) != NULL)
	    window->process_event(event);
	  break;
        case Expose:
	  XEvent ahead;
	  while (XEventsQueued(_display, QueuedAfterReading) > 0) {
	    XPeekEvent(_display, &ahead);
	    if (ahead.type != Expose || 
		ahead.xexpose.window != event.xexpose.window) {
	      break;
	    }
	    XNextEvent(_display, &event);
	  }
          break;
        case ButtonPress:
        case ButtonRelease:
	  if ((window = glx->find_window(event.xbutton.window)) != NULL)
	    window->process_event(event);
	  break;
        case MotionNotify:
	  if ((window = glx->find_window(event.xmotion.window)) != NULL)
	    window->process_event(event);
          break;
        case KeyPress:
        case KeyRelease:
	  if ((window = glx->find_window(event.xmotion.window)) != NULL)
	    window->process_event(event);
	  break;
        case EnterNotify:
        case LeaveNotify:
	  if (event.xcrossing.mode != NotifyNormal ||
	      event.xcrossing.mode == NotifyNonlinearVirtual ||
	      event.xcrossing.mode == NotifyVirtual) {
	    // Ignore "virtual" window enter/leave events
	    break;
	  }
  	  if ((window = glx->find_window(event.xcrossing.window)) != NULL)
	    window->process_event(event);
	  break;
        case UnmapNotify:
        case VisibilityNotify:
	  break;
        case ClientMessage:
	  break;
        case DestroyNotify:
        case CirculateNotify:
        case CreateNotify:
        case GravityNotify:
        case ReparentNotify:
	  break;
        default:
	  break;
      }
    }
  }
  while (XPending(_display));
}

////////////////////////////////////////////////////////////////////
//     Function: idle_wait 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::idle_wait(void)
{
  if (XPending(_display))
    process_events();

  call_idle_callback();
}


////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsWindow::supports_update
//       Access: Public, Virtual
//  Description: Returns true if this particular kind of
//               GraphicsWindow supports use of the update() function
//               to update the graphics one frame at a time, so that
//               the window does not need to be the program's main
//               loop.  Returns false if the only way to update the
//               window is to call main_loop().
////////////////////////////////////////////////////////////////////
bool glxGraphicsWindow::
supports_update() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: update
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::update(void)
{
  _show_code_pcollector.stop();
  PStatClient::main_tick();

  if (_change_mask)
    handle_changes();

  make_current();

  // Always ask for a redisplay for now
  call_draw_callback(true);

  if (_idle_callback) 
    idle_wait();
  else
    process_events();
  _show_code_pcollector.start();
}

////////////////////////////////////////////////////////////////////
//     Function: enable_mouse_input 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::enable_mouse_input(bool val)
{
  long emask = ButtonPressMask | ButtonReleaseMask;

  // See if we need to enable or disable mouse events
  if (_mouse_input_enabled == false && val == true)
    add_event_mask(emask);
  else if (_mouse_input_enabled == true && val == false)
    remove_event_mask(emask);
  _mouse_input_enabled = val;
}

////////////////////////////////////////////////////////////////////
//     Function: enable_mouse_motion 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::enable_mouse_motion(bool val)
{
  // Some window managers (4Dwm by default) will mask motion events unless
  // button presses are selected as well
  long input_mask = ButtonPressMask | ButtonReleaseMask;
  long motion_mask = Button1MotionMask | Button2MotionMask | Button3MotionMask;
  if (_mouse_motion_enabled == false && val == true) {
    add_event_mask(input_mask);
    add_event_mask(motion_mask);
  }
  else if (_mouse_motion_enabled == true && val == false) { 
    remove_event_mask(motion_mask);
    if (_mouse_input_enabled == false)
      remove_event_mask(input_mask);
  }
  _mouse_motion_enabled = val;
}

////////////////////////////////////////////////////////////////////
//     Function: enable_mouse_passive_motion
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::enable_mouse_passive_motion(bool val)
{
  long entry_mask = EnterWindowMask | LeaveWindowMask;
  if (_mouse_passive_motion_enabled == false && val == true) {
    add_event_mask(PointerMotionMask);
    // Passive motion requires watching enters and leaves so a fake passive
    // motion event can be generated on an enter
    if (_mouse_entry_enabled == false)
      add_event_mask(entry_mask);
  }
  else if (_mouse_passive_motion_enabled == true && val == false) {
    remove_event_mask(PointerMotionMask);
    if (_mouse_entry_enabled == false)
      remove_event_mask(entry_mask);
  }
  _mouse_passive_motion_enabled = val;
}

////////////////////////////////////////////////////////////////////
//     Function: enable_mouse_entry
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::enable_mouse_entry(bool val)
{
  if (_mouse_entry_enabled == false && val == true) {
    add_event_mask(EnterWindowMask | LeaveWindowMask);
  }
  else if (_mouse_entry_enabled == true && val == false) {
    remove_event_mask(EnterWindowMask | LeaveWindowMask);
  }
  _mouse_entry_enabled = val;
}

////////////////////////////////////////////////////////////////////
//     Function: make_current
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::make_current(void) {
  PStatTimer timer(_make_current_pcollector);
  glXMakeCurrent(_display, _xwindow, _context);
}

////////////////////////////////////////////////////////////////////
//     Function: unmake_current
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void glxGraphicsWindow::unmake_current(void) {
  glXMakeCurrent(_display, None, NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: glxGraphicsWindow::get_gsg_type
//       Access: Public, Virtual
//  Description: Returns the TypeHandle of the kind of GSG preferred
//               by this kind of window.
////////////////////////////////////////////////////////////////////
TypeHandle glxGraphicsWindow::
get_gsg_type() const {
  return GLGraphicsStateGuardian::get_class_type();
}

GraphicsWindow *glxGraphicsWindow::
make_GlxGraphicsWindow(const FactoryParams &params) {
  GraphicsWindow::WindowPipe *pipe_param;
  if (!get_param_into(pipe_param, params)) {
    glxdisplay_cat.error()
      << "No pipe specified for window creation!" << endl;
    return NULL;
  }

  GraphicsPipe *pipe = pipe_param->get_pipe();
  
  GraphicsWindow::WindowProps *props_param;
  if (!get_param_into(props_param, params)) {
    return new glxGraphicsWindow(pipe);
  } else {
    return new glxGraphicsWindow(pipe, props_param->get_properties());
  }
}

TypeHandle glxGraphicsWindow::get_class_type(void) {
  return _type_handle;
}

void glxGraphicsWindow::init_type(void) {
  GraphicsWindow::init_type();
  register_type(_type_handle, "glxGraphicsWindow",
		GraphicsWindow::get_class_type());
}

TypeHandle glxGraphicsWindow::get_type(void) const {
  return get_class_type();
}
