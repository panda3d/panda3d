// Filename: config_downloader.cxx
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

#include "dconfig.h"
#include "get_config_path.h"
#include "config_downloader.h"
#include "httpChannel.h"


ConfigureDef(config_downloader);
NotifyCategoryDef(downloader, "");

// How often we write to disk is determined by this ratio which is
// relative to the downloader-byte-rate (e.g. if disk-write-ratio is 4,
// we will write every 4 seconds if the frequency is 0.2)
const int downloader_disk_write_frequency =
        config_downloader.GetInt("downloader-disk-write-frequency", 4);

// We'd like this to be about 1 second worth of download assuming a
// 28.8Kb connection (28.8Kb / 8 = 3600 bytes per second).
const int downloader_byte_rate =
        config_downloader.GetInt("downloader-byte-rate", 3600);

// Frequency of download chunk requests in seconds (or fractions of)
// (Estimated 200 msec round-trip to server).
const float downloader_frequency =
        config_downloader.GetFloat("downloader-frequency", 0.2);

const int downloader_timeout =
        config_downloader.GetInt("downloader-timeout", 15);

const int downloader_timeout_retries =
        config_downloader.GetInt("downloader-timeout-retries", 5);

const int decompressor_buffer_size =
        config_downloader.GetInt("decompressor-buffer-size", 4096);

const float decompressor_frequency =
        config_downloader.GetFloat("decompressor-frequency", 0.2);

const int extractor_buffer_size =
        config_downloader.GetInt("extractor-buffer-size", 4096);

const float extractor_frequency =
        config_downloader.GetFloat("extractor-frequency", 0.2);

const int patcher_buffer_size =
        config_downloader.GetInt("patcher-buffer-size", 4096);

// Configure this true (the default) to compute the SSL random seed
// early on in the application (specifically, when the first
// HTTPClient is created), or false to defer this until it is actually
// needed, causing a delay the first time a https connection is
// attempted.
const bool early_random_seed =
config_downloader.GetBool("early-random-seed", true);

// Configure this true (the default) to insist on verifying all SSL
// (e.g. https) servers against a known certificate, or false to allow
// an unverified connection.  This controls the default behavior; the
// specific behavior for a particular HTTPClient can be adjusted at
// runtime with set_verify_ssl().
const bool verify_ssl =
config_downloader.GetBool("verify-ssl", true);

// This is the default value for HTTPClient::set_cipher_list().
const string ssl_cipher_list =
config_downloader.GetString("ssl-cipher-list", "DEFAULT");

// This specifies the proxy that we will contact for all HTTP
// connections that don't specify otherwise.
const string http_proxy =
config_downloader.GetString("http-proxy", "");
const string http_direct_hosts =
config_downloader.GetString("http-direct-hosts", "");
const bool http_try_all_direct =
config_downloader.GetBool("http-try-all-direct", true);
const string http_proxy_username =
config_downloader.GetString("http-proxy-username", "");
const bool http_proxy_tunnel =
config_downloader.GetBool("http-proxy-tunnel", false);

// This is the default amount of time to wait for a TCP/IP connection
// to be established, in seconds.
const double http_connect_timeout =
config_downloader.GetDouble("http-connect-timeout", 10.0);

// This is the default amount of time to wait for the HTTP server to
// finish sending its response to our request, in seconds.  It starts
// counting after the TCP connection has been established
// (http_connect_timeout, above) and the request has been sent.
const double http_timeout =
config_downloader.GetDouble("http-timeout", 20.0);

// This is the maximum number of times to try reconnecting to the
// server on any one document attempt.  This is just a failsafe to
// prevent the code from attempting runaway connections; this limit
// should never be reached in practice.
const int http_max_connect_count =
config_downloader.GetInt("http-max-connect-count", 10);

// These provide a default client certificate to offer up should an
// SSL server demand one.  The files references a PEM-formatted file
// that includes a public and private key specification.  A
// connection-specific certificate may also be specified at runtime on
// the HTTPClient object, but this will require having a different
// HTTPClient object for each differently-certificated connection.
const Filename http_client_certificate_filename =
Filename::expand_from(config_downloader.GetString("http-client-certificate-filename", ""));
const string http_client_certificate_passphrase =
config_downloader.GetString("http-client-certificate-passphrase", "");

ConfigureFn(config_downloader) {
#ifdef HAVE_SSL
  HTTPChannel::init_type();
#endif
}
