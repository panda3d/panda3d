// Filename: p3dCert_strings.h
// Created by:  rdb (25Mar15)
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

enum LanguageIndex {
  LI_en, // English
  LI_nl, // Dutch
  LI_de, // German
  LI_es, // Spanish
  LI_it, // Italian
  LI_eo, // Esperanto
  LI_ru, // Russian
  LI_COUNT,

  LI_default = LI_en
};

extern const char *language_codes[LI_COUNT];
extern const unsigned char language_ids[LI_COUNT];

extern const char *run_title[LI_COUNT];
extern const char *cancel_title[LI_COUNT];
extern const char *show_cert_title[LI_COUNT];

extern const char *new_application_title[LI_COUNT];
extern const char *no_cert_title[LI_COUNT];
extern const char *unverified_cert_title[LI_COUNT];
extern const char *expired_cert_title[LI_COUNT];

extern const char *self_signed_cert_text[LI_COUNT];
extern const char *unknown_auth_cert_text[LI_COUNT];
extern const char *verified_cert_text[LI_COUNT];
extern const char *expired_cert_text[LI_COUNT];
extern const char *generic_error_cert_text[LI_COUNT];
extern const char *no_cert_text[LI_COUNT];
