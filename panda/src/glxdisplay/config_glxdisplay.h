/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_glxdisplay.h
 * @author cary
 * @date 1999-10-07
 */

#ifndef __CONFIG_GLXDISPLAY_H__
#define __CONFIG_GLXDISPLAY_H__

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableString.h"
#include "configVariableBool.h"
#include "configVariableInt.h"

NotifyCategoryDecl(glxdisplay, EXPCL_PANDA_GLXDISPLAY, EXPTP_PANDA_GLXDISPLAY);

extern EXPCL_PANDA_GLXDISPLAY void init_libglxdisplay();

extern ConfigVariableBool glx_get_proc_address;
extern ConfigVariableBool glx_get_os_address;

extern ConfigVariableBool glx_support_fbconfig;
extern ConfigVariableBool glx_support_pbuffer;
extern ConfigVariableBool glx_support_pixmap;

#endif /* __CONFIG_GLXDISPLAY_H__ */
