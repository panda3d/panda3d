/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file iPhoneGraphicsPipe.mm
 * @author drose
 * @date 2009-04-08
 */

#include "iPhoneGraphicsPipe.h"
#include "config_iphonedisplay.h"
#include "iPhoneGraphicsWindow.h"
#include "iPhoneGraphicsStateGuardian.h"
#include "pnmImage.h"
#include "graphicsOutput.h"

IPhoneGraphicsPipe *IPhoneGraphicsPipe::_global_ptr;
TypeHandle IPhoneGraphicsPipe::_type_handle;


/**
 *
 */
IPhoneGraphicsPipe::
IPhoneGraphicsPipe() {
  CGRect screenBounds = [ [ UIScreen mainScreen ] bounds ];

  _window = [ [ UIWindow alloc ] initWithFrame: screenBounds ];
  _view_controller = [ [ ControllerDemoViewController alloc ] initWithPipe: this ];

  [ _window addSubview:_view_controller.view ];
  [ _window makeKeyAndVisible ];
}

/**
 *
 */
IPhoneGraphicsPipe::
~IPhoneGraphicsPipe() {
  [_view_controller release];
  [_window release];
}

/**
 * Returns the name of the rendering interface associated with this
 * GraphicsPipe.  This is used to present to the user to allow him/her to
 * choose between several possible GraphicsPipes available on a particular
 * platform, so the name should be meaningful and unique for a given platform.
 */
std::string IPhoneGraphicsPipe::
get_interface_name() const {
  return "OpenGL ES";
}

/**
 * This function is passed to the GraphicsPipeSelection object to allow the
 * user to make a default IPhoneGraphicsPipe.
 */
PT(GraphicsPipe) IPhoneGraphicsPipe::
pipe_constructor() {
  // There is only one IPhoneGraphicsPipe in the universe for any given
  // application.  Even if you ask for a new one, you just get the same one
  // you had before.
  if (_global_ptr == (IPhoneGraphicsPipe *)NULL) {
    _global_ptr = new IPhoneGraphicsPipe;
    _global_ptr->ref();
  }
  return _global_ptr;
}

/**
 * Returns an indication of the thread in which this GraphicsPipe requires its
 * window processing to be performed: typically either the app thread (e.g.
 * X) or the draw thread (Windows).
 */
GraphicsPipe::PreferredWindowThread
IPhoneGraphicsPipe::get_preferred_window_thread() const {
  return PWT_app;
}

/**
 * Called in response to an orientation change event, this tells all of the
 * windows created on the pipe to resize themselves according to the new
 * orientation.
 */
void IPhoneGraphicsPipe::
rotate_windows() {
  GraphicsWindows::iterator gwi;
  for (gwi = _graphics_windows.begin(); gwi != _graphics_windows.end(); ++gwi) {
    IPhoneGraphicsWindow *win = (*gwi);
    win->rotate_window();
  }
}


/**
 * Creates a new window on the pipe, if possible.
 */
PT(GraphicsOutput) IPhoneGraphicsPipe::
make_output(const string &name,
            const FrameBufferProperties &fb_prop,
            const WindowProperties &win_prop,
            int flags,
            GraphicsEngine *engine,
            GraphicsStateGuardian *gsg,
            GraphicsOutput *host,
            int retry,
            bool &precertify) {
  if (!_is_valid) {
    return NULL;
  }

  IPhoneGraphicsStateGuardian *iphonegsg = 0;
  if (gsg != 0) {
    DCAST_INTO_R(iphonegsg, gsg, NULL);
  }

  // First thing to try: an IPhoneGraphicsWindow

  if (retry == 0) {
    if (((flags&BF_require_parasite)!=0)||
        ((flags&BF_refuse_window)!=0)||
        ((flags&BF_resizeable)!=0)||
        ((flags&BF_size_track_host)!=0)||
        ((flags&BF_can_bind_color)!=0)||
        ((flags&BF_can_bind_every)!=0)) {
      return NULL;
    }
    return new IPhoneGraphicsWindow(engine, this, name, fb_prop, win_prop,
                                    flags, gsg, host);
  }

  // Nothing else left to try.
  return NULL;
}
