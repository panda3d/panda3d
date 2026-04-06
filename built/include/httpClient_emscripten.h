/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpClient_emscripten.h
 * @author rdb
 * @date 2021-02-10
 */

#ifndef HTTPCLIENT_EMSCRIPTEN_H
#define HTTPCLIENT_EMSCRIPTEN_H

#include "pandabase.h"

#ifdef __EMSCRIPTEN__

#include "urlSpec.h"
#include "httpAuthorization.h"
#include "httpEnum.h"
#include "httpChannel.h"
#include "httpCookie.h"
#include "pointerTo.h"
#include "pvector.h"
#include "pmap.h"
#include "referenceCount.h"

class Filename;
class HTTPChannel;

/**
 * Reduced version of HTTPClient that is available in Emscripten.  It uses the
 * browser to make HTTP requests.  As such, there is only a global HTTPClient
 * pointer, and it is not possible to make individual HTTPClient objects.
 */
class EXPCL_PANDA_DOWNLOADER HTTPClient : public ReferenceCount {
private:
  HTTPClient();

PUBLISHED:
  void set_username(const std::string &server, const std::string &realm, const std::string &username);
  std::string get_username(const std::string &server, const std::string &realm) const;

  void set_cookie(const HTTPCookie &cookie);
  bool clear_cookie(const HTTPCookie &cookie);
  void clear_all_cookies();
  //bool has_cookie(const HTTPCookie &cookie) const;
  //HTTPCookie get_cookie(const HTTPCookie &cookie) const;

  void write_cookies(std::ostream &out) const;

  PT(HTTPChannel) make_channel(bool persistent_connection);
  BLOCKING PT(HTTPChannel) post_form(const URLSpec &url, const std::string &body);
  BLOCKING PT(HTTPChannel) get_document(const URLSpec &url);
  BLOCKING PT(HTTPChannel) get_header(const URLSpec &url);

  INLINE static std::string base64_encode(const std::string &s);
  INLINE static std::string base64_decode(const std::string &s);

  static HTTPClient *get_global_ptr();

private:
  void add_http_username(const std::string &http_username);
  std::string select_username(const URLSpec &url, const std::string &realm) const;

  HTTPAuthorization *select_auth(const URLSpec &url, const std::string &last_realm);
  PT(HTTPAuthorization) generate_auth(const URLSpec &url,
                                      const std::string &challenge);

  typedef pmap<std::string, std::string> Usernames;
  Usernames _usernames;

  typedef pmap<std::string, PT(HTTPAuthorization)> Realms;
  class Domain {
  public:
    Realms _realms;
  };
  typedef pmap<std::string, Domain> Domains;
  Domains _www_domains;

  static PT(HTTPClient) _global_ptr;

  friend class HTTPChannel;
};

#include "httpClient_emscripten.I"

#endif  // __EMSCRIPTEN__

#endif
