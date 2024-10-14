/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpClient_emscripten.cxx
 * @author rdb
 * @date 2021-02-10
 */

#include "httpClient_emscripten.h"
#include "httpChannel.h"
#include "config_downloader.h"
#include "httpBasicAuthorization.h"
#include "httpDigestAuthorization.h"

#ifdef __EMSCRIPTEN__

#include <emscripten/em_asm.h>

using std::string;

PT(HTTPClient) HTTPClient::_global_ptr;

/**
 *
 */
HTTPClient::
HTTPClient() {
  ConfigVariableList http_username
    ("http-username",
     PRC_DESC("Adds one or more username/password pairs to all HTTP clients.  The client "
              "will present this username/password when asked to authenticate a request "
              "for a particular server and/or realm.  The username is of the form "
              "server:realm:username:password, where either or both of server and "
              "realm may be empty, or just realm:username:password or username:password.  "
              "If the server or realm is empty, they will match anything."));

  {
    // Also load in the general usernames.
    int num_unique_values = http_username.get_num_unique_values();
    for (int i = 0; i < num_unique_values; i++) {
      string username = http_username.get_unique_value(i);
      add_http_username(username);
    }
  }
}

/**
 * Specifies the username:password string corresponding to a particular server
 * and/or realm, when demanded by the server.  Either or both of the server or
 * realm may be empty; if so, they match anything.  Also, the server may be
 * set to the special string `"*proxy"`, which will match any proxy server.
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
  std::ostringstream stream;
  stream << cookie;
  std::string str = stream.str();

  EM_ASM({
    document.cookie = UTF8ToString($0);
  }, str.c_str());
}

/**
 * Removes the cookie with the matching domain/path/name from the client's
 * list of cookies.  Returns true if it was removed, false if the cookie was
 * not matched.
 */
bool HTTPClient::
clear_cookie(const HTTPCookie &cookie) {
  HTTPCookie expired;
  expired.set_name(cookie.get_name());
  expired.set_path(cookie.get_path());
  expired.set_domain(cookie.get_domain());
  expired.set_expires(HTTPDate((time_t)0));

  std::ostringstream stream;
  stream << expired;
  std::string str = stream.str();

  return (bool)EM_ASM_INT({
    var set = UTF8ToString($0);
    var old = document.cookie;
    document.cookie = set;
    return (document.cookie !== old);
  }, str.c_str());
}

/**
 * Removes the all stored cookies from the client.
 */
void HTTPClient::
clear_all_cookies() {
  // NB. This is imperfect, and won't clear cookies with other domains or paths.
  EM_ASM({
    var cookies = document.cookie.split(";");
    for (var i = 0; i < cookies.length; ++i) {
      var name = cookies[i].split("=", 1)[0];
      document.cookie = name + "=;expires=Thu, 01 Jan 1970 00:00:00 GMT";
    }

    cookies = document.cookie.split(";");
    for (var i = 0; i < cookies.length; ++i) {
      var name = cookies[i].split("=", 1)[0];
      document.cookie = name + "=;path=/;expires=Thu, 01 Jan 1970 00:00:00 GMT";
    }
  });
}

/**
 * Returns true if there is a cookie in the client matching the given cookie's
 * domain/path/name, false otherwise.
 */
//bool HTTPClient::
//has_cookie(const HTTPCookie &cookie) const {
//}

/**
 * Looks up and returns the cookie in the client matching the given cookie's
 * domain/path/name.  If there is no matching cookie, returns an empty cookie.
 */
//HTTPCookie HTTPClient::
//get_cookie(const HTTPCookie &cookie) const {
//  return HTTPCookie();
//}

/**
 * Outputs the complete list of cookies stored on the client, for all domains,
 * including the expired cookies (which will normally not be sent back to a
 * host).
 */
void HTTPClient::
write_cookies(std::ostream &out) const {
  char *str = (char *)EM_ASM_INT({
    var str = document.cookie.replace(/; ?/g, "\n");
    var len = lengthBytesUTF8(str) + 1;
    var buffer = _malloc(len);
    stringToUTF8(str, buffer, len);
    return buffer;
  });

  out << str << "\n";
  free(str);
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
 */
PT(HTTPChannel) HTTPClient::
make_channel(bool persistent_connection) {
  return new HTTPChannel(this);
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
        }
        else {
          // Ok, we have four.
          set_username(http_username.substr(0, c1),
                       http_username.substr(c1 + 1, c2 - (c1 + 1)),
                       http_username.substr(c2 + 1));
        }
      }
      else {
        // We have only three.
        set_username(string(),
                     http_username.substr(0, c1),
                     http_username.substr(c1 + 1));
      }
    }
    else {
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
select_username(const URLSpec &url, const string &realm) const {
  string username;

  // Look in several places in order to find the matching username.

  // Fist, if there's a username on the URL, that always wins (except when we
  // are looking for a proxy username).
  if (url.has_username()) {
    username = url.get_username();
  }

  // Otherwise, start looking on the HTTPClient.
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
select_auth(const URLSpec &url, const string &last_realm) {
  Domains &domains = _www_domains;
  std::string canon = HTTPAuthorization::get_canonical_url(url).get_url();

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
generate_auth(const URLSpec &url, const string &challenge) {
  HTTPAuthorization::AuthenticationSchemes schemes;
  HTTPAuthorization::parse_authentication_schemes(schemes, challenge);

  PT(HTTPAuthorization) auth;
  HTTPAuthorization::AuthenticationSchemes::iterator si;

  si = schemes.find("digest");
  if (si != schemes.end()) {
    auth = new HTTPDigestAuthorization((*si).second, url, false);
  }

  if (auth == nullptr || !auth->is_valid()) {
    si = schemes.find("basic");
    if (si != schemes.end()) {
      auth = new HTTPBasicAuthorization((*si).second, url, false);
    }
  }

  if (auth == nullptr || !auth->is_valid()) {
    downloader_cat.warning()
      << "Don't know how to use any of the server's available authorization schemes:\n";
    for (si = schemes.begin(); si != schemes.end(); ++si) {
      downloader_cat.warning() << (*si).first << "\n";
    }
  }
  else {
    // Now that we've got an authorization, store it under under each of its
    // suggested domains for future use.
    Domains &domains = _www_domains;
    const vector_string &domain = auth->get_domain();
    vector_string::const_iterator si;
    for (si = domain.begin(); si != domain.end(); ++si) {
      domains[(*si)]._realms[auth->get_realm()] = auth;
    }
  }

  return auth;
}

#endif  // __EMSCRIPTEN__
