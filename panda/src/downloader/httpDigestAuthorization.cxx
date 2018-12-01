/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpDigestAuthorization.cxx
 * @author drose
 * @date 2002-10-25
 */

#include "httpDigestAuthorization.h"

#ifdef HAVE_OPENSSL

#include "httpChannel.h"
#include "openSSLWrapper.h"  // must be included before any other openssl.
#include <openssl/ssl.h>
#include <openssl/md5.h>
#include <time.h>

using std::ostream;
using std::ostringstream;
using std::string;

const string HTTPDigestAuthorization::_mechanism = "digest";

/**
 *
 */
HTTPDigestAuthorization::
HTTPDigestAuthorization(const HTTPAuthorization::Tokens &tokens,
                        const URLSpec &url, bool is_proxy) :
  HTTPAuthorization(tokens, url, is_proxy)
{
  Tokens::const_iterator ti;
  ti = tokens.find("nonce");
  if (ti != tokens.end()) {
    _nonce = (*ti).second;
  }
  _nonce_count = 0;

  ti = tokens.find("opaque");
  if (ti != tokens.end()) {
    _opaque = (*ti).second;
  }

  _algorithm = A_md5;
  ti = tokens.find("algorithm");
  if (ti != tokens.end()) {
    string algo_str = HTTPChannel::downcase((*ti).second);
    if (algo_str == "md5") {
      _algorithm = A_md5;
    } else if (algo_str == "md5-sess") {
      _algorithm = A_md5_sess;
    } else {
      _algorithm = A_unknown;
    }
  }

  _qop = 0;
  ti = tokens.find("qop");
  if (ti != tokens.end()) {
    string qop_str = HTTPChannel::downcase((*ti).second);
    // A comma-delimited list of tokens.

    size_t p = 0;
    while (p < qop_str.length()) {
      while (p < qop_str.length() && isspace(qop_str[p])) {
        ++p;
      }
      size_t q = p;
      size_t last_char = p;
      while (q < qop_str.length() && qop_str[q] != ',') {
        if (!isspace(qop_str[q])) {
          qop_str[q] = tolower(qop_str[q]);
          last_char = q;
        }
        ++q;
      }

      if (last_char > p) {
        _qop |= match_qop_token(qop_str.substr(p, last_char - p + 1));
      }
      p = q + 1;
    }
  }

  // Compute an arbitrary client nonce.
  ostringstream strm;
  strm << time(nullptr) << ":" << clock() << ":"
       << url.get_url() << ":Panda";

  _cnonce = calc_md5(strm.str());
}

/**
 *
 */
HTTPDigestAuthorization::
~HTTPDigestAuthorization() {
}

/**
 * Returns true if the authorization challenge was correctly parsed and is
 * usable, or false if there was some unsupported algorithm or some such
 * requested by the server, rendering the challenge unmeetable.
 */
bool HTTPDigestAuthorization::
is_valid() {
  return (_algorithm != A_unknown);
}

/**
 * Returns the type of authorization mechanism, represented as a string, e.g.
 * "digest".
 */
const string &HTTPDigestAuthorization::
get_mechanism() const {
  return _mechanism;
}

/**
 * Generates a suitable authorization string to send to the server, based on
 * the data stored within this object, for retrieving the indicated URL with
 * the given username:password.
 */
string HTTPDigestAuthorization::
generate(HTTPEnum::Method method, const string &request_path,
         const string &username, const string &body) {
  _nonce_count++;

  size_t colon = username.find(':');
  string username_only = username.substr(0, colon);
  string password_only = username.substr(colon + 1);

  string digest = calc_request_digest(username_only, password_only,
                                      method, request_path, body);

  ostringstream strm;
  strm << "Digest username=\"" << username.substr(0, colon) << "\""
       << ", realm=\"" << get_realm() << "\""
       << ", nonce=\"" << _nonce << "\""
       << ", uri=" << request_path
       << ", response=\"" << digest << "\""
       << ", algorithm=" << _algorithm;

  if (!_opaque.empty()) {
    strm << ", opaque=\"" << _opaque << "\"";
  }

  if (_chosen_qop != Q_unused) {
    strm << ", qop=" << _chosen_qop
         << ", cnonce=\"" << _cnonce << "\""
         << ", nc=" << get_hex_nonce_count();
  }

  return strm.str();
}

/**
 * Returns the bitfield corresponding to the indicated qop token string, or 0
 * if the token string is unrecognized.
 */
int HTTPDigestAuthorization::
match_qop_token(const string &token) {
  if (token == "auth") {
    return Q_auth;
  } else if (token == "auth-int") {
    return Q_auth_int;
  }
  return 0;
}

/**
 * Calculates the appropriate digest response, according to RFC 2617.
 */
string HTTPDigestAuthorization::
calc_request_digest(const string &username, const string &password,
                    HTTPEnum::Method method, const string &request_path,
                    const string &body) {
  _chosen_qop = Q_unused;
  string h_a1 = calc_h(get_a1(username, password));
  string h_a2 = calc_h(get_a2(method, request_path, body));

  ostringstream strm;

  if (_qop == 0) {
    _chosen_qop = Q_unused;
    strm << _nonce << ":" << h_a2;

  } else {
    strm << _nonce << ":" << get_hex_nonce_count() << ":"
         << _cnonce << ":" << _chosen_qop << ":"
         << h_a2;
  }

  return calc_kd(h_a1, strm.str());
}

/**
 * Applies the specified checksum algorithm to the data, according to RFC
 * 2617.
 */
string HTTPDigestAuthorization::
calc_h(const string &data) const {
  switch (_algorithm) {
  case A_unknown:
  case A_md5:
  case A_md5_sess:
    return calc_md5(data);
  }

  return string();
}

/**
 * Applies the specified digest algorithm to the indicated data with the
 * indicated secret, according to RFC 2617.
 */
string HTTPDigestAuthorization::
calc_kd(const string &secret, const string &data) const {
  switch (_algorithm) {
  case A_unknown:
  case A_md5:
  case A_md5_sess:
    return calc_h(secret + ":" + data);
  }

  return string();
}

/**
 * Returns the A1 value, as defined by RFC 2617.
 */
string HTTPDigestAuthorization::
get_a1(const string &username, const string &password) {
  switch (_algorithm) {
  case A_unknown:
  case A_md5:
    return username + ":" + get_realm() + ":" + password;

  case A_md5_sess:
    if (_a1.empty()) {
      _a1 = calc_h(username + ":" + get_realm() + ":" + password) +
        ":" + _nonce + ":" + _cnonce;
    }
    return _a1;
  }

  return string();
}

/**
 * Returns the A2 value, as defined by RFC 2617.
 */
string HTTPDigestAuthorization::
get_a2(HTTPEnum::Method method, const string &request_path,
       const string &body) {
  ostringstream strm;

  if ((_qop & Q_auth_int) != 0 && !body.empty()) {
    _chosen_qop = Q_auth_int;
    strm << method << ":" << request_path << ":" << calc_h(body);

  } else {
    _chosen_qop = Q_auth;
    strm << method << ":" << request_path;
  }

  return strm.str();
}

/**
 * Returns the current nonce count (the number of times we have used the
 * server's nonce value, including this time) as an eight-digit hexadecimal
 * value.
 */
string HTTPDigestAuthorization::
get_hex_nonce_count() const {
  ostringstream strm;
  strm << std::hex << std::setfill('0') << std::setw(8) << _nonce_count;
  return strm.str();
}

/**
 * Computes the MD5 of the indicated source string and returns it as a
 * hexadecimal string of 32 ASCII characters.
 */
string HTTPDigestAuthorization::
calc_md5(const string &source) {
  unsigned char binary[MD5_DIGEST_LENGTH];

  MD5((const unsigned char *)source.data(), source.length(), binary);

  string result;
  result.reserve(MD5_DIGEST_LENGTH * 2);

  for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
    result += hexdigit((binary[i] >> 4) & 0xf);
    result += hexdigit(binary[i] & 0xf);
  }

  return result;
}

ostream &
operator << (ostream &out, HTTPDigestAuthorization::Algorithm algorithm) {
  switch (algorithm) {
  case HTTPDigestAuthorization::A_md5:
    out << "MD5";
    break;
  case HTTPDigestAuthorization::A_md5_sess:
    out << "MD5-sess";
    break;
  case HTTPDigestAuthorization::A_unknown:
    out << "unknown";
    break;
  }

  return out;
}

ostream &
operator << (ostream &out, HTTPDigestAuthorization::Qop qop) {
  switch (qop) {
  case HTTPDigestAuthorization::Q_auth:
    out << "auth";
    break;
  case HTTPDigestAuthorization::Q_auth_int:
    out << "auth-int";
    break;
  case HTTPDigestAuthorization::Q_unused:
    out << "unused";
    break;
  }

  return out;
}

#endif  // HAVE_OPENSSL
