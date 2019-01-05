/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file password_hash.cxx
 * @author drose
 * @date 2004-09-01
 */

#include "password_hash.h"

// The functions defined within this file rely on algorithms defined within
// OpenSSL.
#ifdef HAVE_OPENSSL

#include "pnotify.h"
#include <openssl/evp.h>
#include "memoryHook.h"

using std::string;

/**
 * Generates a non-reversible hash of a particular length based on an
 * arbitrary password and a random salt.  This is much stronger than the
 * algorithm implemented by the standard Unix crypt().
 *
 * The resulting hash can be useful for two primary purposes: (1) the hash may
 * be recorded to disk in lieu of recording plaintext passwords, for
 * validation against a password entered by the user later (which should
 * produce the same hash given a particular salt), or (2) the hash may be used
 * as input to an encryption algorithm that requires a key of a particular
 * length.
 *
 * password is the text password provided by a user.
 *
 * salt should be a string of arbitrary random bytes (it need not be
 * crypotographically secure, just different for each different hash).
 *
 * iters should be a number in the thousands to indicate the number of times
 * the hash algorithm should be applied.  In general, iters should be chosen
 * to make the computation as expensive as it can be and still be tolerable,
 * to reduce the attractiveness of a brute-force attack.
 *
 * keylen is the length in bytes of the required key hash.
 */
string
password_hash(const string &password, const string &salt,
              int iters, int keylen) {
  nassertr(iters > 0 && keylen > 0, string());
  unsigned char *dk = (unsigned char *)PANDA_MALLOC_ARRAY(keylen);
  int result =
    PKCS5_PBKDF2_HMAC_SHA1((const char *)password.data(), password.length(),
                           (unsigned char *)salt.data(), salt.length(),
                           iters, keylen, dk);
  nassertr(result > 0, string());

  string hash((char *)dk, keylen);
  PANDA_FREE_ARRAY(dk);
  return hash;
}



#endif  // HAVE_OPENSSL
