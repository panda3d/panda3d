// Filename: httpClient.cxx
// Created by:  drose (24Sep02)
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

#include "httpClient.h"
#include "config_downloader.h"

#ifndef NDEBUG
#include <openssl/err.h>
#endif

bool HTTPClient::_ssl_initialized = false;

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_document
//       Access: Published, Virtual
//  Description: Opens the named document for reading, or if body is
//               nonempty, posts data for a particular URL and
//               retrieves the response.  Returns a new HTTPDocument
//               object whether the document is successfully read or
//               not; you can test is_valid() and get_return_code() to
//               determine whether the document was retrieved.
////////////////////////////////////////////////////////////////////
PT(HTTPDocument) HTTPClient::
get_document(const URLSpec &url, const string &body) {
  BIO *bio;

  if (_proxy.empty()) {
    if (url.get_scheme() == "https") {
      bio = get_https(url, body);
    } else {
      bio = get_http(url, body);
    }
  } else {
    if (url.get_scheme() == "https") {
      bio = get_https_proxy(url, body);
    } else {
      bio = get_http_proxy(url, body);
    }
  }    

  return new HTTPDocument(bio, true);
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::initialize_ssl
//       Access: Private, Static
//  Description: Called once the first time this class is used to
//               initialize the OpenSSL library.
////////////////////////////////////////////////////////////////////
void HTTPClient::
initialize_ssl() {
#ifndef NDEBUG
  ERR_load_crypto_strings();
  ERR_load_SSL_strings();
#endif
  OpenSSL_add_all_algorithms();

  _ssl_initialized = true;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_https
//       Access: Private
//  Description: Opens the indicated URL directly as an ordinary http
//               document.
////////////////////////////////////////////////////////////////////
BIO *HTTPClient::
get_http(const URLSpec &url, const string &body) {
  stringstream server;
  server << url.get_server() << ":" << url.get_port();
  string server_str = server.str();

  BIO *bio = BIO_new_connect((char *)server_str.c_str());

  if (BIO_do_connect(bio) <= 0) {
    downloader_cat.info()
      << "Could not contact server " << server_str << "\n";
#ifndef NDEBUG
    ERR_print_errors_fp(stderr);
#endif
    return NULL;
  }

  send_get_request(bio, url.get_path(), url.get_server(), body);
  return bio;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_https
//       Access: Private
//  Description: Opens the indicated URL directly as an https
//               document.
////////////////////////////////////////////////////////////////////
BIO *HTTPClient::
get_https(const URLSpec &url, const string &body) {
  BIO *sbio = BIO_new_ssl_connect(_ssl_ctx);
  SSL *ssl;
  BIO_get_ssl(sbio, &ssl);
  nassertr(ssl != (SSL *)NULL, NULL);
  SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

  stringstream server;
  server << url.get_server() << ":" << url.get_port();
  string server_str = server.str();

  BIO_set_conn_hostname(sbio, server_str.c_str());

  if (BIO_do_connect(sbio) <= 0) {
    downloader_cat.info()
      << "Could not contact server " << server_str << "\n";
#ifndef NDEBUG
    ERR_print_errors_fp(stderr);
#endif
    return NULL;
  }

  if (BIO_do_handshake(sbio) <= 0) {
    downloader_cat.info()
      << "Could not establish SSL handshake with " << server_str << "\n";
#ifndef NDEBUG
    ERR_print_errors_fp(stderr);
#endif
    return NULL;
  }

  send_get_request(sbio, url.get_path(), url.get_server(), body);
  return sbio;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_http_proxy
//       Access: Private
//  Description: Opens the indicated URL via the proxy as an ordinary
//               http document.
////////////////////////////////////////////////////////////////////
BIO *HTTPClient::
get_http_proxy(const URLSpec &url, const string &body) {
  stringstream proxy_server;
  proxy_server << _proxy.get_server() << ":" << _proxy.get_port();
  string proxy_server_str = proxy_server.str();

  BIO *bio = BIO_new_connect((char *)proxy_server_str.c_str());

  if (BIO_do_connect(bio) <= 0) {
    downloader_cat.info()
      << "Could not contact proxy " << proxy_server_str << "\n";
#ifndef NDEBUG
    ERR_print_errors_fp(stderr);
#endif
    return NULL;
  }

  send_get_request(bio, url, url.get_server(), body);
  return bio;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_https_proxy
//       Access: Private
//  Description: Opens the indicated URL via the proxy as an https
//               document.
////////////////////////////////////////////////////////////////////
BIO *HTTPClient::
get_https_proxy(const URLSpec &url, const string &body) {
  // First, ask the proxy to open a connection for us.
  stringstream proxy_server;
  proxy_server << _proxy.get_server() << ":" << _proxy.get_port();
  string proxy_server_str = proxy_server.str();

  BIO *bio = BIO_new_connect((char *)proxy_server_str.c_str());

  if (BIO_do_connect(bio) <= 0) {
    downloader_cat.info()
      << "Could not contact proxy " << proxy_server_str << "\n";
#ifndef NDEBUG
    ERR_print_errors_fp(stderr);
#endif
    return NULL;
  }

  {
    ostringstream request;
    request 
      << "CONNECT " << url.get_authority() << " HTTP/1.1\r\n"
      << "\r\n";
    string request_str = request.str();
    
    BIO_puts(bio, request_str.c_str());
  }

  // Create a temporary HTTPDocument to read the response from the
  // proxy.
  {
    PT(HTTPDocument) doc = new HTTPDocument(bio, false);
    if (!doc->is_valid()) {
      downloader_cat.info()
        << "proxy would not open connection to " << url.get_authority()
        << ": " << doc->get_status_code() << " "
        << doc->get_status_string() << "\n";
      
      // If the proxy won't open a raw connection for us, see if it will
      // handle the https communication directly.
      BIO_free_all(bio);
      return get_http_proxy(url, body);
    }
  }

  // Ok, we now have a connection to our actual server, so start
  // speaking SSL and then ask for the document we really want.

  BIO *sbio = BIO_new_ssl(_ssl_ctx, true);
  BIO_push(sbio, bio);

  SSL *ssl;
  BIO_get_ssl(sbio, &ssl);
  nassertr(ssl != (SSL *)NULL, NULL);
  SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

  send_get_request(sbio, url.get_path(), url.get_server(), body);
  return sbio;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::send_get_request
//       Access: Private
//  Description: Sends the appropriate GET or POST request to the
//               server on the indicated connection.
////////////////////////////////////////////////////////////////////
void HTTPClient::
send_get_request(BIO *bio, 
                 const string &path, const string &server, 
                 const string &body) const {
  ostringstream request;

  if (body.empty()) {
    request 
      << "GET " << path << " HTTP/1.1\r\n"
      << "Host: " << server << "\r\n"
      << "Connection: close\r\n"
      << "\r\n";
  } else {
    request 
      << "POST " << path << " HTTP/1.1\r\n"
      << "Host: " << server << "\r\n"
      << "Connection: close\r\n"
      << "Content-type: application/x-www-form-urlencoded\r\n"
      << "Content-Length: " << body.length() << "\r\n"
      << "\r\n"
      << body;
  }

  string request_str = request.str();
  BIO_puts(bio, request_str.c_str());
}
