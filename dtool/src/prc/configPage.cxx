/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file configPage.cxx
 * @author drose
 * @date 2004-10-15
 */

#include "configPage.h"
#include "configDeclaration.h"
#include "configVariableCore.h"
#include "configVariableManager.h"
#include "prcKeyRegistry.h"
#include "config_prc.h"
#include "encryptStream.h"

#include <ctype.h>

#ifdef HAVE_OPENSSL
#include <openssl/evp.h>
#endif

using std::istream;
using std::ostream;
using std::string;

ConfigPage *ConfigPage::_default_page = nullptr;
ConfigPage *ConfigPage::_local_page = nullptr;

/**
 * The constructor is private because a ConfigPage should be constructed via
 * the ConfigPageManager make_page() interface.
 */
ConfigPage::
ConfigPage(const string &name, bool implicit_load, int page_seq) :
  _name(name),
  _implicit_load(implicit_load),
  _page_seq(page_seq),
  _sort(implicit_load ? 10 : 0),
  _next_decl_seq(1),
  _trust_level(0)
{
}

/**
 * The destructor is private because a ConfigPage should be deleted via the
 * ConfigPageManager delete_page() interface.
 */
ConfigPage::
~ConfigPage() {
  clear();
}

/**
 * Returns a pointer to the global "default page".  This is the ConfigPage
 * that lists all variables' original default values.
 */
ConfigPage *ConfigPage::
get_default_page() {
  if (_default_page == nullptr) {
    _default_page = new ConfigPage("default", false, 0);
  }
  return _default_page;
}

/**
 * Returns a pointer to the global "local page".  This is the ConfigPage that
 * lists the locally-assigned values for any variables in the world that have
 * such a local assignment.
 */
ConfigPage *ConfigPage::
get_local_page() {
  if (_local_page == nullptr) {
    _local_page = new ConfigPage("local", false, 0);
  }
  return _local_page;
}

/**
 * Changes the explicit sort order of this particular ConfigPage.  Lower-
 * numbered pages supercede higher-numbered pages.  Initially, all explicitly-
 * loaded pages have sort value 0, and implicitly-loaded pages (found on disk)
 * have sort value 10; you may set an individual page higher or lower to
 * influence its priority relative to other pages.
 */
void ConfigPage::
set_sort(int sort) {
  if (_sort != sort) {
    _sort = sort;
    ConfigPageManager::get_global_ptr()->mark_unsorted();
  }
}

/**
 * Removes all of the declarations from the page.
 */
void ConfigPage::
clear() {
  Declarations::iterator di;
  for (di = _declarations.begin(); di != _declarations.end(); ++di) {
    ConfigDeclaration *decl = (*di);
    delete decl;
  }
  _declarations.clear();
  _trust_level = 0;
  _signature = string();
}

/**
 * Reads the contents of a complete prc file, as returned by the indicated
 * istream, into the current page file.  Returns true on success, or false on
 * some I/O error.
 *
 * This is a low-level interface.  Normally you do not need to call it
 * directly.  See the global functions load_prc_file() and unload_prc_file(),
 * defined in panda/src/putil, for a higher-level interface.
 */
bool ConfigPage::
read_prc(istream &in) {
  // We must empty the page before we start to read it; otherwise trust level
  // is meaningless.
  clear();

  // We avoid getline() here because of its notorious problem with last lines
  // that lack a trailing newline character.
  static const size_t buffer_size = 1024;
  char buffer[buffer_size];

#ifdef HAVE_OPENSSL
  // Set up the evp context for verifying the signature, if we find one.
  _md_ctx = EVP_MD_CTX_create();
  EVP_VerifyInit((EVP_MD_CTX *)_md_ctx, EVP_sha1());
#endif  // HAVE_OPENSSL

  string prev_line;

  in.read(buffer, buffer_size);
  size_t count = in.gcount();
  while (count != 0) {
    char *buffer_end = buffer + count;

    // Look for the first line in the buffer..
    char *newline = (char *)memchr((void *)buffer, '\n', count);
    if (newline == nullptr) {
      // The buffer was one long line.  Huh.
      prev_line += string(buffer, count);

    } else {
      // Process the first line in the buffer.
      size_t length = newline - buffer;
      read_prc_line(prev_line + string(buffer, length + 1));

      // Now look for the next line, etc.
      char *start = newline + 1;
      newline = (char *)memchr((void *)start, '\n', buffer_end - start);
      while (newline != nullptr) {
        length = newline - start;
        read_prc_line(string(start, length + 1));
        start = newline + 1;
        newline = (char *)memchr((void *)start, '\n', buffer_end - start);
      }

      // The remaining text in the buffer is the start of the next line.
      length = buffer_end - start;
      prev_line = string(start, length);
    }

    if (in.fail() || in.eof()) {
      // If we got a failure reading the buffer last time, don't keep reading
      // again.  Irix seems to require this test; otherwise, it repeatedly
      // returns the same text at the end of the file.
      count = 0;

    } else {
      in.read(buffer, buffer_size);
      count = in.gcount();
    }
  }

  if (!prev_line.empty()) {
    read_prc_line(prev_line);
  }

#ifdef HAVE_OPENSSL
  // Now validate the signature and free the SSL structures.
  if (!_signature.empty()) {
    PrcKeyRegistry *pkr = PrcKeyRegistry::get_global_ptr();
    int num_keys = pkr->get_num_keys();
    for (int i = 1; i < num_keys && _trust_level == 0; i++) {
      EVP_PKEY *pkey = pkr->get_key(i);
      if (pkey != nullptr) {
        int verify_result =
          EVP_VerifyFinal((EVP_MD_CTX *)_md_ctx,
                          (unsigned char *)_signature.data(),
                          _signature.size(), pkey);
        if (verify_result == 1) {
          _trust_level = i;
        }
      }
    }
    if (_trust_level == 0) {
      prc_cat->info()
        << "invalid signature found in " << get_name() << "\n";
    }
  }
  EVP_MD_CTX_destroy((EVP_MD_CTX *)_md_ctx);
#endif  // HAVE_OPENSSL

  bool failed = (in.fail() && !in.eof());

  return !failed;
}

/**
 * Automatically decrypts and reads the stream, given the indicated password.
 * Note that if the password is incorrect, the result may be garbage.
 */
bool ConfigPage::
read_encrypted_prc(istream &in, const string &password) {
#ifdef HAVE_OPENSSL
  IDecryptStream decrypt(&in, false, password);
  return read_prc(decrypt);
#else
  return false;
#endif  // HAVE_OPENSSL
}

/**
 * Adds the indicated variable/value pair as a new declaration on the page.
 */
ConfigDeclaration *ConfigPage::
make_declaration(const string &variable, const string &value) {
  ConfigVariableManager *variable_mgr = ConfigVariableManager::get_global_ptr();
  return make_declaration(variable_mgr->make_variable(variable), value);
}

/**
 * Adds the indicated variable/value pair as a new declaration on the page.
 */
ConfigDeclaration *ConfigPage::
make_declaration(ConfigVariableCore *variable, const string &value) {
  ConfigDeclaration *decl = new ConfigDeclaration
    (this, variable, value, _next_decl_seq);
  _next_decl_seq++;

  _declarations.push_back(decl);
  make_dirty();

  return decl;
}

/**
 * Removes the indicated declaration from the page and deletes it.  Returns
 * true if the declaration is successfully removed, false if it was not on the
 * page.
 */
bool ConfigPage::
delete_declaration(ConfigDeclaration *decl) {
  Declarations::iterator di;
  for (di = _declarations.begin(); di != _declarations.end(); ++di) {
    if ((*di) == decl) {
      _declarations.erase(di);
      delete decl;
      make_dirty();
      return true;
    }
  }

  return false;
}

/**
 * Returns the number of declarations on the page.
 */
size_t ConfigPage::
get_num_declarations() const {
  return _declarations.size();
}

/**
 * Returns the nth declaration on the page.
 */
const ConfigDeclaration *ConfigPage::
get_declaration(size_t n) const {
  nassertr(n < _declarations.size(), nullptr);
  return _declarations[n];
}

/**
 * Returns a modifiable pointer to the nth declaration on the page.  Any
 * modifications will appear in the output, if the page is written out with
 * ConfigPage::write().
 */
ConfigDeclaration *ConfigPage::
modify_declaration(size_t n) {
  nassertr(n < _declarations.size(), nullptr);
  return _declarations[n];
}

/**
 * Returns the variable named by the nth declaration on the page.
 */
string ConfigPage::
get_variable_name(size_t n) const {
  nassertr(n < _declarations.size(), string());
  return _declarations[n]->get_variable()->get_name();
}

/**
 * Returns the value assigned by the nth declaration on the page.
 */
string ConfigPage::
get_string_value(size_t n) const {
  nassertr(n < _declarations.size(), string());
  return _declarations[n]->get_string_value();
}

/**
 * Returns true if the nth active variable on the page has been used by code,
 * false otherwise.
 */
bool ConfigPage::
is_variable_used(size_t n) const {
  nassertr(n < _declarations.size(), false);
  return _declarations[n]->get_variable()->is_used();
}

/**
 *
 */
void ConfigPage::
output(ostream &out) const {
  out << "ConfigPage " << get_name() << ", " << get_num_declarations()
      << " declarations.";
}

/**
 * Outputs the first few hex digits of the signature.
 */
void ConfigPage::
output_brief_signature(ostream &out) const {
  size_t num_bytes = std::min(_signature.size(), (size_t)8);
  for (size_t p = 0; p < num_bytes; ++p) {
    unsigned int byte = _signature[p];

    unsigned int hi = (byte >> 4) & 0xf;
    if (hi >= 10) {
      out << (char)((hi - 10) + 'a');
    } else {
      out << (char)(hi + '0');
    }

    unsigned int lo = byte & 0xf;
    if (lo >= 10) {
      out << (char)((lo - 10) + 'a');
    } else {
      out << (char)(lo + '0');
    }
  }
}

/**
 *
 */
void ConfigPage::
write(ostream &out) const {
  Declarations::const_iterator di;
  for (di = _declarations.begin(); di != _declarations.end(); ++di) {
    (*di)->write(out);
  }
}

/**
 * Handles reading in a single line from a .prc file.  This is called
 * internally by read_prc() for each line.
 */
void ConfigPage::
read_prc_line(const string &line) {
  if (line.substr(0, 7) == "##!sig ") {
    // This is a signature.  Accumulate it into the signature and return, and
    // don't count it as contributing to the hash.
    for (size_t p = 7; p < line.length() - 1; p += 2) {
      unsigned char digit = (hex_digit(line[p]) << 4) | hex_digit(line[p + 1]);
      _signature += digit;
    }
    return;
  }

#ifdef HAVE_OPENSSL
  // Accumulate any line that's not itself a signature into the hash, so we
  // can validate the signature at the end.
  EVP_VerifyUpdate((EVP_MD_CTX *)_md_ctx, line.data(), line.size());
#endif  // HAVE_OPENSSL

  // Separate the line into a variable and a value.
  size_t p = 0;
  while (p < line.length() && isspace((unsigned char)line[p])) {
    p++;
  }

  if (p == line.length() || line[p] == '#') {
    // Blank line or comment; do nothing.
    return;
  }

  size_t variable_begin = p;
  while (p < line.length() && !isspace((unsigned char)line[p])) {
    p++;
  }
  size_t variable_end = p;

  while (p < line.length() && isspace((unsigned char)line[p])) {
    p++;
  }
  size_t value_begin = p;

  // Is there an embedded comment on this line?
  p = line.find(" #", value_begin);
  if (p == string::npos) {
    // No, the value extends all the way to the end of the line.
    p = line.length();
  }

  // The value extends from here to the end of the line (or to the start of
  // the embedded comment), so trim whitespace backwards off from there.
  while (p > value_begin && isspace((unsigned char)line[p - 1])) {
    p--;
  }
  size_t value_end = p;

  string variable = line.substr(variable_begin, variable_end - variable_begin);
  string value = line.substr(value_begin, value_end - value_begin);

  make_declaration(variable, value);
}

/**
 * Decodes a hex digit into its numeric value.
 */
unsigned int ConfigPage::
hex_digit(unsigned char digit) {
  if (isalpha(digit)) {
    return tolower(digit) - 'a' + 10;
  } else if (isdigit(digit)) {
    return digit - '0';
  } else {
    return 0;
  }
}
