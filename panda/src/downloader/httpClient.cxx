/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpClient.cxx
 * @author drose
 * @date 2002-09-24
 */

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

#ifdef HAVE_OPENSSL

#include "openSSLWrapper.h"

using std::string;

PT(HTTPClient) HTTPClient::_global_ptr;

/**
 *
 */
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

/**
 * Chops the source string up into pieces delimited by any of the characters
 * specified in delimiters.  Repeated delimiter characters represent zero-
 * length tokens.
 *
 * It is the user's responsibility to ensure the output vector is cleared
 * before calling this function; the results will simply be appended to the
 * end of the vector.
 */
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

#ifndef NDEBUG
/**
 * This method is attached as a callback for SSL messages only when debug
 * output is enabled.
 */
static void
ssl_msg_callback(int write_p, int version, int content_type,
                 const void *, size_t len, SSL *, void *) {
  std::ostringstream describe;
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
#endif  // !defined(NDEBUG)

/**
 *
 */
HTTPClient::
HTTPClient() {
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

  ConfigVariableList http_username
    ("http-username",
     PRC_DESC("Adds one or more username/password pairs to all HTTP clients.  The client "
              "will present this username/password when asked to authenticate a request "
              "for a particular server and/or realm.  The username is of the form "
              "server:realm:username:password, where either or both of server and "
              "realm may be empty, or just realm:username:password or username:password.  "
              "If the server or realm is empty, they will match anything."));

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


  ConfigVariableList http_preapproved_server_certificate_filename
    ("http-preapproved-server-certificate-filename",
     PRC_DESC("This specifies a server hostname:port combination, followed by "
              "at least one space, and the name of a PEM file that contains "
              "an SSL certificate that we expect this server to offer.  If "
              "it does, we will accept the cert without further question, "
              "even if the cert does not match any known certificate "
              "authorities.  This option may appear multiple times."));

  ConfigVariableList http_preapproved_server_certificate_name
    ("http-preapproved-server-certificate-name",
     PRC_DESC("This specifies a server hostname:port combination, followed by "
              "at least one space, and a string describing the subject name "
              "of an SSL certificate we expect this server to offer.  If it "
              "it does, we will accept the cert, but only if it also matches "
              "a known certificate authority.  This option may appear "
              "multiple times."));

  _http_version = HTTPEnum::HV_11;
  _verify_ssl = verify_ssl ? VS_normal : VS_no_verify;
  _ssl_ctx = nullptr;

  set_proxy_spec(http_proxy);
  set_direct_host_spec(http_direct_hosts);
  _try_all_direct = http_try_all_direct;

  if (!http_proxy_username.empty()) {
    set_username("*proxy", "", http_proxy_username);
  }

  set_cipher_list(ssl_cipher_list);

  {
    // Also load in the general usernames.
    int num_unique_values = http_username.get_num_unique_values();
    for (int i = 0; i < num_unique_values; i++) {
      string username = http_username.get_unique_value(i);
      add_http_username(username);
    }
  }

  _client_certificate_filename = http_client_certificate_filename;
  _client_certificate_passphrase = http_client_certificate_passphrase;

  _client_certificate_loaded = false;
  _client_certificate_pub = nullptr;
  _client_certificate_priv = nullptr;

  int num_server_certs = http_preapproved_server_certificate_filename.get_num_unique_values();
  int si;
  for (si = 0; si < num_server_certs; si++) {
    string cert_line = http_preapproved_server_certificate_filename.get_unique_value(si);
    string a, b;
    split_whitespace(a, b, cert_line);
    add_preapproved_server_certificate_filename(URLSpec(a, true), b);
  }

  num_server_certs = http_preapproved_server_certificate_name.get_num_unique_values();
  for (si = 0; si < num_server_certs; si++) {
    string cert_line = http_preapproved_server_certificate_name.get_unique_value(si);
    string a, b;
    split_whitespace(a, b, cert_line);
    add_preapproved_server_certificate_name(URLSpec(a, true), b);
  }

  // The first time we create an HTTPClient, we must initialize the OpenSSL
  // library.  The OpenSSLWrapper object does that.
  OpenSSLWrapper::get_global_ptr();
}

/**
 *
 */
HTTPClient::
HTTPClient(const HTTPClient &copy) {
  _ssl_ctx = nullptr;

  (*this) = copy;
}

/**
 *
 */
void HTTPClient::
operator = (const HTTPClient &copy) {
  _proxies_by_scheme = copy._proxies_by_scheme;
  _direct_hosts = copy._direct_hosts;
  _try_all_direct = copy._try_all_direct;
  _http_version = copy._http_version;
  _verify_ssl = copy._verify_ssl;
  _usernames = copy._usernames;
  _cookies = copy._cookies;
}

/**
 *
 */
HTTPClient::
~HTTPClient() {
  if (_ssl_ctx != nullptr) {
#if OPENSSL_VERSION_NUMBER < 0x10100000
    // Before we can free the context, we must remove the X509_STORE pointer
    // from it, so it won't be destroyed along with it (this object is shared
    // among all contexts).
    _ssl_ctx->cert_store = nullptr;
#endif
    SSL_CTX_free(_ssl_ctx);
  }

  unload_client_certificate();
}

/**
 * This may be called once, presumably at the beginning of an application, to
 * initialize OpenSSL's random seed.  On Windows, it is particularly important
 * to call this at startup if you are going to be performing any https
 * operations or otherwise use encryption, since the Windows algorithm for
 * getting a random seed takes 2-3 seconds at startup, but can take 30 seconds
 * or more after you have opened a 3-D graphics window and started rendering.
 *
 * There is no harm in calling this method multiple times, or in not calling
 * it at all.
 */
void HTTPClient::
init_random_seed() {
  // Creating the global OpenSSLWrapper object is nowadays sufficient to
  // ensure that OpenSSL and its random seed have been initialized.
  OpenSSLWrapper::get_global_ptr();
}

/**
 * Specifies the complete set of proxies to use for all schemes.  This is
 * either a semicolon-delimited set of hostname:ports, or a semicolon-
 * delimited set of pairs of the form "scheme=hostname:port", or a
 * combination.  Use the keyword DIRECT, or an empty string, to represent a
 * direct connection.  A particular scheme and/or proxy host may be listed
 * more than once.  This is a convenience function that can be used in place
 * of explicit calls to add_proxy() for each scheme/proxy pair.
 */
void HTTPClient::
set_proxy_spec(const string &proxy_spec) {
  clear_proxy();

  string trim_proxy_spec = trim_blanks(proxy_spec);

  // Tokenize the string based on the semicolons.
  if (!trim_proxy_spec.empty()) {
    vector_string proxies;
    tokenize(trim_proxy_spec, proxies, ";");

    for (vector_string::const_iterator pi = proxies.begin();
         pi != proxies.end();
         ++pi) {
      const string &spec = (*pi);

      // Divide out the scheme and the hostname.
      string scheme;
      string proxy;
      size_t equals = spec.find('=');
      if (equals == string::npos) {
        scheme = "";
        proxy = trim_blanks(spec);
      } else {
        scheme = trim_blanks(spec.substr(0, equals));
        proxy = trim_blanks(spec.substr(equals + 1));
      }

      if (proxy == "DIRECT" || proxy.empty()) {
        add_proxy(scheme, URLSpec());
      } else {
        add_proxy(scheme, URLSpec(proxy, true));
      }
    }
  }
}

/**
 * Returns the complete set of proxies to use for all schemes.  This is a
 * string of the form specified by set_proxy_spec(), above.  Note that the
 * string returned by this function may not be exactly the same as the string
 * passed into set_proxy_spec(), since the string is regenerated from the
 * internal storage structures and may therefore be reordered.
 */
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
      if (url.empty()) {
        result += "DIRECT";
      } else {
        result += url.get_url();
      }
    }
  }

  return result;
}

/**
 * Specifies the set of hosts that should be connected to directly, without
 * using a proxy.  This is a semicolon-separated list of hostnames that may
 * contain wildcard characters ("*").
 */
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

    // We should be careful to avoid adding any empty hostnames to the list.
    // In particular, we will get one empty hostname if the direct_host_spec
    // is empty.
    if (!spec.empty()) {
      add_direct_host(spec);
    }
  }
}

/**
 * Returns the set of hosts that should be connected to directly, without
 * using a proxy, as a semicolon-separated list of hostnames that may contain
 * wildcard characters ("*").
 */
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

/**
 * Resets the proxy spec to empty.  Subsequent calls to add_proxy() may be
 * made to build up the set of proxy servers.
 */
void HTTPClient::
clear_proxy() {
  _proxies_by_scheme.clear();
}

/**
 * Adds the indicated proxy host as a proxy for communications on the given
 * scheme.  Usually the scheme is "http" or "https".  It may be the empty
 * string to indicate a general proxy.  The proxy string may be the empty URL
 * to indicate a direct connection.
 */
void HTTPClient::
add_proxy(const string &scheme, const URLSpec &proxy) {
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

  if (!proxy_url.empty()) {
    // Enforce the scheme that we use to communicate to the proxy itself.
    // This is not the same as lc_scheme, which is the scheme of the requested
    // connection.  Generally, all proxies speak HTTP, except for Socks
    // proxies.

    if (lc_scheme == "socks") {
      // Scheme "socks" implies we talk to the proxy via the "socks" scheme,
      // no matter what scheme the user actually specified.
      proxy_url.set_scheme("socks");

    } else if (!proxy_url.has_scheme()) {
      // Otherwise, if the user didn't specify a scheme to talk to the proxy,
      // the default is "http".
      proxy_url.set_scheme("http");
    }
  }

  _proxies_by_scheme[lc_scheme].push_back(proxy_url);
}

/**
 * Resets the set of direct hosts to empty.  Subsequent calls to
 * add_direct_host() may be made to build up the list of hosts that do not
 * require a proxy connection.
 */
void HTTPClient::
clear_direct_host() {
  _direct_hosts.clear();
}

/**
 * Adds the indicated name to the set of hostnames that are connected to
 * directly, without using a proxy.  This name may be either a DNS name or an
 * IP address, and it may include the * as a wildcard character.
 */
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

/**
 * Fills up the indicated vector with the list of URLSpec objects, in the
 * order in which they should be tried, that are appropriate proxies to try
 * for the indicated URL.  The empty URL is returned for a direct connection.
 *
 * It is the user's responsibility to empty this vector before calling this
 * method; otherwise, the proxy URL's will simply be appended to the existing
 * list.
 */
void HTTPClient::
get_proxies_for_url(const URLSpec &url, pvector<URLSpec> &proxies) const {
  // First, check if the hostname matches any listed in direct_hosts.
  string hostname = url.get_server();

  // If the hostname is empty, treat it as a special case: we don't match any
  // of the hostnames listed in direct_hosts (even "*").
  if (!hostname.empty()) {
    DirectHosts::const_iterator si;
    for (si = _direct_hosts.begin(); si != _direct_hosts.end(); ++si) {
      if ((*si).matches(hostname)) {
        // It matches, so don't use any proxies.
        proxies.push_back(URLSpec());
        return;
      }
    }
  }

  // Build our list of proxies into a temporary vector, so we can pull out
  // duplicates later.
  pvector<URLSpec> temp_list;

  // Now choose the appropriate proxy based on the scheme.
  string scheme = url.get_scheme();
  bool got_any = false;

  if (!scheme.empty()) {
    // If we have a scheme, try to match it.
    if (get_proxies_for_scheme(scheme, temp_list)) {
      got_any = true;
    }
  }

  if (!got_any && (scheme.empty() || url.is_ssl())) {
    // An empty scheme (or an ssl-style scheme) implies we will need to make a
    // direct connection, so fallback to a socks-style andor https-style
    // scheme.

    if (get_proxies_for_scheme("socks", temp_list)) {
      got_any = true;
    }
    if (get_proxies_for_scheme("https", temp_list)) {
      got_any = true;
    }
  }

  if (!got_any) {
    // If we didn't find our scheme of choice, fall back to the default proxy
    // type, if we've got one.
    if (get_proxies_for_scheme("", temp_list)) {
      got_any = true;
    }
  }

  if (_try_all_direct) {
    // We may try a direct connection if all else fails.
    temp_list.push_back(URLSpec());
  }

  // Finally, as a very last resort, fall back to the HTTP proxy.
  if (!got_any) {
    get_proxies_for_scheme("http", temp_list);
  }

  // Now remove duplicate entries as we copy the resulting list out.
  pvector<URLSpec>::iterator pi;
  pset<URLSpec> used;
  for (pi = temp_list.begin(); pi != temp_list.end(); ++pi) {
    if (used.insert(*pi).second) {
      // This is a unique one.
      proxies.push_back(*pi);
    }
  }
}

/**
 * Returns a semicolon-delimited list of proxies, in the order in which they
 * should be tried, that are appropriate for the indicated URL.  The keyword
 * DIRECT indicates a direct connection should be tried.
 */
string HTTPClient::
get_proxies_for_url(const URLSpec &url) const {
  pvector<URLSpec> proxies;
  get_proxies_for_url(url, proxies);

  string result;
  if (!proxies.empty()) {
    pvector<URLSpec>::const_iterator pi = proxies.begin();
    if ((*pi).get_url().empty()) {
      result += "DIRECT";
    } else {
      result += (*pi).get_url();
    }
    ++pi;

    while (pi != proxies.end()) {
      result += ";";
      if ((*pi).get_url().empty()) {
        result += "DIRECT";
      } else {
        result += (*pi).get_url();
      }
      ++pi;
    }
  }

  return result;
}

/**
 * Specifies the username:password string corresponding to a particular server
 * and/or realm, when demanded by the server.  Either or both of the server or
 * realm may be empty; if so, they match anything.  Also, the server may be
 * set to the special string "*proxy", which will match any proxy server.
 *
 * If the username is set to the empty string, this clears the password for
 * the particular server/realm pair.
 */
void HTTPClient::
set_username(const string &server, const string &realm, const string &username) {
  string key = server + ":" + realm;
  if (username.empty()) {
    _usernames.erase(key);
  } else {
    _usernames[key] = username;
  }
}

/**
 * Returns the username:password string set for this server/realm pair, or
 * empty string if nothing has been set.  See set_username().
 */
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

/**
 * Stores the indicated cookie in the client's list of cookies, as if it had
 * been received from a server.
 */
void HTTPClient::
set_cookie(const HTTPCookie &cookie) {
  if (cookie.is_expired()) {
    clear_cookie(cookie);

  } else {
    std::pair<Cookies::iterator, bool> result = _cookies.insert(cookie);
    if (!result.second) {
      // We already had a cookie matching the supplied domainpathname, so
      // replace it.
      const HTTPCookie &orig_cookie = *result.first;
      ((HTTPCookie &)orig_cookie).update_from(cookie);
    }
  }
}

/**
 * Removes the cookie with the matching domain/path/name from the client's
 * list of cookies.  Returns true if it was removed, false if the cookie was
 * not matched.
 */
bool HTTPClient::
clear_cookie(const HTTPCookie &cookie) {
  Cookies::iterator ci = _cookies.find(cookie);
  if (ci != _cookies.end()) {
    _cookies.erase(ci);
    return true;
  }

  return false;
}

/**
 * Removes the all stored cookies from the client.
 */
void HTTPClient::
clear_all_cookies() {
  _cookies.clear();
}

/**
 * Returns true if there is a cookie in the client matching the given cookie's
 * domain/path/name, false otherwise.
 */
bool HTTPClient::
has_cookie(const HTTPCookie &cookie) const {
  Cookies::const_iterator ci = _cookies.find(cookie);
  return (ci != _cookies.end());
}

/**
 * Looks up and returns the cookie in the client matching the given cookie's
 * domain/path/name.  If there is no matching cookie, returns an empty cookie.
 */
HTTPCookie HTTPClient::
get_cookie(const HTTPCookie &cookie) const {
  Cookies::const_iterator ci = _cookies.find(cookie);
  if (ci != _cookies.end()) {
    return (*ci);
  }

  return HTTPCookie();
}

/**
 * Copies all the cookies from the indicated HTTPClient into this one.
 * Existing cookies in this client are not affected, unless they are shadowed
 * by the new cookies.
 */
void HTTPClient::
copy_cookies_from(const HTTPClient &other) {
  Cookies::const_iterator ci;
  for (ci = other._cookies.begin(); ci != other._cookies.end(); ++ci) {
    set_cookie(*ci);
  }
}

/**
 * Outputs the complete list of cookies stored on the client, for all domains,
 * including the expired cookies (which will normally not be sent back to a
 * host).
 */
void HTTPClient::
write_cookies(std::ostream &out) const {
  Cookies::const_iterator ci;
  for (ci = _cookies.begin(); ci != _cookies.end(); ++ci) {
    out << *ci << "\n";
  }
}

/**
 * Writes to the indicated ostream a "Cookie" header line for sending the
 * cookies appropriate to the indicated URL along with an HTTP request.  This
 * also removes expired cookies.
 */
void HTTPClient::
send_cookies(std::ostream &out, const URLSpec &url) {
  HTTPDate now = HTTPDate::now();
  bool any_expired = false;
  bool first_cookie = true;

  Cookies::const_iterator ci;
  for (ci = _cookies.begin(); ci != _cookies.end(); ++ci) {
    const HTTPCookie &cookie = (*ci);
    if (cookie.is_expired(now)) {
      any_expired = true;

    } else if (cookie.matches_url(url)) {
      if (first_cookie) {
        out << "Cookie: ";
        first_cookie = false;
      } else {
        out << "; ";
      }
      out << cookie.get_name() << "=" << cookie.get_value();
    }
  }

  if (!first_cookie) {
    out << "\r\n";
  }

  if (any_expired) {
    Cookies new_cookies;
    Cookies::const_iterator ci;
    for (ci = _cookies.begin(); ci != _cookies.end(); ++ci) {
      const HTTPCookie &cookie = (*ci);
      if (!cookie.is_expired(now)) {
        new_cookies.insert(new_cookies.end(), cookie);
      }
    }
    _cookies.swap(new_cookies);
  }
}

/**
 * Attempts to load the certificate named by set_client_certificate_filename()
 * immediately, and returns true if successful, false otherwise.
 *
 * Normally this need not be explicitly called, since it will be called
 * automatically if the server requests a certificate, but it may be useful to
 * determine ahead of time if the certificate can be loaded correctly.
 */
bool HTTPClient::
load_client_certificate() {
  if (!_client_certificate_loaded) {
    _client_certificate_loaded = true;

    if (!_client_certificate_filename.empty()) {
      _client_certificate_filename.set_text();

      // First, read the complete file into memory.
      VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

      if (!vfs->read_file(_client_certificate_filename,
                          _client_certificate_pem, true)) {
        // Could not find or read file.
        downloader_cat.warning()
          << "Could not read " << _client_certificate_filename << ".\n";
        return false;
      }
    }

    if (!_client_certificate_pem.empty()) {
      // Create an in-memory BIO to read the "file" from the memory buffer,
      // and call the low-level routines to read the keys from the BIO.
      BIO *mbio = BIO_new_mem_buf((void *)_client_certificate_pem.data(),
                                  _client_certificate_pem.length());

      ERR_clear_error();
      _client_certificate_priv =
        PEM_read_bio_PrivateKey(mbio, nullptr, nullptr,
                                (char *)_client_certificate_passphrase.c_str());

      // Rewind the "file" to the beginning in order to read the public key
      // (which might appear first in the file).
      (void)BIO_reset(mbio);

      ERR_clear_error();
      _client_certificate_pub =
        PEM_read_bio_X509(mbio, nullptr, nullptr, nullptr);

      BIO_free(mbio);


      NotifySeverity sev = NS_debug;
      string source = "memory";
      if (!_client_certificate_filename.empty()) {
        // Only report status to "info" severity if we have read the
        // certificate from a file.  If it came from an in-memory image, a
        // failure will presumably be handled by whoever set the image.
        sev = NS_info;
        source = _client_certificate_filename;
      }

      if (downloader_cat.is_on(sev)) {
        if (_client_certificate_priv != nullptr &&
            _client_certificate_pub != nullptr) {
          downloader_cat.out(sev)
            << "Read client certificate from " << source << "\n";

        } else {
          if (_client_certificate_priv == nullptr) {
            downloader_cat.out(sev)
              << "Could not read private key from " << source << "\n";
          }

          if (_client_certificate_pub == nullptr) {
            downloader_cat.out(sev)
              << "Could not read public key from " << source << "\n";
          }
        }
      }
    }
  }

  return (_client_certificate_priv != nullptr &&
          _client_certificate_pub != nullptr);
}

/**
 * Adds the certificate defined in the indicated PEM filename as a "pre-
 * approved" certificate for the indicated server, defined by the hostname and
 * port (only) from the given URL.
 *
 * If the server offers this particular certificate on a secure connection, it
 * will be accepted without question.  This is particularly useful for
 * communicating with a server using a known self-signed certificate.
 *
 * See also the similar add_preapproved_server_certificate_pem(), and the
 * weaker add_preapproved_server_certificate_name().
 */
bool HTTPClient::
add_preapproved_server_certificate_filename(const URLSpec &url, const Filename &filename) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  string pem;

  if (!vfs->read_file(filename, pem, true)) {
    // Could not find or read file.
    downloader_cat.warning()
      << "Could not read " << filename << ".\n";
    return false;
  }

  return add_preapproved_server_certificate_pem(url, pem);
}

/**
 * Adds the certificate defined in the indicated data string, formatted as a
 * PEM block, as a "pre-approved" certificate for the indicated server,
 * defined by the hostname and port (only) from the given URL.
 *
 * If the server offers this particular certificate on a secure connection, it
 * will be accepted without question.  This is particularly useful for
 * communicating with a server using a known self-signed certificate.
 *
 * See also the similar add_preapproved_server_certificate_filename(), and the
 * weaker add_preapproved_server_certificate_name().
 */
bool HTTPClient::
add_preapproved_server_certificate_pem(const URLSpec &url, const string &pem) {
  // Create an in-memory BIO to read the "file" from the memory buffer, and
  // call the low-level routine to read the cert from the BIO.
  BIO *mbio = BIO_new_mem_buf((void *)pem.data(), pem.length());

  ERR_clear_error();
  X509 *cert = PEM_read_bio_X509(mbio, nullptr, nullptr, nullptr);
  BIO_free(mbio);

  if (cert == nullptr) {
    downloader_cat.warning()
      << "Could not parse PEM data\n";
    return false;
  }

  string server_and_port = url.get_server_and_port();
  PreapprovedServerCerts::iterator psci =
    _preapproved_server_certs.insert(PreapprovedServerCerts::value_type(server_and_port, PreapprovedServerCert())).first;

  PreapprovedServerCert &psc = (*psci).second;
  psc._certs.push_back(cert);

  return true;
}

/**
 * Adds the certificate *name* only, as a "pre-approved" certificate name for
 * the indicated server, defined by the hostname and port (only) from the
 * given URL.
 *
 * This is a weaker function than
 * add_preapproved_server_certificate_filename().  This checks only the
 * subject name of the certificate, without checking for a particular
 * certificate by key.  This means that a variety of server certificates may
 * match the indicated name.
 *
 * Because this is a weaker verification, it only applies to server
 * certificates that are signed by a recognized certificate authority.  Thus,
 * it cannot be used to pre-approve self-signed certificates, but it can be
 * used to accept a server certificate offered by a different hostname than
 * the one in the cert itself.
 *
 * The certificate name should be formatted in the form
 * type0=value0/type1=value1/type2=...
 */
bool HTTPClient::
add_preapproved_server_certificate_name(const URLSpec &url, const string &name) {
  X509_NAME *cert_name = parse_x509_name(name);
  if (cert_name == nullptr) {
    downloader_cat.warning()
      << "Could not parse certificate name " << name << "\n";
    return false;
  }

  string server_and_port = url.get_server_and_port();
  PreapprovedServerCerts::iterator psci =
    _preapproved_server_certs.insert(PreapprovedServerCerts::value_type(server_and_port, PreapprovedServerCert())).first;

  PreapprovedServerCert &psc = (*psci).second;
  psc._cert_names.push_back(cert_name);

  return true;
}

/**
 * Removes all preapproved server certificates for the indicated server and
 * port.
 */
void HTTPClient::
clear_preapproved_server_certificates(const URLSpec &url) {
  string server_and_port = url.get_server_and_port();
  _preapproved_server_certs.erase(server_and_port);
}

/**
 * Removes all preapproved server certificates for all servers.
 */
void HTTPClient::
clear_all_preapproved_server_certificates() {
  _preapproved_server_certs.clear();
}

/**
 * Returns the current HTTP version setting as a string, e.g.  "HTTP/1.0" or
 * "HTTP/1.1".
 */
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

/**
 * Matches the string representing a particular HTTP version against any of
 * the known versions and returns the appropriate enumerated value, or
 * HV_other if the version is unknown.
 */
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

/**
 * Reads the certificate(s) (delimited by -----BEGIN CERTIFICATE----- and
 * -----END CERTIFICATE-----) from the indicated file and makes them known as
 * trusted public keys for validating future connections.  Returns true on
 * success, false otherwise.
 */
bool HTTPClient::
load_certificates(const Filename &filename) {
  OpenSSLWrapper *sslw = OpenSSLWrapper::get_global_ptr();
  return (sslw->load_certificates(filename) != 0);
}

/**
 * Returns a new HTTPChannel object that may be used for reading multiple
 * documents using the same connection, for greater network efficiency than
 * calling HTTPClient::get_document() repeatedly (which would force a new
 * connection for each document).
 *
 * Also, HTTPChannel has some additional, less common interface methods than
 * the basic interface methods that exist on HTTPClient; if you wish to call
 * any of these methods you must first obtain an HTTPChannel.
 *
 * Pass true for persistent_connection to gain this network efficiency.  If,
 * on the other hand, your intention is to use the channel to retrieve only
 * one document, then pass false to inform the server that we will be dropping
 * the connection after the first document.
 */
PT(HTTPChannel) HTTPClient::
make_channel(bool persistent_connection) {
  PT(HTTPChannel) doc = new HTTPChannel(this);
  doc->set_persistent_connection(persistent_connection);
  return doc;
}

/**
 * Posts form data to a particular URL and retrieves the response.  Returns a
 * new HTTPChannel object whether the document is successfully read or not;
 * you can test is_valid() and get_return_code() to determine whether the
 * document was retrieved.
 */
PT(HTTPChannel) HTTPClient::
post_form(const URLSpec &url, const string &body) {
  PT(HTTPChannel) doc = new HTTPChannel(this);
  doc->post_form(url, body);
  return doc;
}

/**
 * Opens the named document for reading.  Returns a new HTTPChannel object
 * whether the document is successfully read or not; you can test is_valid()
 * and get_return_code() to determine whether the document was retrieved.
 */
PT(HTTPChannel) HTTPClient::
get_document(const URLSpec &url) {
  PT(HTTPChannel) doc = new HTTPChannel(this);
  doc->get_document(url);
  return doc;
}

/**
 * Like get_document(), except only the header associated with the document is
 * retrieved.  This may be used to test for existence of the document; it
 * might also return the size of the document (if the server gives us this
 * information).
 */
PT(HTTPChannel) HTTPClient::
get_header(const URLSpec &url) {
  PT(HTTPChannel) doc = new HTTPChannel(this);
  doc->get_header(url);
  return doc;
}

/**
 * Returns the default global HTTPClient.
 */
HTTPClient *HTTPClient::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new HTTPClient;
  }
  return _global_ptr;
}


/**
 * Returns the OpenSSL context object, creating it first if needed.
 */
SSL_CTX *HTTPClient::
get_ssl_ctx() {
  if (_ssl_ctx != nullptr) {
    return _ssl_ctx;
  }

  init_random_seed();

  _ssl_ctx = SSL_CTX_new(SSLv23_client_method());

#ifndef NDEBUG
  // If we have debugging enabled, set a callback that allows us to report the
  // SSL messages as they are sent and received.
  if (downloader_cat.is_debug()) {
    SSL_CTX_set_msg_callback(_ssl_ctx, ssl_msg_callback);
  }
#endif

  // Make sure the error strings are loaded.
  OpenSSLWrapper *sslw = OpenSSLWrapper::get_global_ptr();
  sslw->notify_ssl_errors();

  X509_STORE *store = sslw->get_x509_store();
#if OPENSSL_VERSION_NUMBER >= 0x10100000
  if (store != nullptr) {
    X509_STORE_up_ref(store);
  }
#endif
  SSL_CTX_set_cert_store(_ssl_ctx, store);

  return _ssl_ctx;
}

/**
 * Checks to see if the indicated certificate is on the pre-approved list for
 * the indicated server's URL.
 *
 * If the full cert itself (including its key) is on the pre-approved list,
 * sets both cert_preapproved and cert_name_preapproved to true.
 *
 * If the full cert is not on the pre-approved list, but its name matches a
 * name on the pre-approved list, sets cert_name_preapproved to true, and
 * cert_preapproved to false.
 *
 * Otherwise, sets both values to false.  This doesn't mean the cert is
 * necessarily invalid, just that it wasn't on the pre-approved list (which is
 * usually empty anyway).
 */
void HTTPClient::
check_preapproved_server_certificate(const URLSpec &url, X509 *cert,
                                     bool &cert_preapproved, bool &cert_name_preapproved) const {
  cert_preapproved = false;
  cert_name_preapproved = false;

  string server_and_port = url.get_server_and_port();
  PreapprovedServerCerts::const_iterator psci =
    _preapproved_server_certs.find(server_and_port);

  if (psci == _preapproved_server_certs.end()) {
    // No preapproved certs defined for this server.
    return;
  }

  const PreapprovedServerCert &psc = (*psci).second;

  // See if we have an exact match on the cert itself.
  ServerCerts::const_iterator sci;
  for (sci = psc._certs.begin(); sci != psc._certs.end(); ++sci) {
    if (X509_cmp(cert, *sci) == 0) {
      cert_preapproved = true;
      cert_name_preapproved = true;
      downloader_cat.info()
        << "Server certificate is pre-approved.\n";
      return;
    }
  }

  // OK, look for a match on the cert's name only.
  X509_NAME *subject = X509_get_subject_name(cert);
  ServerCertNames::const_iterator scni;
  for (scni = psc._cert_names.begin(); scni != psc._cert_names.end(); ++scni) {
    X509_NAME *cert_name = (*scni);
    if (x509_name_subset(cert_name, subject)) {
      downloader_cat.info()
        << "Server certificate name is pre-approved.\n";
      cert_name_preapproved = true;
      return;
    }
  }

  // Didn't find any match.
  return;
}

/**
 * Adds the proxy servers associated with the indicated scheme, if any, to the
 * list.  Returns true if any were added, false otherwise.
 */
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

/**
 * Handles a Config definition for http-username as
 * server:realm:username:password, where either or both of server and realm
 * may be empty, or just server:username:password or username:password.
 */
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

/**
 * Chooses a suitable username:password string for the given URL and realm.
 */
string HTTPClient::
select_username(const URLSpec &url, bool is_proxy, const string &realm) const {
  string username;

  // Look in several places in order to find the matching username.

  // Fist, if there's a username on the URL, that always wins (except when we
  // are looking for a proxy username).
  if (url.has_username() && !is_proxy) {
    username = url.get_username();
  }

  // Otherwise, start looking on the HTTPClient.
  if (is_proxy) {
    if (username.empty()) {
      // Try the *proxyrealm.
      username = get_username("*proxy", realm);
    }
    if (username.empty()) {
      // Then, try *proxyany realm.
      username = get_username("*proxy", string());
    }
  }
  if (username.empty()) {
    // Try the specific serverrealm.
    username = get_username(url.get_server(), realm);
  }
  if (username.empty()) {
    // Then, try the specific serverany realm.
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

/**
 * Chooses a suitable pre-computed authorization for the indicated URL.
 * Returns NULL if no authorization matches.
 */
HTTPAuthorization *HTTPClient::
select_auth(const URLSpec &url, bool is_proxy, const string &last_realm) {
  Domains &domains = is_proxy ? _proxy_domains : _www_domains;
  string canon = HTTPAuthorization::get_canonical_url(url).get_url();

  // Look for the longest domain string that is a prefix of our canonical URL.
  // We have to make a linear scan through the list.
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
  return nullptr;
}

/**
 * Generates a new authorization entry in response to a 401 or 407 challenge
 * from the server or proxy.  The new authorization entry is stored for future
 * connections to the same server (or, more precisely, the same domain, which
 * may be a subset of the server, or it may include multiple servers).
 */
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

  if (auth == nullptr || !auth->is_valid()) {
    si = schemes.find("basic");
    if (si != schemes.end()) {
      auth = new HTTPBasicAuthorization((*si).second, url, is_proxy);
    }
  }

  if (auth == nullptr || !auth->is_valid()) {
    downloader_cat.warning()
      << "Don't know how to use any of the server's available authorization schemes:\n";
    for (si = schemes.begin(); si != schemes.end(); ++si) {
      downloader_cat.warning() << (*si).first << "\n";
    }

  } else {
    // Now that we've got an authorization, store it under under each of its
    // suggested domains for future use.
    Domains &domains = is_proxy ? _proxy_domains : _www_domains;
    const vector_string &domain = auth->get_domain();
    vector_string::const_iterator si;
    for (si = domain.begin(); si != domain.end(); ++si) {
      domains[(*si)]._realms[auth->get_realm()] = auth;
    }
  }

  return auth;
}

/**
 * Frees the resources allocated by a previous call to
 * load_client_certificate(), and marks the certificate unloaded.
 */
void HTTPClient::
unload_client_certificate() {
  if (_client_certificate_priv != nullptr) {
    EVP_PKEY_free(_client_certificate_priv);
    _client_certificate_priv = nullptr;
  }

  if (_client_certificate_pub != nullptr) {
    X509_free(_client_certificate_pub);
    _client_certificate_pub = nullptr;
  }

  _client_certificate_loaded = false;
}

/**
 * Parses a string of the form type0=value0/type1=value1/type2=... into a
 * newly allocated X509_NAME object.  Returns NULL if the string is invalid.
 */
X509_NAME *HTTPClient::
parse_x509_name(const string &source) {
  X509_NAME *result = nullptr;

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
        return nullptr;
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
          return nullptr;
        }
        added_any = true;
      }
    }
  }

  if (!added_any) {
    downloader_cat.info()
      << "Invalid empty X509 name: " << source << "\n";
    X509_NAME_free(result);
    return nullptr;
  }

  return result;
}

/**
 * Returns true if name_a is a subset of name_b: each property of name_a is
 * defined in name_b, and the defined value is equivalent to that of name_a.
 */
bool HTTPClient::
x509_name_subset(X509_NAME *name_a, X509_NAME *name_b) {
  int count_a = X509_NAME_entry_count(name_a);
  for (int ai = 0; ai < count_a; ai++) {
    X509_NAME_ENTRY *na = X509_NAME_get_entry(name_a, ai);

    int bi = X509_NAME_get_index_by_OBJ(name_b, X509_NAME_ENTRY_get_object(na), -1);
    if (bi < 0) {
      // This entry in name_a is not defined in name_b.
      return false;
    }

    X509_NAME_ENTRY *nb = X509_NAME_get_entry(name_b, bi);
    ASN1_STRING *na_value = X509_NAME_ENTRY_get_data(na);
    ASN1_STRING *nb_value = X509_NAME_ENTRY_get_data(nb);
    if (na_value->length != nb_value->length ||
        memcmp(na_value->data, nb_value->data, na_value->length) != 0) {
      // This entry in name_a doesn't match that of name_b.
      return false;
    }
  }
  return true;
}

/**
 * Puts the first word of c into a, and the remainder into b.
 */
void HTTPClient::
split_whitespace(string &a, string &b, const string &c) {
  size_t p = 0;

  // Skip initial whitespace
  while (p < c.length() && isspace(c[p])) {
    ++p;
  }

  // Find the length of the first word
  size_t q = p;
  while (p < c.length() && !isspace(c[p])) {
    ++p;
  }

  a = c.substr(q, p - q);

  // Skip whitespace between words
  while (p < c.length() && isspace(c[p])) {
    ++p;
  }
  b = c.substr(p);
}

/**
 *
 */
HTTPClient::PreapprovedServerCert::
~PreapprovedServerCert() {
  ServerCerts::const_iterator sci;
  for (sci = _certs.begin(); sci != _certs.end(); ++sci) {
    X509_free(*sci);
  }

  ServerCertNames::const_iterator scni;
  for (scni = _cert_names.begin(); scni != _cert_names.end(); ++scni) {
    X509_NAME_free(*scni);
  }
}

#endif  // HAVE_OPENSSL
