/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_ffmpeg.h
 * @author rdb
 * @date 2013-08-23
 */

#ifndef CONFIG_FFMPEG_H
#define CONFIG_FFMPEG_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "configVariableEnum.h"
#include "configVariableInt.h"
#include "configVariableBool.h"
#include "threadPriority.h"
#include "dconfig.h"

ConfigureDecl(config_ffmpeg, EXPCL_FFMPEG, EXPTP_FFMPEG);
NotifyCategoryDecl(ffmpeg, EXPCL_FFMPEG, EXPTP_FFMPEG);

extern ConfigVariableInt ffmpeg_max_readahead_frames;
extern ConfigVariableBool ffmpeg_show_seek_frames;
extern ConfigVariableBool ffmpeg_support_seek;
extern ConfigVariableBool ffmpeg_global_lock;
extern ConfigVariableEnum<ThreadPriority> ffmpeg_thread_priority;
extern ConfigVariableInt ffmpeg_read_buffer_size;
extern ConfigVariableBool ffmpeg_prefer_libvpx;

extern EXPCL_FFMPEG void init_libffmpeg();

#endif /* CONFIG_FFMPEG_H */
