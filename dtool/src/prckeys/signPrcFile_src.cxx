/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file signPrcFile_src.cxx
 * @author drose
 * @date 2004-10-19
 */

// This file is intended to be #included by a generated *_sign?.cxx file, one
// of the output files of make-prc-key.  This contains the common code to sign
// a prc file with the given signature.

#include "dtoolbase.h"

#include "filename.h"
#include "executionEnvironment.h"
#include "panda_getopt.h"
#include "preprocess_argv.h"

#include <time.h>

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

using std::cerr;
using std::string;

string progname = PROGNAME;

/**
 * A convenience function that is itself a wrapper around the OpenSSL
 * convenience function to output the recent OpenSSL errors.  This function
 * sends the error string to cerr.
 */
void
output_ssl_errors() {
  cerr << "Error occurred in SSL routines.\n";

  static bool strings_loaded = false;
  if (!strings_loaded) {
    ERR_load_crypto_strings();
    strings_loaded = true;
  }

  unsigned long e = ERR_get_error();
  while (e != 0) {
    static const size_t buffer_len = 256;
    char buffer[buffer_len];
    ERR_error_string_n(e, buffer, buffer_len);
    cerr << buffer << "\n";
    e = ERR_get_error();
  }
}

/**
 * Reads a single line of the prc file.
 */
void
read_prc_line(const string &line, string &data) {
  // Strip out lines with this prefix.  These are from a previous signature.
  if (line.substr(0, 3) == "##!") {
    return;
  }

  // Other lines are counted.
  data += line;
  return;
}

/**
 * Reads the entire contents of the file, less any previous signatures, to the
 * indicated string.
 */
void
read_file(std::istream &in, string &data) {
  // We avoid getline() here because of its notorious problem with last lines
  // that lack a trailing newline character.
  static const size_t buffer_size = 1024;
  char buffer[buffer_size];

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
      read_prc_line(prev_line + string(buffer, length + 1), data);

      // Now look for the next line, etc.
      char *start = newline + 1;
      newline = (char *)memchr((void *)start, '\n', buffer_end - start);
      while (newline != nullptr) {
        length = newline - start;
        read_prc_line(string(start, length + 1), data);
        start = newline + 1;
        newline = (char *)memchr((void *)start, '\n', buffer_end - start);
      }

      // The remaining text in the buffer is the start of the next line.
      length = buffer_end - start;
      prev_line = string(start, length);
    }

    in.read(buffer, buffer_size);
    count = in.gcount();
  }

  if (!prev_line.empty()) {
    read_prc_line(prev_line, data);
  }
}

/**
 * Outputs the indicated data stream as a series of hex digits.
 */
void
output_hex(std::ostream &out, const unsigned char *data, size_t size) {
}

/**
 * Applies the signature to the named file.
 */
void
sign_prc(Filename filename, bool no_comments, EVP_PKEY *pkey) {
  filename.set_text();

  pifstream in;
  if (!filename.open_read(in)) {
    cerr << "Unable to read file " << filename << "\n";
    exit(1);
  }

  string data;
  read_file(in, data);
  in.close();

  // If the file didn't end with a newline, make it do so.
  if (!data.empty() && data[data.length() - 1] != '\n') {
    data += '\n';
  }

  // Append the comments before the signature (these get signed too).
  std::ostringstream strm;
  strm << "##!\n";
  if (!no_comments) {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    char formatted[128];
    strftime(formatted, 128, "%I:%M %p %B %d, %Y", t);

    strm << "##! Signed on " << formatted;
    if (ExecutionEnvironment::has_environment_variable("USER")) {
      strm << " by " << ExecutionEnvironment::get_environment_variable("USER");
    }

    time_t generated_time = GENERATED_TIME;
    t = localtime(&generated_time);
    strftime(formatted, 128, "%I:%M %p %B %d, %Y", t);

    strm << "\n"
         << "##! Signed with level " << KEY_NUMBER << " key generated on "
         << formatted << "\n"
         << "##!\n";
  }
  data += strm.str();

  EVP_MD_CTX *md_ctx = EVP_MD_CTX_create();

  EVP_SignInit(md_ctx, EVP_sha1());
  EVP_SignUpdate(md_ctx, data.data(), data.size());

  unsigned int max_size = EVP_PKEY_size(pkey);
  unsigned char *sig_data = new unsigned char[max_size];
  unsigned int sig_size;
  if (!EVP_SignFinal(md_ctx, sig_data, &sig_size, pkey)) {
    output_ssl_errors();
    exit(1);
  }
  assert(sig_size <= max_size);

  EVP_MD_CTX_destroy(md_ctx);

  // Now open the file in write mode and rewrite it with the new signature.
  pofstream out;
  if (!filename.open_write(out)) {
    cerr << "Unable to rewrite file " << filename << "\n";
    exit(1);
  }
  cerr << "Rewriting " << filename << "\n";

  out << data << std::hex << std::setfill('0');
  static const size_t row_width = 32;
  for (size_t p = 0; p < sig_size; p += row_width) {
    out << "##!sig ";

// size_t end = min(sig_size, p + row_width);
    size_t end = sig_size;
    if(end > p+row_width)
       end = p+row_width;

    for (size_t q = p; q < end; q++) {
      out << std::setw(2) << (unsigned int)sig_data[q];
    }
    out << "\n";
  }
  out << std::dec;

  delete[] sig_data;
}


/**
 *
 */
void
usage() {
  time_t generated_time = GENERATED_TIME;
  struct tm *t = localtime(&generated_time);
  char formatted[128];
  strftime(formatted, 128, "%I:%M %p %B %d, %Y", t);

  cerr <<
    "\n" << progname << " [opts] file.prc [file.prc ...]\n\n"

    "This program will sign the named prc file(s) with the trust level " << KEY_NUMBER << "\n"
    "key that was generated on " << formatted << ".  This is necessary\n"
    "for any prc file that defines one or more config variables that require a\n"
    "trust level of " << KEY_NUMBER << ".\n\n"

    "Once the prc file has been signed, it may not be changed without invalidating\n"
    "the signature; if you change the file, you will have to sign it again.\n\n"

    "Each prc file is modified in-place with the new signature.  Any previous\n"
    "signatures in the file are removed.\n\n"

    "Options:\n\n"

    "   -n  Avoids writing a comment to the file that describes the date and\n"
    "       time the signature was applied.\n\n"

    "   -p \"[pass phrase]\"\n"
    "       Uses the indicated pass phrase to decrypt the private key.\n"
    "       If this is not specified on the command line, you will be\n"
    "       prompted interactively.\n\n";
}

/**
 *
 */
int
main(int argc, char **argv) {
  preprocess_argv(argc, argv);
  if (argv[0] != nullptr && *argv[0]) {
    // Get the program name from the command-line arguments, if the OS
    // provides it.
    Filename progfile = Filename::from_os_specific(argv[0]);
    progname = progfile.get_basename_wo_extension();
  }

  extern char *optarg;
  extern int optind;
  const char *optstr = "np:h";

  bool no_comments = false;
  string pass_phrase;
  bool got_pass_phrase = false;

  int flag = getopt(argc, argv, optstr);

  while (flag != EOF) {
    switch (flag) {
    case 'n':
      no_comments = true;
      break;

    case 'p':
      pass_phrase = optarg;
      got_pass_phrase = true;
      break;

    case 'h':
      usage();
      exit(0);

    default:
      exit(1);
    }
    flag = getopt(argc, argv, optstr);
  }

  argc -= (optind-1);
  argv += (optind-1);

  if (argc < 2) {
    usage();
    exit(1);
  }

  // Seed the random number generator.
  RAND_status();

  // Load the OpenSSL algorithms.
  OpenSSL_add_all_algorithms();

  // Convert the compiled-in data to an EVP_PKEY.
  const char *pp = nullptr;
  if (got_pass_phrase) {
    pp = pass_phrase.c_str();
  }

  BIO *mbio = BIO_new_mem_buf((void *)KEY_DATA, KEY_LENGTH);
  EVP_PKEY *pkey = PEM_read_bio_PrivateKey(mbio, nullptr, nullptr, (void *)pp);
  BIO_free(mbio);

  if (pkey == nullptr) {
    // Actually, we're not 100% sure this was the problem, but we can't really
    // tell why it failed, and we're 99% sure anyway.
    cerr << "Invalid pass phrase.\n";
    exit(1);
  }

  for (int i = 1; i < argc; i++) {
    sign_prc(Filename::from_os_specific(argv[1]), no_comments, pkey);
  }

  return (0);
}
