// Filename: config_downloader.h
// Created by:  mike (19Mar00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef CONFIG_DOWNLOADER_H
#define CONFIG_DOWNLOADER_H

#include "pandabase.h"
#include "notifyCategoryProxy.h"
#include "dconfig.h"

ConfigureDecl(config_downloader, EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS);
NotifyCategoryDecl(downloader, EXPCL_PANDAEXPRESS, EXPTP_PANDAEXPRESS);

extern const int downloader_disk_write_frequency;
extern const int downloader_byte_rate;
extern const float downloader_frequency;
extern const int downloader_timeout;
extern const int downloader_timeout_retries;

extern const int decompressor_buffer_size;
extern const float decompressor_frequency;

extern const int extractor_buffer_size;
extern const float extractor_frequency;

extern const int patcher_buffer_size;

extern const bool early_random_seed;
extern const bool verify_ssl;
extern const string ssl_cipher_list;
extern const string http_proxy;
extern const string http_direct_hosts;
extern const bool http_try_all_direct;
extern const string http_proxy_username;
extern const bool http_proxy_tunnel;
extern const double http_connect_timeout;
extern const double http_timeout;
extern const int http_max_connect_count;
extern const string http_client_certificate_filename;
extern const string http_client_certificate_passphrase;

#endif
