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

#define REPORT_SSL_ERRORS 1

#include <openssl/rand.h>
#ifdef REPORT_SSL_ERRORS
#include <openssl/err.h>
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
  _verify_ssl = verify_ssl;
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
  _verify_ssl = verify_ssl;
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
  set_verify_ssl(copy._verify_ssl);
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
//     Function: HTTPClient::get_http_version_string
//       Access: Published
//  Description: Returns the current HTTP version setting as a string,
//               e.g. "HTTP/1.0" or "HTTP/1.1".
////////////////////////////////////////////////////////////////////
string HTTPClient::
get_http_version_string() const {
  switch (_http_version) {
  case HV_10:
    return "HTTP/1.0";

  case HV_11:
    return "HTTP/1.1";
  }

  return "unknown";
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

  // Insist on verifying servers if we are configured to.
  if (_verify_ssl) {
    SSL_CTX_set_verify(_ssl_ctx, SSL_VERIFY_PEER, NULL);
  } else {
    SSL_CTX_set_verify(_ssl_ctx, SSL_VERIFY_NONE, NULL);
  }

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
//     Function: HTTPClient::make_request
//       Access: Private
//  Description: Chooses a suitable mechanism to handle this request,
//               based on whether it is an http or https request, and
//               according to whether we have a proxy in place.
//               Issues the request and returns an HTTPDocument that
//               represents the results.
////////////////////////////////////////////////////////////////////
PT(HTTPDocument) HTTPClient::
make_request(const string &method, const URLSpec &url, const string &body) {
  BIO *bio;

  if (_proxy.empty()) {
    if (url.get_scheme() == "https") {
      bio = get_https(method, url, body);
    } else {
      bio = get_http(method, url, body);
    }
  } else {
    if (url.get_scheme() == "https") {
      bio = get_https_proxy(method, url, body);
    } else {
      bio = get_http_proxy(method, url, body);
    }
  }    

  return new HTTPDocument(bio, true);
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_https
//       Access: Private
//  Description: Opens the indicated URL directly as an ordinary http
//               document.
////////////////////////////////////////////////////////////////////
BIO *HTTPClient::
get_http(const string &method, const URLSpec &url, const string &body) {
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
#ifdef REPORT_SSL_ERRORS
    ERR_print_errors_fp(stderr);
#endif
    return NULL;
  }

  send_request(bio, method, url.get_path(), url.get_server(), body);
  return bio;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_https
//       Access: Private
//  Description: Opens the indicated URL directly as an https
//               document.
////////////////////////////////////////////////////////////////////
BIO *HTTPClient::
get_https(const string &method, const URLSpec &url, const string &body) {
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
#ifdef REPORT_SSL_ERRORS
    ERR_print_errors_fp(stderr);
#endif
    return NULL;
  }

  BIO *sbio = make_https_connection(bio, url);

  if (sbio != (BIO *)NULL) {
    send_request(sbio, method, url.get_path(), url.get_server(), body);
  }
  return sbio;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_http_proxy
//       Access: Private
//  Description: Opens the indicated URL via the proxy as an ordinary
//               http document.
////////////////////////////////////////////////////////////////////
BIO *HTTPClient::
get_http_proxy(const string &method, const URLSpec &url, const string &body) {
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
#ifdef REPORT_SSL_ERRORS
    ERR_print_errors_fp(stderr);
#endif
    return NULL;
  }

  send_request(bio, method, url, url.get_server(), body);
  return bio;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_https_proxy
//       Access: Private
//  Description: Opens the indicated URL via the proxy as an https
//               document.
////////////////////////////////////////////////////////////////////
BIO *HTTPClient::
get_https_proxy(const string &method, const URLSpec &url, const string &body) {
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
#ifdef REPORT_SSL_ERRORS
    ERR_print_errors_fp(stderr);
#endif
    return NULL;
  }

  {
    ostringstream request;
    request 
      << "CONNECT " << url.get_authority() << " " << get_http_version_string()
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
          return get_http_proxy(method, url, body);
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
  BIO *sbio = make_https_connection(bio, url);

  if (sbio != (BIO *)NULL) {
    send_request(sbio, method, url.get_path(), url.get_server(), body);
  }
  return sbio;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::make_https_connection
//       Access: Private
//  Description: Starts speaking SSL over the opened connection and
//               returns the appropriately modified bio.  Returns NULL
//               if the connection cannot be established for some
//               reason.
////////////////////////////////////////////////////////////////////
BIO *HTTPClient::
make_https_connection(BIO *bio, const URLSpec &url) const {
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
#ifdef REPORT_SSL_ERRORS
    ERR_print_errors_fp(stderr);
#endif
    BIO_free_all(bio);
    return NULL;
  }

  long verify_result = SSL_get_verify_result(ssl);
  if (verify_result != X509_V_OK) {
    downloader_cat.warning()
      << "Unable to verify identity of " << url.get_server() << ":" 
      << url.get_port() << ", verify error code " << verify_result << "\n";
  }

  X509 *cert = SSL_get_peer_certificate(ssl);
  if (cert == (X509 *)NULL) {
    downloader_cat.info()
      << "No certificate was presented by server.\n";

    if (!_expected_servers.empty()) {
      downloader_cat.info()
        << "Not allowing connection since no certificates could be matched.\n";
      BIO_free_all(sbio);
      return NULL;
    }

  } else {
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << "Received certificate from server:\n" << flush;
      X509_print_fp(stderr, cert);
      fflush(stderr);
    }

    X509_NAME *subject = X509_get_subject_name(cert);

    string org_name = get_x509_name_component(subject, NID_organizationName);
    string org_unit_name = get_x509_name_component(subject, NID_organizationalUnitName);
    string common_name = get_x509_name_component(subject, NID_commonName);

    downloader_cat.info()
      << "Server is " << common_name << " from " << org_unit_name
      << " / " << org_name << "\n";

    if (!verify_server(subject)) {
      downloader_cat.info()
        << "Server does not match any expected server.\n";
      BIO_free_all(sbio);
      return NULL;
    }
      
    X509_free(cert);
  }

  return sbio;
}



/*
   Certificate verify error codes:

0 X509_V_OK: ok

    the operation was successful.

2 X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT: unable to get issuer certificate

    the issuer certificate could not be found: this occurs if the
    issuer certificate of an untrusted certificate cannot be found.

3 X509_V_ERR_UNABLE_TO_GET_CRL unable to get certificate CRL

    the CRL of a certificate could not be found. Unused.

4 X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE: unable to decrypt
certificate's signature

    the certificate signature could not be decrypted. This means that
    the actual signature value could not be determined rather than it
    not matching the expected value, this is only meaningful for RSA
    keys.

5 X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE: unable to decrypt CRL's signature

    the CRL signature could not be decrypted: this means that the
    actual signature value could not be determined rather than it not
    matching the expected value. Unused.

6 X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY: unable to decode
issuer public key

    the public key in the certificate SubjectPublicKeyInfo could not
    be read.

7 X509_V_ERR_CERT_SIGNATURE_FAILURE: certificate signature failure

    the signature of the certificate is invalid.

8 X509_V_ERR_CRL_SIGNATURE_FAILURE: CRL signature failure

    the signature of the certificate is invalid. Unused.

9 X509_V_ERR_CERT_NOT_YET_VALID: certificate is not yet valid

    the certificate is not yet valid: the notBefore date is after the
    current time.

10 X509_V_ERR_CERT_HAS_EXPIRED: certificate has expired

    the certificate has expired: that is the notAfter date is before
    the current time.

11 X509_V_ERR_CRL_NOT_YET_VALID: CRL is not yet valid

    the CRL is not yet valid. Unused.

12 X509_V_ERR_CRL_HAS_EXPIRED: CRL has expired

    the CRL has expired. Unused.

13 X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD: format error in
certificate's notBefore field

    the certificate notBefore field contains an invalid time.

14 X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD: format error in
certificate's notAfter field

    the certificate notAfter field contains an invalid time.

15 X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD: format error in CRL's
lastUpdate field

    the CRL lastUpdate field contains an invalid time. Unused.

16 X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD: format error in CRL's
nextUpdate field

    the CRL nextUpdate field contains an invalid time. Unused.

17 X509_V_ERR_OUT_OF_MEM: out of memory

    an error occurred trying to allocate memory. This should never
    happen.

18 X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT: self signed certificate

    the passed certificate is self signed and the same certificate
    cannot be found in the list of trusted certificates.

19 X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN: self signed certificate in
certificate chain

    the certificate chain could be built up using the untrusted
    certificates but the root could not be found locally.

20 X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY: unable to get local
issuer certificate

    the issuer certificate of a locally looked up certificate could
    not be found. This normally means the list of trusted certificates
    is not complete.

21 X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE: unable to verify the
first certificate

    no signatures could be verified because the chain contains only
    one certificate and it is not self signed.

22 X509_V_ERR_CERT_CHAIN_TOO_LONG: certificate chain too long

    the certificate chain length is greater than the supplied maximum
    depth. Unused.

23 X509_V_ERR_CERT_REVOKED: certificate revoked

    the certificate has been revoked. Unused.

24 X509_V_ERR_INVALID_CA: invalid CA certificate

    a CA certificate is invalid. Either it is not a CA or its
    extensions are not consistent with the supplied purpose.

25 X509_V_ERR_PATH_LENGTH_EXCEEDED: path length constraint exceeded

    the basicConstraints pathlength parameter has been exceeded.

26 X509_V_ERR_INVALID_PURPOSE: unsupported certificate purpose

    the supplied certificate cannot be used for the specified purpose.

27 X509_V_ERR_CERT_UNTRUSTED: certificate not trusted

    the root CA is not marked as trusted for the specified purpose.

28 X509_V_ERR_CERT_REJECTED: certificate rejected

    the root CA is marked to reject the specified purpose.

29 X509_V_ERR_SUBJECT_ISSUER_MISMATCH: subject issuer mismatch

    the current candidate issuer certificate was rejected because its
    subject name did not match the issuer name of the current
    certificate. Only displayed when the -issuer_checks option is set.

30 X509_V_ERR_AKID_SKID_MISMATCH: authority and subject key identifier
mismatch

    the current candidate issuer certificate was rejected because its
    subject key identifier was present and did not match the authority
    key identifier current certificate. Only displayed when the
    -issuer_checks option is set.

31 X509_V_ERR_AKID_ISSUER_SERIAL_MISMATCH: authority and issuer serial
number mismatch

    the current candidate issuer certificate was rejected because its
    issuer name and serial number was present and did not match the
    authority key identifier of the current certificate. Only
    displayed when the -issuer_checks option is set.

32 X509_V_ERR_KEYUSAGE_NO_CERTSIGN:key usage does not include
certificate signing

    the current candidate issuer certificate was rejected because its
    keyUsage extension does not permit certificate signing.

50 X509_V_ERR_APPLICATION_VERIFICATION: application verification failure

    an application specific error. Unused.

*/


////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::send_request
//       Access: Private
//  Description: Sends the appropriate GET or POST (or whatever)
//               request to the server on the indicated connection.
////////////////////////////////////////////////////////////////////
void HTTPClient::
send_request(BIO *bio, const string &method, 
             const string &path, const string &server,
             const string &body) const {
  ostringstream request;

  request 
    << method << " " << path << " " << get_http_version_string() << "\r\n";

  if (_http_version > HV_10) {
    request 
      << "Host: " << server << "\r\n"
      << "Connection: close\r\n";
  }

  if (!body.empty()) {
    request
      << "Content-type: application/x-www-form-urlencoded\r\n"
      << "Content-Length: " << body.length() << "\r\n";
  }

  request
    << "\r\n"
    << body;

  string request_str = request.str();
#ifndef NDEBUG
  if (downloader_cat.is_debug()) {
    show_send(request_str);
  }
#endif
  BIO_puts(bio, request_str.c_str());
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::verify_server
//       Access: Private
//  Description: Returns true if the indicated server matches one of
//               our expected servers (or the list of expected servers
//               is empty), or false if it does not match any of our
//               expected servers.
////////////////////////////////////////////////////////////////////
bool HTTPClient::
verify_server(X509_NAME *subject) const {
  if (_expected_servers.empty()) {
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << "No expected servers on list; allowing any https connection.\n";
    }
    return true;
  }

  if (downloader_cat.is_debug()) {
    downloader_cat.debug() << "checking server: " << flush;
    X509_NAME_print_ex_fp(stderr, subject, 0, 0);
    fflush(stderr);
    downloader_cat.debug(false) << "\n";
  }

  ExpectedServers::const_iterator ei;
  for (ei = _expected_servers.begin();
       ei != _expected_servers.end();
       ++ei) {
    X509_NAME *expected_name = (*ei);
    if (x509_name_subset(expected_name, subject)) {
      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << "Match found!\n";
      }
      return true;
    }
  }

  // None of the lines matched.
  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "No match found against any of the following expected servers:\n";

    for (ei = _expected_servers.begin();
         ei != _expected_servers.end();
         ++ei) {
      X509_NAME *expected_name = (*ei);
      X509_NAME_print_ex_fp(stderr, expected_name, 0, 0);
      fprintf(stderr, "\n");
    }
    fflush(stderr);      
  }
  
  return false;
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

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_x509_name_component
//       Access: Private, Static
//  Description: Returns the indicated component of the X509 name as a
//               string, if defined, or empty string if it is not.
////////////////////////////////////////////////////////////////////
string HTTPClient::
get_x509_name_component(X509_NAME *name, int nid) {
  ASN1_OBJECT *obj = OBJ_nid2obj(nid);

  if (obj == NULL) {
    // Unknown nid.  See openssl/objects.h.
    return string();
  }

  int i = X509_NAME_get_index_by_OBJ(name, obj, -1);
  if (i < 0) {
    return string();
  }

  ASN1_STRING *data = X509_NAME_ENTRY_get_data(X509_NAME_get_entry(name, i));
  return string((char *)data->data, data->length);  
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::x509_name_subset
//       Access: Private, Static
//  Description: Returns true if name_a is a subset of name_b: each
//               property of name_a is defined in name_b, and the
//               defined value is equivalent to that of name_a.
////////////////////////////////////////////////////////////////////
bool HTTPClient::
x509_name_subset(X509_NAME *name_a, X509_NAME *name_b) {
  int count_a = X509_NAME_entry_count(name_a);
  for (int ai = 0; ai < count_a; ai++) {
    X509_NAME_ENTRY *na = X509_NAME_get_entry(name_a, ai);

    int bi = X509_NAME_get_index_by_OBJ(name_b, na->object, -1);
    if (bi < 0) {
      // This entry in name_a is not defined in name_b.
      return false;
    }

    X509_NAME_ENTRY *nb = X509_NAME_get_entry(name_b, bi);
    if (na->value->length != nb->value->length ||
        memcmp(na->value->data, nb->value->data, na->value->length) != 0) {
      // This entry in name_a doesn't match that of name_b.
      return false;
    }
  }
  return true;
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
