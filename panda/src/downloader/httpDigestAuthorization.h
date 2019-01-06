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

#ifdef HAVE_OPENSSL

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

  virtual std::string generate(HTTPEnum::Method method, const std::string &request_path,
                          const std::string &username, const std::string &body);

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
  static int match_qop_token(const std::string &token);

  std::string calc_request_digest(const std::string &username, const std::string &password,
                             HTTPEnum::Method method,
                             const std::string &request_path, const std::string &body);
  std::string calc_h(const std::string &data) const;
  std::string calc_kd(const std::string &secret, const std::string &data) const;
  std::string get_a1(const std::string &username, const std::string &password);
  std::string get_a2(HTTPEnum::Method method, const std::string &request_path,
                const std::string &body);
  std::string get_hex_nonce_count() const;

  static std::string calc_md5(const std::string &source);
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
