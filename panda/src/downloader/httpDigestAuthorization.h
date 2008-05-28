// Filename: httpDigestAuthorization.h
// Created by:  drose (25Oct02)
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

#ifndef HTTPDIGESTAUTHORIZATION_H
#define HTTPDIGESTAUTHORIZATION_H

#include "pandabase.h"

// This module requires OpenSSL to compile, even though it doesn't
// actually use any OpenSSL code, because it is a support module for
// HTTPChannel, which *does* use OpenSSL code.

#ifdef HAVE_OPENSSL

#include "httpAuthorization.h"

////////////////////////////////////////////////////////////////////
//       Class : HTTPDigestAuthorization
// Description : Implements the "Digest" type of HTTP authorization.
//               This is designed to be an improvement over "Basic"
//               authorization, in that it does not send passwords
//               over the net in cleartext, and it is harder to spoof.
////////////////////////////////////////////////////////////////////
class HTTPDigestAuthorization : public HTTPAuthorization {
public:
  HTTPDigestAuthorization(const Tokens &tokens, const URLSpec &url,
                          bool is_proxy);
  virtual ~HTTPDigestAuthorization();

  virtual const string &get_mechanism() const;
  virtual bool is_valid();

  virtual string generate(HTTPEnum::Method method, const string &request_path,
                          const string &username, const string &body);

public:
  enum Algorithm {
    A_unknown,
    A_md5,
    A_md5_sess,
  };
  enum Qop {
    // These are used as a bitfield.
    Q_unused   = 0x000,
    Q_auth     = 0x001,
    Q_auth_int = 0x002,
  };

private:
  static int match_qop_token(const string &token);

  string calc_request_digest(const string &username, const string &password,
                             HTTPEnum::Method method, 
                             const string &request_path, const string &body);
  string calc_h(const string &data) const;
  string calc_kd(const string &secret, const string &data) const;
  string get_a1(const string &username, const string &password);
  string get_a2(HTTPEnum::Method method, const string &request_path, 
                const string &body);
  string get_hex_nonce_count() const;

  static string calc_md5(const string &source);
  INLINE static char hexdigit(int value);

  string _cnonce;
  string _nonce;
  int _nonce_count;
  string _opaque;

  Algorithm _algorithm;
  string _a1;

  int _qop;
  Qop _chosen_qop;

  static const string _mechanism;
};

ostream &operator << (ostream &out, HTTPDigestAuthorization::Algorithm algorithm);
ostream &operator << (ostream &out, HTTPDigestAuthorization::Qop qop);

#include "httpDigestAuthorization.I"

#endif  // HAVE_OPENSSL

#endif

