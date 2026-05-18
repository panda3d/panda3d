/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpDigestAuthorization.h
 * @author drose
 * @date 2002-10-25
 */

#ifndef HTTPDIGESTAUTHORIZATION_H
#define HTTPDIGESTAUTHORIZATION_H

#include "pandabase.h"

// This module requires OpenSSL to compile, even though it doesn't actually
// use any OpenSSL code, because it is a support module for HTTPChannel, which
// *does* use OpenSSL code.

#if defined(HAVE_OPENSSL) || defined(__EMSCRIPTEN__)

#include "httpAuthorization.h"

/**
 * Implements the "Digest" type of HTTP authorization.  This is designed to be
 * an improvement over "Basic" authorization, in that it does not send
 * passwords over the net in cleartext, and it is harder to spoof.
 */
class HTTPDigestAuthorization : public HTTPAuthorization {
public:
  HTTPDigestAuthorization(const Tokens &tokens, const URLSpec &url,
                          bool is_proxy);
  virtual ~HTTPDigestAuthorization();

  virtual const std::string &get_mechanism() const;
  virtual bool is_valid();

  virtual std::string generate(HTTPEnum::Method method, std::string_view request_path,
                          std::string_view username, std::string_view body);

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
  static int match_qop_token(std::string_view token);

  std::string calc_request_digest(std::string_view username, std::string_view password,
                             HTTPEnum::Method method,
                             std::string_view request_path, std::string_view body);
  std::string calc_h(std::string_view data) const;
  std::string calc_kd(std::string_view secret, std::string_view data) const;
  std::string get_a1(std::string_view username, std::string_view password);
  std::string get_a2(HTTPEnum::Method method, std::string_view request_path,
                std::string_view body);
  std::string get_hex_nonce_count() const;

  static std::string calc_md5(std::string_view source);
  INLINE static char hexdigit(int value);

  std::string _cnonce;
  std::string _nonce;
  int _nonce_count;
  std::string _opaque;

  Algorithm _algorithm;
  std::string _a1;

  int _qop;
  Qop _chosen_qop;

  static const std::string _mechanism;
};

std::ostream &operator << (std::ostream &out, HTTPDigestAuthorization::Algorithm algorithm);
std::ostream &operator << (std::ostream &out, HTTPDigestAuthorization::Qop qop);

#include "httpDigestAuthorization.I"

#endif  // HAVE_OPENSSL

#endif
