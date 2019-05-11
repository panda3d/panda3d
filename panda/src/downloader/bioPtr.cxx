/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bioPtr.cxx
 * @author drose
 * @date 2002-10-15
 */

#include "bioPtr.h"

#ifdef HAVE_OPENSSL

#include "urlSpec.h"
#include "config_downloader.h"

#include "openSSLWrapper.h"  // must be included before any other openssl.
#include <openssl/ssl.h>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#endif

using std::string;

#ifdef _WIN32
static string format_error() {
  PVOID buffer;
  DWORD len;
  len = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                      nullptr, WSAGetLastError(), 0, (LPTSTR)&buffer, 0, nullptr);
  if (len == 0) {
    return string("Unknown error message");
  }

  const char *text = (const char *)buffer;
  while (len > 0 && isspace(text[len - 1])) {
    --len;
  }

  string result(text, len);
  LocalFree(buffer);
  return result;
}
#else
#define format_error() strerror(errno)
#endif

/**
 * This flavor of the constructor automatically creates a socket BIO and feeds
 * it the server and port name from the indicated URL.  It doesn't call
 * BIO_do_connect(), though.
 */
BioPtr::
BioPtr(const URLSpec &url) : _connecting(false) {
  if (url.get_scheme() == "file") {
    // We're just reading a disk file.
    string filename = URLSpec::unquote(url.get_path());
#ifdef _WIN32
    // On Windows, we have to munge the filename specially, because it's been
    // URL-munged.  It might begin with a leading slash as well as a drive
    // letter.  Clean up that nonsense.
    if (!filename.empty()) {
      if (filename[0] == '/' || filename[0] == '\\') {
        Filename fname = Filename::from_os_specific(filename.substr(1));
        if (fname.is_local()) {
          // Put the slash back on.
          fname = string("/") + fname.get_fullpath();
        }
        filename = fname.to_os_specific();
      }
    }
#endif  // _WIN32
    _server_name = "";
    _port = 0;
    _bio = BIO_new_file(filename.c_str(), "rb");

  } else {
    // A normal network-based URL.  We don't use BIO_new_connect since it
    // doesn't handle IPv6 properly.
    _server_name = url.get_server();
    _port = url.get_port();
    _bio = nullptr;

    // These hints tell getaddrinfo what kind of address to return.
    struct addrinfo hints, *res = nullptr;
    memset(&hints, 0, sizeof(hints));
    hints.ai_flags = AI_V4MAPPED | AI_ADDRCONFIG;
    hints.ai_family = support_ipv6 ? AF_UNSPEC : AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    // Resolve the hostname or address string.
    int result = getaddrinfo(_server_name.c_str(), nullptr, &hints, &res);
    if (result != 0) {
      const char *errmsg;
#ifndef _WIN32
      if (result == EAI_SYSTEM && errno != 0) {
        errmsg = strerror(errno);
      } else
#endif
      {
        errmsg = gai_strerror(result);
      }
      downloader_cat.error()
        << "Failed to resolve " << url.get_server() << ": " << errmsg << "\n";
      return;
    }
    nassertv(res != nullptr && res->ai_addr != nullptr);

    // Store the real resolved address.
    char buf[48];
    buf[0] = 0;
#ifdef _WIN32
    DWORD bufsize = sizeof(buf);
    WSAAddressToStringA(res->ai_addr, res->ai_addrlen, nullptr, buf, &bufsize);
#else
    if (res->ai_addr->sa_family == AF_INET) {
      inet_ntop(AF_INET, (char *)&((sockaddr_in *)res->ai_addr)->sin_addr, buf, sizeof(buf));

    } else if (res->ai_addr->sa_family == AF_INET6) {
      inet_ntop(AF_INET6, (char *)&((sockaddr_in6 *)res->ai_addr)->sin6_addr, buf, sizeof(buf));
    }
#endif

    if (buf[0]) {
      _server_name = buf;
    }
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << "Resolved " << url.get_server() << " to " << buf << "\n";
    }

    // Create the socket.
    int fd = socket(res->ai_family, SOCK_STREAM, IPPROTO_TCP);
    if (fd < 0) {
      downloader_cat.error()
        << "Failed to create socket: " << format_error() << "\n";
      _bio = nullptr;
      freeaddrinfo(res);
      return;
    }

    // Store the address and length for later use in connect().
    nassertv(res->ai_addrlen <= sizeof(_addr));
    memcpy(&_addr, res->ai_addr, res->ai_addrlen);
    _addrlen = res->ai_addrlen;
    freeaddrinfo(res);

    // Also set the port we'd like to connect to.
    if (_addr.ss_family == AF_INET) {
      ((sockaddr_in &)_addr).sin_port = htons(_port);
    } else if (_addr.ss_family == AF_INET6) {
      ((sockaddr_in6 &)_addr).sin6_port = htons(_port);
    }

    _bio = BIO_new_socket(fd, 1);
  }
}

/**
 * Sets the non-blocking flag on the socket.
 */
void BioPtr::
set_nbio(bool nbio) {
  if (_bio == nullptr) {
    return;
  }

  int fd = -1;
  BIO_get_fd(_bio, &fd);
  nassertv_always(fd >= 0);

  BIO_socket_nbio(fd, nbio);
}

/**
 * Connects to the socket.  Returns true on success.
 */
bool BioPtr::
connect() {
  if (_bio == nullptr) {
    return false;
  }

  int fd = -1;
  BIO_get_fd(_bio, &fd);
  nassertr(fd >= 0, false);

  int result;
  if (_connecting) {
    result = BIO_sock_error(fd);
  } else {
    result = ::connect(fd, (sockaddr *)&_addr, _addrlen);

    if (result != 0 && BIO_sock_should_retry(-1)) {
      // It's still in progress; we should retry later.  This causes
      // should_retry() to return true.
      BIO_set_flags(_bio, BIO_FLAGS_SHOULD_RETRY);
      _connecting = true;
      return false;
    }
  }
  BIO_clear_retry_flags(_bio);
  _connecting = false;

  if (result != 0) {
    downloader_cat.warning()
      << "Failed to connect to " << _server_name << " port " << _port
      << ": " << format_error() << "\n";
    return false;
  }

  return true;
}

/**
 *
 */
bool BioPtr::
should_retry() const {
  return (_bio != nullptr) && BIO_should_retry(_bio);
}

/**
 *
 */
BioPtr::
~BioPtr() {
  if (_bio != nullptr) {
    if (downloader_cat.is_debug() && !_server_name.empty()) {
      downloader_cat.debug()
        << "Dropping connection to " << _server_name << " port " << _port << "\n";
    }

    BIO_free_all(_bio);
    _bio = nullptr;
  }
}

#endif  // HAVE_OPENSSL
