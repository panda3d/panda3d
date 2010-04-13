////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef __CONFIG_OSXDISPLAY_H__
#define __CONFIG_OSXDISPLAY_H__

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableInt.h"

NotifyCategoryDecl( osxdisplay , EXPCL_PANDAGL, EXPTP_PANDAGL);

extern EXPCL_PANDAGL void init_libosxdisplay();

extern ConfigVariableBool show_resize_box;
extern ConfigVariableBool osx_support_gl_buffer;
extern ConfigVariableBool osx_disable_event_loop;
extern ConfigVariableInt osx_mouse_wheel_scale;

#endif /* __CONFIG_OSXDISPLAY_H__ */
