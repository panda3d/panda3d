// Filename: config_downloader.cxx
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

#include "dconfig.h"
#include "config_downloader.h"
#include "httpChannel.h"
#include "pandaSystem.h"


ConfigureDef(config_downloader);
NotifyCategoryDef(downloader, "");

ConfigVariableInt downloader_disk_write_frequency
("downloader-disk-write-frequency", 4,
 PRC_DESC("How often we write to disk is determined by this ratio which is "
          "relative to the downloader-byte-rate (e.g. if disk-write-ratio is 4, "
          "we will write every 4 seconds if the frequency is 0.2)"));

ConfigVariableInt downloader_byte_rate
("downloader-byte-rate", 3600,
 PRC_DESC("We'd like this to be about 1 second worth of download assuming a "
          "28.8Kb connection (28.8Kb / 8 = 3600 bytes per second)."));

ConfigVariableDouble downloader_frequency
("downloader-frequency", 0.2,
 PRC_DESC("Frequency of download chunk requests in seconds (or fractions of) "
          "(Estimated 200 msec round-trip to server)."));

ConfigVariableInt downloader_timeout
("downloader-timeout", 15);

ConfigVariableInt downloader_timeout_retries
("downloader-timeout-retries", 5);

ConfigVariableInt decompressor_buffer_size
("decompressor-buffer-size", 4096);

ConfigVariableDouble decompressor_frequency
("decompressor-frequency", 0.2);

ConfigVariableInt extractor_buffer_size
("extractor-buffer-size", 4096);

ConfigVariableDouble extractor_frequency
("extractor-frequency", 0.2);

ConfigVariableInt patcher_buffer_size
("patcher-buffer-size", 4096);

ConfigVariableBool early_random_seed
("early-random-seed", true,
 PRC_DESC("Configure this true (the default) to compute the SSL random seed "
          "early on in the application (specifically, when the first "
          "HTTPClient is created), or false to defer this until it is actually "
          "needed, causing a delay the first time a https connection is "
          "attempted."));

ConfigVariableBool verify_ssl
("verify-ssl", true,
 PRC_DESC("Configure this true (the default) to insist on verifying all SSL "
          "(e.g. https) servers against a known certificate, or false to allow "
          "an unverified connection.  This controls the default behavior; the "
          "specific behavior for a particular HTTPClient can be adjusted at "
          "runtime with set_verify_ssl()."));

ConfigVariableString ssl_cipher_list
("ssl-cipher-list", "DEFAULT",
 PRC_DESC("This is the default value for HTTPClient::set_cipher_list()."));

ConfigVariableList expected_ssl_server
("expected-ssl-server");

ConfigVariableList ssl_certificates
("ssl-certificates");

ConfigVariableString http_proxy
("http-proxy", "",
 PRC_DESC("This specifies the default value for HTTPClient::set_proxy_spec().  "
          "It is a semicolon-delimited list of proxies that we use to contact "
          "all HTTP hosts that don't specify otherwise.  See "
          "set_proxy_spec() for more information."));
ConfigVariableString http_direct_hosts
("http-direct-hosts", "",
 PRC_DESC("This specifies the default value for HTTPClient::set_direct_host_spec().  "
          "It is a semicolon-delimited list of host names that do not require a "
          "proxy.  See set_direct_host_spec() for more information."));
ConfigVariableBool http_try_all_direct
("http-try-all-direct", true,
 PRC_DESC("This specifies the default value for HTTPClient::set_try_all_direct().  "
          "If this is true, a direct connection will always be attempted after an "
          "attempt to connect through a proxy fails."));
ConfigVariableString http_proxy_username
("http-proxy-username", "",
 PRC_DESC("This specifies a default username:password to pass to the proxy."));
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

ConfigVariableInt http_max_connect_count
("http-max-connect-count", 10,
 PRC_DESC("This is the maximum number of times to try reconnecting to the "
          "server on any one document attempt.  This is just a failsafe to "
          "prevent the code from attempting runaway connections; this limit "
          "should never be reached in practice."));

ConfigVariableFilename http_client_certificate_filename
("http-client-certificate-filename", "",
 PRC_DESC("This provides a default client certificate to offer up should an "
          "SSL server demand one.  The file names a PEM-formatted file "
          "that includes a public and private key specification.  A "
          "connection-specific certificate may also be specified at runtime on "
          "the HTTPClient object, but this will require having a different "
          "HTTPClient object for each differently-certificated connection."));

ConfigVariableString http_client_certificate_passphrase
("http-client-certificate-passphrase", "",
 PRC_DESC("This specifies the passphrase to use to decode the certificate named "
          "by http-client-certificate-filename."));

ConfigVariableList http_username
("http-username",
 PRC_DESC("Adds one or more username/password pairs to all HTTP clients.  The client "
          "will present this username/password when asked to authenticate a request "
          "for a particular server and/or realm.  The username is of the form "
          "server:realm:username:password, where either or both of server and "
          "realm may be empty, or just server:username:password or username:password.  "
          "If the server or realm is empty, they will match anything."));

ConfigureFn(config_downloader) {
#ifdef HAVE_SSL
  HTTPChannel::init_type();

  PandaSystem *ps = PandaSystem::get_global_ptr();
  ps->add_system("OpenSSL");
#endif
}
