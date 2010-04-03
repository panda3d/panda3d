// Filename: config_downloader.h
// Created by:  mike (19Mar00)
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

#ifndef CONFIG_DOWNLOADER_H
#define CONFIG_DOWNLOADER_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"
#include "configVariableInt.h"
#include "configVariableDouble.h"
#include "configVariableBool.h"
#include "configVariableString.h"
#include "configVariableFilename.h"
#include "configVariableList.h"

ConfigureDecl(config_downloader, EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS);
NotifyCategoryDecl(downloader, EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS);

extern ConfigVariableInt downloader_byte_rate;
extern ConfigVariableBool download_throttle;
extern ConfigVariableDouble downloader_frequency;
extern ConfigVariableInt downloader_timeout;
extern ConfigVariableInt downloader_timeout_retries;

extern ConfigVariableDouble decompressor_step_time;
extern ConfigVariableDouble extractor_step_time;
extern ConfigVariableInt patcher_buffer_size;

extern ConfigVariableBool http_proxy_tunnel;
extern ConfigVariableDouble http_connect_timeout;
extern ConfigVariableDouble http_timeout;
extern ConfigVariableInt http_skip_body_size;
extern ConfigVariableDouble http_idle_timeout;
extern ConfigVariableInt http_max_connect_count;

extern EXPCL_PANDAEXPRESS ConfigVariableInt tcp_header_size;

extern EXPCL_PANDAEXPRESS void init_libdownloader();

#endif
