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
#include "filename.h"
#include "config_express.h"
#include "virtualFileSystem.h"
#include "executionEnvironment.h"

#ifdef HAVE_SSL

#include <openssl/rand.h>
#ifndef NDEBUG
#include <openssl/err.h>
#endif

bool HTTPClient::_ssl_initialized = false;

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::load_certificates
//       Access: Published
//  Description: Reads the certificate(s) (delimited by -----BEGIN
//               CERTIFICATE----- and -----END CERTIFICATE-----) from
//               the indicated file and makes them known as trusted
//               public keys for validating future connections.
//               Returns true on success, false otherwise.
////////////////////////////////////////////////////////////////////
bool HTTPClient::
load_certificates(const Filename &filename) {
  int result;
  
  if (use_vfs) {
    result = load_verify_locations(_ssl_ctx, filename);

  } else {
    string os_specific = filename.to_os_specific();
    result =
      SSL_CTX_load_verify_locations(_ssl_ctx, os_specific.c_str(), NULL);
  }

  if (result <= 0) {
    downloader_cat.info()
      << "Could not load certificates from " << filename << ".\n";
#ifndef NDEBUG
    ERR_print_errors_fp(stderr);
#endif
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_document
//       Access: Published
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
//     Function: HTTPClient::make_ctx
//       Access: Private
//  Description: Creates the OpenSSL context object.  This is only
//               called by the constructor.
////////////////////////////////////////////////////////////////////
void HTTPClient::
make_ctx() {
  if (!_ssl_initialized) {
    initialize_ssl();
  }
  _ssl_ctx = SSL_CTX_new(SSLv23_client_method());

#if defined(SSL_097) && !defined(NDEBUG)
  // If we have debugging enabled, set a callback that allows us to
  // report the SSL messages as they are sent and received.
  if (downloader_cat.is_debug()) {
    SSL_CTX_set_msg_callback(_ssl_ctx, ssl_msg_callback);
  }
#endif

  // By default, insist on verifying servers.
  set_verify_ssl(true);

  // Load in any default certificates listed in the Configrc file.
  Config::ConfigTable::Symbol cert_files;
  config_express.GetAll("ssl-certificates", cert_files);
  
  // When we use GetAll(), we might inadvertently read duplicate
  // lines.  Filter them out with a set.
  pset<string> already_read;
  
  Config::ConfigTable::Symbol::iterator si;
  for (si = cert_files.begin(); si != cert_files.end(); ++si) {
    string cert_file = (*si).Val();
    if (already_read.insert(cert_file).second) {
      Filename filename = Filename::from_os_specific(ExecutionEnvironment::expand_string(cert_file));
      if (load_certificates(filename)) {
        downloader_cat.info()
          << "Appending SSL certificates from " << cert_file << "\n";
      }
    }
  }
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

  // Call RAND_status() here to force the random number generator to
  // initialize early.
  RAND_status();

  _ssl_initialized = true;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::load_verify_locations
//       Access: Private, Static
//  Description: An implementation of the OpenSSL-provided
//               SSL_CTX_load_verify_locations() that takes a Filename
//               (and supports Panda vfs).
//
//               This reads the certificates from the named ca_file
//               and makes them available to the given SSL context.
//               It returns a positive number on success, or <= 0 on
//               failure.
////////////////////////////////////////////////////////////////////
int HTTPClient::
load_verify_locations(SSL_CTX *ctx, const Filename &ca_file) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  // First, read the complete file into memory.
  string data;
  if (!vfs->read_file(ca_file, data)) {
    // Could not find or read file.
    downloader_cat.info()
      << "Could not read " << ca_file << ".\n";
    return 0;
  }

  STACK_OF(X509_INFO) *inf;

  // Now create an in-memory BIO to read the "file" from the buffer we
  // just read, and call the low-level routines to read the
  // certificates from the BIO.
  BIO *mbio = BIO_new_mem_buf((void *)data.data(), data.length());
  inf = PEM_X509_INFO_read_bio(mbio, NULL, NULL, NULL);
  BIO_free(mbio);

  if (!inf) {
    // Could not scan certificates.
    return 0;
  }

  // Now add the certificates to the context.
  X509_STORE *store = ctx->cert_store;

  int count = 0;
  for (int i = 0; i < sk_X509_INFO_num(inf); i++) {
    X509_INFO *itmp = sk_X509_INFO_value(inf, i);

    if (itmp->x509) {
      X509_STORE_add_cert(store, itmp->x509);
      count++;

    } else if (itmp->crl) {
      X509_STORE_add_crl(store, itmp->crl);
      count++;
    }
  }
  sk_X509_INFO_pop_free(inf, X509_INFO_free);

  return count;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_https
//       Access: Private
//  Description: Opens the indicated URL directly as an ordinary http
//               document.
////////////////////////////////////////////////////////////////////
BIO *HTTPClient::
get_http(const URLSpec &url, const string &body) {
  ostringstream server;
  server << url.get_server() << ":" << url.get_port();
  string server_str = server.str();

  BIO *bio = BIO_new_connect((char *)server_str.c_str());

  if (downloader_cat.is_debug()) {
    downloader_cat.debug() << "connecting to " << server_str << "\n";
  }
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
  ostringstream server;
  server << url.get_server() << ":" << url.get_port();
  string server_str = server.str();

  BIO *bio = BIO_new_connect((char *)server_str.c_str());

  if (downloader_cat.is_debug()) {
    downloader_cat.debug() << "connecting to " << server_str << "\n";
  }
  if (BIO_do_connect(bio) <= 0) {
    downloader_cat.info()
      << "Could not contact server " << server_str << "\n";
#ifndef NDEBUG
    ERR_print_errors_fp(stderr);
#endif
    return NULL;
  }

  BIO *sbio = BIO_new_ssl(_ssl_ctx, true);
  BIO_push(sbio, bio);

  SSL *ssl;
  BIO_get_ssl(sbio, &ssl);
  nassertr(ssl != (SSL *)NULL, NULL);
  SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "performing SSL handshake\n";
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
  ostringstream proxy_server;
  proxy_server << _proxy.get_server() << ":" << _proxy.get_port();
  string proxy_server_str = proxy_server.str();

  BIO *bio = BIO_new_connect((char *)proxy_server_str.c_str());

  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "connecting to proxy " << proxy_server_str << "\n";
  }
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
  ostringstream proxy_server;
  proxy_server << _proxy.get_server() << ":" << _proxy.get_port();
  string proxy_server_str = proxy_server.str();

  BIO *bio = BIO_new_connect((char *)proxy_server_str.c_str());

  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "connecting to proxy " << proxy_server_str << "\n";
  }
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

#ifndef NDEBUG
    if (downloader_cat.is_debug()) {
      show_send(request_str);
    }
#endif
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

      if (downloader_cat.is_debug()) {
        doc->write_headers(downloader_cat.debug(false));
      }
      
      if (!get_verify_ssl()) {
        // If the proxy refused to open a raw connection for us, see
        // if it will handle the https communication itself.  For
        // other error codes, just return error.  (We can only
        // reliably do this if verify_ssl is not true, since we're not
        // sure whether to trust the proxy to do the verification for
        // us.)
        if ((doc->get_status_code() / 100) == 4) {
          BIO_free_all(bio);
          return get_http_proxy(url, body);
        }
      }
      return NULL;
    }
  }

  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "connection established to " << url.get_authority() << "\n";
  }

  // Ok, we now have a connection to our actual server, so start
  // speaking SSL and then ask for the document we really want.

  BIO *sbio = BIO_new_ssl(_ssl_ctx, true);
  BIO_push(sbio, bio);

  SSL *ssl;
  BIO_get_ssl(sbio, &ssl);
  nassertr(ssl != (SSL *)NULL, NULL);
  SSL_set_mode(ssl, SSL_MODE_AUTO_RETRY);

  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "performing SSL handshake\n";
  }
  if (BIO_do_handshake(sbio) <= 0) {
    downloader_cat.info()
      << "Could not establish SSL handshake with " 
      << url.get_server() << ":" << url.get_port() << "\n";
#ifndef NDEBUG
    ERR_print_errors_fp(stderr);
#endif
    return NULL;
  }

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
#ifndef NDEBUG
  if (downloader_cat.is_debug()) {
    show_send(request_str);
  }
#endif
  BIO_puts(bio, request_str.c_str());
}

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::show_send
//       Access: Private, Static
//  Description: Writes the outgoing message, one line at a time, to
//               the debugging log.
////////////////////////////////////////////////////////////////////
void HTTPClient::
show_send(const string &message) {
  size_t start = 0;
  size_t newline = message.find('\n', start);
  while (newline != string::npos) {
    downloader_cat.debug()
      << "send: " << message.substr(start, newline - start + 1);
    start = newline + 1;
    newline = message.find('\n', start);
  }

  if (start < message.length()) {
    downloader_cat.debug()
      << "send: " << message.substr(start) << " (no newline)\n";
  }
}
#endif   // NDEBUG

#if defined(SSL_097) && !defined(NDEBUG)
////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::ssl_msg_callback
//       Access: Private, Static
//  Description: This method is attached as a callback for SSL
//               messages only when debug output is enabled.
////////////////////////////////////////////////////////////////////
void HTTPClient::
ssl_msg_callback(int write_p, int version, int content_type,
                 const void *, size_t len, SSL *, void *) {
  ostringstream describe;
  if (write_p) {
    describe << "sent ";
  } else {
    describe << "received ";
  }
  switch (version) {
  case SSL2_VERSION:
    describe << "SSL 2.0 ";
    break;

  case SSL3_VERSION:
    describe << "SSL 3.0 ";
    break;

  case TLS1_VERSION: 
    describe << "TLS 1.0 ";
    break;

  default:
    describe << "unknown protocol ";
  }

  describe << "message: ";

  if (version != SSL2_VERSION) {
    switch (content_type) {
    case 20:
      describe << "change cipher spec, ";
      break;
      
    case 21:
      describe << "alert, ";
      break;
      
    case 22:
      describe << "handshake, ";
      break;
      
    case 23:
      describe << "application data, ";
      break;
      
    default:
      describe << "unknown content type, ";
    }
  }

  describe << len << " bytes.\n";

  downloader_cat.debug() << describe.str();
}
#endif  // defined(SSL_097) && !defined(NDEBUG)

#endif  // HAVE_SSL
