/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_downloader.cxx
 * @author mike
 * @date 2000-03-19
 */

#include "dconfig.h"
#include "config_downloader.h"
#include "httpChannel.h"
#include "virtualFileHTTP.h"
#include "virtualFileMountHTTP.h"
#include "pandaSystem.h"


#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_DOWNLOADER)
  #error Buildsystem error: BUILDING_PANDA_DOWNLOADER not defined
#endif

ConfigureDef(config_downloader);
NotifyCategoryDef(downloader, "");

ConfigVariableInt downloader_byte_rate
("downloader-byte-rate", 500000,
 PRC_DESC("Specifies the default max bytes per second of throughput that is "
          "supported by any HTTP connections with download-throttle enabled.  "
          "This may also be set on a per-channel basis with "
          "HTTPChannel::set_max_bytes_per_second().  It has no effect unless "
          "download-throttle (or HTTPChannel::set_download_throttle) is true."));

ConfigVariableBool download_throttle
("download-throttle", false,
 PRC_DESC("When this is true, all HTTP channels will be bandwidth-limited "
          "so as not to consume more than downloader-byte-rate bytes per "
          "second."));

ConfigVariableDouble downloader_frequency
("downloader-frequency", 0.2,
 PRC_DESC("Frequency of download chunk requests in seconds (or fractions of) "
          "(Estimated 200 msec round-trip to server)."));

ConfigVariableInt downloader_timeout
("downloader-timeout", 15);

ConfigVariableInt downloader_timeout_retries
("downloader-timeout-retries", 5);

ConfigVariableDouble decompressor_step_time
("decompressor-step-time", 0.005,
 PRC_DESC("Specifies the maximum amount of time that should be consumed by "
          "a single call to Decompressor::run()."));

ConfigVariableDouble extractor_step_time
("extractor-step-time", 0.005,
 PRC_DESC("Specifies the maximum amount of time that should be consumed by "
          "a single call to Extractor::step()."));

ConfigVariableInt patcher_buffer_size
("patcher-buffer-size", 16384,
  PRC_DESC("Limits the size of the buffer used in a single call to "
           "Patcher::run().  Increasing this may help the Patcher "
           "perform more work before returning."));

ConfigVariableBool http_proxy_tunnel
("http-proxy-tunnel", false,
 PRC_DESC("This specifies the default value for HTTPChannel::set_proxy_tunnel().  "
          "If this is true, we will tunnel through a proxy for all connections, "
          "instead of asking the proxy to serve documents normally."));

ConfigVariableDouble http_connect_timeout
("http-connect-timeout", 10.0,
 PRC_DESC("This is the default amount of time to wait for a TCP/IP connection "
          "to be established, in seconds."));

ConfigVariableDouble http_timeout
("http-timeout", 20.0,
 PRC_DESC("This is the default amount of time to wait for the HTTP server (or "
          "proxy) to finish sending its response to our request, in seconds. "
          "It starts counting after the TCP connection has been established "
          "(http_connect_timeout, above) and the request has been sent."));

ConfigVariableInt http_skip_body_size
("http-skip-body-size", 8192,
 PRC_DESC("This is the maximum number of bytes in a received "
          "(but unwanted) body that will be skipped past, in "
          "order to reset to a new request.  "
          "See HTTPChannel::set_skip_body_size()."));

ConfigVariableDouble http_idle_timeout
("http-idle-timeout", 5.0,
 PRC_DESC("This the amount of time, in seconds, in which a "
          "previously-established connection is allowed to remain open "
          "and unused.  If a previous connection has remained unused for "
          "at least this number of seconds, it will be closed and a new "
          "connection will be opened; otherwise, the same connection "
          "will be reused for the next request (for a particular "
          "HTTPChannel)."));

ConfigVariableInt http_max_connect_count
("http-max-connect-count", 10,
 PRC_DESC("This is the maximum number of times to try reconnecting to the "
          "server on any one document attempt.  This is just a failsafe to "
          "prevent the code from attempting runaway connections; this limit "
          "should never be reached in practice."));

ConfigVariableInt tcp_header_size
("tcp-header-size", 2,
 PRC_DESC("Specifies the number of bytes to use to specify the datagram "
          "length when writing a datagram on a TCP stream.  This may be "
          "0, 2, or 4.  The server and client must agree on this value."));

ConfigVariableBool support_ipv6
("support-ipv6", true,
 PRC_DESC("Specifies whether IPv6 support should be enabled.  This should "
          "be true unless you are experiencing issues with Panda's IPv6 "
          "support or are using a misconfigured system."));

ConfigureFn(config_downloader) {
  init_libdownloader();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libdownloader() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

#ifdef HAVE_OPENSSL
  HTTPChannel::init_type();
  VirtualFileHTTP::init_type();
  VirtualFileMountHTTP::init_type();

  VirtualFileMountHTTP::reload_vfs_mount_url();

  // We need to define this here, rather than above, to guarantee that it has
  // been initialized by the time we check it.
  ConfigVariableBool early_random_seed
    ("early-random-seed", false,
     PRC_DESC("Configure this true to compute the SSL random seed "
              "early on in the application (specifically, when the libpandaexpress "
              "library is loaded), or false to defer this until it is actually "
              "needed (which will be the first time you open an https connection "
              "or otherwise use encryption services).  You can also call "
              "HTTPClient::init_random_seed() to "
              "do this when you are ready.  The issue is that on Windows, "
              "OpenSSL will attempt to "
              "randomize its seed by crawling through the entire heap of "
              "allocated memory, which can be extremely large in a Panda "
              "application, especially if you have already opened a window and "
              "started rendering; and so this can take as much as 30 seconds "
              "or more.  For this reason it is best to initialize the random "
              "seed at startup, when the application is still very small."));
  if (early_random_seed) {
    HTTPClient::init_random_seed();
  }

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("OpenSSL");
#endif  // HAVE_OPENSSL
}
