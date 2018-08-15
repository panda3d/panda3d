/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file httpAuthorization.cxx
 * @author drose
 * @date 2002-10-22
 */

#include "httpAuthorization.h"
#include "httpChannel.h"
#include "urlSpec.h"

#ifdef HAVE_OPENSSL

using std::string;

static const char base64_table[64] = {
  'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
  'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
  'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
  'w', 'x', 'y', 'z', '0', '1', '2', '3',
  '4', '5', '6', '7', '8', '9', '+', '/',
};

static unsigned char base64_invert[128];
static bool got_base64_invert = false;

/**
 *
 */
HTTPAuthorization::
HTTPAuthorization(const HTTPAuthorization::Tokens &tokens,
                  const URLSpec &url, bool is_proxy) {
  Tokens::const_iterator ti;
  ti = tokens.find("realm");
  if (ti != tokens.end()) {
    _realm = (*ti).second;
  }

  URLSpec canon = get_canonical_url(url);

  ti = tokens.find("domain");
  if (ti != tokens.end() && !is_proxy) {
    // Now the domain consists of a series of space-separated URL prefixes.
    const string &domain = (*ti).second;
    size_t p = 0;
    while (p < domain.length()) {
      while (p < domain.length() && isspace(domain[p])) {
        ++p;
      }
      size_t q = p;
      while (q < domain.length() && !isspace(domain[q])) {
        ++q;
      }
      if (q > p) {
        string domain_str = domain.substr(p, q - p);
        URLSpec domain_url(domain_str);
        if (domain_url.has_server()) {
          // A fully-qualified URL.
          _domain.push_back(get_canonical_url(domain_url).get_url());
        } else {
          // A relative URL; relative to this path.
          domain_url = canon;
          domain_url.set_path(domain_str);
          _domain.push_back(domain_url.get_url());
        }
      }
      p = q;
    }

  } else {
    // If no domain is defined by the server, use the supplied URL. Truncate
    // it to the rightmost slash.
    string canon_str = canon.get_url();
    size_t slash = canon_str.rfind('/');
    nassertv(slash != string::npos);
    _domain.push_back(canon_str.substr(0, slash + 1));
  }
}

/**
 *
 */
HTTPAuthorization::
~HTTPAuthorization() {
}

/**
 * Returns true if the authorization challenge was correctly parsed and is
 * usable, or false if there was some unsupported algorithm or some such
 * requested by the server, rendering the challenge unmeetable.
 */
bool HTTPAuthorization::
is_valid() {
  return true;
}

/**
 * Decodes the text following a WWW-Authenticate: or Proxy-Authenticate:
 * header field.
 */
void HTTPAuthorization::
parse_authentication_schemes(HTTPAuthorization::AuthenticationSchemes &schemes,
                             const string &field_value) {
  // This string will consist of one or more records of the form: scheme
  // token=value[,token=value[,...]] If there are multiple records, they will
  // be comma-delimited, which makes parsing just a bit tricky.

  // Start by skipping initial whitespace.
  size_t p = 0;
  while (p < field_value.length() && isspace(field_value[p])) {
    ++p;
  }

  if (p < field_value.length()) {
    size_t q = p;
    while (q < field_value.length() && !isspace(field_value[q])) {
      ++q;
    }
    // Here's our first scheme.
    string scheme = HTTPChannel::downcase(field_value.substr(p, q - p));
    Tokens *tokens = &(schemes[scheme]);

    // Now pull off the tokens, one at a time.
    p = q + 1;
    while (p < field_value.length()) {
      q = p;
      while (q < field_value.length() && field_value[q] != '=' &&
             field_value[q] != ',' && !isspace(field_value[q])) {
        ++q;
      }
      if (field_value[q] == '=') {
        // This is a token.
        string token = HTTPChannel::downcase(field_value.substr(p, q - p));
        string value;
        p = scan_quoted_or_unquoted_string(value, field_value, q + 1);
        (*tokens)[token] = value;

        // Skip trailing whitespace and extra commas.
        while (p < field_value.length() &&
               (field_value[p] == ',' || isspace(field_value[p]))) {
          ++p;
        }

      } else {
        // This is not a token; it must be the start of a new scheme.
        scheme = HTTPChannel::downcase(field_value.substr(p, q - p));
        tokens = &(schemes[scheme]);
        p = q + 1;
      }
    }
  }
}

/**
 * Returns the "canonical" URL corresponding to this URL.  This is the same
 * URL with an explicit port indication, an explicit scheme, and a non-empty
 * path, etc.
 */
URLSpec HTTPAuthorization::
get_canonical_url(const URLSpec &url) {
  URLSpec canon = url;
  canon.set_scheme(canon.get_scheme());
  canon.set_username(string());
  canon.set_port(canon.get_port());
  canon.set_path(canon.get_path());

  return canon;
}

/**
 * Returns the input string encoded using base64.  No respect is paid to
 * maintaining a 76-char line length.
 */
string HTTPAuthorization::
base64_encode(const string &s) {
  // Collect the string 3 bytes at a time into 24-bit words, then output each
  // word using 4 bytes.
  size_t num_words = (s.size() + 2) / 3;
  string result;
  result.reserve(num_words * 4);
  size_t p;
  for (p = 0; p + 2 < s.size(); p += 3) {
    unsigned int word =
      ((unsigned)s[p] << 16) |
      ((unsigned)s[p + 1] << 8) |
      ((unsigned)s[p + 2]);
    result += base64_table[(word >> 18) & 0x3f];
    result += base64_table[(word >> 12) & 0x3f];
    result += base64_table[(word >> 6) & 0x3f];
    result += base64_table[(word) & 0x3f];
  }
  // What's left over?
  if (p < s.size()) {
    unsigned int word = ((unsigned)s[p] << 16);
    p++;
    if (p < s.size()) {
      word |= ((unsigned)s[p] << 8);
      p++;
      nassertr(p == s.size(), result);

      result += base64_table[(word >> 18) & 0x3f];
      result += base64_table[(word >> 12) & 0x3f];
      result += base64_table[(word >> 6) & 0x3f];
      result += '=';
    } else {
      result += base64_table[(word >> 18) & 0x3f];
      result += base64_table[(word >> 12) & 0x3f];
      result += '=';
      result += '=';
    }
  }

  return result;
}

/**
 * Returns the string decoded from base64.
 */
string HTTPAuthorization::
base64_decode(const string &s) {
  // Build up the invert table if this is the first time.
  if (!got_base64_invert) {
    int i;
    for (i = 0; i < 128; i++) {
      base64_invert[i] = 0xff;
    }

    for (int i = 0; i < 64; i++) {
      base64_invert[(int)base64_table[i]] = i;
    }

    base64_invert[(int)'='] = 0;

    got_base64_invert = true;
  }

  // Collect the string 4 bytes at a time; decode this back into a 24-bit word
  // and output the 3 corresponding bytes.
  size_t num_words = s.size() / 4;
  string result;
  result.reserve(num_words * 3);
  size_t p;
  for (p = 0; p < s.size(); p += 4) {
    unsigned int c0 = base64_invert[s[p] & 0x7f];
    unsigned int c1 = base64_invert[s[p + 1] & 0x7f];
    unsigned int c2 = base64_invert[s[p + 2] & 0x7f];
    unsigned int c3 = base64_invert[s[p + 3] & 0x7f];

    unsigned int word =
      (c0 << 18) | (c1 << 12) | (c2 << 6) | c3;

    result += (char)((word >> 16) & 0xff);
    if (s[p + 2] != '=') {
      result += (char)((word >> 8) & 0xff);
      if (s[p + 3] != '=') {
        result += (char)(word & 0xff);
      }
    }
  }

  return result;
}

/**
 * Scans the string source beginning at character position start, to identify
 * either the (space-delimited) unquoted string there, or the (quote-
 * delimited) quoted string.  In either case, fills the string found into
 * result, and returns the next character position after the string (or after
 * its closing quote mark).
 */
size_t HTTPAuthorization::
scan_quoted_or_unquoted_string(string &result, const string &source,
                               size_t start) {
  result = string();

  if (start < source.length()) {
    if (source[start] == '"') {
      // Quoted string.
      size_t p = start + 1;
      while (p < source.length() && source[p] != '"') {
        if (source[p] == '\\') {
          // Backslash escapes.
          ++p;
          if (p < source.length()) {
            result += source[p];
            ++p;
          }
        } else {
          result += source[p];
          ++p;
        }
      }
      if (p < source.length()) {
        ++p;
      }
      return p;
    }

    // Unquoted string.
    size_t p = start;
    while (p < source.length() && source[p] != ',' && !isspace(source[p])) {
      result += source[p];
      ++p;
    }

    return p;
  }

  // Empty string.
  return start;
}

#endif  // HAVE_OPENSSL
