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

#ifndef __CONFIG_MOVIES_H__
#define __CONFIG_MOVIES_H__

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableEnum.h"
#include "configVariableDouble.h"
#include "dconfig.h"

#include "movieVideo.h"
#include "movieVideoCursor.h"

#include "movieAudio.h"
#include "movieAudioCursor.h"

#include "inkblotVideo.h"
#include "inkblotVideoCursor.h"

#include "ffmpegVideo.h"
#include "ffmpegVideoCursor.h"

#include "ffmpegAudio.h"
#include "ffmpegAudioCursor.h"

ConfigureDecl(config_movies, EXPCL_PANDA_MOVIES, EXPTP_PANDA_MOVIES);
NotifyCategoryDecl(movies, EXPCL_PANDA_MOVIES, EXPTP_PANDA_MOVIES);

extern EXPCL_PANDA_MOVIES void init_libmovies();

#endif /* __CONFIG_MOVIES_H__ */
