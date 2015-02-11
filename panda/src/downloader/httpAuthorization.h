// Filename: httpAuthorization.h
// Created by:  drose (22Oct02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef HTTPAUTHORIZATION_H
#define HTTPAUTHORIZATION_H

#include "pandabase.h"

// This module requires OpenSSL to compile, even though it doesn't
// actually use any OpenSSL code, because it is a support module for
// HTTPChannel, which *does* use OpenSSL code.

#ifdef HAVE_OPENSSL

#include "referenceCount.h"
#include "httpEnum.h"
#include "pmap.h"

class URLSpec;

////////////////////////////////////////////////////////////////////
//       Class : HTTPAuthorization
// Description : A base class for storing information used to fulfill
//               authorization requests in the past, which can
//               possibly be re-used for future requests to the same
//               server.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS HTTPAuthorization : public ReferenceCount {
public:
  typedef pmap<string, string> Tokens;
  typedef pmap<string, Tokens> AuthenticationSchemes;

protected:
  HTTPAuthorization(const Tokens &tokens, const URLSpec &url,
                    bool is_proxy);
public:
  virtual ~HTTPAuthorization();

  virtual const string &get_mechanism() const=0;
  virtual bool is_valid();

  INLINE const string &get_realm() const;
  INLINE const vector_string &get_domain() const;

  virtual string generate(HTTPEnum::Method method, const string &request_path,
                          const string &username, const string &body)=0;

  static void parse_authentication_schemes(AuthenticationSchemes &schemes,
                                           const string &field_value);
  static URLSpec get_canonical_url(const URLSpec &url);
  static string base64_encode(const string &s);
  static string base64_decode(const string &s);

protected:
  static size_t scan_quoted_or_unquoted_string(string &result, 
                                               const string &source, 
                                               size_t start);

protected:
  string _realm;
  vector_string _domain;
};

#include "httpAuthorization.I"

#endif  // HAVE_OPENSSL

#endif

