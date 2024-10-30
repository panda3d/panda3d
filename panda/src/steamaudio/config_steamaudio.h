/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_steamaudio.h
 * @author Jackson Sutherland
 */

#ifndef CONFIG_STEAMAUDIO_H
#define CONFIG_STEAMAUDIO_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "audioManager.h"

ConfigureDecl(config_steamaudio, EXPTP_PANDA_STEAMAUDIO, EXPTP_PANDA_STEAMAUDIO);
NotifyCategoryDecl(steamaudio, EXPCL_PANDA_STEAMAUDIO, EXPTP_PANDA_STEAMAUDIO);

extern "C" EXPCL_PANDA_STEAMAUDIO void init_libsteamaudio();

#endif
