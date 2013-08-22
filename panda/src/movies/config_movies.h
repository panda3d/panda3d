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
#include "configVariableInt.h"
#include "configVariableBool.h"
#include "threadPriority.h"
#include "dconfig.h"

ConfigureDecl(config_movies, EXPCL_PANDA_MOVIES, EXPTP_PANDA_MOVIES);
NotifyCategoryDecl(movies, EXPCL_PANDA_MOVIES, EXPTP_PANDA_MOVIES);
NotifyCategoryDecl(ffmpeg, EXPCL_PANDA_MOVIES, EXPTP_PANDA_MOVIES);

extern ConfigVariableInt ffmpeg_max_readahead_frames;
extern ConfigVariableBool ffmpeg_show_seek_frames;
extern ConfigVariableBool ffmpeg_support_seek;
extern ConfigVariableBool ffmpeg_global_lock;
extern ConfigVariableEnum<ThreadPriority> ffmpeg_thread_priority;
extern ConfigVariableInt ffmpeg_read_buffer_size;

extern EXPCL_PANDA_MOVIES void init_libmovies();

#endif /* __CONFIG_MOVIES_H__ */
