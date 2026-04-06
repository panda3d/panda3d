/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_windisplay.h
 * @author drose
 * @date 2002-12-20
 */

#ifndef CONFIG_WINDISPLAY_H
#define CONFIG_WINDISPLAY_H

#include "pandabase.h"
#include "filename.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"

NotifyCategoryDecl(windisplay, EXPCL_PANDAWIN, EXPTP_PANDAWIN);

extern ConfigVariableBool do_vidmemsize_check;
extern ConfigVariableBool auto_cpu_data;
extern ConfigVariableBool ime_hide;
extern ConfigVariableBool request_dxdisplay_information;
extern ConfigVariableBool dpi_aware;
extern ConfigVariableBool dpi_window_resize;
extern ConfigVariableBool paste_emit_keystrokes;
extern ConfigVariableBool disable_message_loop;

extern EXPCL_PANDAWIN void init_libwindisplay();

#endif
