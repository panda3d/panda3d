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
#include "httpChannel.h"
#include "config_downloader.h"
#include "filename.h"
#include "config_express.h"
#include "virtualFileSystem.h"
#include "executionEnvironment.h"

#ifdef HAVE_SSL

#define REPORT_SSL_ERRORS 1

#include <openssl/rand.h>
#ifdef REPORT_SSL_ERRORS
#include <openssl/err.h>
#endif

// Windows may define this macro inappropriately.
#ifdef X509_NAME
#undef X509_NAME
#endif

bool HTTPClient::_ssl_initialized = false;

// This is created once and never freed.
X509_STORE *HTTPClient::_x509_store = NULL;


////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
HTTPClient::
HTTPClient() {
  _http_version = HV_11;
  _verify_ssl = verify_ssl ? VS_normal : VS_no_verify;
  make_ctx();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
HTTPClient::
HTTPClient(const HTTPClient &copy) {
  // We can initialize these to default values because the operator =
  // function will copy them in a second.
  _http_version = HV_11;
  _verify_ssl = verify_ssl ? VS_normal : VS_no_verify;
  make_ctx();

  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void HTTPClient::
operator = (const HTTPClient &copy) {
  _proxy = copy._proxy;
  _http_version = copy._http_version;
  _verify_ssl = copy._verify_ssl;
  clear_expected_servers();

  ExpectedServers::const_iterator ei;
  for (ei = copy._expected_servers.begin();
       ei != copy._expected_servers.end();
       ++ei) {
    X509_NAME *orig_name = (*ei);
    X509_NAME *new_name = X509_NAME_dup(orig_name);
    _expected_servers.push_back(new_name);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
HTTPClient::
~HTTPClient() {
  // Before we can free the context, we must remove the X509_STORE
  // pointer from it, so it won't be destroyed along with it (this
  // object is shared among all contexts).
  nassertv(_ssl_ctx->cert_store == _x509_store);
  _ssl_ctx->cert_store = NULL;
  SSL_CTX_free(_ssl_ctx);

  // Free all of the expected server definitions.
  clear_expected_servers();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::set_username
//       Access: Published
//  Description: Specifies the username:password string corresponding
//               to a particular server and/or realm, when demanded by
//               the server.  Either or both of the server or realm
//               may be empty; if so, they match anything.  Also, the
//               server may be set to the special string "*proxy",
//               which will match any proxy server.
//
//               If the username is set to the empty string, this
//               clears the password for the particular server/realm
//               pair.
////////////////////////////////////////////////////////////////////
void HTTPClient::
set_username(const string &server, const string &realm, const string &username) {
  string key = server + ":" + realm;
  if (username.empty()) {
    _usernames.erase(key);
  } else {
    _usernames[key] = username;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_username
//       Access: Published
//  Description: Returns the username:password string set for this
//               server/realm pair, or empty string if nothing has
//               been set.  See set_username().
////////////////////////////////////////////////////////////////////
string HTTPClient::
get_username(const string &server, const string &realm) const {
  string key = server + ":" + realm;
  Usernames::const_iterator ui;
  ui = _usernames.find(key);
  if (ui != _usernames.end()) {
    return (*ui).second;
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_http_version_string
//       Access: Published
//  Description: Returns the current HTTP version setting as a string,
//               e.g. "HTTP/1.0" or "HTTP/1.1".
////////////////////////////////////////////////////////////////////
string HTTPClient::
get_http_version_string() const {
  switch (_http_version) {
  case HV_09:
    return "HTTP/0.9";

  case HV_10:
    return "HTTP/1.0";

  case HV_11:
    return "HTTP/1.1";

  case HV_other:
    // Report the best we can do.
    return "HTTP/1.1";
  }

  return "unknown";
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::parse_http_version_string
//       Access: Published
//  Description: Matches the string representing a particular HTTP
//               version against any of the known versions and returns
//               the appropriate enumerated value, or HV_other if the
//               version is unknown.
////////////////////////////////////////////////////////////////////
HTTPClient::HTTPVersion HTTPClient::
parse_http_version_string(const string &version) {
  if (version == "HTTP/1.0") {
    return HV_10;
  } else if (version == "HTTP/1.1") {
    return HV_11;
  } else if (version.substr(0, 6) == "HTTP/0") {
    return HV_09;
  } else {
    return HV_other;
  }
}

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
#ifdef REPORT_SSL_ERRORS
    ERR_print_errors_fp(stderr);
#endif
    return false;
  }

  downloader_cat.info()
    << "Appending " << result << " SSL certificates from "
    << filename << "\n";

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::add_expected_server
//       Access: Published
//  Description: Adds the indicated string as a definition of a valid
//               server to contact via https.  If no servers have been
//               been added, an https connection will be allowed to
//               any server.  If at least one server has been added,
//               an https connection will be allowed to any of the
//               named servers, but none others.
//
//               The string passed in defines a subset of the server
//               properties that are to be insisted on, using the X509
//               naming convention, e.g. O=WDI/OU=VRStudio/CN=ttown.
//
//               It makes sense to use this in conjunction with
//               set_verify_ssl(), which insists that the https
//               connection uses a verifiable certificate.
////////////////////////////////////////////////////////////////////
bool HTTPClient::
add_expected_server(const string &server_attributes) {
  X509_NAME *name = parse_x509_name(server_attributes);
  if (name == (X509_NAME *)NULL) {
    return false;
  }

  _expected_servers.push_back(name);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::clear_expected_servers
//       Access: Published
//  Description: Clears the set of expected servers; the HTTPClient
//               will allow an https connection to any server.
////////////////////////////////////////////////////////////////////
void HTTPClient::
clear_expected_servers() {
  for (ExpectedServers::iterator ei = _expected_servers.begin();
       ei != _expected_servers.end();
       ++ei) {
    X509_NAME *name = (*ei);
    X509_NAME_free(name);
  }
  _expected_servers.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::make_channel
//       Access: Published
//  Description: Returns a new HTTPChannel object that may be used
//               for reading multiple documents using the same
//               connection, for greater network efficiency than
//               calling HTTPClient::get_document() repeatedly (and
//               thus forcing a new connection for each document).
////////////////////////////////////////////////////////////////////
PT(HTTPChannel) HTTPClient::
make_channel() {
  PT(HTTPChannel) doc = new HTTPChannel(this);
  doc->set_persistent_connection(true);
  return doc;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::post_form
//       Access: Published
//  Description: Posts form data to a particular URL and retrieves the
//               response.  Returns a new HTTPChannel object whether
//               the document is successfully read or not; you can
//               test is_valid() and get_return_code() to determine
//               whether the document was retrieved.
////////////////////////////////////////////////////////////////////
PT(HTTPChannel) HTTPClient::
post_form(const URLSpec &url, const string &body) {
  PT(HTTPChannel) doc = new HTTPChannel(this);
  doc->post_form(url, body);
  return doc;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_document
//       Access: Published
//  Description: Opens the named document for reading, or if body is
//               nonempty, posts data for a particular URL and
//               retrieves the response.  Returns a new HTTPChannel
//               object whether the document is successfully read or
//               not; you can test is_valid() and get_return_code() to
//               determine whether the document was retrieved.
////////////////////////////////////////////////////////////////////
PT(HTTPChannel) HTTPClient::
get_document(const URLSpec &url, const string &body) {
  PT(HTTPChannel) doc = new HTTPChannel(this);
  if (body.empty()) {
    doc->get_document(url);
  } else {
    doc->post_form(url, body);
  }
  return doc;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_header
//       Access: Published
//  Description: Like get_document(), except only the header
//               associated with the document is retrieved.  This may
//               be used to test for existence of the document; it
//               might also return the size of the document (if the
//               server gives us this information).
////////////////////////////////////////////////////////////////////
PT(HTTPChannel) HTTPClient::
get_header(const URLSpec &url) {
  PT(HTTPChannel) doc = new HTTPChannel(this);
  doc->get_header(url);
  return doc;
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

  // Get the configured set of expected servers.
  {
    // Load in any default certificates listed in the Configrc file.
    Config::ConfigTable::Symbol expected_servers;
    config_express.GetAll("expected-ssl-server", expected_servers);
    
    // When we use GetAll(), we might inadvertently read duplicate
    // lines.  Filter them out with a set.
    pset<string> already_read;
    
    Config::ConfigTable::Symbol::iterator si;
    for (si = expected_servers.begin(); si != expected_servers.end(); ++si) {
      string expected_server = (*si).Val();
      if (already_read.insert(expected_server).second) {
        add_expected_server(expected_server);
      }
    }
  }

  if (_x509_store != (X509_STORE *)NULL) {
    // If we've already created an x509 store object, share it with
    // this context.  It would be better to make a copy of the store
    // object for each context, so we could locally add certificates,
    // but (a) there doesn't seem to be an interface for this, and (b)
    // something funny about loading certificates that seems to save
    // some persistent global state anyway.
    SSL_CTX_set_cert_store(_ssl_ctx, _x509_store);

  } else {
    // Create the first x509 store object, and fill it up with our
    // certificates.
    _x509_store = X509_STORE_new();
    SSL_CTX_set_cert_store(_ssl_ctx, _x509_store);

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
        load_certificates(filename);
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
#ifdef REPORT_SSL_ERRORS
  ERR_load_crypto_strings();
  ERR_load_SSL_strings();
#endif
  OpenSSL_add_all_algorithms();

  // Call RAND_status() here to force the random number generator to
  // initialize early.
  if (early_random_seed) {
    RAND_status();
  }

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
//     Function: HTTPClient::parse_x509_name
//       Access: Private, Static
//  Description: Parses a string of the form
//               /type0=value0/type1=value1/type2=... into a newly
//               allocated X509_NAME object.  Returns NULL if the
//               string is invalid.
////////////////////////////////////////////////////////////////////
X509_NAME *HTTPClient::
parse_x509_name(const string &source) {
  X509_NAME *result = NULL;

  result = X509_NAME_new();
  bool added_any = false;

  string::const_iterator si;
  si = source.begin();
  while (si != source.end()) {
    if ((*si) == '/') {
      // Skip a slash delimiter.
      ++si;
    } else {
      string type;
      while (si != source.end() && (*si) != '=' && (*si) != '/') {
        if ((*si) == '\\') {
          ++si;
          if (si != source.end()) {
            type += (*si);
            ++si;
          }
        } else {
          type += (*si);
          ++si;
        }
      }

      int nid = OBJ_txt2nid((char *)type.c_str());
      if (nid == NID_undef) {
        downloader_cat.info()
          << "Unknown type " << type << " in X509 name: " << source
          << "\n";
        X509_NAME_free(result);
        return NULL;
      }

      string value;
      
      if (si != source.end() && (*si) == '=') {
        ++si;
        while (si != source.end() && (*si) != '/') {
          if ((*si) == '\\') {
            ++si;
            if (si != source.end()) {
              value += (*si);
              ++si;
            }
          } else {
            value += (*si);
            ++si;
          }
        }
      }

      if (!value.empty()) {
        int add_result =
          X509_NAME_add_entry_by_NID(result, nid, V_ASN1_APP_CHOOSE, 
                                     (unsigned char *)value.c_str(), -1, -1, 0);
        if (!add_result) {
          downloader_cat.info()
            << "Unable to add " << type << "=" << value << " in X509 name: "
            << source << "\n";
          X509_NAME_free(result);
          return NULL;
        }
        added_any = true;
      }
    }
  }

  if (!added_any) {
    downloader_cat.info()
      << "Invalid empty X509 name: " << source << "\n";
    X509_NAME_free(result);
    return NULL;
  }

  return result;
}

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

