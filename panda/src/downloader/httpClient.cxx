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
#include "httpBasicAuthorization.h"
#include "httpDigestAuthorization.h"
#include "globPattern.h"

#ifdef HAVE_SSL

#include <openssl/rand.h>
#ifdef REPORT_OPENSSL_ERRORS
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
//     Function: trim_blanks
//  Description:
////////////////////////////////////////////////////////////////////
static string
trim_blanks(const string &str) {
  size_t start = 0;
  while (start < str.length() && isspace(str[start])) {
    start++;
  }

  size_t end = str.length();
  while (end > start && isspace(str[end - 1])) {
    end--;
  }

  return str.substr(start, end - start);
}

////////////////////////////////////////////////////////////////////
//     Function: tokenize
//  Description: Chops the source string up into pieces delimited by
//               any of the characters specified in delimiters.
//               Repeated delimiter characters represent zero-length
//               tokens.
//
//               It is the user's responsibility to ensure the output
//               vector is cleared before calling this function; the
//               results will simply be appended to the end of the
//               vector.
////////////////////////////////////////////////////////////////////
static void
tokenize(const string &str, vector_string &words, const string &delimiters) {
  size_t p = 0;
  while (p < str.length()) {
    size_t q = str.find_first_of(delimiters, p);
    if (q == string::npos) {
      words.push_back(str.substr(p));
      return;
    }
    words.push_back(str.substr(p, q - p));
    p = q + 1;
  }
  words.push_back(string());
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
HTTPClient::
HTTPClient() {
  _http_version = HTTPEnum::HV_11;
  _verify_ssl = verify_ssl ? VS_normal : VS_no_verify;
  _ssl_ctx = (SSL_CTX *)NULL;

  set_proxy_spec(http_proxy);
  set_direct_host_spec(http_direct_hosts);

  if (!http_proxy_username.empty()) {
    set_username("*proxy", "", http_proxy_username);
  }

  set_cipher_list(ssl_cipher_list);

  {
    // Also load in the general usernames.
    Config::ConfigTable::Symbol http_usernames;
    config_downloader.GetAll("http-username", http_usernames);
    
    // When we use GetAll(), we might inadvertently read duplicate
    // lines.  Filter them out with a set.
    pset<string> already_read;
    
    Config::ConfigTable::Symbol::iterator si;
    for (si = http_usernames.begin(); si != http_usernames.end(); ++si) {
      string http_username = (*si).Val();
      if (already_read.insert(http_username).second) {
        add_http_username(http_username);
      }
    }
  }

  // The first time we create an HTTPClient, we must initialize the
  // OpenSSL library.
  if (!_ssl_initialized) {
    initialize_ssl();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
HTTPClient::
HTTPClient(const HTTPClient &copy) {
  _ssl_ctx = (SSL_CTX *)NULL;

  (*this) = copy;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void HTTPClient::
operator = (const HTTPClient &copy) {
  _proxies_by_scheme = copy._proxies_by_scheme;
  _direct_hosts = copy._direct_hosts;
  _http_version = copy._http_version;
  _verify_ssl = copy._verify_ssl;
  _usernames = copy._usernames;
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
  if (_ssl_ctx != (SSL_CTX *)NULL) {
    nassertv(_ssl_ctx->cert_store == _x509_store);
    _ssl_ctx->cert_store = NULL;
    SSL_CTX_free(_ssl_ctx);
  }

  // Free all of the expected server definitions.
  clear_expected_servers();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::set_proxy
//       Access: Published
//  Description: Specifies the proxy URL to handle all http and
//               https requests.  Deprecated.
////////////////////////////////////////////////////////////////////
void HTTPClient::
set_proxy(const URLSpec &proxy) {
  set_proxy_spec(proxy.get_url());
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_proxy
//       Access: Published
//  Description: Returns the proxy URL to handle all http and
//               https requests.  Deprecated.
////////////////////////////////////////////////////////////////////
URLSpec HTTPClient::
get_proxy() const {
  pvector<URLSpec> proxies;
  get_proxies_for_url(URLSpec("http://"), proxies);
  if (!proxies.empty()) {
    return proxies[0];
  }
  return URLSpec();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::set_proxy_spec
//       Access: Published
//  Description: Specifies the complete set of proxies to use for all
//               schemes.  This is either a semicolon-delimited set of
//               hostname:ports, or a semicolon-delimited set of pairs
//               of the form "scheme=hostname:port", or a combination.
//               A particular scheme and/or proxy host may be listed
//               more than once.  This is a convenience function that
//               can be used in place of explicit calls to add_proxy()
//               for each scheme/proxy pair.
////////////////////////////////////////////////////////////////////
void HTTPClient::
set_proxy_spec(const string &proxy_spec) {
  clear_proxy();

  // Tokenize the string based on the semicolons.
  vector_string proxies;
  tokenize(proxy_spec, proxies, ";");
  
  for (vector_string::const_iterator pi = proxies.begin();
       pi != proxies.end();
       ++pi) {
    const string &spec = (*pi);

    // Divide out the scheme and the hostname.
    string scheme;
    URLSpec url;
    size_t equals = spec.find('=');
    if (equals == string::npos) {
      scheme = "";
      url = URLSpec(spec, true);
    } else {
      scheme = trim_blanks(spec.substr(0, equals));
      url = URLSpec(spec.substr(equals + 1), true);
    }

    add_proxy(scheme, url);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_proxy_spec
//       Access: Published
//  Description: Returns the complete set of proxies to use for all
//               schemes.  This is a string of the form specified by
//               set_proxy_spec(), above.  Note that the string
//               returned by this function may not be exactly the same
//               as the string passed into set_proxy_spec(), since the
//               string is regenerated from the internal storage
//               structures and may therefore be reordered.
////////////////////////////////////////////////////////////////////
string HTTPClient::
get_proxy_spec() const {
  string result;

  ProxiesByScheme::const_iterator si;
  for (si = _proxies_by_scheme.begin(); si != _proxies_by_scheme.end(); ++si) {
    const string &scheme = (*si).first;
    const Proxies &proxies = (*si).second;
    Proxies::const_iterator pi;
    for (pi = proxies.begin(); pi != proxies.end(); ++pi) {
      const URLSpec &url = (*pi);
      if (!result.empty()) {
        result += ";";
      }
      if (!scheme.empty()) {
        result += scheme;
        result += "=";
      }
      result += url.get_url();
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::set_direct_host_spec
//       Access: Published
//  Description: Specifies the set of hosts that should be connected
//               to directly, without using a proxy.  This is a
//               semicolon-separated list of hostnames that may
//               contain wildcard characters ("*").
////////////////////////////////////////////////////////////////////
void HTTPClient::
set_direct_host_spec(const string &direct_host_spec) {
  clear_direct_host();

  // Tokenize the string based on the semicolons.
  vector_string hosts;
  tokenize(direct_host_spec, hosts, ";");
  
  for (vector_string::const_iterator hi = hosts.begin();
       hi != hosts.end();
       ++hi) {
    string spec = trim_blanks(*hi);

    // We should be careful to avoid adding any empty hostnames to the
    // list.  In particular, we will get one empty hostname if the
    // direct_host_spec is empty.
    if (!spec.empty()) {
      add_direct_host(spec);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_direct_host_spec
//       Access: Published
//  Description: Returns the set of hosts that should be connected
//               to directly, without using a proxy, as a
//               semicolon-separated list of hostnames that may
//               contain wildcard characters ("*").
////////////////////////////////////////////////////////////////////
string HTTPClient::
get_direct_host_spec() const {
  string result;

  DirectHosts::const_iterator si;
  for (si = _direct_hosts.begin(); si != _direct_hosts.end(); ++si) {
    const GlobPattern &host = (*si);

    if (!result.empty()) {
      result += ";";
    }
    result += host.get_pattern();
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::clear_proxy
//       Access: Published
//  Description: Resets the proxy spec to empty.  Subsequent calls to
//               add_proxy() may be made to build up the set of proxy
//               servers.
////////////////////////////////////////////////////////////////////
void HTTPClient::
clear_proxy() {
  _proxies_by_scheme.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::add_proxy
//       Access: Published
//  Description: Adds the indicated proxy host as a proxy for
//               communications on the given scheme.  Usually the
//               scheme is "http" or "https".  It may be the empty
//               string to indicate a general proxy.
////////////////////////////////////////////////////////////////////
void HTTPClient::
add_proxy(const string &scheme, const URLSpec &proxy) {
  if (!proxy.empty()) {
    URLSpec proxy_url(proxy);

    // The scheme is always converted to lowercase.
    string lc_scheme;
    lc_scheme.reserve(scheme.length());
    string::const_iterator si;
    for (si = scheme.begin(); si != scheme.end(); ++si) {
      lc_scheme += tolower(*si);
    }
    
    // Remove the trailing colon, if there is one.
    if (!lc_scheme.empty() && lc_scheme[lc_scheme.length() - 1] == ':') {
      lc_scheme = lc_scheme.substr(0, lc_scheme.length() - 1);
    }
    
    if (lc_scheme == "socks") {
      // Scheme "socks" implies we talk to the proxy via the "socks"
      // scheme, no matter what scheme the user actually specified.
      proxy_url.set_scheme("socks");
      
    } else if (!proxy_url.has_scheme()) {
      // Otherwise, if the user didn't specify a scheme to talk to the
      // proxy, the default is "http".
      proxy_url.set_scheme("http");
    }
    
    _proxies_by_scheme[lc_scheme].push_back(proxy_url);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::clear_direct_host
//       Access: Published
//  Description: Resets the set of direct hosts to empty.  Subsequent
//               calls to add_direct_host() may be made to build up
//               the list of hosts that do not require a proxy
//               connection.
////////////////////////////////////////////////////////////////////
void HTTPClient::
clear_direct_host() {
  _direct_hosts.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::add_direct_host
//       Access: Published
//  Description: Adds the indicated name to the set of hostnames that
//               are connected to directly, without using a proxy.
//               This name may be either a DNS name or an IP address,
//               and it may include the * as a wildcard character.
////////////////////////////////////////////////////////////////////
void HTTPClient::
add_direct_host(const string &hostname) {
  // The hostname is always converted to lowercase.
  string lc_hostname;
  lc_hostname.reserve(hostname.length());
  for (string::const_iterator si = hostname.begin(); 
       si != hostname.end(); 
       ++si) {
    lc_hostname += tolower(*si);
  }

  _direct_hosts.push_back(GlobPattern(lc_hostname));
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_proxies_for_url
//       Access: Published
//  Description: Fills up the indicated vector with the list of
//               URLSpec objects, in the order in which they should be
//               tried, that are appropriate for the indicated URL.
//               The vector is left empty if a direct connection
//               should be used.
//
//               It is the user's responsibility to empty this vector
//               before calling this method; otherwise, the proxy
//               URL's will simply be appended to the existing list.
////////////////////////////////////////////////////////////////////
void HTTPClient::
get_proxies_for_url(const URLSpec &url, pvector<URLSpec> &proxies) const {
  // First, check if the hostname matches any listed in direct_hosts.
  string hostname = url.get_server();

  // If the hostname is empty, treat it as a special case: we don't
  // match any of the hostnames listed in direct_hosts (even "*").
  if (!hostname.empty()) {
    DirectHosts::const_iterator si;
    for (si = _direct_hosts.begin(); si != _direct_hosts.end(); ++si) {
      if ((*si).matches(hostname)) {
        // It matches, so don't use any proxies.
        return;
      }
    }
  }

  // Now choose the appropriate proxy based on the scheme.
  string scheme = url.get_scheme();
  bool got_any = false;

  if (scheme.empty()) {
    // An empty scheme implies we will want to make a direct
    // connection to this host, so we will need a socks-style or
    // https-style scheme.
    if (get_proxies_for_scheme("socks", proxies)) {
      got_any = true;
    }
    if (get_proxies_for_scheme("https", proxies)) {
      got_any = true;
    }

  } else {
    // Otherwise, try to match the proxy to the scheme.
    if (get_proxies_for_scheme(scheme, proxies)) {
      got_any = true;
    }
  }

  // If we didn't find our scheme of choice, fall back to the default
  // proxy type.
  if (!got_any) {
    if (get_proxies_for_scheme("", proxies)) {
      got_any = true;
    }
  }

  // Failing that, use SOCKS for an SSL-type connection.
  if (!got_any && scheme == "https") {
    if (get_proxies_for_scheme("socks", proxies)) {
      got_any = true;
    }
  }

  // And failing that, try the http proxy.
  if (!got_any) {
    get_proxies_for_scheme("http", proxies);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_proxies_for_url
//       Access: Published
//  Description: Returns a semicolon-delimited list of proxies, in the
//               order in which they should be tried, that are
//               appropriate for the indicated URL.  The empty string
//               is returned if a direct connection should be used.
////////////////////////////////////////////////////////////////////
string HTTPClient::
get_proxies_for_url(const URLSpec &url) const {
  pvector<URLSpec> proxies;
  get_proxies_for_url(url, proxies);

  string result;
  if (!proxies.empty()) {
    pvector<URLSpec>::const_iterator pi = proxies.begin();
    result += (*pi).get_url();
    ++pi;

    while (pi != proxies.end()) {
      result += ";";
      result += (*pi).get_url();
      ++pi;
    }
  }

  return result;
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
  case HTTPEnum::HV_09:
    return "HTTP/0.9";

  case HTTPEnum::HV_10:
    return "HTTP/1.0";

  case HTTPEnum::HV_11:
    return "HTTP/1.1";

  case HTTPEnum::HV_other:
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
HTTPEnum::HTTPVersion HTTPClient::
parse_http_version_string(const string &version) {
  if (version == "HTTP/1.0") {
    return HTTPEnum::HV_10;
  } else if (version == "HTTP/1.1") {
    return HTTPEnum::HV_11;
  } else if (version.substr(0, 6) == "HTTP/0") {
    return HTTPEnum::HV_09;
  } else {
    return HTTPEnum::HV_other;
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
#ifdef REPORT_OPENSSL_ERRORS
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
//               calling HTTPClient::get_document() repeatedly (which
//               would force a new connection for each document).
//
//               Also, HTTPChannel has some additional, less common
//               interface methods than the basic interface methods
//               that exist on HTTPClient; if you wish to call any of
//               these methods you must first obtain an HTTPChannel.
//
//               Pass true for persistent_connection to gain this
//               network efficiency.  If, on the other hand, your
//               intention is to use the channel to retrieve only one
//               document, then pass false to inform the server that
//               we will be dropping the connection after the first
//               document.
////////////////////////////////////////////////////////////////////
PT(HTTPChannel) HTTPClient::
make_channel(bool persistent_connection) {
  PT(HTTPChannel) doc = new HTTPChannel(this);
  doc->set_persistent_connection(persistent_connection);
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
//  Description: Opens the named document for reading.  Returns a new
//               HTTPChannel object whether the document is
//               successfully read or not; you can test is_valid() and
//               get_return_code() to determine whether the document
//               was retrieved.
////////////////////////////////////////////////////////////////////
PT(HTTPChannel) HTTPClient::
get_document(const URLSpec &url) {
  PT(HTTPChannel) doc = new HTTPChannel(this);
  doc->get_document(url);
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
//     Function: HTTPClient::get_ssl_ctx
//       Access: Public
//  Description: Returns the OpenSSL context object, creating it first
//               if needed.
////////////////////////////////////////////////////////////////////
SSL_CTX *HTTPClient::
get_ssl_ctx() {
  if (_ssl_ctx != (SSL_CTX *)NULL) {
    return _ssl_ctx;
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
    config_downloader.GetAll("expected-ssl-server", expected_servers);
    
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
    config_downloader.GetAll("ssl-certificates", cert_files);
    
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

  return _ssl_ctx;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::get_proxies_for_scheme
//       Access: Private
//  Description: Adds the proxy servers associated with the indicated
//               scheme, if any, to the list.  Returns true if any
//               were added, false otherwise.
////////////////////////////////////////////////////////////////////
bool HTTPClient::
get_proxies_for_scheme(const string &scheme, pvector<URLSpec> &proxies) const {
  ProxiesByScheme::const_iterator si = _proxies_by_scheme.find(scheme);
  if (si == _proxies_by_scheme.end()) {
    return false;
  }
  const Proxies &scheme_proxies = (*si).second;
  if (scheme_proxies.empty()) {
    return false;
  }

  Proxies::const_iterator pi;
  for (pi = scheme_proxies.begin(); pi != scheme_proxies.end(); ++pi) {
    proxies.push_back(*pi);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::add_http_username
//       Access: Private
//  Description: Handles a Configrc definition for http-username as
//               server:realm:username:password, where either or both
//               of server and realm may be empty, or just
//               server:username:password or username:password.
////////////////////////////////////////////////////////////////////
void HTTPClient::
add_http_username(const string &http_username) {
  size_t c1 = http_username.find(':');
  if (c1 != string::npos) {
    size_t c2 = http_username.find(':', c1 + 1);
    if (c2 != string::npos) {
      size_t c3 = http_username.find(':', c2 + 1);
      if (c3 != string::npos) {
        size_t c4 = http_username.find(':', c3 + 1);
        if (c4 != string::npos) {
          // Oops, we have five?  Problem.
          downloader_cat.error()
            << "Invalid http-username " << http_username << "\n";

        } else {
          // Ok, we have four.
          set_username(http_username.substr(0, c1),
                       http_username.substr(c1 + 1, c2 - (c1 + 1)),
                       http_username.substr(c2 + 1));
        }

      } else {
        // We have only three.
        set_username(string(),
                     http_username.substr(0, c1),
                     http_username.substr(c1 + 1));
      }
    } else {
      // We have only two.
      set_username(string(), string(), http_username);
    }
  } else {
    // We have only one?  Problem.
    downloader_cat.error()
      << "Invalid http-username " << http_username << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::select_username
//       Access: Private
//  Description: Chooses a suitable username:password string for the
//               given URL and realm.
////////////////////////////////////////////////////////////////////
string HTTPClient::
select_username(const URLSpec &url, bool is_proxy, const string &realm) const {
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
      username = get_username("*proxy", realm);
    }
    if (username.empty()) {
      // Then, try *proxy/any realm.
      username = get_username("*proxy", string());
    }
  }
  if (username.empty()) {
    // Try the specific server/realm.
    username = get_username(url.get_server(), realm);
  }
  if (username.empty()) {
    // Then, try the specific server/any realm.
    username = get_username(url.get_server(), string());
  }
  if (username.empty()) {
    // Then, try any server with this realm.
    username = get_username(string(), realm);
  }
  if (username.empty()) {
    // Then, take the general password.
    username = get_username(string(), string());
  }

  return username;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::select_auth
//       Access: Private
//  Description: Chooses a suitable pre-computed authorization for the
//               indicated URL.  Returns NULL if no authorization
//               matches.
////////////////////////////////////////////////////////////////////
HTTPAuthorization *HTTPClient::
select_auth(const URLSpec &url, bool is_proxy, const string &last_realm) {
  Domains &domains = is_proxy ? _proxy_domains : _www_domains;
  string canon = HTTPAuthorization::get_canonical_url(url).get_url();

  // Look for the longest domain string that is a prefix of our
  // canonical URL.  We have to make a linear scan through the list.
  Domains::const_iterator best_di = domains.end();
  size_t longest_length = 0;
  Domains::const_iterator di;
  for (di = domains.begin(); di != domains.end(); ++di) {
    const string &domain = (*di).first;
    size_t length = domain.length();
    if (domain == canon.substr(0, length)) {
      // This domain string matches.  Is it the longest?
      if (length > longest_length) {
        best_di = di;
        longest_length = length;
      }
    }
  }

  if (best_di != domains.end()) {
    // Ok, we found a matching domain.  Use it.
    if (downloader_cat.is_spam()) {
      downloader_cat.spam()
        << "Choosing domain " << (*best_di).first << " for " << url << "\n";
    }
    const Realms &realms = (*best_di).second._realms;
    // First, try our last realm.
    Realms::const_iterator ri;
    ri = realms.find(last_realm);
    if (ri != realms.end()) {
      return (*ri).second;
    }

    if (!realms.empty()) {
      // Oh well, just return the first realm.
      return (*realms.begin()).second;
    }
  }

  // No matching domains.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::generate_auth
//       Access: Private
//  Description: Generates a new authorization entry in response to a
//               401 or 407 challenge from the server or proxy.  The
//               new authorization entry is stored for future
//               connections to the same server (or, more precisely,
//               the same domain, which may be a subset of the server,
//               or it may include multiple servers).
////////////////////////////////////////////////////////////////////
PT(HTTPAuthorization) HTTPClient::
generate_auth(const URLSpec &url, bool is_proxy, const string &challenge) {
  HTTPAuthorization::AuthenticationSchemes schemes;
  HTTPAuthorization::parse_authentication_schemes(schemes, challenge);

  PT(HTTPAuthorization) auth;
  HTTPAuthorization::AuthenticationSchemes::iterator si;

  si = schemes.find("digest");
  if (si != schemes.end()) {
    auth = new HTTPDigestAuthorization((*si).second, url, is_proxy);
  }

  if (auth == (HTTPAuthorization *)NULL || !auth->is_valid()) {
    si = schemes.find("basic");
    if (si != schemes.end()) {
      auth = new HTTPBasicAuthorization((*si).second, url, is_proxy);
    }
  }

  if (auth == (HTTPAuthorization *)NULL || !auth->is_valid()) {
    downloader_cat.warning() 
      << "Don't know how to use any of the server's available authorization schemes:\n";
    for (si = schemes.begin(); si != schemes.end(); ++si) {
      downloader_cat.warning() << (*si).first << "\n";
    }

  } else {
    // Now that we've got an authorization, store it under under each
    // of its suggested domains for future use.
    Domains &domains = is_proxy ? _proxy_domains : _www_domains;
    const vector_string &domain = auth->get_domain();
    vector_string::const_iterator si;
    for (si = domain.begin(); si != domain.end(); ++si) {
      domains[(*si)]._realms[auth->get_realm()] = auth;
    }
  }

  return auth;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPClient::initialize_ssl
//       Access: Private, Static
//  Description: Called once the first time this class is used to
//               initialize the OpenSSL library.
////////////////////////////////////////////////////////////////////
void HTTPClient::
initialize_ssl() {
#ifdef REPORT_OPENSSL_ERRORS
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

  // We have to be sure and clear the OpenSSL error state before we
  // call this function, or it will get confused.
#ifdef REPORT_OPENSSL_ERRORS
  ERR_clear_error();
#endif
  inf = PEM_X509_INFO_read_bio(mbio, NULL, NULL, NULL);
  BIO_free(mbio);

  if (!inf) {
    // Could not scan certificates.
    downloader_cat.info()
      << "PEM_X509_INFO_read_bio() returned NULL.\n";
#ifdef REPORT_OPENSSL_ERRORS
    ERR_print_errors_fp(stderr);
#endif
    return 0;
  }
  
  if (downloader_cat.is_spam()) {
    downloader_cat.spam()
      << "PEM_X509_INFO_read_bio() found " << sk_X509_INFO_num(inf)
      << " entries.\n";
  }

  // Now add the certificates to the context.
  X509_STORE *store = ctx->cert_store;

  int count = 0;
  int num_entries = sk_X509_INFO_num(inf);
  for (int i = 0; i < num_entries; i++) {
    X509_INFO *itmp = sk_X509_INFO_value(inf, i);

    if (itmp->x509) {
      X509_STORE_add_cert(store, itmp->x509);
      count++;
      if (downloader_cat.is_spam()) {
        downloader_cat.spam()
          << "Entry " << i << " is x509\n";
      }

    } else if (itmp->crl) {
      X509_STORE_add_crl(store, itmp->crl);
      count++;
      if (downloader_cat.is_spam()) {
        downloader_cat.spam()
          << "Entry " << i << " is crl\n";
      }

    } else {
      if (downloader_cat.is_spam()) {
        downloader_cat.spam()
          << "Entry " << i << " is unknown type\n";
      }
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
