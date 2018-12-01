/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpAuthorization.h
 * @author drose
 * @date 2002-10-22
 */

#ifndef HTTPAUTHORIZATION_H
#define HTTPAUTHORIZATION_H

#include "pandabase.h"

// This module requires OpenSSL to compile, even though it doesn't actually
// use any OpenSSL code, because it is a support module for HTTPChannel, which
// *does* use OpenSSL code.

#ifdef HAVE_OPENSSL

#include "referenceCount.h"
#include "httpEnum.h"
#include "pmap.h"

class URLSpec;

/**
 * A base class for storing information used to fulfill authorization requests
 * in the past, which can possibly be re-used for future requests to the same
 * server.
 */
class EXPCL_PANDA_DOWNLOADER HTTPAuthorization : public ReferenceCount {
public:
  typedef pmap<std::string, std::string> Tokens;
  typedef pmap<std::string, Tokens> AuthenticationSchemes;

protected:
  HTTPAuthorization(const Tokens &tokens, const URLSpec &url,
                    bool is_proxy);
public:
  virtual ~HTTPAuthorization();

  virtual const std::string &get_mechanism() const=0;
  virtual bool is_valid();

  INLINE const std::string &get_realm() const;
  INLINE const vector_string &get_domain() const;

  virtual std::string generate(HTTPEnum::Method method, const std::string &request_path,
                          const std::string &username, const std::string &body)=0;

  static void parse_authentication_schemes(AuthenticationSchemes &schemes,
                                           const std::string &field_value);
  static URLSpec get_canonical_url(const URLSpec &url);
  static std::string base64_encode(const std::string &s);
  static std::string base64_decode(const std::string &s);

protected:
  static size_t scan_quoted_or_unquoted_string(std::string &result,
                                               const std::string &source,
                                               size_t start);

protected:
  std::string _realm;
  vector_string _domain;
};

#include "httpAuthorization.I"

#endif  // HAVE_OPENSSL

#endif
