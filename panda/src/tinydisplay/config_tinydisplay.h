/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_tinydisplay.h
 * @author drose
 * @date 2008-04-24
 */

#ifndef CONFIG_TINYDISPLAY_H
#define CONFIG_TINYDISPLAY_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableString.h"
#include "configVariableBool.h"
#include "configVariableInt.h"

NotifyCategoryDecl(tinydisplay, EXPCL_TINYDISPLAY, EXPTP_TINYDISPLAY);

extern EXPCL_TINYDISPLAY void init_libtinydisplay();
extern "C" EXPCL_TINYDISPLAY int get_pipe_type_p3tinydisplay();

extern ConfigVariableInt td_texture_ram;
extern ConfigVariableBool td_ignore_mipmaps;
extern ConfigVariableBool td_ignore_clamp;
extern ConfigVariableBool td_perspective_textures;

#endif
