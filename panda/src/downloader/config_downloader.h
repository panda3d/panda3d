// Filename: config_downloader.h
// Created by:  mike (19Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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

extern ConfigVariableInt downloader_disk_write_frequency;
extern ConfigVariableInt downloader_byte_rate;
extern ConfigVariableDouble downloader_frequency;
extern ConfigVariableInt downloader_timeout;
extern ConfigVariableInt downloader_timeout_retries;

extern ConfigVariableInt decompressor_buffer_size;
extern ConfigVariableDouble decompressor_frequency;

extern ConfigVariableInt extractor_buffer_size;
extern ConfigVariableDouble extractor_frequency;

extern ConfigVariableInt patcher_buffer_size;

extern ConfigVariableBool verify_ssl;
extern ConfigVariableString ssl_cipher_list;
extern ConfigVariableList expected_ssl_server;
extern ConfigVariableList ssl_certificates;

extern ConfigVariableString http_proxy;
extern ConfigVariableString http_direct_hosts;
extern ConfigVariableBool http_try_all_direct;
extern ConfigVariableString http_proxy_username;
extern ConfigVariableBool http_proxy_tunnel;
extern ConfigVariableDouble http_connect_timeout;
extern ConfigVariableDouble http_timeout;
extern ConfigVariableInt http_max_connect_count;
extern ConfigVariableFilename http_client_certificate_filename;
extern ConfigVariableString http_client_certificate_passphrase;
extern ConfigVariableList http_username;

extern EXPCL_PANDAEXPRESS void init_libdownloader();

#endif
