// Filename: config_movies.h
// Created by:  jyelon (02Jul07)
//
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

#ifndef CONFIG_MOVIES_H
#define CONFIG_MOVIES_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableBool.h"
#include "configVariableList.h"
#include "threadPriority.h"
#include "dconfig.h"

ConfigureDecl(config_movies, EXPCL_PANDA_MOVIES, EXPTP_PANDA_MOVIES);
NotifyCategoryDecl(movies, EXPCL_PANDA_MOVIES, EXPTP_PANDA_MOVIES);

extern ConfigVariableList load_audio_type;
extern ConfigVariableList load_video_type;

extern ConfigVariableBool vorbis_enable_seek;
extern ConfigVariableBool vorbis_seek_lap;

extern EXPCL_PANDA_MOVIES void init_libmovies();

#endif /* CONFIG_MOVIES_H */
