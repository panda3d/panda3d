// Filename: httpDocument.cxx
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

#include "httpDocument.h"
#include "httpClient.h"
#include "bioStream.h"
#include "chunkedStream.h"
#include "identityStream.h"

#ifdef HAVE_SSL

TypeHandle HTTPDocument::_type_handle;

static const char base64_table[64] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '+', '/',
};

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::Constructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
HTTPDocument::
HTTPDocument(HTTPClient *client, BIO *bio) :
  _client(client),
  _bio(bio)
{
  _owns_bio = false;
  _source = (IBioStream *)NULL;
  _persistent_connection = false;
  _state = S_new;
  _read_index = 0;
  _file_size = 0;
  _status_code = 0;
  _status_string = "No connection";
  _proxy = _client->get_proxy();
  _http_version = _client->get_http_version();
  _http_version_string = _client->get_http_version_string();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::send_request
//       Access: Private
//  Description: This is normally called immediately after
//               construction to send the request to the server and
//               read the response.  It can't be called as part of the
//               constructor because it may involve an up-and-down
//               change in the reference count of the HTTPDocument
//               object, which would inadvertently cause the object to
//               be deleted if it hasn't returned from its constructor
//               yet!
////////////////////////////////////////////////////////////////////
bool HTTPDocument::
send_request(const string &method, const URLSpec &url, const string &body) {
  // Let's call this before we call make_header(), so we'll get the
  // right HTTP version and proxy information etc.
  set_url(url);
  if (!prepare_for_next()) {
    return false;
  }

  string header;
  make_header(header, method, url, body);
  send_request(header, body);

  if ((get_status_code() / 100) == 3 && get_status_code() != 305) {
    // Redirect.  Should we handle it automatically?
    if (!get_redirect().empty() && (method == "GET" || method == "HEAD")) {
      // Sure!
      pset<URLSpec> already_seen;
      bool keep_going;
      do {
        keep_going = false;
        if (downloader_cat.is_debug()) {
          downloader_cat.debug()
            << "following redirect to " << get_redirect() << "\n";
        }
        URLSpec new_url = get_redirect();
        if (already_seen.insert(new_url).second) {
          if (url.has_username()) {
            new_url.set_username(url.get_username());
          }
          set_url(new_url);
          if (prepare_for_next()) {
            make_header(header, method, new_url, body);
            send_request(header, body);
            keep_going =
              ((get_status_code() / 100) == 3 && get_status_code() != 305);
          }
        }
      } while (keep_going);
    }
  }

  return is_valid();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::send_request
//       Access: Private
//  Description: This is a lower-level interface than the above
//               send_request(); it accepts a header and body string
//               that have already been defined.
////////////////////////////////////////////////////////////////////
bool HTTPDocument::
send_request(const string &header, const string &body) {
  if (prepare_for_next()) {
    // Tack on a proxy authorization if it is called for.  Assume we
    // can use the same authorization we used last time.
    string proxy_auth_header = header;
    if (!_proxy.empty() && !_client->_proxy_authorization.empty()) {
      proxy_auth_header += "Proxy-Authorization: ";
      proxy_auth_header += _client->_proxy_authorization;
      proxy_auth_header += "\r\n";
    }
    issue_request(proxy_auth_header, body);

    if (get_status_code() == 407 && !_proxy.empty()) {
      // 407: not authorized to proxy.  Try to get the authorization.
      string authenticate_request = get_header_value("Proxy-Authenticate");
      string authorization;
      if (get_authorization(authorization, authenticate_request, _proxy, true)) {
        if (_client->_proxy_authorization != authorization) {
          // Change the authorization.
          _client->_proxy_authorization = authorization;
          proxy_auth_header = header;
          proxy_auth_header += "Proxy-Authorization: ";
          proxy_auth_header += _client->_proxy_authorization;
          proxy_auth_header += "\r\n";
          if (prepare_for_next()) {
            issue_request(proxy_auth_header, body);
          }
        }
      }
    }

    if (get_status_code() == 401) {
      // 401: not authorized to remote server.  Try to get the authorization.
      string authenticate_request = get_header_value("WWW-Authenticate");
      string authorization;
      if (get_authorization(authorization, authenticate_request, _url, false)) {
        string web_auth_header = proxy_auth_header;
        web_auth_header += "Authorization: ";
        web_auth_header += authorization;
        web_auth_header += "\r\n";
        if (prepare_for_next()) {
          issue_request(web_auth_header, body);
        }
      }
    }
  }

  return is_valid();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
HTTPDocument::
~HTTPDocument() {
  free_bio();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::get_file_system
//       Access: Published, Virtual
//  Description: Returns the VirtualFileSystem this file is associated
//               with.
////////////////////////////////////////////////////////////////////
VirtualFileSystem *HTTPDocument::
get_file_system() const {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::get_filename
//       Access: Published, Virtual
//  Description: Returns the full pathname to this file within the
//               virtual file system.
////////////////////////////////////////////////////////////////////
Filename HTTPDocument::
get_filename() const {
  return Filename();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::is_regular_file
//       Access: Published, Virtual
//  Description: Returns true if this file represents a regular file
//               (and read_file() may be called), false otherwise.
////////////////////////////////////////////////////////////////////
bool HTTPDocument::
is_regular_file() const {
  return is_valid();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::will_close_connection
//       Access: Public
//  Description: Returns true if the server has indicated it will
//               close the connection after this document has been
//               read, or false if it will remain open (and future
//               documents may be requested on the same connection).
////////////////////////////////////////////////////////////////////
bool HTTPDocument::
will_close_connection() const {
  if (get_http_version() < HTTPClient::HV_11) {
    // pre-HTTP 1.1 always closes.
    return true;
  }

  string connection = get_header_value("Connection");
  if (downcase(connection) == "close") {
    // The server says it will close.
    return true;
  }

  // Assume the serve will keep it open.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::open_read_file
//       Access: Public, Virtual
//  Description: Opens the document for reading.  Returns a newly
//               allocated istream on success (which you should
//               eventually delete when you are done reading).
//               Returns NULL on failure.  This may only be called
//               once for a particular HTTPDocument.
////////////////////////////////////////////////////////////////////
istream *HTTPDocument::
open_read_file() const {
  // TODO: make this smarter about reference-counting the source
  // stream properly so we can return an istream and not worry about
  // future interference to or from the HTTPDocument.
  bool persist = (get_persistent_connection() && !will_close_connection());
  return ((HTTPDocument *)this)->read_body(!persist);
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::get_header_value
//       Access: Published
//  Description: Returns the HTML header value associated with the
//               indicated key, or empty string if the key was not
//               defined in the message returned by the server.
////////////////////////////////////////////////////////////////////
string HTTPDocument::
get_header_value(const string &key) const {
  Headers::const_iterator hi = _headers.find(downcase(key));
  if (hi != _headers.end()) {
    return (*hi).second;
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::write_headers
//       Access: Published
//  Description: Outputs a list of all headers defined by the server
//               to the indicated output stream.
////////////////////////////////////////////////////////////////////
void HTTPDocument::
write_headers(ostream &out) const {
  Headers::const_iterator hi;
  for (hi = _headers.begin(); hi != _headers.end(); ++hi) {
    out << (*hi).first << ": " << (*hi).second << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::establish_connection
//       Access: Private
//  Description: Establishes a connection to the server, using the
//               appropriate means.  Returns true if a connection is
//               successfully established (and _bio represents the
//               connection), or false otherwise (and _bio is either
//               NULL or an invalid connection.)
////////////////////////////////////////////////////////////////////
bool HTTPDocument::
establish_connection() {
  nassertr(_bio == (BIO *)NULL, false);

  bool result;
  if (_proxy.empty()) {
    if (_url.get_scheme() == "https") {
      result = establish_https();
    } else {
      result = establish_http();
    }
  } else {
    if (_url.get_scheme() == "https") {
      result = establish_https_proxy();
    } else {
      result = establish_http_proxy();
    }
  }    

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::establish_http
//       Access: Private
//  Description: Establishes a connection to the server directly,
//               without using a proxy.
////////////////////////////////////////////////////////////////////
bool HTTPDocument::
establish_http() {
  ostringstream server;
  server << _url.get_server() << ":" << _url.get_port();
  string server_str = server.str();

  _bio = BIO_new_connect((char *)server_str.c_str());

  if (downloader_cat.is_debug()) {
    downloader_cat.debug() << "connecting to " << server_str << "\n";
  }
  if (BIO_do_connect(_bio) <= 0) {
    downloader_cat.info()
      << "Could not contact server " << server_str << "\n";
#ifdef REPORT_SSL_ERRORS
    ERR_print_errors_fp(stderr);
#endif
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::establish_https
//       Access: Private
//  Description: Establishes a connection to the secure server
//               directly, without using a proxy.
////////////////////////////////////////////////////////////////////
bool HTTPDocument::
establish_https() {
  ostringstream server;
  server << _url.get_server() << ":" << _url.get_port();
  string server_str = server.str();

  _bio = BIO_new_connect((char *)server_str.c_str());

  if (downloader_cat.is_debug()) {
    downloader_cat.debug() << "connecting to " << server_str << "\n";
  }
  if (BIO_do_connect(_bio) <= 0) {
    downloader_cat.info()
      << "Could not contact server " << server_str << "\n";
#ifdef REPORT_SSL_ERRORS
    ERR_print_errors_fp(stderr);
#endif
    return false;
  }

  return make_https_connection();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::establish_http_proxy
//       Access: Private
//  Description: Establishes a connection to the server through a
//               proxy.
////////////////////////////////////////////////////////////////////
bool HTTPDocument::
establish_http_proxy() {
  ostringstream proxy_server;
  proxy_server << _proxy.get_server() << ":" << _proxy.get_port();
  string proxy_server_str = proxy_server.str();

  _bio = BIO_new_connect((char *)proxy_server_str.c_str());

  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "connecting to proxy " << proxy_server_str << "\n";
  }
  if (BIO_do_connect(_bio) <= 0) {
    downloader_cat.info()
      << "Could not contact proxy " << proxy_server_str << "\n";
#ifdef REPORT_SSL_ERRORS
    ERR_print_errors_fp(stderr);
#endif
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::establish_https_proxy
//       Access: Private
//  Description: Establishes a connection to the secure server through
//               a proxy.
////////////////////////////////////////////////////////////////////
bool HTTPDocument::
establish_https_proxy() {
  // First, ask the proxy to open a connection for us.
  ostringstream proxy_server;
  proxy_server << _proxy.get_server() << ":" << _proxy.get_port();
  string proxy_server_str = proxy_server.str();

  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "connecting to proxy " << proxy_server_str << "\n";
  }

  ostringstream request;
  request 
    << "CONNECT " << _url.get_server() << ":" << _url.get_port()
    << " " << get_http_version_string() << "\r\n";
  if (_http_version >= HTTPClient::HV_11) {
    request 
      << "Host: " << _url.get_server() << "\r\n";
  }
  string connect_header = request.str();

  _bio = BIO_new_connect((char *)proxy_server_str.c_str());
  if (BIO_do_connect(_bio) <= 0) {
    downloader_cat.info()
      << "Could not contact proxy " << proxy_server_str << ".\n";
#ifdef REPORT_SSL_ERRORS
    ERR_print_errors_fp(stderr);
#endif
    return false;
  }

  // Now issue the request and read the response from the proxy.

  // Temporarily flag our bio as being owned externally, so our call
  // to send_request() won't end up with a recursive call back to
  // establish_connection().
  _owns_bio = false;

  string old_proxy_authorization = _client->_proxy_authorization;
  bool connected = send_request(connect_header, string());
  if (!connected && get_status_code() == 407 &&
      _client->_proxy_authorization != old_proxy_authorization) {
    // If we ended up with a 407 (proxy authorization required), and
    // we changed authorization strings recently, then try the new
    // authorization string, once.  (Normally, send_request() would
    // have tried it again automatically, but we may have prevented
    // that by setting _owns_bio to false.)
    if (!prepare_for_next()) {
      free_bio();
      _bio = BIO_new_connect((char *)proxy_server_str.c_str());
      if (BIO_do_connect(_bio) <= 0) {
        downloader_cat.info()
          << "Could not contact proxy " << proxy_server_str 
          << " a second time.\n";
#ifdef REPORT_SSL_ERRORS
        ERR_print_errors_fp(stderr);
#endif
        _owns_bio = true;
        return false;
      }
    }
    connected = send_request(connect_header, string());
  }

  // Now that we've connected, be honest with the _owns_bio flag: if
  // we're here, we know we really do own the BIO pointer (we just
  // created it, after all.)
  _owns_bio = true;

  if (!connected) {
    downloader_cat.info()
      << "proxy would not open connection to " << _url.get_authority()
      << ": " << get_status_code() << " "
      << get_status_string() << "\n";
    
    if (_client->get_verify_ssl() == HTTPClient::VS_no_verify) {
      // If the proxy refused to open a raw connection for us, see
      // if it will handle the https communication itself.  For
      // other error codes, just return error.  (We can only
      // reliably do this if verify_ssl is minimal, since we're not
      // sure whether to trust the proxy to do the verification for
      // us.)
      if ((get_status_code() / 100) == 4) {
        free_bio();
        return establish_http_proxy();
      }
    }
    return false;
  }

  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "connection established to " << _url.get_authority() << "\n";
  }

  // Reset the state to make it appear like we just opened this
  // connection, even though we've already gone through an HTTP
  // handshake.
  _state = S_new;

  // Also reset the HTTP version, because we don't want to limit
  // ourselves to whatever version the proxy returned after
  // successfully connecting.
  _http_version = _client->get_http_version();
  _http_version_string = _client->get_http_version_string();

  // Ok, we now have a connection to our actual server, so start
  // speaking SSL and then ask for the document we really want.
  return make_https_connection();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::make_https_connection
//       Access: Private
//  Description: Starts speaking SSL over the opened connection.
//               Returns true on success, false if the SSL connection
//               cannot be established for some reason.
////////////////////////////////////////////////////////////////////
bool HTTPDocument::
make_https_connection() {
  BIO *sbio = BIO_new_ssl(_client->_ssl_ctx, true);
  BIO_push(sbio, _bio);

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
      << _url.get_server() << ":" << _url.get_port() << "\n";
#ifdef REPORT_SSL_ERRORS
    ERR_print_errors_fp(stderr);
#endif
    // It seems to be an error to free sbio at this point; perhaps
    // it's already been freed?
    return false;
  }

  // Now that we've made an SSL handshake, we can use the SSL bio to
  // do all of our communication henceforth.
  _bio = sbio;

  long verify_result = SSL_get_verify_result(ssl);
  if (verify_result == X509_V_ERR_CERT_HAS_EXPIRED) {
    downloader_cat.info()
      << "Expired certificate from " << _url.get_server() << ":"
      << _url.get_port() << "\n";
    if (_client->get_verify_ssl() == HTTPClient::VS_normal) {
      return false;
    }

  } else if (verify_result == X509_V_ERR_CERT_NOT_YET_VALID) {
    downloader_cat.info()
      << "Premature certificate from " << _url.get_server() << ":"
      << _url.get_port() << "\n";
    if (_client->get_verify_ssl() == HTTPClient::VS_normal) {
      return false;
    }

  } else if (verify_result != X509_V_OK) {
    downloader_cat.info()
      << "Unable to verify identity of " << _url.get_server() << ":" 
      << _url.get_port() << ", verify error code " << verify_result << "\n";
    if (_client->get_verify_ssl() != HTTPClient::VS_no_verify) {
      return false;
    }
  }

  X509 *cert = SSL_get_peer_certificate(ssl);
  if (cert == (X509 *)NULL) {
    downloader_cat.info()
      << "No certificate was presented by server.\n";
    if (_client->get_verify_ssl() != HTTPClient::VS_no_verify ||
        !_client->_expected_servers.empty()) {
      return false;
    }

  } else {
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << "Received certificate from server:\n" << flush;
      X509_print_fp(stderr, cert);
      fflush(stderr);
    }

    X509_NAME *subject = X509_get_subject_name(cert);

    if (downloader_cat.is_debug()) {
      string org_name = get_x509_name_component(subject, NID_organizationName);
      string org_unit_name = get_x509_name_component(subject, NID_organizationalUnitName);
      string common_name = get_x509_name_component(subject, NID_commonName);
      
      downloader_cat.debug()
        << "Server is " << common_name << " from " << org_unit_name
        << " / " << org_name << "\n";
    }

    if (!verify_server(subject)) {
      downloader_cat.info()
        << "Server does not match any expected server.\n";
      return false;
    }
      
    X509_free(cert);
  }

  return true;
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
//     Function: HTTPDocument::verify_server
//       Access: Private
//  Description: Returns true if the indicated server matches one of
//               our expected servers (or the list of expected servers
//               is empty), or false if it does not match any of our
//               expected servers.
////////////////////////////////////////////////////////////////////
bool HTTPDocument::
verify_server(X509_NAME *subject) const {
  if (_client->_expected_servers.empty()) {
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

  HTTPClient::ExpectedServers::const_iterator ei;
  for (ei = _client->_expected_servers.begin();
       ei != _client->_expected_servers.end();
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

    for (ei = _client->_expected_servers.begin();
         ei != _client->_expected_servers.end();
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
//     Function: HTTPDocument::get_x509_name_component
//       Access: Private, Static
//  Description: Returns the indicated component of the X509 name as a
//               string, if defined, or empty string if it is not.
////////////////////////////////////////////////////////////////////
string HTTPDocument::
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
//     Function: HTTPDocument::x509_name_subset
//       Access: Private, Static
//  Description: Returns true if name_a is a subset of name_b: each
//               property of name_a is defined in name_b, and the
//               defined value is equivalent to that of name_a.
////////////////////////////////////////////////////////////////////
bool HTTPDocument::
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

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::make_header
//       Access: Private
//  Description: Formats the appropriate GET or POST (or whatever)
//               request to send to the server.  Also saves the
//               indicated url.
////////////////////////////////////////////////////////////////////
void HTTPDocument::
make_header(string &header, const string &method, 
            const URLSpec &url, const string &body) {
  set_url(url);
  _method = method;

  string path;
  if (_proxy.empty()) {
    path = _url.get_path();
  } else {
    URLSpec url_no_username = _url;
    url_no_username.set_username(string());
    path = url_no_username.get_url();
  }

  ostringstream stream;

  stream 
    << method << " " << path << " " << get_http_version_string() << "\r\n";

  if (_http_version >= HTTPClient::HV_11) {
    stream 
      << "Host: " << _url.get_server() << "\r\n";
    if (!get_persistent_connection()) {
      stream
        << "Connection: close\r\n";
    }
  }

  if (!body.empty()) {
    stream
      << "Content-Type: application/x-www-form-urlencoded\r\n"
      << "Content-Length: " << body.length() << "\r\n";
  }

  header = stream.str();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::set_url
//       Access: Private
//  Description: Specifies the document's URL before attempting a
//               connection.  This controls the name of the server to
//               be contacted, etc.
////////////////////////////////////////////////////////////////////
void HTTPDocument::
set_url(const URLSpec &url) {
  // If we change between http and https, we have to reset the
  // connection regardless of proxy.  Otherwise, we have to drop the
  // connection if the server or port changes, unless we're
  // communicating through a proxy.

  if (url.get_scheme() != _url.get_scheme() ||
      (_proxy.empty() && (url.get_server() != _url.get_server() || 
                          url.get_port() != _url.get_port()))) {
    free_bio();
  }
  _url = url;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::issue_request
//       Access: Private
//  Description: Issues the request to the HTTP server and waits for a
//               response.
////////////////////////////////////////////////////////////////////
void HTTPDocument::
issue_request(const string &header, const string &body) {
  if (_bio != (BIO *)NULL) {
    string request = header;
    request += "\r\n";
    request += body;
#ifndef NDEBUG
    if (downloader_cat.is_spam()) {
      show_send(request);
    }
#endif
    BIO_puts(_bio, request.c_str());
    read_http_response();

    if (_source->eof() || _source->fail()) {
      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << "Whoops, socket closed.\n";
        free_bio();
        if (prepare_for_next()) {
#ifndef NDEBUG
          if (downloader_cat.is_spam()) {
            show_send(request);
          }
#endif
          BIO_puts(_bio, request.c_str());
          read_http_response();
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::read_http_response
//       Access: Private
//  Description: Reads all of the responses from the server up until
//               the first blank line, and stores the list of header
//               key:value pairs so retrieved.
////////////////////////////////////////////////////////////////////
void HTTPDocument::
read_http_response() {
  nassertv(_source != (IBioStream *)NULL);
  _headers.clear();
  _realm = string();

  // The first line back should include the HTTP version and the
  // result code.
  string line;
  getline(*_source, line);
  if (!line.empty() && line[line.length() - 1] == '\r') {
    line = line.substr(0, line.length() - 1);
  }
  if (downloader_cat.is_spam()) {
    downloader_cat.spam() << "recv: " << line << "\n";
  }

  if (!(*_source) || line.length() < 5 || line.substr(0, 5) != "HTTP/") {
    // Not an HTTP response.
    _status_code = 0;
    _status_string = "Not an HTTP response";
    return;
  }

  // Split out the first line into its three components.
  size_t p = 5;
  while (p < line.length() && !isspace(line[p])) {
    p++;
  }
  _http_version_string = line.substr(0, p);
  _http_version = HTTPClient::parse_http_version_string(_http_version_string);

  while (p < line.length() && isspace(line[p])) {
    p++;
  }
  size_t q = p;
  while (q < line.length() && !isspace(line[q])) {
    q++;
  }
  string status_code = line.substr(p, q - p);
  _status_code = atoi(status_code.c_str());

  while (q < line.length() && isspace(line[q])) {
    q++;
  }
  _status_string = line.substr(q, line.length() - q);

  // Now read the rest of the lines.  These will be field: value
  // pairs.
  string field_name;
  string field_value;

  getline(*_source, line);
  if (!line.empty() && line[line.length() - 1] == '\r') {
    line = line.substr(0, line.length() - 1);
  }
  if (downloader_cat.is_spam()) {
    downloader_cat.spam() << "recv: " << line << "\n";
  }

  while (!_source->eof() && !_source->fail() && !line.empty()) {
    if (isspace(line[0])) {
      // If the line begins with a space, that continues the previous
      // field.
      p = 0;
      while (p < line.length() && isspace(line[p])) {
        p++;
      }
      field_value += line.substr(p - 1);

    } else {
      // If the line does not begin with a space, that defines a new
      // field.
      if (!field_name.empty()) {
        store_header_field(field_name, field_value);
        field_value = string();
      }

      size_t colon = line.find(':');
      if (colon != string::npos) {
        field_name = downcase(line.substr(0, colon));
        p = colon + 1;
        while (p < line.length() && isspace(line[p])) {
          p++;
        }
        field_value = line.substr(p);
      }
    }

    getline(*_source, line);
    if (!line.empty() && line[line.length() - 1] == '\r') {
      line = line.substr(0, line.length() - 1);
    }
    if (downloader_cat.is_spam()) {
      downloader_cat.spam() << "recv: " << line << "\n";
    }
  }
  if (!field_name.empty()) {
    store_header_field(field_name, field_value);
    field_value = string();
  }

  // A blank line terminates the headers.
  _state = S_read_header;

  if (get_status_code() / 100 == 1 ||
      get_status_code() == 204 ||
      get_status_code() == 304 || 
      (_method == "HEAD" || _method == "CONNECT")) {
    // These status codes, or method HEAD or CONNECT, indicate we have
    // no body.  Therefore, we have already read the (nonexistent)
    // body.
    _state = S_read_trailer;
  }

  _file_size = 0;
  string content_length = get_header_value("Content-Length");
  if (!content_length.empty()) {
    _file_size = atoi(content_length.c_str());
  }
  _redirect = get_header_value("Location");
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::store_header_field
//       Access: Private
//  Description: Stores a single name: value pair in the header list,
//               or appends the value to the end of the existing
//               value, if the header has been repeated.
////////////////////////////////////////////////////////////////////
void HTTPDocument::
store_header_field(const string &field_name, const string &field_value) {
  pair<Headers::iterator, bool> insert_result =
    _headers.insert(Headers::value_type(field_name, field_value));

  if (!insert_result.second) {
    // It didn't insert; thus, the field already existed.  Append the
    // new value.
    Headers::iterator hi = insert_result.first;
    (*hi).second += ", ";
    (*hi).second += field_value;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::get_authorization
//       Access: Private
//  Description: Looks for a username:password to satisfy the given
//               authenticate_request string from the server or proxy.
//               If found, fills in authorization and returns true;
//               otherwise, returns false.
////////////////////////////////////////////////////////////////////
bool HTTPDocument::
get_authorization(string &authorization, const string &authenticate_request,
                  const URLSpec &url, bool is_proxy) {
  AuthenticationSchemes schemes;
  parse_authentication_schemes(schemes, authenticate_request);

  AuthenticationSchemes::iterator si;
  si = schemes.find("basic");
  if (si != schemes.end()) {
    return get_basic_authorization(authorization, (*si).second, url, is_proxy);
  }

  downloader_cat.warning() 
    << "Don't know how to use any of the server's available authorization schemes:\n";
  for (si = schemes.begin(); si != schemes.end(); ++si) {
    downloader_cat.warning() << (*si).first << "\n";
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::downcase
//       Access: Private, Static
//  Description: Returns the input string with all uppercase letters
//               converted to lowercase.
////////////////////////////////////////////////////////////////////
string HTTPDocument::
downcase(const string &s) {
  string result;
  result.reserve(s.size());
  string::const_iterator p;
  for (p = s.begin(); p != s.end(); ++p) {
    result += tolower(*p);
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::base64_encode
//       Access: Private, Static
//  Description: Returns the input string encoded using base64.  No
//               respect is paid to maintaining a 76-char line length.
////////////////////////////////////////////////////////////////////
string HTTPDocument::
base64_encode(const string &s) {
  // Collect the string 3 bytes at a time into 24-bit words, then
  // output each word using 4 bytes.
  size_t num_words = (s.size() + 2) / 3;
  string result;
  result.reserve(num_words * 4);
  size_t p;
  for (p = 0; p + 2 < s.size(); p += 3) {
    unsigned int word = 
      ((unsigned)s[p] << 16) |
      ((unsigned)s[p + 1] << 8) |
      ((unsigned)s[p + 2]);
    result += base64_table[(word >> 18) & 0x3f];
    result += base64_table[(word >> 12) & 0x3f];
    result += base64_table[(word >> 6) & 0x3f];
    result += base64_table[(word) & 0x3f];
  }
  // What's left over?
  if (p < s.size()) {
    unsigned int word = ((unsigned)s[p] << 16);
    p++;
    if (p < s.size()) {
      word |= ((unsigned)s[p] << 8);
      p++;
      nassertr(p == s.size(), result);

      result += base64_table[(word >> 18) & 0x3f];
      result += base64_table[(word >> 12) & 0x3f];
      result += base64_table[(word >> 6) & 0x3f];
      result += '=';
    } else {
      result += base64_table[(word >> 18) & 0x3f];
      result += base64_table[(word >> 12) & 0x3f];
      result += '=';
      result += '=';
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::scan_quoted_or_unquoted_string
//       Access: Private, Static
//  Description: Scans the string source beginning at character
//               position start, to identify either the
//               (spaced-delimited) unquoted string there, or the
//               (quote-delimited) quoted string.  In either case,
//               fills the string found into result, and returns the
//               next character position after the string (or after
//               its closing quote mark).
////////////////////////////////////////////////////////////////////
size_t HTTPDocument::
scan_quoted_or_unquoted_string(string &result, const string &source, 
                               size_t start) {
  result = string();

  if (start < source.length()) {
    if (source[start] == '"') {
      // Quoted string.
      size_t p = start + 1;
      while (p < source.length() && source[p] != '"') {
        if (source[p] == '\\') {
          // Backslash escapes.
          ++p;
          if (p < source.length()) {
            result += source[p];
            ++p;
          }
        } else {
          result += source[p];
          ++p;
        }
      }
      if (p < source.length()) {
        ++p;
      }
      return p;
    }

    // Unquoted string.
    size_t p = start;
    while (p < source.length() && source[p] != ',' && !isspace(source[p])) {
      result += source[p];
      ++p;
    }

    return p;
  }

  // Empty string.
  return start;
}

#ifndef NDEBUG
////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::show_send
//       Access: Private, Static
//  Description: Writes the outgoing message, one line at a time, to
//               the debugging log.
////////////////////////////////////////////////////////////////////
void HTTPDocument::
show_send(const string &message) {
  size_t start = 0;
  size_t newline = message.find('\n', start);
  while (newline != string::npos) {
    downloader_cat.spam()
      << "send: " << message.substr(start, newline - start + 1);
    start = newline + 1;
    newline = message.find('\n', start);
  }

  if (start < message.length()) {
    downloader_cat.spam()
      << "send: " << message.substr(start) << " (no newline)\n";
  }
}
#endif   // NDEBUG

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::parse_authentication_schemes
//       Access: Private, Static
//  Description: Decodes the text following a WWW-Authenticate: or
//               Proxy-Authenticate: header field.
////////////////////////////////////////////////////////////////////
void HTTPDocument::
parse_authentication_schemes(HTTPDocument::AuthenticationSchemes &schemes,
                             const string &field_value) {
  // This string will consist of one or more records of the form:
  //
  //  scheme token=value[,token=value[,...]]
  //
  // If there are multiple records, they will be comma-delimited,
  // which makes parsing just a bit tricky.

  // Start by skipping initial whitespace.
  size_t p = 0;
  while (p < field_value.length() && isspace(field_value[p])) {
    ++p;
  }

  if (p < field_value.length()) {
    size_t q = p;
    while (q < field_value.length() && !isspace(field_value[q])) {
      ++q;
    }
    // Here's our first scheme.
    string scheme = downcase(field_value.substr(p, q - p));
    Tokens *tokens = &(schemes[scheme]);
    
    // Now pull off the tokens, one at a time.
    p = q + 1;
    while (p < field_value.length()) {
      q = p;
      while (q < field_value.length() && field_value[q] != '=' && 
             field_value[q] != ',' && !isspace(field_value[q])) {
        ++q;
      }
      if (field_value[q] == '=') {
        // This is a token.
        string token = downcase(field_value.substr(p, q - p));
        string value;
        p = scan_quoted_or_unquoted_string(value, field_value, q + 1);
        (*tokens)[token] = value;

        // Skip trailing whitespace and extra commas.
        while (p < field_value.length() && 
               (field_value[p] == ',' || isspace(field_value[p]))) {
          ++p;
        }

      } else {
        // This is not a token; it must be the start of a new scheme.
        scheme = downcase(field_value.substr(p, q - p));
        tokens = &(schemes[scheme]);
        p = q + 1;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::read_body
//       Access: Private
//  Description: Returns a newly-allocated istream suitable for
//               reading the body of the document.  If owns_source is
//               true, the ownership of the _source pointer will be
//               passed to the istream; otherwise, it will be
//               retained.  (owns_source must be true in order to read
//               "identity" encoded documents.)
////////////////////////////////////////////////////////////////////
istream *HTTPDocument::
read_body(bool owns_source) {
  if (_state != S_read_header || _source == (IBioStream *)NULL) {
    return NULL;
  }

  string transfer_coding = downcase(get_header_value("Transfer-Encoding"));
  string content_length = get_header_value("Content-Length");

  istream *result;
  if (transfer_coding == "chunked") {
    // Chunked can be used directly.
    _file_size = 0;
    _state = S_started_body;
    _read_index++;
    result = new IChunkedStream(_source, owns_source, (HTTPDocument *)this);
    if (owns_source) {
      _source = (IBioStream *)NULL;
    }

  } else if (!content_length.empty()) {
    // If we have a content length, we can use an IdentityStream.
    _state = S_started_body;
    _read_index++;
    result = new IIdentityStream(_source, owns_source, (HTTPDocument *)this, _file_size);
    if (owns_source) {
      _source = (IBioStream *)NULL;
    }

  } else if (owns_source) {
    // If we own the source, we can return it.
    _state = S_started_body;
    result = _source;
    _source = (IBioStream *)NULL;

  } else {
    // Otherwise, we don't own the source; too bad.
    result = NULL;
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::prepare_for_next
//       Access: Private
//  Description: Resets the state to prepare it for sending a new
//               request to the server.  This might mean closing the
//               connection and opening a new one, or it might mean
//               skipping past the unread body in the persistent
//               connection, or it might do nothing at all if the body
//               has already been completely read.
////////////////////////////////////////////////////////////////////
bool HTTPDocument::
prepare_for_next() {
  if (get_persistent_connection() && !will_close_connection() &&
      _proxy == _client->get_proxy()) {
    // See if we can reuse the current connection.
    if (_state == S_read_header) {
      // We have read the header; now skip past the body.
      istream *body = read_body(false); 
      if (body != (istream *)NULL) {
        string line;
        getline(*body, line);
        while (!body->fail() && !body->eof()) {
          if (downloader_cat.is_spam()) {
            downloader_cat.spam() << "skip: " << line << "\n";
          }
          getline(*body, line);
        }
        nassertr(body != _source, false);
        delete body;
      }
    }

    if (_source == (IBioStream *)NULL) {
      _source = new IBioStream(_bio, false);
    }

    if (_state == S_read_body) {
      // We have read the body, but there's a trailer to read.
      string line;
      getline(*_source, line);
      if (!line.empty() && line[line.length() - 1] == '\r') {
        line = line.substr(0, line.length() - 1);
      }
      if (downloader_cat.is_spam()) {
        downloader_cat.spam() << "skip: " << line << "\n";
      }
      while (!_source->eof() && !_source->fail() && !line.empty()) {
        getline(*_source, line);
        if (!line.empty() && line[line.length() - 1] == '\r') {
          line = line.substr(0, line.length() - 1);
        }
        if (downloader_cat.is_spam()) {
          downloader_cat.spam() << "skip: " << line << "\n";
        }
      }
      _state = S_read_trailer;
    }

    if (_state == S_read_trailer) {
      // Great; this connection is ready to go!
      return true;
    }
  }

  if (_bio != (BIO *)NULL && _state == S_new) {
    // If we have a BIO and the _state is S_new, then we haven't done
    // anything with the BIO yet, so we can still use it.
    if (_source == (IBioStream *)NULL) {
      _source = new IBioStream(_bio, false);
    }
    return true;
  }

  // Either the client will close the connection after reading the
  // body, or we were only partly through reading the body elsewhere;
  // or possibly we don't have a connection yet at all.  In any case,
  // we must now get a new connection.
  if (_bio != (BIO *)NULL && !_owns_bio) {
    // We have a connection, but we don't own it, so we can't close
    // it.  Too bad.
    return false;
  }
  
  // Go ahead and close the old BIO.
  free_bio();

  _proxy = _client->get_proxy();
  _http_version = _client->get_http_version();
  _http_version_string = _client->get_http_version_string();
  _owns_bio = true;
  if (establish_connection()) {
    _source = new IBioStream(_bio, false);
    return true;
  }

  free_bio();
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::free_bio
//       Access: Private
//  Description: Frees the BIO and its IBioStream object, if
//               allocated.  This will close the connection if it is
//               open.
////////////////////////////////////////////////////////////////////
void HTTPDocument::
free_bio() {
  if (_source != (IBioStream *)NULL) {
    delete _source;
    _source = (IBioStream *)NULL;
  }
  if (_bio != (BIO *)NULL) {
    if (_owns_bio) {
      // TODO: We should be more careful here to manage reference
      // counts so we don't free the bio out from under a BIOStreamBuf
      // that's trying to read from it.
      if (downloader_cat.is_debug()) {
        const URLSpec &url = _proxy.empty() ? _url : _proxy;
        downloader_cat.debug()
          << "Dropping connection to " << url.get_server() << "\n";
      }
      BIO_free_all(_bio);
    }
    _bio = (BIO *)NULL;
  }
  _read_index++;
  _state = S_new;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPDocument::get_basic_authorization
//       Access: Private
//  Description: Looks for a username:password to satisfy the "Basic"
//               scheme authorization request from the server or
//               proxy.
////////////////////////////////////////////////////////////////////
bool HTTPDocument::
get_basic_authorization(string &authorization, const HTTPDocument::Tokens &tokens, const URLSpec &url, bool is_proxy) {
  Tokens::const_iterator ti;
  ti = tokens.find("realm");
  if (ti != tokens.end()) {
    _realm = (*ti).second;
  }

  string username;

  // Look in several places in order to find the matching username.

  // Fist, if there's a username on the URL, that always wins (except
  // when we are looking for a proxy username).
  if (url.has_username() && !is_proxy) {
    username = url.get_username();
  }

  // Otherwise, start looking on the HTTPClient.  
  if (is_proxy) {
    if (username.empty()) {
      // Try the *proxy/realm.
      username = _client->get_username("*proxy", _realm);
    }
    if (username.empty()) {
      // Then, try *proxy/any realm.
      username = _client->get_username("*proxy", string());
    }
  }
  if (username.empty()) {
    // Try the specific server/realm.
    username = _client->get_username(url.get_server(), _realm);
  }
  if (username.empty()) {
    // Then, try the specific server/any realm.
    username = _client->get_username(url.get_server(), string());
  }
  if (username.empty()) {
    // Then, try any server with this realm.
    username = _client->get_username(string(), _realm);
  }
  if (username.empty()) {
    // Then, take the general password.
    username = _client->get_username(string(), string());
  }

  if (username.empty()) {
    // No username:password available.
    return false;
  }

  authorization = "Basic " + base64_encode(username);
  return true;
}

#endif  // HAVE_SSL
